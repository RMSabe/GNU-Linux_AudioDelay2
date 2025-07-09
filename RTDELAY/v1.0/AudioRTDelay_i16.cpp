/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 1.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioRTDelay_i16.hpp"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

AudioRTDelay_i16::AudioRTDelay_i16(const audiortdelay_init_params_t *p_params) : AudioRTDelay(p_params)
{
}

AudioRTDelay_i16::~AudioRTDelay_i16(void)
{
	this->stop_all_threads();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

bool AudioRTDelay_i16::audio_hw_init(void)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames = 0u;
	int n_ret = -1;
	uint32_t rate = 0u;

	this->audio_hw_deinit(); /*Deinit any previous instance of audio hw.*/

	n_ret = snd_pcm_open(&(this->p_audiodev), this->AUDIO_DEV_DESC.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if(n_ret < 0)
	{
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to open audio device.";
		return false;
	}

	snd_pcm_hw_params_malloc(&p_hwparams);
	snd_pcm_hw_params_any(this->p_audiodev, p_hwparams);

	n_ret = snd_pcm_hw_params_set_access(this->p_audiodev, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to set hardware access.";

		return false;
	}

	n_ret = snd_pcm_hw_params_set_format(this->p_audiodev, p_hwparams, SND_PCM_FORMAT_S16_LE);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to set format.";

		return false;
	}

	n_ret = snd_pcm_hw_params_set_channels(this->p_audiodev, p_hwparams, (uint16_t) this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to set number of channels.";

		return false;
	}

	rate = this->SAMPLE_RATE;
	n_ret = snd_pcm_hw_params_set_rate_near(this->p_audiodev, p_hwparams, &rate, 0);
	if((n_ret < 0) || (rate < this->SAMPLE_RATE))
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to set sample rate.";

		return false;
	}

	n_frames = (snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES_DEFAULT;
	n_ret = snd_pcm_hw_params_set_period_size_near(this->p_audiodev, p_hwparams, &n_frames, NULL);

	if((n_ret < 0) || (n_frames != ((snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES_DEFAULT)))
		std::cout << "AudioRTDelay_i16::audio_hw_init: Warning: could not set audio buffer size to default value.\n";

	n_ret = snd_pcm_hw_params(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay_i16::audio_hw_init: Error: failed to validate device settings.";

		return false;
	}

	snd_pcm_hw_params_get_period_size(p_hwparams, &n_frames, 0);

	this->BUFFER_SEGMENT_SIZE_FRAMES = (size_t) n_frames;
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = (this->BUFFER_SEGMENT_SIZE_SAMPLES)*2u;

	this->BUFFER_N_SEGMENTS = this->BUFFER_SIZE_FRAMES/this->BUFFER_SEGMENT_SIZE_FRAMES;

	this->BUFFER_SIZE_SAMPLES = (this->BUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SIZE_BYTES = (this->BUFFER_SIZE_SAMPLES)*2u;

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

bool AudioRTDelay_i16::buffer_alloc(void)
{
	size_t n_seg = 0u;

	this->buffer_free(); /*Clear any previous allocations*/

	if(!this->buffer_fxparams_alloc()) return false;

	this->p_bufferinput = malloc(this->BUFFER_SIZE_BYTES);
	this->p_bufferoutput = malloc(this->BUFFER_SIZE_BYTES);

	this->pp_bufferinput_segments = (void**) malloc((this->BUFFER_N_SEGMENTS)*sizeof(void*));
	this->pp_bufferoutput_segments = (void**) malloc((this->BUFFER_N_SEGMENTS)*sizeof(void*));

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
		pp_bufferinput_segments[n_seg] = &((uint8_t*) this->p_bufferinput)[n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES)];
		pp_bufferoutput_segments[n_seg] = &((uint8_t*) this->p_bufferoutput)[n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES)];
	}

	return true;
}

void AudioRTDelay_i16::buffer_free(void)
{
	this->buffer_fxparams_free();

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

	return;
}

void AudioRTDelay_i16::buffer_load(void)
{
	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->stop = true;
		return;
	}

	memset(this->pp_bufferinput_segments[this->n_loadsegment], 0, this->BUFFER_SEGMENT_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, this->pp_bufferinput_segments[this->n_loadsegment], this->BUFFER_SEGMENT_SIZE_BYTES);
	this->filein_pos += (__offset) this->BUFFER_SEGMENT_SIZE_BYTES;

	return;
}

void AudioRTDelay_i16::run_dsp(void)
{
	size_t n_prevsample = 0u;
	size_t n_currsample = 0u;
	size_t n_channel = 0u;

	size_t n_frame = 0u;
	size_t n_fx = 0u;
	size_t buf_prev_nframe = 0u;

	size_t n_delay = 0u;
	float f_amp = 0.0f;

	float p_samples[this->N_CHANNELS];

	for(n_frame = 0u; n_frame < this->BUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		n_currsample = n_frame*(this->N_CHANNELS);

		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_samples[n_channel] = (float) ((int16_t**) this->pp_bufferinput_segments)[this->n_loadsegment][n_currsample];
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

			this->retrieve_prev_nframe(this->n_loadsegment, n_frame, n_delay, &buf_prev_nframe, NULL, NULL);

			n_prevsample = buf_prev_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_samples[n_channel] += f_amp*((float) ((int16_t*) this->p_bufferinput)[n_prevsample]);
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

			this->retrieve_prev_nframe(this->n_loadsegment, n_frame, n_delay, &buf_prev_nframe, NULL, NULL);

			n_prevsample = buf_prev_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_samples[n_channel] += f_amp*((float) ((int16_t*) this->p_bufferoutput)[n_prevsample]);
				n_prevsample++;
			}

			n_fx++;
		}

		n_currsample = n_frame*(this->N_CHANNELS);
		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			if(p_samples[n_channel] >= this->SAMPLE_MAX_VALUE_F) ((int16_t**) this->pp_bufferoutput_segments)[this->n_loadsegment][n_currsample] = this->SAMPLE_MAX_VALUE;
			else if(p_samples[n_channel] <= this->SAMPLE_MIN_VALUE_F) ((int16_t**) this->pp_bufferoutput_segments)[this->n_loadsegment][n_currsample] = this->SAMPLE_MIN_VALUE;
			else ((int16_t**) this->pp_bufferoutput_segments)[this->n_loadsegment][n_currsample] = (int16_t) roundf(p_samples[n_channel]);

			n_currsample++;
		}
	}

	return;
}

