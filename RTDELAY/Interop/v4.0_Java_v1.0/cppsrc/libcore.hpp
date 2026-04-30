/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
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

#include "AudioRTDelay.hpp"
#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define RTDELAY_BUFFER_SIZE_FRAMES 65536U
#define RTDELAY_N_FFCH 4U
#define RTDELAY_N_FBCH 4U

extern bool core_init(void);
extern void core_deinit(void);

extern const __tchar_t* get_last_errmsg(void);
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

extern float rtdelay_getDryInputAmplitude(void);
extern bool rtdelay_setDryInputAmplitude(float amp);

extern float rtdelay_getOutputAmplitude(void);
extern bool rtdelay_setOutputAmplitude(float amp);

extern ssize_t rtdelay_getFFDelay(size_t n_fx);
extern bool rtdelay_setFFDelay(size_t n_fx, uint32_t delay);

extern float rtdelay_getFFAmplitude(size_t n_fx);
extern bool rtdelay_setFFAmplitude(size_t n_fx, float amp);

extern ssize_t rtdelay_getFBDelay(size_t n_fx);
extern bool rtdelay_setFBDelay(size_t n_fx, uint32_t delay);

extern float rtdelay_getFBAmplitude(size_t n_fx);
extern bool rtdelay_setFBAmplitude(size_t n_fx, float amp);

extern bool rtdelay_resetFFParams(void);
extern bool rtdelay_resetFBParams(void);

#endif /*LIBCORE_HPP*/

