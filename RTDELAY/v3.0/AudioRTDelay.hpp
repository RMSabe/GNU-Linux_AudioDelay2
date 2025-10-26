/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIORTDELAY_HPP
#define AUDIORTDELAY_HPP

#include "globldef.h"
#include "strdef.hpp"

#include "shared.hpp"

struct _audiortdelay_init_params {
	size_t buffer_size_frames;
	size_t buffer_n_segments;
	size_t n_channels;
	size_t n_ff_delays;
	size_t n_fb_delays;
};

typedef struct _audiortdelay_init_params audiortdelay_init_params_t;

struct _audiortdelay_fx_params {
	uint32_t delay;
	float amp;
};

typedef struct _audiortdelay_fx_params audiortdelay_fx_params_t;

class AudioRTDelay {
	public:
		AudioRTDelay(const audiortdelay_init_params_t *p_params);
		~AudioRTDelay(void);

		bool setInitParameters(const audiortdelay_init_params_t *p_params);
		bool initialize(void);
		bool runDSP(size_t n_segment);

		float* getInputBuffer(void);
		float* getOutputBuffer(void);

		float* getInputBufferSegment(size_t n_segment);
		float* getOutputBufferSegment(size_t n_segment);

		float getDryInputAmplitude(void);
		float getOutputAmplitude(void);

		bool getFFParams(size_t n_fx, audiortdelay_fx_params_t *p_params);
		bool getFBParams(size_t n_fx, audiortdelay_fx_params_t *p_params);

		bool setDryInputAmplitude(float amp);
		bool setOutputAmplitude(float amp);

		bool setFFDelay(size_t n_fx, uint32_t delay);
		bool setFFAmplitude(size_t n_fx, float amp);

		bool setFBDelay(size_t n_fx, uint32_t delay);
		bool setFBAmplitude(size_t n_fx, float amp);

		bool resetFFParams(void);
		bool resetFBParams(void);

		std::string getLastErrorMessage(void);

		enum Status {
			STATUS_ERROR_MEMALLOC = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

	private:
		static constexpr size_t BUFFER_SIZE_FRAMES_MIN = 128u; /*Probably not a good idea having a buffer smaller than this anyway*/
		static constexpr size_t BUFFER_N_SEGMENTS_MIN = 2u; /*Must have at least 2 segments*/
		static constexpr size_t N_CHANNELS_MIN = 1u;

		size_t BUFFER_SIZE_FRAMES = 0u;
		size_t BUFFER_SIZE_SAMPLES = 0u;
		size_t BUFFER_SIZE_BYTES = 0u;

		size_t BUFFER_N_SEGMENTS = 0u;

		size_t BUFFER_SEGMENT_SIZE_FRAMES = 0u;
		size_t BUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		size_t BUFFER_SEGMENT_SIZE_BYTES = 0u;

		/*
		 * ..._PARAMS_LENGTH = size (in number of elements)
		 * ..._PARAMS_SIZE = size (in number of bytes)
		 */

		size_t P_FF_PARAMS_LENGTH = 0u;
		size_t P_FF_PARAMS_SIZE = 0u;

		size_t P_FB_PARAMS_LENGTH = 0u;
		size_t P_FB_PARAMS_SIZE = 0u;

		size_t N_CHANNELS = 0u;

		float *p_bufferinput = NULL;
		float *p_bufferoutput = NULL;

		float **pp_bufferinput_segments = NULL;
		float **pp_bufferoutput_segments = NULL;

		audiortdelay_fx_params_t *p_ff_params = NULL;
		audiortdelay_fx_params_t *p_fb_params = NULL;

		float dryinput_amp = 0.0f;
		float output_amp = 0.0f;

		std::string err_msg = "";
		int status = this->STATUS_UNINITIALIZED;

		bool buffer_alloc(void);
		void buffer_free(void);

		bool buffer_fxparams_alloc(void);
		void buffer_fxparams_free(void);

		/*
		 * retrive_prev_nframe(): get the previous (delayed) frame index from the current frame index and the delay value (number of frames).
		 *
		 * Naming:
		 * ..._nframe: frame index in context.
		 *
		 * ..._buf_nframe: frame index within the whole buffer
		 * ..._seg_nframe: frame index within a specific buffer segment
		 * ..._nseg: buffer segment index in context.
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
};

#endif /*AUDIORTDELAY_HPP*/

