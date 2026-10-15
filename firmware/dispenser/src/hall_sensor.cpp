#include "hall_sensor.h"

HallSensor::HallSensor(uint8_t analogPin) : _analogPin(analogPin), _value(0) {}

void HallSensor::begin() {
  pinMode(_analogPin, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(_analogPin, ADC_11db);
  _value = analogRead(_analogPin);
}

void HallSensor::update() {
  _value = analogRead(_analogPin);
}

int HallSensor::getValue() const {
  return _value;
}

bool HallSensor::isMagnetDetected() const {
  return _value >= MAGNET_THRESHOLD;
}
