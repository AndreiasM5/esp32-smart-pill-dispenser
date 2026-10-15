#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>

class AudioManager {
 public:
  explicit AudioManager(uint8_t audioPin);

  void begin();
  void update();
  void beepShort();
  void beepLong();
  void alertMelody();
  void stop();
  void setAlertRepeating(bool enabled);

 private:
  void startTone(uint16_t frequency, unsigned long durationMs);

  uint8_t _audioPin;
  bool _toneActive;
  bool _alertRepeating;
  uint8_t _melodyStep;
  unsigned long _toneStartedAt;
  unsigned long _toneDurationMs;
  unsigned long _nextAlertStepAt;

  static constexpr uint8_t LEDC_CHANNEL = 7;
  static constexpr uint8_t LEDC_RESOLUTION_BITS = 8;
};

#endif
