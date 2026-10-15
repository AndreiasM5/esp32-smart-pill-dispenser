#ifndef CAMERA_CLIENT_H
#define CAMERA_CLIENT_H

#include <Arduino.h>

class CameraClient {
 public:
  void begin();
  void capturePhoto();
};

#endif
