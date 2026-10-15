#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoManager {
 public:
  explicit ServoManager(uint8_t signalPin);

  void begin();
  void update();
  void stopServo();
  void rotateForward(int speed);
  void rotateBackward(int speed);
  void dispenseOneDose();
  bool isDispensing() const;

 private:
  int speedToPulse(int speed, bool forward) const;

  Servo _servo;
  uint8_t _signalPin;
  bool _dispensing;
  unsigned long _dispenseStartedAt;

  static constexpr int STOP_US = 1500;
  static constexpr int MAX_OFFSET_US = 500;
  static constexpr int MIN_PULSE_US = 1000;
  static constexpr int MAX_PULSE_US = 2000;
  static constexpr int DEFAULT_DISPENSE_SPEED = 25;
  static constexpr unsigned long DISPENSE_DURATION_MS = 650; // fine tunning
};

#endif
