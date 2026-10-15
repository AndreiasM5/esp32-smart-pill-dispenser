#include "esp_now_client.h"

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "capture_protocol.h"

namespace {
constexpr uint8_t ESPNOW_CHANNEL = 1;
constexpr uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
}

bool EspNowClient::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    _ready = false;
    return false;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, BROADCAST_MAC, sizeof(BROADCAST_MAC));
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(BROADCAST_MAC) && esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW broadcast peer add failed");
    _ready = false;
    return false;
  }

  Serial.print("ESP-NOW sender ready, STA MAC: ");
  Serial.println(WiFi.macAddress());
  _ready = true;
  return true;
}

bool EspNowClient::sendCaptureRequest(uint8_t hour, uint8_t minute) {
  if (!_ready) {
    return false;
  }

  CaptureRequestPacket packet = {
      CAPTURE_PACKET_MAGIC,
      CAPTURE_NETWORK_ID,
      CAPTURE_PACKET_VERSION,
      CAPTURE_DEVICE_DISPENSER,
      CAPTURE_DEVICE_CAMERA,
      hour,
      minute,
      ++_doseCounter,
  };

  const esp_err_t result = esp_now_send(BROADCAST_MAC, reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
  if (result != ESP_OK) {
    Serial.printf("ESP-NOW capture send failed: 0x%x\n", result);
    return false;
  }

  Serial.printf("ESP-NOW capture request sent for %02u:%02u dose %lu\n", hour, minute, static_cast<unsigned long>(_doseCounter));
  return true;
}
