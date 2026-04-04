/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.2 (Interop Java version 1.1)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include "cstrdef.h"

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

	this->FILEIN_DIR = p_params->file_dir;
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
		this->status = this->STATUS_ERROR_MEMORY;
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
			this->status = this->STATUS_ERROR_MEMORY;
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

		this->status = this->STATUS_ERROR_GENERIC;
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

bool AudioPB::loadAudioDeviceList(void)
{
	void **pp_hints = NULL;
	char *p_devname = NULL;
	char *p_devdesc = NULL;
	char *p_devioid = NULL;
	char *p_audiodevicelist_entry_name = NULL;
	char *p_audiodevicelist_entry_desc = NULL;

	size_t n_hint;
	size_t n_sndctl;
	size_t n_sndctl_count;
	size_t audiodevicelist_byteindex;

	int n_ret;
	int _i32_sndctlindex;

	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	this->audiodevicelist_free();
	this->audiodevicelist_n_entries = 0u;

	n_sndctl_count = 0u;
	_i32_sndctlindex = -1;

	while(true)
	{
		n_ret = snd_card_next(&_i32_sndctlindex);
		if(n_ret < 0)
		{
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_card_next failed.");
			this->status = this->STATUS_ERROR_AUDIOHW;
			return false;
		}

		if(_i32_sndctlindex < 0) break;

		n_sndctl_count++;
	}

	n_sndctl = 0u;
	while(n_sndctl < n_sndctl_count)
	{
		n_ret = snd_device_name_hint((int) n_sndctl, "pcm", &pp_hints);
		if((n_ret < 0) || (pp_hints == NULL))
		{
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
			this->status = this->STATUS_ERROR_AUDIOHW;
			return false;
		}

		n_hint = 0u;
		while(pp_hints[n_hint] != NULL)
		{
			p_devname = snd_device_name_get_hint((const char*) pp_hints[n_hint], "NAME");
			if(p_devname == NULL)
			{
				snd_device_name_free_hint(pp_hints);
				this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				this->status = this->STATUS_ERROR_AUDIOHW;
				return false;
			}

			/*Consider only hardware interface devices ("hw:..." "plughw:...")*/

			if(_cstr_char_compare_upto_char(p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(p_devname, "plughw", ':', true))
			{
				p_devioid = snd_device_name_get_hint((const char*) pp_hints[n_hint], "IOID");

				/*Count only playback devices*/
				if((p_devioid == NULL) || _cstr_char_compare(p_devioid, "Output")) this->audiodevicelist_n_entries++;

				if(p_devioid != NULL)
				{
					free(p_devioid);
					p_devioid = NULL;
				}
			}

			free(p_devname);
			p_devname = NULL;

			n_hint++;
		}

		snd_device_name_free_hint(pp_hints);
		pp_hints = NULL;

		n_sndctl++;
	}

	if(!this->audiodevicelist_n_entries)
	{
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: no playback devices found.");
		return false;
	}

	if(!this->audiodevicelist_alloc())
	{
		this->status = this->STATUS_ERROR_MEMORY;
		return false;
	}

	audiodevicelist_byteindex = 0u;
	n_sndctl = 0u;

	while(n_sndctl < n_sndctl_count)
	{
		n_ret = snd_device_name_hint((int) n_sndctl, "pcm", &pp_hints);
		if((n_ret < 0) || (pp_hints == NULL))
		{
			this->audiodevicelist_free();
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
			this->status = this->STATUS_ERROR_AUDIOHW;
			return false;
		}

		n_hint = 0u;
		while(pp_hints[n_hint] != NULL)
		{
			p_devname = snd_device_name_get_hint((const char*) pp_hints[n_hint], "NAME");
			if(p_devname == NULL)
			{
				snd_device_name_free_hint(pp_hints);
				this->audiodevicelist_free();
				this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				this->status = this->STATUS_ERROR_AUDIOHW;
				return false;
			}

			if(_cstr_char_compare_upto_char(p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(p_devname, "plughw", ':', true))
			{
				p_devioid = snd_device_name_get_hint((const char*) pp_hints[n_hint], "IOID");

				if((p_devioid == NULL) || _cstr_char_compare(p_devioid, "Output"))
				{
					p_devdesc = snd_device_name_get_hint((const char*) pp_hints[n_hint], "DESC");
					if(p_devdesc == NULL)
					{
						if(p_devioid != NULL) free(p_devioid);
						free(p_devname);
						snd_device_name_free_hint(pp_hints);
						this->audiodevicelist_free();
						this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
						this->status = this->STATUS_ERROR_AUDIOHW;
						return false;
					}

					p_audiodevicelist_entry_name = ((audiodevicelist_entry_t*) (((uintptr_t) this->p_audiodevicelist) + audiodevicelist_byteindex))->name;
					p_audiodevicelist_entry_desc = ((audiodevicelist_entry_t*) (((uintptr_t) this->p_audiodevicelist) + audiodevicelist_byteindex))->desc;

					_cstr_char_copy(p_devname, p_audiodevicelist_entry_name, __AUDIODEVICELIST_ENTRY_TEXTLENGTH);
					_cstr_char_copy(p_devdesc, p_audiodevicelist_entry_desc, __AUDIODEVICELIST_ENTRY_TEXTLENGTH);

					audiodevicelist_byteindex += sizeof(audiodevicelist_entry_t);

					free(p_devdesc);
					p_devdesc = NULL;
				}

				if(p_devioid != NULL)
				{
					free(p_devioid);
					p_devioid = NULL;
				}
			}

			free(p_devname);
			p_devname = NULL;

			n_hint++;
		}

		snd_device_name_free_hint(pp_hints);
		pp_hints = NULL;

		n_sndctl++;
	}

	this->status = this->STATUS_UNINITIALIZED;
	return true;
}

ssize_t AudioPB::getAudioDeviceListEntryCount(void)
{
	return (ssize_t) this->audiodevicelist_n_entries;
}

const audiodevicelist_entry_t* AudioPB::getAudioDeviceListEntry(size_t index)
{
	if(this->p_audiodevicelist == NULL)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: audio device list is not loaded.");
		return NULL;
	}

	if(index >= this->audiodevicelist_n_entries)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: given index is out of bounds.");
		return NULL;
	}

	return (const audiodevicelist_entry_t*) (((uintptr_t) this->p_audiodevicelist) + index*sizeof(audiodevicelist_entry_t));
}

