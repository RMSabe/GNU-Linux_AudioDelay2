/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i16.hpp"
#include <stdlib.h>
#include <string.h>
#include <math.h>

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i16::~AudioPB_i16(void)
{
	this->stop_playback = true;

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

bool AudioPB_i16::audio_hw_init(void)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames = 0u;
	int n_ret = -1;

	this->audio_hw_deinit(); /*clear any previous instance of audio hw.*/

	/*OPEN AUDIO DEVICE*/

	n_ret = snd_pcm_open(&(this->p_audiodev), this->AUDIODEV_DESC.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(n_ret < 0)
	{
		this->p_audiodev = NULL;
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_open failed.";
		return false;
	}

	/*ALLOCATE DEVICE PARAMS OBJ*/

	n_ret = snd_pcm_hw_params_malloc(&p_hwparams);
	if((n_ret < 0) || (p_hwparams == NULL))
	{
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_malloc failed.";
		return false;
	}

	/*PRELOAD DEVICE PARAMETERS*/

	n_ret = snd_pcm_hw_params_any(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_any failed.";
		return false;
	}

	/*ENABLE/DISABLE DEVICE RESAMPLING*/
	/*
	 * 0 = Disable (better performance)
	 * 1 = Enable (better compatibility)
	 */

	n_ret = snd_pcm_hw_params_set_rate_resample(this->p_audiodev, p_hwparams, 1u);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_rate_resample failed.";
		return false;
	}

	/*SET DEVICE ACCESS*/

	n_ret = snd_pcm_hw_params_set_access(this->p_audiodev, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_access failed.";
		return false;
	}

	/*SET DEVICE FORMAT*/

	n_ret = snd_pcm_hw_params_set_format(this->p_audiodev, p_hwparams, SND_PCM_FORMAT_S16_LE);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_format failed.";
		return false;
	}

	/*SET DEVICE CHANNELS*/

	n_ret = snd_pcm_hw_params_set_channels(this->p_audiodev, p_hwparams, (unsigned int) this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_channels failed.";
		return false;
	}

	/*SET DEVICE SAMPLING RATE*/

	n_ret = snd_pcm_hw_params_set_rate(this->p_audiodev, p_hwparams, (unsigned int) this->SAMPLE_RATE, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_rate failed.";
		return false;
	}

	/*GET/SET DEVICE BUFFER SIZE*/

	n_ret = snd_pcm_hw_params_get_buffer_size(p_hwparams, &n_frames);
	if(n_ret < 0)
	{
		n_frames = (snd_pcm_uframes_t) _get_closest_power2_ceil(this->SAMPLE_RATE);
		n_ret = snd_pcm_hw_params_set_buffer_size_near(this->p_audiodev, p_hwparams, &n_frames);

		if(n_ret < 0)
		{
			snd_pcm_hw_params_free(p_hwparams);
			this->audio_hw_deinit();
			this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_buffer_size_near failed.";
			return false;
		}
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (size_t) n_frames;
	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	/*SET DEVICE BUFFER SEGMENT SIZE (PERIOD SIZE)*/

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/4u);

	n_ret = snd_pcm_hw_params_set_period_size(this->p_audiodev, p_hwparams, (snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params_set_period_size failed.";
		return false;
	}

	/*APPLY SETTINGS TO DEVICE*/

	n_ret = snd_pcm_hw_params(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i16::audio_hw_init: Error: snd_pcm_hw_params failed.";
		return false;
	}

	n_ret = snd_pcm_hw_params_get_period_size(p_hwparams, &n_frames, NULL);
	if(n_ret >= 0) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = (size_t) n_frames; /*Not really necessary, but just to be safe.*/

	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = (this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BUFFER_N_SEGMENTS = (this->AUDIORTDELAY_BUFFER_SIZE_FRAMES)/(this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

bool AudioPB_i16::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	this->p_bufferinput = malloc(this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->p_bufferoutput0 = malloc(this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->p_bufferoutput1 = malloc(this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

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

	memset(this->p_bufferinput, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	memset(this->p_bufferoutput0, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	memset(this->p_bufferoutput1, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	return true;
}

void AudioPB_i16::buffer_free(void)
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

	this->p_loadoutput = NULL;
	this->p_playoutput = NULL;
	return;
}

void AudioPB_i16::buffer_load_in(void)
{
	size_t n_sample = 0u;
	float *p_loadseg_f32 = NULL;
	int16_t *p_input = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->stop_playback = true;
		return;
	}

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->n_segment);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = "CRITICAL ERROR OCCURRED: " + this->p_delay->getLastErrorMessage();
		app_exit(1, this->err_msg.c_str());
	}

	p_input = (int16_t*) this->p_bufferinput;

	memset(p_input, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, p_input, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->filein_pos += (__offset) this->AUDIOBUFFER_SEGMENT_SIZE_BYTES;

	factor = this->SAMPLE_FACTOR;

	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = (float) p_input[n_sample];
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;
	}

	return;
}

void AudioPB_i16::buffer_load_out(void)
{
	size_t n_sample = 0u;
	float *p_loadseg_f32 = NULL;
	int16_t *p_output = NULL;

	float factor = 0.0f;
	float f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->n_segment);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = "CRITICAL ERROR OCCURRED: " + this->p_delay->getLastErrorMessage();
		app_exit(1, this->err_msg.c_str());
	}

	p_output = (int16_t*) this->p_loadoutput;

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (int16_t) roundf(f32);
	}

	return;
}

