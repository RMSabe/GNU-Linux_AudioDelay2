/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 1.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIORTDELAY_HPP
#define AUDIORTDELAY_HPP

#include "globldef.h"
#include "filedef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"

#include <alsa/asoundlib.h>

struct _audiortdelay_init_params {
	const char *file_dir;
	const char *audio_dev_desc;
	__offset audio_data_begin;
	__offset audio_data_end;
	size_t buffer_size_frames;
	size_t n_ff_delays;
	size_t n_fb_delays;
	uint32_t sample_rate;
	uint16_t n_channels;
};

struct _audiortdelay_fx_params {
	uint32_t delay;
	float amp;
};

typedef struct _audiortdelay_init_params audiortdelay_init_params_t;
typedef struct _audiortdelay_fx_params audiortdelay_fx_params_t;

class AudioRTDelay {
	public:
		AudioRTDelay(const audiortdelay_init_params_t *p_params);

		bool setInitParameters(const audiortdelay_init_params_t *p_params);
		bool initialize(void);
		bool runPlayback(void);

		std::string getLastErrorMessage(void);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

	protected:
		static constexpr size_t BUFFER_SIZE_FRAMES_MIN = 128u; /*Probably not a good idea having a buffer smaller than this.*/
		static constexpr size_t BUFFER_N_SEGMENTS_MIN = 2u; /*Must have at least 2 segments*/
		static constexpr size_t N_CHANNELS_MIN = 1u;

		static constexpr size_t AUDIOBUFFER_SIZE_FRAMES_DEFAULT = 1024u;

		size_t BUFFER_SIZE_FRAMES = 0u;
		size_t BUFFER_SIZE_SAMPLES = 0u;
		size_t BUFFER_SIZE_BYTES = 0u;

		size_t BUFFER_SEGMENT_SIZE_FRAMES = 0u;
		size_t BUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		size_t BUFFER_SEGMENT_SIZE_BYTES = 0u;

		size_t BUFFER_N_SEGMENTS = 0u;

		size_t N_CHANNELS = 0u;

		/*
		 * ..._PARAMS_LENGTH = size (in number of elements)
		 * ..._PARAMS_SIZE = size (in number of bytes)
		 */

		size_t P_FF_PARAMS_LENGTH = 0u;
		size_t P_FF_PARAMS_SIZE = 0u;

		size_t P_FB_PARAMS_LENGTH = 0u;
		size_t P_FB_PARAMS_SIZE = 0u;

		__offset AUDIO_DATA_BEGIN = 0;
		__offset AUDIO_DATA_END = 0;
		uint32_t SAMPLE_RATE = 0u;

		std::string FILEIN_DIR = "";
		std::string AUDIO_DEV_DESC = "";

		size_t n_loadsegment = 0u;
		size_t n_playsegment = 0u;

		void *p_bufferinput = NULL;
		void *p_bufferoutput = NULL;

		void **pp_bufferinput_segments = NULL;
		void **pp_bufferoutput_segments = NULL;

		audiortdelay_fx_params_t *p_ff_params = NULL;
		audiortdelay_fx_params_t *p_fb_params = NULL;

		snd_pcm_t *p_audiodev = NULL;

		int h_filein = -1;
		__offset filein_size = 0;
		__offset filein_pos = 0;

		std::string err_msg = "";
		std::string user_cmd = "";

		std::thread loadthread;
		std::thread playthread;
		std::thread userthread;

		int status = this->STATUS_UNINITIALIZED;
		bool stop = false;

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		void wait_all_threads(void);
		void stop_all_threads(void);

		virtual bool buffer_alloc(void) = 0;
		virtual void buffer_free(void) = 0;

		bool buffer_fxparams_alloc(void);
		void buffer_fxparams_free(void);

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);

		void buffer_segment_remap(void);
		void buffer_play(void);

		virtual void buffer_load(void) = 0;
		virtual void run_dsp(void) = 0;

		/*
		 * retrieve_prev_nframe() : get the previous (delayed) frame index from the current frame index and the delay value (number of frames).
		 *
		 * Naming:
		 * ..._nframe: frame index in context.
		 *
		 * ..._buf_nframe: frame index within the whole buffer
		 * ..._seg_nframe: frame index within a specific buffer segment
		 * ..._nseg: buffer segment index in context
		 *
		 * Inputs:
		 *
		 * retrieve_prev_nframe(curr_buf_nframe, n_delay, ...):
		 * curr_buf_nframe: the current buffer frame index.
		 * n_delay: number of frames to be delayed.
		 *
		 * retrieve_prev_nframe(curr_nseg, curr_seg_nframe, n_delay, ...):
		 * curr_nseg & curr_seg_nframe: the current segment index and frame index within segment.
		 * n_delay: number of frames to be delayed.
		 *
		 * Outputs:
		 *
		 * p_prev_buf_nframe: pointer to variable that receives the delayed frame index within the buffer. Set to NULL if unused.
		 * p_prev_nseg: pointer to variable that receives the delayed segment index. Set to NULL if unused.
		 * p_prev_seg_nframe: pointer to variable that receives the delayed frame index within the delayed segment. Set to NULL if unused.
		 *
		 * returns true if successful, false otherwise.
		 */

		bool retrieve_prev_nframe(size_t curr_buf_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe);
		bool retrieve_prev_nframe(size_t curr_nseg, size_t curr_seg_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe);

		void loadthread_proc(void);
		void playthread_proc(void);
		void userthread_proc(void);

		void cmdui_cmd_decode(void);

		bool cmdui_cmd_compare(const char *auth, const char *input, size_t stop_index);

		void cmdui_print_help_text(void);
		void cmdui_print_current_params(void);

		bool cmdui_get_nfx_from_cmd(const char *cmdargnfx, size_t *p_nfx, size_t *p_cmdargvalrelindex);

		bool cmdui_attempt_update_ffdelay(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_ffamp(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_fbdelay(const char *cmdargval, size_t n_fx);
		bool cmdui_attempt_update_fbamp(const char *cmdargval, size_t n_fx);
};

#endif /*AUDIORTDELAY_HPP*/

