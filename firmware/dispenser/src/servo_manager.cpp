#include "servo_manager.h"

ServoManager::ServoManager(uint8_t signalPin)
    : _signalPin(signalPin), _dispensing(false), _dispenseStartedAt(0) {}

void ServoManager::begin() {
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  _servo.setPeriodHertz(50);
  _servo.attach(_signalPin, MIN_PULSE_US, MAX_PULSE_US);
  stopServo();
}

void ServoManager::update() {
  if (_dispensing && (millis() - _dispenseStartedAt >= DISPENSE_DURATION_MS)) {
    stopServo();
    _dispensing = false;
  }
}

void ServoManager::stopServo() {
  _servo.writeMicroseconds(STOP_US);
}

void ServoManager::rotateForward(int speed) {
  _servo.writeMicroseconds(speedToPulse(speed, true));
}

void ServoManager::rotateBackward(int speed) {
  _servo.writeMicroseconds(speedToPulse(speed, false));
}

void ServoManager::dispenseOneDose() {
  _dispensing = true;
  _dispenseStartedAt = millis();
  rotateForward(DEFAULT_DISPENSE_SPEED);
  Serial.println("Servo dispensing: forward calibrated short pulse");
}

bool ServoManager::isDispensing() const {
  return _dispensing;
}

int ServoManager::speedToPulse(int speed, bool forward) const {
  const int constrainedSpeed = constrain(speed, 0, 100);
  const int offset = map(constrainedSpeed, 0, 100, 0, MAX_OFFSET_US);
  return forward ? STOP_US + offset : STOP_US - offset;
}
