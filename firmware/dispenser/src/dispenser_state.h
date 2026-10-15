#ifndef DISPENSER_STATE_H
#define DISPENSER_STATE_H

#include <Arduino.h>

enum class DispenserState {
  IDLE,
  TIME_TO_TAKE_MEDICINE,
  DISPENSING,
  WAITING_CONFIRMATION,
  MEDICINE_CONFIRMED,
  ERROR
};

const char *dispenserStateToString(DispenserState state);

#endif
