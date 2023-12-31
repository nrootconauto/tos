#include "sound.h"

static SDL_AudioDeviceID output;
static u64               sample, freq;
static SDL_AudioSpec     have;
static f64               volume = .1;

static void AudioCB(void* ud, Uint8* out, int len) {
  (void)ud;
  for (int i = 0; i < len / have.channels; ++i) {
    f64   t     = (f64)++sample / have.freq;
    f64   amp   = -1. + 2. * roundf(fmod(2. * t * freq, 1.));
    Sint8 maxed = ((amp > 0) ? 127 : -127) * volume;
    if (!freq)
      maxed = 0;
    for (Uint8 j = 0; j < have.channels; ++j)
      out[have.channels * i + j] = maxed;
  }
}

void InitSound(void) {
  if (SDL_Init(SDL_INIT_AUDIO)) {
    fprintf(stderr,
            "Failed to init SDL's sound subsystem with the following message: "
            "\"%s\"\n",
            SDL_GetError());
    fflush(stderr);
    _Exit(1);
  }
  output = SDL_OpenAudioDevice(NULL, 0,
                               &(SDL_AudioSpec){
                                   .freq     = 24000,
                                   .format   = AUDIO_S8,
                                   .channels = 2,
                                   .samples  = 256,
                                   .callback = AudioCB,
                               },
                               &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  SDL_PauseAudioDevice(output, 0);
}

void SndFreq(u64 f) {
  freq = f;
}

void SetVolume(f64 v) {
  volume = v;
}

f64 GetVolume(void) {
  return volume;
}

// vim: set expandtab ts=2 sw=2 :
