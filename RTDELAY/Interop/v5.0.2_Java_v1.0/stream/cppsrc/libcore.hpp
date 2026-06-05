/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2 (Audio Stream Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef LIBCORE_HPP
#define LIBCORE_HPP

#include "globldef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include "shared.hpp"

#include "AudioDelay.hpp"
#include "AudioStream.hpp"
#include "AudioStream_i16.hpp"
#include "AudioStream_i24.hpp"

extern size_t __AUDIO_STREAMBUFFER_N_SEGMENTS;
extern size_t __AUDIO_DELAY_BUFFER_SIZE_FRAMES;
extern size_t __AUDIO_DELAY_N_FFCH;
extern size_t __AUDIO_DELAY_N_FBCH;

extern __string err_msg;

extern size_t audio_samplerate;
extern size_t audio_nchannels;
extern int audio_format;

extern bool core_init(void);
extern void core_deinit(void);

extern bool createaudioobject(void);

extern bool load_audiodevicelist(bool output);
extern ssize_t audiodevicelist_get_entry_count(bool output);
extern const char* audiodevicelist_get_entry_info(size_t index, bool output, bool info);
extern bool choose_device(size_t index, bool defaultdev, bool enable_resampling, bool output);

extern bool audioobject_init(void);
extern int audioobject_getstatus(void);

extern bool run_stream(void);

extern void pause_stream(void);
extern void resume_stream(void);
extern void stop_stream(void);

extern float delay_getDryInputAmplitude(void);
extern bool delay_setDryInputAmplitude(float amp);

extern float delay_getOutputAmplitude(void);
extern bool delay_setOutputAmplitude(float amp);

extern ssize_t delay_getFFDelay(size_t n_fx);
extern bool delay_setFFDelay(size_t n_fx, uint32_t delay);

extern float delay_getFFAmplitude(size_t n_fx);
extern bool delay_setFFAmplitude(size_t n_fx, float amp);

extern ssize_t delay_getFBDelay(size_t n_fx);
extern bool delay_setFBDelay(size_t n_fx, uint32_t delay);

extern float delay_getFBAmplitude(size_t n_fx);
extern bool delay_setFBAmplitude(size_t n_fx, float amp);

extern bool delay_resetFFParams(void);
extern bool delay_resetFBParams(void);

#endif /*LIBCORE_HPP*/

