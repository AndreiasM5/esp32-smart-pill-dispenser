#ifndef COMMS_STUB_H
#define COMMS_STUB_H

#include <Arduino.h>

#include "capture_protocol.h"

class CommsStub {
 public:
  void begin();
  void update();
  void queueCaptureRequest(const CaptureRequestPacket &packet);
  bool captureRequested();
  bool captureRequested(CaptureRequestPacket &packet);

 private:
  volatile bool _requestPending = false;
  CaptureRequestPacket _pendingPacket = {};
  uint32_t _serialCounter = 0;
};

#endif
