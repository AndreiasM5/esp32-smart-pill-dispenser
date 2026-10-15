#include "display_manager.h"

#include "soc/gpio_struct.h"

namespace {
constexpr uint16_t COLOR_BLACK = 0x0000;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_RED = 0xF800;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_BLUE = 0x001F;
constexpr uint16_t COLOR_CYAN = 0x07FF;
constexpr uint16_t COLOR_YELLOW = 0xFFE0;
constexpr uint16_t COLOR_ORANGE = 0xFD20;
constexpr uint16_t COLOR_MAGENTA = 0xF81F;

const uint8_t *glyphFor(char character) {
  static const uint8_t space[5] = {0, 0, 0, 0, 0};
  static const uint8_t colon[5] = {0, 2, 0, 2, 0};
  static const uint8_t dash[5] = {0, 0, 7, 0, 0};
  static const uint8_t dot[5] = {0, 0, 0, 0, 2};
  static const uint8_t underscore[5] = {0, 0, 0, 0, 7};
  static const uint8_t question[5] = {7, 1, 3, 0, 2};
  static const uint8_t digits[][5] = {
      {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7},
      {5, 5, 7, 1, 1}, {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1},
      {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}};
  static const uint8_t letters[][5] = {
      {2, 5, 7, 5, 5}, {6, 5, 6, 5, 6}, {7, 4, 4, 4, 7}, {6, 5, 5, 5, 6},
      {7, 4, 6, 4, 7}, {7, 4, 6, 4, 4}, {7, 4, 5, 5, 7}, {5, 5, 7, 5, 5},
      {7, 2, 2, 2, 7}, {1, 1, 1, 5, 7}, {5, 5, 6, 5, 5}, {4, 4, 4, 4, 7},
      {5, 7, 7, 5, 5}, {5, 7, 7, 7, 5}, {7, 5, 5, 5, 7}, {7, 5, 7, 4, 4},
      {7, 5, 5, 7, 1}, {7, 5, 7, 6, 5}, {7, 4, 7, 1, 7}, {7, 2, 2, 2, 2},
      {5, 5, 5, 5, 7}, {5, 5, 5, 5, 2}, {5, 5, 7, 7, 5}, {5, 5, 2, 5, 5},
      {5, 5, 2, 2, 2}, {7, 1, 2, 4, 7}};

  if (character >= 'a' && character <= 'z') {
    character -= 32;
  }
  if (character >= '0' && character <= '9') {
    return digits[character - '0'];
  }
  if (character >= 'A' && character <= 'Z') {
    return letters[character - 'A'];
  }
  if (character == ' ') {
    return space;
  }
  if (character == ':') {
    return colon;
  }
  if (character == '-') {
    return dash;
  }
  if (character == '.') {
    return dot;
  }
  if (character == '_') {
    return underscore;
  }
  return question;
}
}  // namespace

DisplayManager::DisplayManager(int8_t csPin, int8_t clockPin, int8_t dataPin, int8_t resetPin, int8_t backlightPin)
    : _csPin(csPin),
      _clockPin(clockPin),
      _dataPin(dataPin),
      _resetPin(resetPin),
      _backlightPin(backlightPin),
      _csMask(csPin >= 0 ? (1UL << csPin) : 0),
      _clockMask(clockPin >= 0 ? (1UL << clockPin) : 0),
      _dataMask(dataPin >= 0 ? (1UL << dataPin) : 0),
      _resetMask(resetPin >= 0 ? (1UL << resetPin) : 0),
      _backlightMask(backlightPin >= 0 ? (1UL << backlightPin) : 0),
      _lastState(DispenserState::ERROR),
      _lastHallValue(-1),
      _lastMagnetDetected(false),
      _lastButtonPressed(false),
      _lastHour(255),
      _lastMinute(255),
      _lastNextDoseHour(255),
      _lastNextDoseMinute(255),
      _hasLastFrame(false) {}

void DisplayManager::begin() {
  if (_csPin >= 0) {
    pinMode(_csPin, OUTPUT);
    pinHigh(_csPin, _csMask);
  }

  pinMode(_clockPin, OUTPUT);
  pinMode(_dataPin, OUTPUT);
  pinLow(_clockPin, _clockMask);
  pinLow(_dataPin, _dataMask);

  if (_backlightPin >= 0) {
    pinMode(_backlightPin, OUTPUT);
    pinHigh(_backlightPin, _backlightMask);
  }

  resetDisplay();
  initDisplay();
  fillScreen(COLOR_BLACK);
}

void DisplayManager::runColorTest() {
  fillScreen(COLOR_RED);
  delay(700);
  fillScreen(COLOR_GREEN);
  delay(700);
  fillScreen(COLOR_BLUE);
  delay(700);
  fillScreen(COLOR_BLACK);
}

void DisplayManager::showStartup() {
  fillScreen(COLOR_BLACK);
  drawHeader("PORNIRE", COLOR_CYAN);
  drawText("PASTILE", 28, 72, 5, COLOR_WHITE);
  drawText("SMART", 36, 126, 4, COLOR_GREEN);
  drawText("ASTEAPTA", 30, 188, 3, COLOR_YELLOW);
  _hasLastFrame = false;
}

