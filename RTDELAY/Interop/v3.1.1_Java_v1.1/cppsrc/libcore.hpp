/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.1)
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

#include "AudioRTDelay.hpp"
#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define RTDELAY_BUFFER_SIZE_FRAMES 65536U
#define RTDELAY_N_FF_DELAYS 4U
#define RTDELAY_N_FB_DELAYS 4U

/*
 * procctx_t : Process Context
 *
 * A set of global variables exclusive to each process using the shared object.
 * Because we're building a shared object, static variables must not be used, (unless they're meant to be shared among processes).
 *
 * To isolate each process' exclusive variables, we'll store them in the Process Context object.
 * Each process using the shared object will have its own Process Context, (its own set of global variables), isolated from other processes.
 */

struct _procctx {
	__attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio;
	__attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;
	__attribute__((__aligned__(PTR_SIZE_BITS))) std::string *p_fileindir;
	__attribute__((__aligned__(PTR_SIZE_BITS))) std::string *p_audiodevdesc;
	__attribute__((__aligned__(PTR_SIZE_BITS))) __string *p_errmsg;
	__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_jvm;
	__attribute__((__aligned__(32))) int h_filein;
};

typedef struct _procctx procctx_t;

extern procctx_t* procctx_alloc(void);
extern bool procctx_free(procctx_t *p_procctx);

extern bool core_init(procctx_t *p_procctx);
extern void core_deinit(procctx_t *p_procctx);

extern const __tchar_t* get_last_errmsg(procctx_t *p_procctx);
extern bool set_filein_dir(procctx_t *p_procctx, const char *fdir);
extern bool set_audiodev_desc(procctx_t *p_procctx, const char *desc);

extern bool playback_init(procctx_t *p_procctx);
extern bool playback_run(procctx_t *p_procctx);

extern void stop_playback(procctx_t *p_procctx);

extern float rtdelay_getDryInputAmplitude(procctx_t *p_procctx);
extern bool rtdelay_setDryInputAmplitude(procctx_t *p_procctx, float amp);

extern float rtdelay_getOutputAmplitude(procctx_t *p_procctx);
extern bool rtdelay_setOutputAmplitude(procctx_t *p_procctx, float amp);

extern ssize_t rtdelay_getFFDelay(procctx_t *p_procctx, size_t n_fx);
extern bool rtdelay_setFFDelay(procctx_t *p_procctx, size_t n_fx, uint32_t delay);

extern float rtdelay_getFFAmplitude(procctx_t *p_procctx, size_t n_fx);
extern bool rtdelay_setFFAmplitude(procctx_t *p_procctx, size_t n_fx, float amp);

extern ssize_t rtdelay_getFBDelay(procctx_t *p_procctx, size_t n_fx);
extern bool rtdelay_setFBDelay(procctx_t *p_procctx, size_t n_fx, uint32_t delay);

extern float rtdelay_getFBAmplitude(procctx_t *p_procctx, size_t n_fx);
extern bool rtdelay_setFBAmplitude(procctx_t *p_procctx, size_t n_fx, float amp);

extern bool rtdelay_resetFFParams(procctx_t *p_procctx);
extern bool rtdelay_resetFBParams(procctx_t *p_procctx);

#endif /*LIBCORE_HPP*/

