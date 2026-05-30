/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i24.hpp"
#include <string.h>
#include <math.h>

AudioPB_i24::AudioPB_i24(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 4u;
	this->FILE_BYTES_PER_SAMPLE = 3u;
	this->AUDIODEV_FORMAT = SND_PCM_FORMAT_S24_LE;
}

AudioPB_i24::~AudioPB_i24(void)
{
	this->deinitialize();
}

void AudioPB_i24::delaybuffer_loadin(void)
{
	size_t n_sample = 0u;
	size_t n_byte = 0u;

	float *p_loadseg_f32 = NULL;
	uint8_t *p_byteinput = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;
	int32_t i32 = 0;

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

	p_byteinput = (uint8_t*) this->p_inputbuffer;

	memset(p_byteinput, 0, this->INPUTBUFFER_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, p_byteinput, this->INPUTBUFFER_SIZE_BYTES);
	this->filein_pos += (__offset_t) this->INPUTBUFFER_SIZE_BYTES;

	factor = this->SAMPLE_FACTOR;

	n_byte = 0u;
	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		i32 = ((p_byteinput[n_byte + 2u] << 16) | (p_byteinput[n_byte + 1u] << 8) | p_byteinput[n_byte]);

		if(i32 & 0x00800000) i32 |= 0xff800000;
		else i32 &= 0x007fffff; /*Not really necessary, but just to be safe.*/

		f32 = (float) i32;
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;

		n_byte += this->FILE_BYTES_PER_SAMPLE;
	}

	return;
}

void AudioPB_i24::delaybuffer_loadout(void)
{
	size_t n_sample = 0u;
	size_t output_nseg = 0u;
	float *p_loadseg_f32 = NULL;
	int32_t *p_output = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = __TEXT("CRITICAL ERROR OCCURRED: ") + this->p_delay->getLastErrorMessage();
		app_exit(-1, this->err_msg.c_str());
	}

	output_nseg = (this->streambuffer_nseg_playout + 1u)%(this->STREAMBUFFER_N_SEGMENTS);
	p_output = (int32_t*) (((uintptr_t) (this->p_streambuffer)) + output_nseg*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (int32_t) roundf(f32);
	}

	return;
}

