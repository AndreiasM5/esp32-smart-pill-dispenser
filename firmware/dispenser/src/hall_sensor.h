#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <Arduino.h>

class HallSensor {
 public:
  explicit HallSensor(uint8_t analogPin);

  void begin();
  void update();
  int getValue() const;
  bool isMagnetDetected() const;

  static constexpr int MAGNET_THRESHOLD = 2200;

 private:
  uint8_t _analogPin;
  int _value;
};

#endif
