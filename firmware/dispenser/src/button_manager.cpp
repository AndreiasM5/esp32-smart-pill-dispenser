#include "button_manager.h"

ButtonManager::ButtonManager(uint8_t pin)
    : _pin(pin),
      _stableState(false),
      _lastReading(false),
      _pressedEvent(false),
      _lastDebounceTime(0) {}

void ButtonManager::begin() {
  pinMode(_pin, INPUT_PULLUP);
  _stableState = digitalRead(_pin) == LOW;
  _lastReading = _stableState;
  _pressedEvent = false;
  _lastDebounceTime = millis();
}

void ButtonManager::update() {
  const bool reading = digitalRead(_pin) == LOW;

  if (reading != _lastReading) {
    _lastDebounceTime = millis();
    _lastReading = reading;
  }

  if ((millis() - _lastDebounceTime) >= DEBOUNCE_DELAY_MS && reading != _stableState) {
    _stableState = reading;

    if (_stableState) {
      _pressedEvent = true;
    }
  }
}

bool ButtonManager::isPressed() const {
  return _stableState;
}

bool ButtonManager::wasPressed() {
  if (!_pressedEvent) {
    return false;
  }

  _pressedEvent = false;
  return true;
}
