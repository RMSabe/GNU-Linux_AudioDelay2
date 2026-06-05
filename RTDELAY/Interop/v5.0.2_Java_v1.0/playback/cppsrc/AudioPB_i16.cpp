/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i16.hpp"
#include <string.h>
#include <math.h>

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 2u;
	this->FILE_BYTES_PER_SAMPLE = 2u;
	this->AUDIODEV_FORMAT = SND_PCM_FORMAT_S16_LE;
}

AudioPB_i16::~AudioPB_i16(void)
{
	this->deinitialize();
}

void AudioPB_i16::delaybuffer_loadin(void)
{
	size_t n_sample = 0u;
	float *p_loadseg_f32 = NULL;
	int16_t *p_input = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->status = this->STATUS_STOPPED;
		return;
	}

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = __TEXT("CRITICAL ERROR OCCURRED: ") + this->p_delay->getLastErrorMessage();
		app_exit(-1, this->err_msg.c_str());
	}

	p_input = (int16_t*) this->p_inputbuffer;

	memset(p_input, 0, this->INPUTBUFFER_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, p_input, this->INPUTBUFFER_SIZE_BYTES);
	this->filein_pos += (__offset_t) this->INPUTBUFFER_SIZE_BYTES;

	factor = this->SAMPLE_FACTOR;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = (float) p_input[n_sample];
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;
	}

	return;
}

void AudioPB_i16::delaybuffer_loadout(void)
{
	size_t n_sample = 0u;
	size_t output_nseg = 0u;
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

	output_nseg = (this->streambuffer_nseg_playout + 1u)%(this->STREAMBUFFER_N_SEGMENTS);
	p_output = (int16_t*) (((uintptr_t) (this->p_streambuffer)) + output_nseg*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

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