bool AudioPB::chooseDevice(size_t index)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(this->p_audiodevicelist == NULL)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: audio device list is not loaded.");
		return false;
	}

	if(index >= this->audiodevicelist_n_entries)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: given index is out of bounds.");
		return false;
	}

	this->AUDIODEV_DESC = this->p_audiodevicelist[index].name;
	return true;
}

bool AudioPB::chooseDefaultDevice(void)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	this->AUDIODEV_DESC = "default";
	return true;
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

void AudioPB::deinitialize(void)
{
	this->stop_playback = true;

	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->audiodevicelist_free();

	if(this->p_delay != NULL)
	{
		delete this->p_delay;
		this->p_delay = NULL;
	}

	return;
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
	snd_pcm_hw_free(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);

	this->p_audiodev = NULL;
	return;
}

bool AudioPB::audiodevicelist_alloc(void)
{
	this->audiodevicelist_free();

	if(!this->audiodevicelist_n_entries)
	{
		this->err_msg = __TEXT("AudioPB::audiodevicelist_alloc: Error: list has no entries.");
		return false;
	}

	this->p_audiodevicelist = (audiodevicelist_entry_t*) malloc((this->audiodevicelist_n_entries)*(sizeof(audiodevicelist_entry_t)));
	if(this->p_audiodevicelist == NULL)
	{
		this->err_msg = __TEXT("AudioPB::audiodevicelist_alloc: Error: memory allocate failed.");
		return false;
	}

	memset(this->p_audiodevicelist, 0, ((this->audiodevicelist_n_entries)*(sizeof(audiodevicelist_entry_t))));
	return true;
}

void AudioPB::audiodevicelist_free(void)
{
	if(this->p_audiodevicelist != NULL)
	{
		free(this->p_audiodevicelist);
		this->p_audiodevicelist = NULL;
	}

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

