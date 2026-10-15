#ifndef CAMERA_PINS_H
#define CAMERA_PINS_H

#include <Arduino.h>

namespace CameraPins {

constexpr int PWDN = -1;
constexpr int RESET = -1;
constexpr int XCLK = 15;
constexpr int SIOD = 4;
constexpr int SIOC = 5;

constexpr int Y9 = 16;
constexpr int Y8 = 17;
constexpr int Y7 = 18;
constexpr int Y6 = 12;
constexpr int Y5 = 10;
constexpr int Y4 = 8;
constexpr int Y3 = 9;
constexpr int Y2 = 11;

constexpr int VSYNC = 6;
constexpr int HREF = 7;
constexpr int PCLK = 13;
}  // namespace CameraPins

namespace SdPins {
constexpr int CLK = 39;
constexpr int CMD = 38;
constexpr int D0 = 40;
}  // namespace SdPins

#endif
