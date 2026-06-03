/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOSTREAM_HPP
#define AUDIOSTREAM_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"
#include "AudioDelay.hpp"

#include <alsa/asoundlib.h>

constexpr size_t __AUDIODEVICELIST_ENTRY_TEXTLENGTH = 256u;

struct _audiodevicelist_entry {
	char name[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
	char desc[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
};

typedef struct _audiodevicelist_entry audiodevicelist_entry_t;

struct _audiodevicelist {
	audiodevicelist_entry_t *p_entries;
	size_t n_entries;
};

typedef struct _audiodevicelist audiodevicelist_t;

struct _audiostream_params {
	size_t sample_rate;
	size_t n_channels;
	size_t audiobuffer_size_frames;
	size_t streambuffer_segment_size_frames;
	size_t streambuffer_n_segments;
	size_t delay_buffer_size_frames;
	size_t delay_n_ff_delays;
	size_t delay_n_fb_delays;
};

typedef struct _audiostream_params audiostream_params_t;

class AudioStream {
	public:
		AudioStream(const audiostream_params_t *p_params);

		bool setParameters(const audiostream_params_t *p_params);
		bool initialize(void);
		bool runStream(void);
		void pauseStream(void);
		void resumeStream(void);
		void stopStream(void);

		bool loadAudioDeviceList(bool output);
		ssize_t getAudioDeviceListEntryCount(bool output);
		const audiodevicelist_entry_t* getAudioDeviceListEntry(size_t index, bool output);

		bool chooseDevice(const char *name, bool output);
		bool chooseDevice(size_t index, bool output);
		bool chooseDefaultDevice(bool output);

		float delayGetDryInputAmplitude(void);
		float delayGetOutputAmplitude(void);

		bool delaySetDryInputAmplitude(float amp);
		bool delaySetOutputAmplitude(float amp);

		int32_t delayGetFFDelay(size_t n_fx);
		float delayGetFFAmplitude(size_t n_fx);

		bool delaySetFFDelay(size_t n_fx, uint32_t delay);
		bool delaySetFFAmplitude(size_t n_fx, float amp);

		int32_t delayGetFBDelay(size_t n_fx);
		float delayGetFBAmplitude(size_t n_fx);

		bool delaySetFBDelay(size_t n_fx, uint32_t delay);
		bool delaySetFBAmplitude(size_t n_fx, float amp);

		bool delayResetFFParams(void);
		bool delayResetFBParams(void);

		int getStatus(void);
		__string getLastErrorMessage(void);

		enum Status {
			STATUS_ERROR_INVALIDPARAMS = -5,
			STATUS_ERROR_MEMORY = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2, /*unused*/
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_RUNNING = 2,
			STATUS_PAUSED = 3,
			STATUS_STOPPED = 4
		};

	protected:
		static constexpr size_t N_CHANNELS_MIN = 1u;
		static constexpr size_t STREAMBUFFER_N_SEGMENTS_MIN = 2u;
		static constexpr size_t STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN = 32u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIODELAY_BUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIODELAY_BUFFER_N_SEGMENTS = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIODELAY_N_FF_DELAYS = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIODELAY_N_FB_DELAYS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_N_SEGMENTS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t SAMPLE_RATE = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t N_CHANNELS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIO_BYTES_PER_SAMPLE = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t delaybuffer_nseg = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_in_nseg = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_out_nseg = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_streambuffer_in = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_streambuffer_out = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) snd_pcm_t *p_audiodev_in = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) snd_pcm_t *p_audiodev_out = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) AudioDelay *p_delay = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::thread capture_thread;

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string AUDIODEVDESC_IN = "";
		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string AUDIODEVDESC_OUT = "";

		__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");

		__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevicelist_t audiodevlist_in = {
			.p_entries = NULL,
			.n_entries = 0u
		};

		__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevicelist_t audiodevlist_out = {
			.p_entries = NULL,
			.n_entries = 0u
		};

		__attribute__((__aligned__(32))) int AUDIODEV_FORMAT = -1;
		__attribute__((__aligned__(32))) int status = this->STATUS_UNINITIALIZED;

		void deinitialize(void);

		bool audio_hw_init(snd_pcm_t **pp_audiodev, const char *audiodev_desc, int stream_mode, int nonblock);
		void audio_hw_deinit(snd_pcm_t **pp_audiodev);

		bool buffer_alloc(void);
		void buffer_free(void);

		void stream_proc(void);
		void stream_init(void);

		void streambuffer_in_nseg_update(void);
		void streambuffer_out_nseg_update(void);
		void delaybuffer_nseg_update(void);

		void streambuffer_in_capture(void);
		void streambuffer_out_playback(void);

		virtual void delaybuffer_loadin(void) = 0;
		virtual void delaybuffer_loadout(void) = 0;

		void capture_thread_proc(void);
		void playback_dsp_thread_proc(void);
};

#endif /*AUDIOSTREAM_HPP*/

