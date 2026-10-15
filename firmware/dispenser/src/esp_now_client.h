#ifndef ESP_NOW_CLIENT_H
#define ESP_NOW_CLIENT_H

#include <Arduino.h>

class EspNowClient {
 public:
  bool begin();
  bool sendCaptureRequest(uint8_t hour, uint8_t minute);

 private:
  uint32_t _doseCounter = 0;
  bool _ready = false;
};

#endif
