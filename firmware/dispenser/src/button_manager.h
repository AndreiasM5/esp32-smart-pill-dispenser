#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

class ButtonManager {
 public:
  explicit ButtonManager(uint8_t pin);

  void begin();
  void update();
  bool isPressed() const;
  bool wasPressed();

 private:
  uint8_t _pin;
  bool _stableState;
  bool _lastReading;
  bool _pressedEvent;
  unsigned long _lastDebounceTime;

  static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;
};

#endif
