#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

#include "dispenser_state.h"

class DisplayManager {
 public:
  DisplayManager(int8_t csPin, int8_t clockPin, int8_t dataPin, int8_t resetPin, int8_t backlightPin);

  void begin();
  void runColorTest();
  void showStartup();
  void updateStatus(
      DispenserState state,
      uint8_t hour,
      uint8_t minute,
      uint8_t nextDoseHour,
      uint8_t nextDoseMinute,
      bool buttonPressed);
  void showMedicineAlert();
  void showMedicineConfirmed();
  void showError(const char *message);

 private:
  void resetDisplay();
  void initDisplay();
  void select();
  void deselect();
  void pinHigh(int8_t pin, uint32_t mask);
  void pinLow(int8_t pin, uint32_t mask);
  void clockPulse();
  void send9(bool isData, uint8_t value);
  void command(uint8_t value);
  void data(uint8_t value);
  void dataBytes(const uint8_t *values, size_t count);
  void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void fillScreen(uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawHeader(const char *title, uint16_t color);
  void drawText(const char *text, int16_t x, int16_t y, uint8_t scale, uint16_t color);
  void drawChar(char character, int16_t x, int16_t y, uint8_t scale, uint16_t color);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) const;

  int8_t _csPin;
  int8_t _clockPin;
  int8_t _dataPin;
  int8_t _resetPin;
  int8_t _backlightPin;
  uint32_t _csMask;
  uint32_t _clockMask;
  uint32_t _dataMask;
  uint32_t _resetMask;
  uint32_t _backlightMask;
  DispenserState _lastState;
  int _lastHallValue;
  bool _lastMagnetDetected;
  bool _lastButtonPressed;
  uint8_t _lastHour;
  uint8_t _lastMinute;
  uint8_t _lastNextDoseHour;
  uint8_t _lastNextDoseMinute;
  bool _hasLastFrame;

  static constexpr uint16_t SCREEN_WIDTH = 240;
  static constexpr uint16_t SCREEN_HEIGHT = 240;
  static constexpr uint16_t COLUMN_OFFSET = 0;
  static constexpr uint16_t ROW_OFFSET = 40;
};

#endif
