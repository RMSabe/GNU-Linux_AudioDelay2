/*
 * Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioDelay.hpp"
#include <stdlib.h>
#include <string.h>

AudioDelay::AudioDelay(const audiodelay_init_params_t *p_params)
{
	this->setInitParameters(p_params);
}

AudioDelay::~AudioDelay(void)
{
	this->deinitialize();
}

bool AudioDelay::setInitParameters(const audiodelay_init_params_t *p_params)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioDelay::setInitParameters: Error: init params object pointer is null.");
		return false;
	}

	this->BUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->buffer_size_frames);
	this->BUFFER_N_SEGMENTS = _get_closest_power2_ceil(p_params->buffer_n_segments);
	this->N_CHANNELS = p_params->n_channels;
	this->P_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->P_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	return true;
}

bool AudioDelay::initialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(this->BUFFER_SIZE_FRAMES < this->BUFFER_SIZE_FRAMES_MIN)
	{
		this->err_msg = __TEXT("AudioDelay::initialize: Error: invalid buffer size.");
		return false;
	}

	if(this->BUFFER_N_SEGMENTS < this->BUFFER_N_SEGMENTS_MIN)
	{
		this->err_msg = __TEXT("AudioDelay::initialize: Error: invalid buffer segment count.");
		return false;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->err_msg = __TEXT("AudioDelay::initialize: Error: invalid number of channels.");
		return false;
	}

	this->BUFFER_SIZE_SAMPLES = (this->BUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SIZE_BYTES = (this->BUFFER_SIZE_SAMPLES)*sizeof(float);

	this->BUFFER_SEGMENT_SIZE_FRAMES = (this->BUFFER_SIZE_FRAMES)/(this->BUFFER_N_SEGMENTS);
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = (this->BUFFER_SEGMENT_SIZE_SAMPLES)*sizeof(float);

	this->P_FF_PARAMS_SIZE = (this->P_FF_PARAMS_LENGTH)*sizeof(audiodelay_fx_params_t);
	this->P_FB_PARAMS_SIZE = (this->P_FB_PARAMS_LENGTH)*sizeof(audiodelay_fx_params_t);

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioDelay::initialize: Error: memory allocate failed.");
		return false;
	}

	this->status = this->STATUS_INITIALIZED;
	return true;
}

bool AudioDelay::runDSP(size_t n_segment)
{
	float *p_curr_seg_in = NULL;
	float *p_curr_seg_out = NULL;

	size_t n_prevsample = 0u;
	size_t n_currsample = 0u;
	size_t n_channel = 0u;

	size_t n_frame = 0u;
	size_t n_fx = 0u;
	size_t buf_prev_nframe = 0u;

	size_t n_delay = 0u;
	float f_amp = 0.0f;

	if(this->status < 1) return false;

	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = __TEXT("AudioDelay::runDSP: Error: given segment index is out of bounds.");
		return false;
	}

	p_curr_seg_in = (float*) (((uintptr_t) (this->p_bufferinput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
	p_curr_seg_out = (float*) (((uintptr_t) (this->p_bufferoutput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));

	for(n_frame = 0u; n_frame < this->BUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		n_currsample = n_frame*(this->N_CHANNELS);
		f_amp = this->dryinput_amp;

		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_curr_seg_out[n_currsample] = f_amp*(p_curr_seg_in[n_currsample]);
			n_currsample++;
		}

		n_fx = 0u;
		while(n_fx < this->P_FF_PARAMS_LENGTH)
		{
			n_delay = (size_t) this->p_ff_params[n_fx].delay;
			f_amp = this->p_ff_params[n_fx].amp;

			if(f_amp == 0.0f)
			{
				n_fx++;
				continue;
			}

			this->retrieve_prev_nframe(n_segment, n_frame, n_delay, &buf_prev_nframe, NULL, NULL);

			n_currsample = n_frame*(this->N_CHANNELS);
			n_prevsample = buf_prev_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_curr_seg_out[n_currsample] += f_amp*(this->p_bufferinput[n_prevsample]);
				n_currsample++;
				n_prevsample++;
			}

			n_fx++;
		}

		n_fx = 0u;
		while(n_fx < this->P_FB_PARAMS_LENGTH)
		{
			n_delay = (size_t) this->p_fb_params[n_fx].delay;
			f_amp = this->p_fb_params[n_fx].amp;

			if(f_amp == 0.0f)
			{
				n_fx++;
				continue;
			}

			this->retrieve_prev_nframe(n_segment, n_frame, n_delay, &buf_prev_nframe, NULL, NULL);

			n_currsample = n_frame*(this->N_CHANNELS);
			n_prevsample = buf_prev_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_curr_seg_out[n_currsample] += f_amp*(this->p_bufferoutput[n_prevsample]);
				n_currsample++;
				n_prevsample++;
			}

			n_fx++;
		}

		n_currsample = n_frame*(this->N_CHANNELS);
		f_amp = this->output_amp;

		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_curr_seg_out[n_currsample] *= f_amp;
			n_currsample++;
		}
	}

	return true;
}

float* AudioDelay::getInputBuffer(void)
{
	if(this->status < 1) return NULL;

	return this->p_bufferinput;
}

float* AudioDelay::getOutputBuffer(void)
{
	if(this->status < 1) return NULL;

	return this->p_bufferoutput;
}

float* AudioDelay::getInputBufferSegment(size_t n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = __TEXT("AudioDelay::getInputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return (float*) (((uintptr_t) (this->p_bufferinput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
}

float* AudioDelay::getOutputBufferSegment(size_t n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = __TEXT("AudioDelay::getOutputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return (float*) (((uintptr_t) (this->p_bufferoutput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
}

float AudioDelay::getDryInputAmplitude(void)
{
	return this->dryinput_amp;
}

float AudioDelay::getOutputAmplitude(void)
{
	return this->output_amp;
}

bool AudioDelay::getFFParams(size_t n_fx, audiodelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::getFFParams: Error: given fx index is out of bounds.");
		return false;
	}

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioDelay::getFFParams: Error: given params object pointer is null.");
		return false;
	}

	memcpy(p_params, &(this->p_ff_params[n_fx]), sizeof(audiodelay_fx_params_t));
	return true;
}

bool AudioDelay::getFBParams(size_t n_fx, audiodelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::getFBParams: Error: given fx index is out of bounds.");
		return false;
	}

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioDelay::getFBParams: Error: given params object pointer is null.");
		return false;
	}

	memcpy(p_params, &(this->p_fb_params[n_fx]), sizeof(audiodelay_fx_params_t));
	return true;
}

bool AudioDelay::setDryInputAmplitude(float amp)
{
	if(this->status < 1) return false;

	this->dryinput_amp = amp;
	return true;
}

bool AudioDelay::setOutputAmplitude(float amp)
{
	if(this->status < 1) return false;

	this->output_amp = amp;
	return true;
}

bool AudioDelay::setFFDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::setFFDelay: Error: given fx index is out of bounds.");
		return false;
	}

	if(((size_t) delay) >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = __TEXT("AudioDelay::setFFDelay: Error: given delay value is too big.");
		return false;
	}

	this->p_ff_params[n_fx].delay = delay;
	return true;
}

bool AudioDelay::setFFAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::setFFAmplitude: Error: given fx index is out of bounds.");
		return false;
	}

	this->p_ff_params[n_fx].amp = amp;
	return true;
}

bool AudioDelay::setFBDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::setFBDelay: Error: given fx index is out of bounds.");
		return false;
	}

	if(((size_t) delay) >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = __TEXT("AudioDelay::setFBDelay: Error: given delay value is too big.");
		return false;
	}

	this->p_fb_params[n_fx].delay = delay;
	return true;
}

bool AudioDelay::setFBAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = __TEXT("AudioDelay::setFBAmplitude: Error: given fx index is out of bounds.");
		return false;
	}

	this->p_fb_params[n_fx].amp = amp;
	return true;
}

bool AudioDelay::resetFFParams(void)
{
	if(this->status < 1) return false;

	if(this->P_FF_PARAMS_LENGTH) memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);

	return true;
}

bool AudioDelay::resetFBParams(void)
{
	if(this->status < 1) return false;

	if(this->P_FB_PARAMS_LENGTH) memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);

	return true;
}

int AudioDelay::getStatus(void)
{
	return this->status;
}

__string AudioDelay::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return __TEXT("AudioDelay object not initialized\nExtended error message: ") + this->err_msg;

	return this->err_msg;
}

void AudioDelay::deinitialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->buffer_free();

	return;
}

bool AudioDelay::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	if(!this->buffer_fxparams_alloc()) return false;

	this->p_bufferinput = (float*) malloc(this->BUFFER_SIZE_BYTES);
	this->p_bufferoutput = (float*) malloc(this->BUFFER_SIZE_BYTES);

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_bufferoutput == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_bufferinput, 0, this->BUFFER_SIZE_BYTES);
	memset(this->p_bufferoutput, 0, this->BUFFER_SIZE_BYTES);

	return true;
}

void AudioDelay::buffer_free(void)
{
	if(this->p_bufferinput != NULL)
	{
		free(this->p_bufferinput);
		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput != NULL)
	{
		free(this->p_bufferoutput);
		this->p_bufferoutput = NULL;
	}

	this->buffer_fxparams_free();
	return;
}

bool AudioDelay::buffer_fxparams_alloc(void)
{
	this->buffer_fxparams_free(); /*Clear any previous fxparams allocations*/

	if(this->P_FF_PARAMS_LENGTH)
	{
		this->p_ff_params = (audiodelay_fx_params_t*) malloc(this->P_FF_PARAMS_SIZE);
		if(this->p_ff_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);
	}

	if(this->P_FB_PARAMS_LENGTH)
	{
		this->p_fb_params = (audiodelay_fx_params_t*) malloc(this->P_FB_PARAMS_SIZE);
		if(this->p_fb_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);
	}

	return true;
}

