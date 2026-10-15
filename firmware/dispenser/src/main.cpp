#include <Arduino.h>

#include "audio_manager.h"
#include "button_manager.h"
#include "camera_client.h"
#include "display_manager.h"
#include "dispenser_state.h"
#include "esp_now_client.h"
// #include "hall_sensor.h"
#include "servo_manager.h"

namespace Pins {
constexpr int8_t TFT_CS = 5;
constexpr int8_t TFT_SCL = 18;
constexpr int8_t TFT_SDA = 23;
constexpr int8_t TFT_RST = 17;
constexpr int8_t TFT_BL = -1;  // ST7789 BLK is wired directly to 3.3V.

constexpr uint8_t HALL_AO = 34;
constexpr uint8_t SERVO_SIGNAL = 13;
constexpr uint8_t BUTTON_CONFIRM = 32;
constexpr uint8_t AUDIO_OUT = 25;
}  // namespace Pins

constexpr uint8_t START_HOUR = 12;
constexpr uint8_t START_MINUTE = 0;
constexpr unsigned long DOSE_INTERVAL_MS = 60UL * 1000UL;
constexpr unsigned long ALERT_BEFORE_DISPENSE_MS = 2000;
constexpr unsigned long CONFIRMED_SCREEN_MS = 4000;
constexpr unsigned long SENSOR_LOG_INTERVAL_MS = 200;
constexpr unsigned long DISPLAY_REFRESH_INTERVAL_MS = 1000;

DisplayManager display(Pins::TFT_CS, Pins::TFT_SCL, Pins::TFT_SDA, Pins::TFT_RST, Pins::TFT_BL);
ButtonManager confirmButton(Pins::BUTTON_CONFIRM);
// HallSensor hallSensor(Pins::HALL_AO);
ServoManager servoManager(Pins::SERVO_SIGNAL);
AudioManager audioManager(Pins::AUDIO_OUT);
CameraClient cameraClient;
EspNowClient espNowClient;

DispenserState currentState = DispenserState::IDLE;
unsigned long bootTime = 0;
unsigned long stateEnteredAt = 0;
unsigned long nextDoseAt = 0;
unsigned long lastSensorLogAt = 0;
unsigned long lastDisplayRefreshAt = 0;

void getClockTime(unsigned long atMillis, uint8_t &hour, uint8_t &minute) {
  const unsigned long elapsedMinutes = (atMillis - bootTime) / 60000UL;
  const unsigned int totalMinutes = START_HOUR * 60U + START_MINUTE + elapsedMinutes;
  hour = (totalMinutes / 60U) % 24U;
  minute = totalMinutes % 60U;
}

void getNextDoseTime(uint8_t &hour, uint8_t &minute) {
  getClockTime(nextDoseAt, hour, minute);
}

void setState(DispenserState newState) {
  if (currentState == newState) {
    return;
  }

  currentState = newState;
  stateEnteredAt = millis();

  Serial.print("State changed to: ");
  Serial.println(dispenserStateToString(currentState));
}

void updateInputs() {
  confirmButton.update();
  // hallSensor.update();
}

void logSensors() {
  const unsigned long now = millis();
  if (now - lastSensorLogAt < SENSOR_LOG_INTERVAL_MS) {
    return;
  }

  lastSensorLogAt = now;
  // Serial.print("Hall value: ");
  // Serial.print(hallSensor.getValue());
  // Serial.print(" | Magnet: ");
  // Serial.print(hallSensor.isMagnetDetected() ? "YES" : "NO");
  // Serial.print(" | Button: ");
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t nextHour = 0;
  uint8_t nextMinute = 0;
  getClockTime(now, hour, minute);
  getNextDoseTime(nextHour, nextMinute);

  Serial.print("Time: ");
  if (hour < 10) {
    Serial.print('0');
  }
  Serial.print(hour);
  Serial.print(':');
  if (minute < 10) {
    Serial.print('0');
  }
  Serial.print(minute);
  Serial.print(" | Next dose: ");
  if (nextHour < 10) {
    Serial.print('0');
  }
  Serial.print(nextHour);
  Serial.print(':');
  if (nextMinute < 10) {
    Serial.print('0');
  }
  Serial.print(nextMinute);
  Serial.print(" | Button: ");
  Serial.println(confirmButton.isPressed() ? "PRESSED" : "READY");
}

void refreshDisplay() {
  const unsigned long now = millis();
  if (now - lastDisplayRefreshAt < DISPLAY_REFRESH_INTERVAL_MS) {
    return;
  }

  lastDisplayRefreshAt = now;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t nextHour = 0;
  uint8_t nextMinute = 0;
  getClockTime(now, hour, minute);
  getNextDoseTime(nextHour, nextMinute);

  display.updateStatus(
      currentState,
      hour,
      minute,
      nextHour,
      nextMinute,
      confirmButton.isPressed());
}

void updateStateMachine() {
  const unsigned long now = millis();

  switch (currentState) {
    case DispenserState::IDLE:
      if (now >= nextDoseAt) {
        setState(DispenserState::TIME_TO_TAKE_MEDICINE);
        audioManager.alertMelody();
      }
      break;

    case DispenserState::TIME_TO_TAKE_MEDICINE:
      if (now - stateEnteredAt >= ALERT_BEFORE_DISPENSE_MS) {
        setState(DispenserState::DISPENSING);
        servoManager.dispenseOneDose();
      }
      break;

    case DispenserState::DISPENSING:
      if (!servoManager.isDispensing()) {
        setState(DispenserState::WAITING_CONFIRMATION);
        audioManager.alertMelody();
      }
      break;

    case DispenserState::WAITING_CONFIRMATION:
      if (confirmButton.wasPressed()) {
        uint8_t confirmHour = 0;
        uint8_t confirmMinute = 0;
        getClockTime(now, confirmHour, confirmMinute);
        audioManager.stop();
        audioManager.beepShort();
        cameraClient.capturePhoto();
        espNowClient.sendCaptureRequest(confirmHour, confirmMinute);
        setState(DispenserState::MEDICINE_CONFIRMED);
        nextDoseAt = now + DOSE_INTERVAL_MS;
      }
      break;

    case DispenserState::MEDICINE_CONFIRMED:
      if (now - stateEnteredAt >= CONFIRMED_SCREEN_MS) {
        setState(DispenserState::IDLE);
      }
      break;

    case DispenserState::ERROR:
      audioManager.beepLong();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("Smart Pill Dispenser starting...");

  display.begin();
  display.showStartup();

  confirmButton.begin();
  // hallSensor.begin();
  servoManager.begin();
  audioManager.begin();
  audioManager.beepShort();
  cameraClient.begin();
  espNowClient.begin();

  bootTime = millis();
  stateEnteredAt = bootTime;
  nextDoseAt = bootTime + DOSE_INTERVAL_MS;

  Serial.println("Initialization complete");
}

void loop() {
  updateInputs();
  servoManager.update();
  audioManager.update();
  updateStateMachine();
  logSensors();
  refreshDisplay();
}
