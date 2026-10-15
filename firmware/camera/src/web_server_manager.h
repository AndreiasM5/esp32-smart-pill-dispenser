#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>

#include "camera_manager.h"
#include "sd_manager.h"

class WebServerManager {
 public:
  WebServerManager(CameraManager &cameraManager, SdManager &sdManager);

  void begin();
  void update();
  void addLog(const String &message);

 private:
  void handleRoot();
  void handlePreviewJpeg();
  void handleCapture();
  void handlePhotos();
  void handlePhotoFile();
  void handleLogs();
  bool captureAndSave(String &savedPath);

  CameraManager &_cameraManager;
  SdManager &_sdManager;
  WebServer _server;
  String _logs[20];
  uint8_t _logIndex = 0;
  uint8_t _logCount = 0;
};

#endif
