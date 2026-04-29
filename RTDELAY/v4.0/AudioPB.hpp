/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "filedef.h"
#include "strdef.hpp"

#include "shared.hpp"

#include "AudioRTDelay.hpp"

#include <alsa/asoundlib.h>

constexpr size_t __AUDIODEVICELIST_ENTRY_TEXTLENGTH = 256u;

struct _audiodevicelist_entry {
	char name[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
	char desc[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
};

typedef struct _audiodevicelist_entry audiodevicelist_entry_t;

struct _audiopb_params {
	const char *file_dir;
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
		void pausePlayback(void);
		void resumePlayback(void);
		void stopPlayback(void);

		bool loadAudioDeviceList(void);
		ssize_t getAudioDeviceListEntryCount(void);
		const audiodevicelist_entry_t* getAudioDeviceListEntry(size_t index);

		bool chooseDevice(const char *name);
		bool chooseDevice(size_t index);
		bool chooseDefaultDevice(void);

		__offset_t getAudioDataSizeFrames(void);
		__offset_t getAudioDataPositionFrames(void);
		bool setAudioDataPositionFrames(__offset_t position);

		float rtdelayGetDryInputAmplitude(void);
		float rtdelayGetOutputAmplitude(void);

		bool rtdelaySetDryInputAmplitude(float amp);
		bool rtdelaySetOutputAmplitude(float amp);

		int32_t rtdelayGetFFDelay(size_t n_fx);
		float rtdelayGetFFAmplitude(size_t n_fx);

		bool rtdelaySetFFDelay(size_t n_fx, uint32_t delay);
		bool rtdelaySetFFAmplitude(size_t n_fx, float amp);

		int32_t rtdelayGetFBDelay(size_t n_fx);
		float rtdelayGetFBAmplitude(size_t n_fx);

		bool rtdelaySetFBDelay(size_t n_fx, uint32_t delay);
		bool rtdelaySetFBAmplitude(size_t n_fx, float amp);

		bool rtdelayResetFFParams(void);
		bool rtdelayResetFBParams(void);

		__string getLastErrorMessage(void);
		int getStatus(void);

		enum Status {
			STATUS_ERROR_MEMORY = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2,
			STATUS_PAUSED = 3,
			STATUS_STOPPED = 4
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

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t FILE_BYTES_PER_SAMPLE = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIO_BYTES_PER_SAMPLE = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string FILEIN_DIR = "";
		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string AUDIODEV_DESC = "";

		__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");

		__attribute__((__aligned__(32))) int status = this->STATUS_UNINITIALIZED;

		__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevicelist_entry_t *p_audiodevicelist = NULL;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t audiodevicelist_n_entries = 0u;

		bool buffer_cycle = false;

		void deinitialize(void);

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		virtual bool buffer_alloc(void) = 0;
		virtual void buffer_free(void) = 0;

		bool audiodevicelist_alloc(void);
		void audiodevicelist_free(void);

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);

		void buffer_segment_update(void);
		void buffer_remap(void);

		void buffer_play(void);

		virtual void buffer_load_in(void) = 0;
		virtual void buffer_load_out(void) = 0;
};

#endif /*AUDIOPB_HPP*/

