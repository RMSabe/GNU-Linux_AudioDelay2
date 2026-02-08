/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.1)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include <stdlib.h>
#include <string.h>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

bool AudioPB::setParameters(const audiopb_params_t *p_params)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: AudioPB object is already initialized.");
		return false;
	}

	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: given p_params object pointer is NULL.");
		return false;
	}

	if(p_params->file_dir == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: p_params: given file_dir value is invalid.");
		return false;
	}

	if(p_params->audio_dev_desc == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: p_params: given audio_dev_desc value is invalid.");
		return false;
	}

	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIODEV_DESC = p_params->audio_dev_desc;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = p_params->n_channels;
	this->AUDIORTDELAY_BUFFER_SIZE_FRAMES = p_params->rtdelay_buffer_size_frames;
	this->AUDIORTDELAY_N_FF_DELAYS = p_params->rtdelay_n_ff_delays;
	this->AUDIORTDELAY_N_FB_DELAYS = p_params->rtdelay_n_fb_delays;

	return true;
}

bool AudioPB::initialize(void)
{
	audiortdelay_init_params_t delay_params;

	if(this->status > 0) return true;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = __TEXT("AudioPB::initialize: Error: failed to open input file.");
		return false;
	}

	if(!this->audio_hw_init())
	{
		this->filein_close();
		this->status = this->STATUS_ERROR_AUDIOHW;
		return false;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = __TEXT("AudioPB::initialize: Error: memory allocate failed.");
		return false;
	}

	delay_params.buffer_size_frames = this->AUDIORTDELAY_BUFFER_SIZE_FRAMES;
	delay_params.buffer_n_segments = this->BUFFER_N_SEGMENTS;
	delay_params.n_channels = this->N_CHANNELS;
	delay_params.n_ff_delays = this->AUDIORTDELAY_N_FF_DELAYS;
	delay_params.n_fb_delays = this->AUDIORTDELAY_N_FB_DELAYS;

	if(this->p_delay == NULL)
	{
		this->p_delay = new AudioRTDelay(&delay_params);
		if(this->p_delay == NULL)
		{
			this->filein_close();
			this->audio_hw_deinit();
			this->buffer_free();
			this->status = this->STATUS_ERROR_MEMALLOC;
			this->err_msg = __TEXT("AudioPB::initialize: Error: failed to allocate delay object.");
			return false;
		}
	}
	else this->p_delay->setInitParameters(&delay_params);

	if(!this->p_delay->initialize())
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->buffer_free();

		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	this->status = this->STATUS_READY;
	return true;
}

bool AudioPB::runPlayback(void)
{
	if(this->status != this->STATUS_READY)
	{
		this->err_msg = __TEXT("AudioPB::runPlayback: Error: AudioPB object is either not initialized or already playing.");
		return false;
	}

	this->playback_proc();

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return true;
}

void AudioPB::stopPlayback(void)
{
	this->stop_playback = true;
	return;
}

float AudioPB::rtdelayGetDryInputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getDryInputAmplitude();
}

float AudioPB::rtdelayGetOutputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getOutputAmplitude();
}

bool AudioPB::rtdelayGetFFParams(size_t n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(!this->p_delay->getFFParams(n_fx, p_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelayGetFBParams(size_t n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return false;

	if(!this->p_delay->getFBParams(n_fx, p_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetDryInputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setDryInputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetOutputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setOutputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetFFDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetFFAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetFBDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelaySetFBAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelayResetFFParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFFParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::rtdelayResetFBParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFBParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

__string AudioPB::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (__TEXT("Error: AudioPB object not initialized\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

int AudioPB::getStatus(void)
{
	return this->status;
}

bool AudioPB::filein_open(void)
{
	this->filein_close(); /*Close any existing open handles*/

	this->h_filein = open(this->FILEIN_DIR.c_str(), O_RDONLY);
	if(this->h_filein < 0) return false;

	this->filein_size = __LSEEK(this->h_filein, 0, SEEK_END);
	return true;
}

void AudioPB::filein_close(void)
{
	if(this->h_filein < 0) return;

	close(this->h_filein);
	this->h_filein = -1;
	this->filein_size = 0;

	return;
}

void AudioPB::audio_hw_deinit(void)
{
	if(this->p_audiodev == NULL) return;

	snd_pcm_drop(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);

	this->p_audiodev = NULL;
	return;
}

void AudioPB::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();

	snd_pcm_drain(this->p_audiodev);
	return;
}

void AudioPB::playback_init(void)
{
	this->n_segment = 0u;
	this->stop_playback = false;
	this->filein_pos = this->AUDIO_DATA_BEGIN;

	this->p_delay->setDryInputAmplitude(1.0f);
	this->p_delay->setOutputAmplitude(1.0f);
	this->p_delay->resetFFParams();
	this->p_delay->resetFBParams();

	this->buffer_remap();
	return;
}

void AudioPB::playback_loop(void)
{
	while(!this->stop_playback)
	{
		this->buffer_play();

		this->buffer_load_in();
		this->p_delay->runDSP(this->n_segment);
		this->buffer_load_out();
		this->buffer_segment_update();

		snd_pcm_wait(this->p_audiodev, -1);

		this->buffer_remap();
	}

	return;
}

void AudioPB::buffer_segment_update(void)
{
	this->n_segment++;
	this->n_segment %= this->BUFFER_N_SEGMENTS;

	return;
}

void AudioPB::buffer_remap(void)
{
	if(this->buffer_cycle)
	{
		this->p_loadoutput = this->p_bufferoutput1;
		this->p_playoutput = this->p_bufferoutput0;
	}
	else
	{
		this->p_loadoutput = this->p_bufferoutput0;
		this->p_playoutput = this->p_bufferoutput1;
	}

	this->buffer_cycle = !this->buffer_cycle;
	return;
}

void AudioPB::buffer_play(void)
{
	ssize_t n_ret = 0;

	n_ret = (ssize_t) snd_pcm_writei(this->p_audiodev, this->p_playoutput, (snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret < 0)
	{
		if(n_ret == -EPIPE)
		{
			n_ret = (ssize_t) snd_pcm_prepare(this->p_audiodev);
			if(n_ret < 0) app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_prepare failed."));
		}
		else app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_writei failed."));
	}

	return;
}