void DisplayManager::updateStatus(
    DispenserState state,
    uint8_t hour,
    uint8_t minute,
    uint8_t nextDoseHour,
    uint8_t nextDoseMinute,
    bool buttonPressed) {
  const bool stateChanged = !_hasLastFrame || state != _lastState;
  const bool timeChanged = hour != _lastHour || minute != _lastMinute;
  const bool nextDoseChanged =
      nextDoseHour != _lastNextDoseHour || nextDoseMinute != _lastNextDoseMinute;

  if (_hasLastFrame &&
      state == _lastState &&
      !timeChanged &&
      !nextDoseChanged &&
      buttonPressed == _lastButtonPressed) {
    return;
  }

  if (stateChanged || timeChanged || nextDoseChanged) {
    fillScreen(COLOR_BLACK);

    uint16_t headerColor = COLOR_GREEN;
    if (state == DispenserState::TIME_TO_TAKE_MEDICINE) {
      headerColor = COLOR_ORANGE;
    } else if (state == DispenserState::ERROR) {
      headerColor = COLOR_RED;
    } else if (state == DispenserState::DISPENSING ||
               state == DispenserState::WAITING_CONFIRMATION) {
      headerColor = COLOR_YELLOW;
    } else if (state == DispenserState::MEDICINE_CONFIRMED) {
      headerColor = COLOR_GREEN;
    }

    char timeText[8];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", hour, minute);
    drawHeader("ORA", headerColor);
    drawText(timeText, 34, 48, 8, COLOR_WHITE);

    switch (state) {
      case DispenserState::IDLE:
        drawText("TOTUL E OK", 16, 118, 4, COLOR_GREEN);
        break;
      case DispenserState::TIME_TO_TAKE_MEDICINE:
        drawText("E TIMPUL", 28, 112, 4, COLOR_YELLOW);
        drawText("PASTILA", 32, 152, 4, COLOR_YELLOW);
        break;
      case DispenserState::DISPENSING:
        drawText("PREGATESC", 16, 118, 4, COLOR_YELLOW);
        drawText("PASTILA", 32, 158, 4, COLOR_YELLOW);
        break;
      case DispenserState::WAITING_CONFIRMATION:
        drawText("IA PASTILA", 18, 100, 4, COLOR_YELLOW);
        drawText("APASA", 48, 146, 4, COLOR_WHITE);
        drawText("BUTONUL", 32, 186, 4, COLOR_WHITE);
        break;
      case DispenserState::MEDICINE_CONFIRMED:
        drawText("MULTUMESC", 18, 120, 4, COLOR_GREEN);
        drawText("CONFIRMAT", 18, 160, 4, COLOR_GREEN);
        break;
      case DispenserState::ERROR:
        drawText("EROARE", 48, 126, 4, COLOR_RED);
        break;
    }
  } else {
    fillRect(0, 212, SCREEN_WIDTH, 26, COLOR_BLACK);
  }

  char line[32];
  if (state == DispenserState::IDLE || state == DispenserState::MEDICINE_CONFIRMED) {
    snprintf(line, sizeof(line), "URMATOAREA %02u:%02u", nextDoseHour, nextDoseMinute);
    drawText(line, 8, 214, 2, COLOR_CYAN);
  } else if (state == DispenserState::WAITING_CONFIRMATION) {
    fillRect(0, 214, SCREEN_WIDTH, 24, COLOR_BLACK);
  } else if (!buttonPressed) {
    drawText("ASTEAPTA APASARE", 8, 214, 2, COLOR_CYAN);
  } else {
    drawText("BUTON APASAT", 8, 214, 2, COLOR_GREEN);
  }

  _lastState = state;
  _lastButtonPressed = buttonPressed;
  _lastHour = hour;
  _lastMinute = minute;
  _lastNextDoseHour = nextDoseHour;
  _lastNextDoseMinute = nextDoseMinute;
  _hasLastFrame = true;
}

void DisplayManager::showMedicineAlert() {
  fillRect(0, 42, SCREEN_WIDTH, 70, COLOR_BLACK);
  drawText("TAKE", 36, 52, 5, COLOR_YELLOW);
  drawText("MEDICINE", 18, 88, 3, COLOR_YELLOW);
}

void DisplayManager::showMedicineConfirmed() {
  fillRect(0, 42, SCREEN_WIDTH, 70, COLOR_BLACK);
  drawText("MEDICINE", 18, 56, 3, COLOR_GREEN);
  drawText("CONFIRMED", 18, 86, 3, COLOR_GREEN);
}

void DisplayManager::showError(const char *message) {
  fillScreen(COLOR_BLACK);
  drawHeader("ERROR", COLOR_RED);
  drawText(message, 12, 64, 2, COLOR_WHITE);
  _hasLastFrame = false;
}

