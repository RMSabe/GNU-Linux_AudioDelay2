/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "filedef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"

#include "AudioRTDelay.hpp"

#include <alsa/asoundlib.h>

struct _audiopb_params {
	const char *file_dir;
	const char *audio_dev_desc;
	__offset_t audio_data_begin;
	__offset_t audio_data_end;
	size_t sample_rate;
	size_t n_channels;
	size_t rtdelay_buffer_size_frames;
	size_t rtdelay_n_ff_delays;
	size_t rtdelay_n_fb_delays;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		bool setParameters(const audiopb_params_t *p_params);
		bool initialize(void);
		bool runPlayback(void);
		void stopPlayback(void);

		float rtdelayGetDryInputAmplitude(void);
		float rtdelayGetOutputAmplitude(void);

		bool rtdelayGetFFParams(size_t n_fx, audiortdelay_fx_params_t *p_params);
		bool rtdelayGetFBParams(size_t n_fx, audiortdelay_fx_params_t *p_params);

		bool rtdelaySetDryInputAmplitude(float amp);
		bool rtdelaySetOutputAmplitude(float amp);

		bool rtdelaySetFFDelay(size_t n_fx, uint32_t delay);
		bool rtdelaySetFFAmplitude(size_t n_fx, float amp);

		bool rtdelaySetFBDelay(size_t n_fx, uint32_t delay);
		bool rtdelaySetFBAmplitude(size_t n_fx, float amp);

		bool rtdelayResetFFParams(void);
		bool rtdelayResetFBParams(void);

		__string getLastErrorMessage(void);
		int getStatus(void);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2
		};

	protected:
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIORTDELAY_BUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIORTDELAY_N_FF_DELAYS = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIORTDELAY_N_FB_DELAYS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t BUFFER_N_SEGMENTS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t n_segment = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_bufferinput = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_bufferoutput0 = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_bufferoutput1 = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_loadoutput = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_playoutput = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) snd_pcm_t *p_audiodev = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) AudioRTDelay *p_delay = NULL;

		__attribute__((__aligned__(32))) int h_filein = -1;
		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t filein_size = 0;
		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t filein_pos = 0;

		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t AUDIO_DATA_BEGIN = 0;
		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t AUDIO_DATA_END = 0;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t SAMPLE_RATE = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t N_CHANNELS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string FILEIN_DIR = "";
		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string AUDIODEV_DESC = "";

		__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::thread playthread;

		__attribute__((__aligned__(32))) int status = this->STATUS_UNINITIALIZED;

		bool buffer_cycle = false;
		bool stop_playback = false;

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		virtual bool buffer_alloc(void) = 0;
		virtual void buffer_free(void) = 0;

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);

		void buffer_segment_update(void);
		void buffer_remap(void);

		void buffer_play(void);

		virtual void buffer_load_in(void) = 0;
		virtual void buffer_load_out(void) = 0;

		void loadthread_proc(void); /*loadthread_proc will be run by main thread.*/
		void playthread_proc(void); /*playthread_proc will be run by playthread*/
};

#endif /*AUDIOPB_HPP*/

