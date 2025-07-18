/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 2.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i24.hpp"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

AudioPB_i24::AudioPB_i24(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i24::~AudioPB_i24(void)
{
	this->stop_all_threads();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	if(this->p_delay != NULL)
	{
		delete this->p_delay;
		this->p_delay = NULL;
	}
}

bool AudioPB_i24::audio_hw_init(void)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames = 0u;
	int n_ret = -1;
	uint32_t rate = 0u;

	this->audio_hw_deinit(); /*Deinit any previous instance of audio hw.*/

	n_ret = snd_pcm_open(&(this->p_audiodev), this->AUDIO_DEV_DESC.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if(n_ret < 0)
	{
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to open audio device.";
		return false;
	}

	snd_pcm_hw_params_malloc(&p_hwparams);
	snd_pcm_hw_params_any(this->p_audiodev, p_hwparams);

	n_ret = snd_pcm_hw_params_set_access(this->p_audiodev, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to set hardware access.";

		return false;
	}

	n_ret = snd_pcm_hw_params_set_format(this->p_audiodev, p_hwparams, SND_PCM_FORMAT_S24_LE);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to set format.";

		return false;
	}

	n_ret = snd_pcm_hw_params_set_channels(this->p_audiodev, p_hwparams, this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to set number of channels.";

		return false;
	}

	rate = this->SAMPLE_RATE;
	n_ret = snd_pcm_hw_params_set_rate_near(this->p_audiodev, p_hwparams, &rate, 0);
	if((n_ret < 0) || (rate < this->SAMPLE_RATE))
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to set sample rate.";

		return false;
	}

	n_frames = (snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES_DEFAULT;
	n_ret = snd_pcm_hw_params_set_period_size_near(this->p_audiodev, p_hwparams, &n_frames, NULL);

	if((n_ret < 0) || (n_frames != ((snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES_DEFAULT)))
		std::cout << "AudioPB_i24::audio_hw_init: Warning: could not set audio buffer size to default value.\n";

	n_ret = snd_pcm_hw_params(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: failed to validate device settings.";

		return false;
	}

	snd_pcm_hw_params_get_period_size(p_hwparams, &n_frames, 0);

	this->AUDIOBUFFER_SIZE_FRAMES = (size_t) n_frames;
	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*((size_t) this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = (this->AUDIOBUFFER_SIZE_SAMPLES)*4u;

	this->BUFFER_N_SEGMENTS = this->AUDIORTDELAY_BUFFER_SIZE_FRAMES/this->AUDIOBUFFER_SIZE_FRAMES;

	this->BYTEBUF_SIZE = (this->AUDIOBUFFER_SIZE_SAMPLES)*3u;

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

bool AudioPB_i24::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	this->p_bufferinput = malloc(this->BYTEBUF_SIZE);
	this->p_bufferoutput0 = malloc(this->AUDIOBUFFER_SIZE_BYTES);
	this->p_bufferoutput1 = malloc(this->AUDIOBUFFER_SIZE_BYTES);

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_bufferoutput0 == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_bufferoutput1 == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_bufferinput, 0, this->BYTEBUF_SIZE);
	memset(this->p_bufferoutput0, 0, this->AUDIOBUFFER_SIZE_BYTES);
	memset(this->p_bufferoutput1, 0, this->AUDIOBUFFER_SIZE_BYTES);

	return true;
}

void AudioPB_i24::buffer_free(void)
{
	if(this->p_bufferinput != NULL)
	{
		free(this->p_bufferinput);
		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput0 != NULL)
	{
		free(this->p_bufferoutput0);
		this->p_bufferoutput0 = NULL;
	}

	if(this->p_bufferoutput1 != NULL)
	{
		free(this->p_bufferoutput1);
		this->p_bufferoutput1 = NULL;
	}

	return;
}

void AudioPB_i24::buffer_load_in(void)
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
		this->stop = true;
		return;
	}

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->n_segment);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = "CRITICAL ERROR OCCURRED: " + this->p_delay->getLastErrorMessage();
		app_exit(1, this->err_msg.c_str());
	}

	p_byteinput = (uint8_t*) this->p_bufferinput;

	memset(p_byteinput, 0, this->BYTEBUF_SIZE);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, p_byteinput, this->BYTEBUF_SIZE);
	this->filein_pos += (__offset) this->BYTEBUF_SIZE;

	factor = this->SAMPLE_FACTOR;

	n_byte = 0u;
	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SIZE_SAMPLES; n_sample++)
	{
		i32 = ((p_byteinput[n_byte + 2u] << 16) | (p_byteinput[n_byte + 1u] << 8) | p_byteinput[n_byte]);

		if(i32 & 0x00800000) i32 |= 0xff800000;
		else i32 &= 0x007fffff; /*Not really necessary, but just to be safe.*/

		f32 = (float) i32;
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;

		n_byte += 3u;
	}

	return;
}

void AudioPB_i24::buffer_load_out(void)
{
	size_t n_sample = 0u;
	float *p_loadseg_f32 = NULL;
	int32_t *p_output = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->n_segment);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = "CRITICAL ERROR OCCURRED: " + this->p_delay->getLastErrorMessage();
		app_exit(1, this->err_msg.c_str());
	}

	p_output = (int32_t*) this->p_loadoutput;

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (int32_t) roundf(f32);
	}

	return;
}

