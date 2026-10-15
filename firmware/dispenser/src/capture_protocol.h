#ifndef CAPTURE_PROTOCOL_H
#define CAPTURE_PROTOCOL_H

#include <Arduino.h>

constexpr uint32_t CAPTURE_PACKET_MAGIC = 0x50494C4CUL;
constexpr uint32_t CAPTURE_NETWORK_ID = 0xA11E2026UL;
constexpr uint8_t CAPTURE_PACKET_VERSION = 1;
constexpr uint8_t CAPTURE_DEVICE_DISPENSER = 1;
constexpr uint8_t CAPTURE_DEVICE_CAMERA = 2;

struct CaptureRequestPacket {
  uint32_t magic;
  uint32_t networkId;
  uint8_t version;
  uint8_t sourceDevice;
  uint8_t targetDevice;
  uint8_t hour;
  uint8_t minute;
  uint32_t doseCounter;
};

#endif
