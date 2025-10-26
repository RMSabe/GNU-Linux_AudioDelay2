/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.0
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
	__offset audio_data_begin;
	__offset audio_data_end;
	uint32_t sample_rate;
	uint16_t n_channels;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		bool setParameters(const audiopb_params_t *p_params);
		bool initialize(void);
		bool runPlayback(void);

		std::string getLastErrorMessage(void);

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
		static constexpr size_t AUDIORTDELAY_BUFFER_SIZE_FRAMES = 65536u;
		static constexpr size_t AUDIORTDELAY_N_FF_DELAYS = 4u;
		static constexpr size_t AUDIORTDELAY_N_FB_DELAYS = 4u;

		size_t AUDIOBUFFER_SIZE_FRAMES = 0u;
		size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		size_t AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		size_t AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		size_t BUFFER_N_SEGMENTS = 0u;

		size_t n_segment = 0u;

		void *p_bufferinput = NULL;
		void *p_bufferoutput0 = NULL;
		void *p_bufferoutput1 = NULL;

		void *p_loadoutput = NULL;
		void *p_playoutput = NULL;

		snd_pcm_t *p_audiodev = NULL;

		AudioRTDelay *p_delay = NULL;

		int h_filein = -1;
		__offset filein_size = 0;
		__offset filein_pos = 0;

		__offset AUDIO_DATA_BEGIN = 0;
		__offset AUDIO_DATA_END = 0;

		size_t SAMPLE_RATE = 0u;
		size_t N_CHANNELS = 0u;

		std::string FILEIN_DIR = "";
		std::string AUDIODEV_DESC = "";

		std::string err_msg = "";
		std::string user_cmd = "";

		std::thread playthread;
		std::thread userthread;

		int status = this->STATUS_UNINITIALIZED;

		bool buffer_cycle = false;
		bool stop_playback = false;

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		void wait_all_threads(void);
		void stop_all_threads(void);

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

		void cmdui_cmd_decode(void);

		bool cmdui_cmd_compare(const char *auth, const char *input, size_t stop_index);

		void cmdui_print_help_text(void);
		void cmdui_print_current_params(void);

		bool cmdui_get_nfx_from_cmd(const char *cmdargnfx, size_t *p_nfx, size_t *p_cmdargvalrelindex);

		bool cmdui_attempt_update_dryamp(const char *cmdarg);
		bool cmdui_attempt_update_outamp(const char *cmdarg);

		bool cmdui_attempt_update_ffdelay(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_ffamp(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_fbdelay(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_fbamp(const char *cmdargval, size_t n_fx);

		void loadthread_proc(void); /*loadthread_proc will be run by main thread.*/
		void playthread_proc(void); /*playthread_proc will be run by playthread*/
		void userthread_proc(void); /*userthread_proc will be run by userthread*/
};

#endif /*AUDIOPB_HPP*/

