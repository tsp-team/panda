// Filename: audio_load_wav.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"

Configure(audio_load_wav);

#include "audio_trait.h"

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename filename) {
  return MikModSample::load_wav(filename);
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

EXPCL_MISC AudioTraits::SoundClass* AudioLoadWav(Filename filename) {
  return WinSample::load_wav(filename);
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename) {
  audio_cat->error() << "Linux driver does not natively support WAV."
                     << "  Try the 'st' loader." << endl;
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_NULL)

// Null driver
#include "audio_null_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename) {
  return new NullSound();
}

#else /* AUDIO_USE_NULL */

#error "unknown implementation driver"

#endif /* AUDIO_USE_NULL */

ConfigureFn(audio_load_wav) {
  AudioPool::register_sound_loader("wav", AudioLoadWav);
}
