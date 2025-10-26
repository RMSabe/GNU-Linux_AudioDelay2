/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioRTDelay.hpp"
#include <stdlib.h>
#include <string.h>

AudioRTDelay::AudioRTDelay(const audiortdelay_init_params_t *p_params)
{
	this->setInitParameters(p_params);
}

AudioRTDelay::~AudioRTDelay(void)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->buffer_free();
}

bool AudioRTDelay::setInitParameters(const audiortdelay_init_params_t *p_params)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = "AudioRTDelay::setInitParameters: Error: init params object pointer is null.";
		return false;
	}

	this->BUFFER_SIZE_FRAMES = p_params->buffer_size_frames;
	this->BUFFER_N_SEGMENTS = p_params->buffer_n_segments;
	this->N_CHANNELS = p_params->n_channels;
	this->P_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->P_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	return true;
}

bool AudioRTDelay::initialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(this->BUFFER_SIZE_FRAMES < this->BUFFER_SIZE_FRAMES_MIN)
	{
		this->err_msg = "AudioRTDelay::initialize: Error: BUFFER SIZE FRAMES value is invalid.";
		return false;
	}

	if(this->BUFFER_N_SEGMENTS < this->BUFFER_N_SEGMENTS_MIN)
	{
		this->err_msg = "AudioRTDelay::initialize: Error: BUFFER N SEGMENTS value is invalid.";
		return false;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->err_msg = "AudioRTDelay::initialize: Error: N CHANNELS value is invalid.";
		return false;
	}

	this->BUFFER_SIZE_SAMPLES = (this->BUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SIZE_BYTES = (this->BUFFER_SIZE_SAMPLES)*sizeof(float);

	this->BUFFER_SEGMENT_SIZE_FRAMES = (this->BUFFER_SIZE_FRAMES)/(this->BUFFER_N_SEGMENTS);
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = (this->BUFFER_SEGMENT_SIZE_SAMPLES)*sizeof(float);

	this->P_FF_PARAMS_SIZE = (this->P_FF_PARAMS_LENGTH)*sizeof(audiortdelay_fx_params_t);
	this->P_FB_PARAMS_SIZE = (this->P_FB_PARAMS_LENGTH)*sizeof(audiortdelay_fx_params_t);

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = "AudioRTDelay::initialize: Error: memory allocate failed.";
		return false;
	}

	this->status = this->STATUS_INITIALIZED;
	return true;
}

bool AudioRTDelay::runDSP(size_t n_segment)
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
		this->err_msg = "AudioRTDelay::runDSP: Error: given segment index is out of bounds.";
		return false;
	}

	p_curr_seg_in = this->pp_bufferinput_segments[n_segment];
	p_curr_seg_out = this->pp_bufferoutput_segments[n_segment];

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

float* AudioRTDelay::getInputBuffer(void)
{
	if(this->status < 1) return NULL;

	return this->p_bufferinput;
}

float* AudioRTDelay::getOutputBuffer(void)
{
	if(this->status < 1) return NULL;

	return this->p_bufferoutput;
}

float* AudioRTDelay::getInputBufferSegment(size_t n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = "AudioRTDelay::getInputBufferSegment: Error: given segment index is out of bounds.";
		return NULL;
	}

	return this->pp_bufferinput_segments[n_segment];
}

float* AudioRTDelay::getOutputBufferSegment(size_t n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = "AudioRTDelay::getOutputBufferSegment: Error: given segment index is out of bounds.";
		return NULL;
	}

	return this->pp_bufferoutput_segments[n_segment];
}

float AudioRTDelay::getDryInputAmplitude(void)
{
	return this->dryinput_amp;
}

float AudioRTDelay::getOutputAmplitude(void)
{
	return this->output_amp;
}

bool AudioRTDelay::getFFParams(size_t n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::getFFParams: Error: given fx index is out of bounds.";
		return false;
	}

	if(p_params == NULL)
	{
		this->err_msg = "AudioRTDelay::getFFParams: Error: given params object pointer is null.";
		return false;
	}

	memcpy(p_params, &(this->p_ff_params[n_fx]), sizeof(audiortdelay_fx_params_t));
	return true;
}

bool AudioRTDelay::getFBParams(size_t n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::getFBParams: Error: given fx index is out of bounds.";
		return false;
	}

	if(p_params == NULL)
	{
		this->err_msg = "AudioRTDelay::getFBParams: Error: given params object pointer is null.";
		return false;
	}

	memcpy(p_params, &(this->p_fb_params[n_fx]), sizeof(audiortdelay_fx_params_t));
	return true;
}

bool AudioRTDelay::setDryInputAmplitude(float amp)
{
	if(this->status < 1) return false;

	this->dryinput_amp = amp;
	return true;
}

