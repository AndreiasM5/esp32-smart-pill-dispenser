#include <Arduino.h>

#include "camera_manager.h"
#include "capture_protocol.h"
#include "comms_stub.h"
#include "sd_manager.h"
#include "web_server_manager.h"

CameraManager cameraManager;
SdManager sdManager;
CommsStub comms;
WebServerManager webServer(cameraManager, sdManager);

bool captureAndSavePhoto(const char *pathOverride = nullptr) {
  camera_fb_t *frameBuffer = cameraManager.capture();
  if (!frameBuffer) {
    return false;
  }

  char path[32];
  if (pathOverride) {
    snprintf(path, sizeof(path), "%s", pathOverride);
  } else {
    snprintf(path, sizeof(path), "/photo_%05lu.jpg", static_cast<unsigned long>(sdManager.nextPhotoNumber()));
  }

  const bool saved = sdManager.saveJpeg(frameBuffer, path);

  cameraManager.release(frameBuffer);
  return saved;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("ESP32-S3-CAM pill dispenser camera node");

  if (!cameraManager.begin()) {
    Serial.println("Camera unavailable; check pin map and camera ribbon");
  }

  if (!sdManager.begin()) {
    Serial.println("SD unavailable; check card format and slot wiring");
  }

  webServer.begin();
  comms.begin();

  if (sdManager.isReady()) {
    Serial.println("Taking boot test photo...");
    if (captureAndSavePhoto()) {
      webServer.addLog("Boot test photo saved");
    }
  }
}

void loop() {
  comms.update();
  webServer.update();

  CaptureRequestPacket packet = {};
  if (comms.captureRequested(packet)) {
    char path[40];
    if (packet.hour <= 23 && packet.minute <= 59) {
      snprintf(
          path,
          sizeof(path),
          "/poza_%02u_%02u_%03lu.jpg",
          packet.hour,
          packet.minute,
          static_cast<unsigned long>(packet.doseCounter % 1000));
    } else {
      snprintf(path, sizeof(path), "/photo_%05lu.jpg", static_cast<unsigned long>(sdManager.nextPhotoNumber()));
    }

    webServer.addLog(
        String("Capture request received for ") +
        (packet.hour <= 23 ? String(packet.hour) : String("--")) +
        ":" +
        (packet.minute <= 59 ? String(packet.minute) : String("--")));

    if (captureAndSavePhoto(path)) {
      webServer.addLog(String("ESP-NOW/serial photo saved: ") + path);
    }
  }

  delay(10);
}
