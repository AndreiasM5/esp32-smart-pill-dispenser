#include "camera_manager.h"

#include "camera_pins.h"

bool CameraManager::begin() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CameraPins::Y2;
  config.pin_d1 = CameraPins::Y3;
  config.pin_d2 = CameraPins::Y4;
  config.pin_d3 = CameraPins::Y5;
  config.pin_d4 = CameraPins::Y6;
  config.pin_d5 = CameraPins::Y7;
  config.pin_d6 = CameraPins::Y8;
  config.pin_d7 = CameraPins::Y9;
  config.pin_xclk = CameraPins::XCLK;
  config.pin_pclk = CameraPins::PCLK;
  config.pin_vsync = CameraPins::VSYNC;
  config.pin_href = CameraPins::HREF;
  config.pin_sccb_sda = CameraPins::SIOD;
  config.pin_sccb_scl = CameraPins::SIOC;
  config.pin_pwdn = CameraPins::PWDN;
  config.pin_reset = CameraPins::RESET;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  const esp_err_t error = esp_camera_init(&config);
  if (error != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", error);
    return false;
  }

  logSensorInfo();
  return true;
}

camera_fb_t *CameraManager::capture() {
  camera_fb_t *frameBuffer = esp_camera_fb_get();
  if (!frameBuffer) {
    Serial.println("Camera capture failed");
    return nullptr;
  }

  Serial.printf("Captured image: %u bytes\n", frameBuffer->len);
  return frameBuffer;
}

void CameraManager::release(camera_fb_t *frameBuffer) {
  if (frameBuffer) {
    esp_camera_fb_return(frameBuffer);
  }
}

void CameraManager::logSensorInfo() {
  sensor_t *sensor = esp_camera_sensor_get();
  if (!sensor) {
    Serial.println("Camera sensor object unavailable");
    return;
  }

  Serial.printf("Camera sensor PID: 0x%04x\n", sensor->id.PID);


  sensor->set_framesize(sensor, FRAMESIZE_SVGA);
  sensor->set_quality(sensor, 12);
  sensor->set_brightness(sensor, 0);
  sensor->set_contrast(sensor, 0);
  sensor->set_saturation(sensor, 0);
}
