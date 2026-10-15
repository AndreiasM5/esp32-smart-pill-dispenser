#include "dispenser_state.h"

const char *dispenserStateToString(DispenserState state) {
  switch (state) {
    case DispenserState::IDLE:
      return "IDLE";
    case DispenserState::TIME_TO_TAKE_MEDICINE:
      return "TIME_TO_TAKE_MEDICINE";
    case DispenserState::DISPENSING:
      return "DISPENSING";
    case DispenserState::WAITING_CONFIRMATION:
      return "WAITING_CONFIRMATION";
    case DispenserState::MEDICINE_CONFIRMED:
      return "MEDICINE_CONFIRMED";
    case DispenserState::ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