bool AudioRTDelay::setOutputAmplitude(float amp)
{
	if(this->status < 1) return false;

	this->output_amp = amp;
	return true;
}

bool AudioRTDelay::setFFDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::setFFDelay: Error: given fx index is out of bounds.";
		return false;
	}

	if(((size_t) delay) >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = "AudioRTDelay::setFFDelay: Error: given delay value is too big.";
		return false;
	}

	this->p_ff_params[n_fx].delay = delay;
	return true;
}

bool AudioRTDelay::setFFAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::setFFAmplitude: Error: given fx index is out of bounds.";
		return false;
	}

	this->p_ff_params[n_fx].amp = amp;
	return true;
}

bool AudioRTDelay::setFBDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::setFBDelay: Error: given fx index is out of bounds.";
		return false;
	}

	if(((size_t) delay) >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = "AudioRTDelay::setFBDelay: Error: given delay value is too big.";
		return false;
	}

	this->p_fb_params[n_fx].delay = delay;
	return true;
}

bool AudioRTDelay::setFBAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = "AudioRTDelay::setFBAmplitude: Error: given fx index is out of bounds.";
		return false;
	}

	this->p_fb_params[n_fx].amp = amp;
	return true;
}

bool AudioRTDelay::resetFFParams(void)
{
	if(this->status < 1) return false;

	if(this->P_FF_PARAMS_LENGTH) memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);

	return true;
}

bool AudioRTDelay::resetFBParams(void)
{
	if(this->status < 1) return false;

	if(this->P_FB_PARAMS_LENGTH) memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);

	return true;
}

std::string AudioRTDelay::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return "AudioRTDelay object not initialized\nExtended error message: " + this->err_msg;

	return this->err_msg;
}

bool AudioRTDelay::buffer_alloc(void)
{
	size_t n_seg = 0u;

	this->buffer_free(); /*Clear any previous allocations*/

	if(!this->buffer_fxparams_alloc()) return false;

	this->p_bufferinput = (float*) malloc(this->BUFFER_SIZE_BYTES);
	this->p_bufferoutput = (float*) malloc(this->BUFFER_SIZE_BYTES);

	this->pp_bufferinput_segments = (float**) malloc((this->BUFFER_N_SEGMENTS)*sizeof(float*));
	this->pp_bufferoutput_segments = (float**) malloc((this->BUFFER_N_SEGMENTS)*sizeof(float*));

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

	if(this->pp_bufferinput_segments == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->pp_bufferoutput_segments == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_bufferinput, 0, this->BUFFER_SIZE_BYTES);
	memset(this->p_bufferoutput, 0, this->BUFFER_SIZE_BYTES);

	for(n_seg = 0u; n_seg < this->BUFFER_N_SEGMENTS; n_seg++)
	{
		this->pp_bufferinput_segments[n_seg] = (float*) (((size_t) this->p_bufferinput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));
		this->pp_bufferoutput_segments[n_seg] = (float*) (((size_t) this->p_bufferoutput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));
	}

	return true;
}

void AudioRTDelay::buffer_free(void)
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

	if(this->pp_bufferinput_segments != NULL)
	{
		free(this->pp_bufferinput_segments);
		this->pp_bufferinput_segments = NULL;
	}

	if(this->pp_bufferoutput_segments != NULL)
	{
		free(this->pp_bufferoutput_segments);
		this->pp_bufferoutput_segments = NULL;
	}

	this->buffer_fxparams_free();
	return;
}

bool AudioRTDelay::buffer_fxparams_alloc(void)
{
	this->buffer_fxparams_free(); /*Clear any previous fxparams allocations*/

	if(this->P_FF_PARAMS_LENGTH)
	{
		this->p_ff_params = (audiortdelay_fx_params_t*) malloc(this->P_FF_PARAMS_SIZE);
		if(this->p_ff_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);
	}

	if(this->P_FB_PARAMS_LENGTH)
	{
		this->p_fb_params = (audiortdelay_fx_params_t*) malloc(this->P_FB_PARAMS_SIZE);
		if(this->p_fb_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);
	}

	return true;
}

void AudioRTDelay::buffer_fxparams_free(void)
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

bool AudioRTDelay::retrieve_prev_nframe(size_t curr_buf_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
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

bool AudioRTDelay::retrieve_prev_nframe(size_t curr_nseg, size_t curr_seg_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
{
	size_t curr_buf_nframe = 0u;

	if(curr_nseg >= this->BUFFER_N_SEGMENTS) return false;
	if(curr_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return false;

	curr_buf_nframe = curr_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + curr_seg_nframe;

	return this->retrieve_prev_nframe(curr_buf_nframe, n_delay, p_prev_buf_nframe, p_prev_nseg, p_prev_seg_nframe);
}

