#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include "esp_camera.h"

class SdManager {
 public:
  bool begin();
  bool isReady() const;
  bool saveJpeg(camera_fb_t *frameBuffer, const char *path);
  uint32_t nextPhotoNumber();
  String buildPhotoListHtml();

 private:
  bool _ready = false;
  uint32_t _photoCounter = 0;
};

#endif
