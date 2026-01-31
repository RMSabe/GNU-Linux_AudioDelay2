/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef LIBCORE_HPP
#define LIBCORE_HPP

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "strdef.hpp"

#include "shared.hpp"

#define RTDELAY_BUFFER_SIZE_FRAMES 65536U
#define RTDELAY_N_FF_DELAYS 4U
#define RTDELAY_N_FB_DELAYS 4U

extern bool core_init(void);
extern void core_deinit(void);

extern const __tchar_t* get_last_errmsg(void);
extern bool set_filein_dir(const char *fdir);
extern bool set_audiodev_desc(const char *desc);

extern bool playback_init(void);
extern bool playback_run(void);

extern void stop_playback(void);

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

extern void libjni_trigger_exitProcess(int exitCode);

#endif /*LIBCORE_HPP*/

