#include "comms_stub.h"

#include <WiFi.h>
#include <esp_now.h>

namespace {
CommsStub *activeComms = nullptr;

void onEspNowReceived(const uint8_t *mac, const uint8_t *data, int length) {
  if (!activeComms || length != sizeof(CaptureRequestPacket)) {
    return;
  }

  CaptureRequestPacket packet = {};
  memcpy(&packet, data, sizeof(packet));
  if (packet.magic != CAPTURE_PACKET_MAGIC || packet.networkId != CAPTURE_NETWORK_ID ||
      packet.version != CAPTURE_PACKET_VERSION || packet.sourceDevice != CAPTURE_DEVICE_DISPENSER ||
      packet.targetDevice != CAPTURE_DEVICE_CAMERA) {
    return;
  }

  activeComms->queueCaptureRequest(packet);
}
}  // namespace

void CommsStub::begin() {
  Serial.println("Comms ready");
  Serial.println("Type c + Enter to capture a photo");

  activeComms = this;

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onEspNowReceived);
  Serial.print("ESP-NOW receiver ready, STA MAC: ");
  Serial.println(WiFi.macAddress());
}

void CommsStub::update() {
  while (Serial.available() > 0) {
    const char input = static_cast<char>(Serial.read());
    if (input == 'c' || input == 'C') {
      _pendingPacket = {
          CAPTURE_PACKET_MAGIC,
          CAPTURE_NETWORK_ID,
          CAPTURE_PACKET_VERSION,
          CAPTURE_DEVICE_DISPENSER,
          CAPTURE_DEVICE_CAMERA,
          0,
          0,
          ++_serialCounter,
      };
      _requestPending = true;
    }
  }
}

void CommsStub::queueCaptureRequest(const CaptureRequestPacket &packet) {
  noInterrupts();
  _pendingPacket = packet;
  _requestPending = true;
  interrupts();
}

bool CommsStub::captureRequested() {
  CaptureRequestPacket ignored = {};
  return captureRequested(ignored);
}

bool CommsStub::captureRequested(CaptureRequestPacket &packet) {
  if (!_requestPending) {
    return false;
  }

  noInterrupts();
  packet = _pendingPacket;
  _requestPending = false;
  interrupts();
  return true;
}