void AudioDelay::buffer_fxparams_free(void)
{
	if(this->p_ff_params != NULL)
	{
		free(this->p_ff_params);
		this->p_ff_params = NULL;
	}

	if(this->p_fb_params != NULL)
	{
		free(this->p_fb_params);
		this->p_fb_params = NULL;
	}

	return;
}

bool AudioDelay::retrieve_prev_nframe(size_t curr_buf_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
{
	size_t prev_buf_nframe = 0u;
	size_t prev_nseg = 0u;
	size_t prev_seg_nframe = 0u;

	if(curr_buf_nframe >= this->BUFFER_SIZE_FRAMES) return false;
	if(n_delay >= this->BUFFER_SIZE_FRAMES) return false;

	if(n_delay > curr_buf_nframe) prev_buf_nframe = this->BUFFER_SIZE_FRAMES - (n_delay - curr_buf_nframe);
	else prev_buf_nframe = curr_buf_nframe - n_delay;

	prev_nseg = prev_buf_nframe/(this->BUFFER_SEGMENT_SIZE_FRAMES);
	prev_seg_nframe = prev_buf_nframe%(this->BUFFER_SEGMENT_SIZE_FRAMES);

	if(p_prev_buf_nframe != NULL) *p_prev_buf_nframe = prev_buf_nframe;
	if(p_prev_nseg != NULL) *p_prev_nseg = prev_nseg;
	if(p_prev_seg_nframe != NULL) *p_prev_seg_nframe = prev_seg_nframe;

	return true;
}

bool AudioDelay::retrieve_prev_nframe(size_t curr_nseg, size_t curr_seg_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
{
	size_t curr_buf_nframe = 0u;

	if(curr_nseg >= this->BUFFER_N_SEGMENTS) return false;
	if(curr_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return false;

	curr_buf_nframe = curr_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + curr_seg_nframe;

	return this->retrieve_prev_nframe(curr_buf_nframe, n_delay, p_prev_buf_nframe, p_prev_nseg, p_prev_seg_nframe);
}

