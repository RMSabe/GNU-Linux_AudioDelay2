/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioStream_i16.hpp"
#include <math.h>

AudioStream_i16::AudioStream_i16(const audiostream_params_t *p_params) : AudioStream(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 2u;
	this->AUDIODEV_FORMAT = SND_PCM_FORMAT_S16_LE;
}

AudioStream_i16::~AudioStream_i16(void)
{
	this->deinitialize();
}

void AudioStream_i16::delaybuffer_loadin(void)
{
	size_t n_sample = 0u;
	size_t n_inputseg = 0u;
	float *p_loadseg_f32 = NULL;
	int16_t *p_input = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = __TEXT("CRITICAL ERROR OCCURRED: ") + this->p_delay->getLastErrorMessage();
		app_exit(-1, this->err_msg.c_str());
	}

	n_inputseg = (this->streambuffer_in_nseg - 1u)%(this->STREAMBUFFER_N_SEGMENTS); /*Safe to use as long as STREAMBUFFER_N_SEGMENTS is a power of 2*/

	/*if(!this->streambuffer_in_nseg) n_inputseg = this->STREAMBUFFER_N_SEGMENTS - 1u;
	else n_inputseg = this->streambuffer_in_nseg - 1u;*/

	p_input = (int16_t*) (((uintptr_t) (this->p_streambuffer_in)) + n_inputseg*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	factor = this->SAMPLE_FACTOR;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = (float) p_input[n_sample];
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;
	}

	return;
}

void AudioStream_i16::delaybuffer_loadout(void)
{
	size_t n_sample = 0u;
	size_t n_outputseg = 0u;
	float *p_loadseg_f32 = NULL;
	int16_t *p_output = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = __TEXT("CRITICAL ERROR OCCURRED: ") + this->p_delay->getLastErrorMessage();
		app_exit(-1, this->err_msg.c_str());
	}

	n_outputseg = (this->streambuffer_out_nseg + 1u)%(this->STREAMBUFFER_N_SEGMENTS);

	p_output = (int16_t*) (((uintptr_t) (this->p_streambuffer_out)) + n_outputseg*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (int16_t) roundf(f32);
	}

	return;
}