void DisplayManager::resetDisplay() {
  if (_resetPin < 0) {
    return;
  }

  pinMode(_resetPin, OUTPUT);
  pinHigh(_resetPin, _resetMask);
  delay(10);
  pinLow(_resetPin, _resetMask);
  delay(20);
  pinHigh(_resetPin, _resetMask);
  delay(120);
}

void DisplayManager::initDisplay() {
  command(0x01);
  delay(150);
  command(0x11);
  delay(120);
  command(0x3A);
  data(0x55);
  command(0x36);
  data(0x00);
  command(0x21);
  command(0x29);
  delay(50);
}

void DisplayManager::select() {
  if (_csPin >= 0) {
    pinLow(_csPin, _csMask);
  }
}

void DisplayManager::deselect() {
  if (_csPin >= 0) {
    pinHigh(_csPin, _csMask);
  }
}

void DisplayManager::pinHigh(int8_t pin, uint32_t mask) {
  if (pin >= 0 && pin < 32) {
    GPIO.out_w1ts = mask;
  } else if (pin >= 0) {
    digitalWrite(pin, HIGH);
  }
}

void DisplayManager::pinLow(int8_t pin, uint32_t mask) {
  if (pin >= 0 && pin < 32) {
    GPIO.out_w1tc = mask;
  } else if (pin >= 0) {
    digitalWrite(pin, LOW);
  }
}

void DisplayManager::clockPulse() {
  pinHigh(_clockPin, _clockMask);
  pinLow(_clockPin, _clockMask);
}

void DisplayManager::send9(bool isData, uint8_t value) {
  const uint16_t frame = (uint16_t(isData) << 8) | value;
  for (int8_t bit = 8; bit >= 0; --bit) {
    if ((frame >> bit) & 0x01) {
      pinHigh(_dataPin, _dataMask);
    } else {
      pinLow(_dataPin, _dataMask);
    }
    clockPulse();
  }
}

void DisplayManager::command(uint8_t value) {
  select();
  send9(false, value);
  deselect();
}

void DisplayManager::data(uint8_t value) {
  select();
  send9(true, value);
  deselect();
}

void DisplayManager::dataBytes(const uint8_t *values, size_t count) {
  select();
  for (size_t index = 0; index < count; ++index) {
    send9(true, values[index]);
  }
  deselect();
}

void DisplayManager::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  x0 += COLUMN_OFFSET;
  x1 += COLUMN_OFFSET;
  y0 += ROW_OFFSET;
  y1 += ROW_OFFSET;

  command(0x2A);
  const uint8_t xData[4] = {uint8_t(x0 >> 8), uint8_t(x0), uint8_t(x1 >> 8), uint8_t(x1)};
  dataBytes(xData, sizeof(xData));

  command(0x2B);
  const uint8_t yData[4] = {uint8_t(y0 >> 8), uint8_t(y0), uint8_t(y1 >> 8), uint8_t(y1)};
  dataBytes(yData, sizeof(yData));

  command(0x2C);
}

void DisplayManager::fillScreen(uint16_t color) {
  fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (w <= 0 || h <= 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
    return;
  }

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > SCREEN_WIDTH) {
    w = SCREEN_WIDTH - x;
  }
  if (y + h > SCREEN_HEIGHT) {
    h = SCREEN_HEIGHT - y;
  }
  if (w <= 0 || h <= 0) {
    return;
  }

  setWindow(x, y, x + w - 1, y + h - 1);
  const uint8_t highByte = color >> 8;
  const uint8_t lowByte = color & 0xFF;

  select();
  for (uint32_t index = 0; index < uint32_t(w) * uint32_t(h); ++index) {
    send9(true, highByte);
    send9(true, lowByte);
  }
  deselect();
}

void DisplayManager::drawHeader(const char *title, uint16_t color) {
  fillRect(0, 0, SCREEN_WIDTH, 36, color);
  drawText(title, 8, 10, strlen(title) > 18 ? 1 : 2, COLOR_BLACK);
}

void DisplayManager::drawText(const char *text, int16_t x, int16_t y, uint8_t scale, uint16_t color) {
  const int16_t step = 4 * scale;
  int16_t cursorX = x;
  int16_t cursorY = y;

  for (size_t index = 0; text[index] != '\0'; ++index) {
    if (text[index] == '\n') {
      cursorX = x;
      cursorY += 6 * scale;
      continue;
    }

    if (cursorX + 3 * scale > SCREEN_WIDTH) {
      cursorX = x;
      cursorY += 6 * scale;
    }

    drawChar(text[index], cursorX, cursorY, scale, color);
    cursorX += step;
  }
}

void DisplayManager::drawChar(char character, int16_t x, int16_t y, uint8_t scale, uint16_t color) {
  const uint8_t *rows = glyphFor(character);

  for (uint8_t row = 0; row < 5; ++row) {
    for (uint8_t col = 0; col < 3; ++col) {
      if (rows[row] & (1 << (2 - col))) {
        fillRect(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

uint16_t DisplayManager::color565(uint8_t r, uint8_t g, uint8_t b) const {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
