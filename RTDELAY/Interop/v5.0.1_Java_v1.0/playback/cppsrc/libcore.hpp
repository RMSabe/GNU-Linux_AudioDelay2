/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1 (File Playback Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef LIBCORE_HPP
#define LIBCORE_HPP

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include "shared.hpp"

#include "AudioDelay.hpp"
#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

extern size_t __AUDIO_STREAMBUFFER_N_SEGMENTS;
extern size_t __AUDIO_DELAY_BUFFER_SIZE_FRAMES;
extern size_t __AUDIO_DELAY_N_FFCH;
extern size_t __AUDIO_DELAY_N_FBCH;

extern __string err_msg;

extern bool core_init(void);
extern void core_deinit(void);

extern bool set_filein_dir(const char *fdir);
extern bool loadfile_createaudioobject(void);

extern bool load_audiodevicelist(void);
extern ssize_t audiodevicelist_get_entry_count(void);
extern const char* audiodevicelist_get_entry_info(size_t index, bool info);
extern bool choose_device(size_t index, bool defaultdev);

extern bool audioobject_init(void);
extern int audioobject_getstatus(void);

extern bool run_playback(void);

extern void pause_playback(void);
extern void resume_playback(void);
extern void stop_playback(void);

extern __offset_t audiodata_get_size_frames(void);
extern __offset_t audiodata_get_position_frames(void);
extern bool audiodata_set_position_frames(__offset_t position);

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

