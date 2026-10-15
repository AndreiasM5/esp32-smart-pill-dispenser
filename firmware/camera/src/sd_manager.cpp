#include "sd_manager.h"

#include <FS.h>
#include <SD_MMC.h>

#include "camera_pins.h"

bool SdManager::begin() {
  SD_MMC.setPins(SdPins::CLK, SdPins::CMD, SdPins::D0);

  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD_MMC mount failed");
    _ready = false;
    return false;
  }

  const uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card detected");
    _ready = false;
    return false;
  }

  Serial.printf("SD card size: %llu MB\n", SD_MMC.cardSize() / (1024ULL * 1024ULL));
  _ready = true;
  return true;
}

bool SdManager::isReady() const {
  return _ready;
}

bool SdManager::saveJpeg(camera_fb_t *frameBuffer, const char *path) {
  if (!_ready || !frameBuffer || !path) {
    return false;
  }

  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to open %s for writing\n", path);
    return false;
  }

  const size_t written = file.write(frameBuffer->buf, frameBuffer->len);
  file.close();

  if (written != frameBuffer->len) {
    Serial.printf("Incomplete write to %s: %u/%u bytes\n", path, written, frameBuffer->len);
    return false;
  }

  Serial.printf("Saved %s (%u bytes)\n", path, frameBuffer->len);
  return true;
}

uint32_t SdManager::nextPhotoNumber() {
  return ++_photoCounter;
}

String SdManager::buildPhotoListHtml() {
  if (!_ready) {
    return "<p>SD card unavailable</p>";
  }

  File root = SD_MMC.open("/");
  if (!root) {
    return "<p>Cannot open SD root</p>";
  }

  String html;
  html.reserve(1024);
  html += "<ul>";

  File file = root.openNextFile();
  bool hasPhotos = false;
  while (file) {
    const String name = file.name();
    const String lowerName = name;
    if (!file.isDirectory() && (lowerName.endsWith(".jpg") || lowerName.endsWith(".JPG"))) {
      hasPhotos = true;
      html += "<li><a href=\"/photo?name=";
      html += name;
      html += "\" target=\"_blank\">";
      html += name;
      html += "</a> <span>";
      html += String(file.size());
      html += " bytes</span></li>";
    }
    file = root.openNextFile();
  }

  if (!hasPhotos) {
    html += "<li>No photos yet</li>";
  }

  html += "</ul>";
  root.close();
  return html;
}
