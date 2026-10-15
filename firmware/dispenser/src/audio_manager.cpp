#include "audio_manager.h"

AudioManager::AudioManager(uint8_t audioPin)
    : _audioPin(audioPin),
      _toneActive(false),
      _alertRepeating(false),
      _melodyStep(0),
      _toneStartedAt(0),
      _toneDurationMs(0),
      _nextAlertStepAt(0) {}

void AudioManager::begin() {
  ledcSetup(LEDC_CHANNEL, 2000, LEDC_RESOLUTION_BITS);
  ledcAttachPin(_audioPin, LEDC_CHANNEL);
  stop();
}

void AudioManager::update() {
  const unsigned long now = millis();

  if (_toneActive && (now - _toneStartedAt >= _toneDurationMs)) {
    ledcWriteTone(LEDC_CHANNEL, 0);
    _toneActive = false;
  }

  if (!_alertRepeating || _toneActive || now < _nextAlertStepAt) {
    return;
  }

  const uint16_t melody[] = {1200, 1600, 1200, 0};
  const unsigned long durations[] = {120, 120, 180, 900};
  const unsigned long gaps[] = {120, 120, 120, 0};

  const uint16_t frequency = melody[_melodyStep];
  const unsigned long duration = durations[_melodyStep];

  if (frequency > 0) {
    startTone(frequency, duration);
  }

  _nextAlertStepAt = now + duration + gaps[_melodyStep];
  _melodyStep = (_melodyStep + 1) % 4;
}

void AudioManager::beepShort() {
  setAlertRepeating(false);
  startTone(1800, 120);
}

void AudioManager::beepLong() {
  setAlertRepeating(false);
  startTone(900, 500);
}

void AudioManager::alertMelody() {
  setAlertRepeating(true);
}

void AudioManager::stop() {
  ledcWriteTone(LEDC_CHANNEL, 0);
  _toneActive = false;
  _alertRepeating = false;
  _melodyStep = 0;
}

void AudioManager::setAlertRepeating(bool enabled) {
  _alertRepeating = enabled;
  if (enabled) {
    _melodyStep = 0;
    _nextAlertStepAt = 0;
  }
}

void AudioManager::startTone(uint16_t frequency, unsigned long durationMs) {
  ledcWriteTone(LEDC_CHANNEL, frequency);
  _toneActive = true;
  _toneStartedAt = millis();
  _toneDurationMs = durationMs;
}
