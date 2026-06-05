/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include "cstrdef.h"
#include "delay.h"

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

	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->FILEIN_DIR = p_params->file_dir;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = p_params->n_channels;
	this->AUDIOBUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->audiobuffer_size_frames);
	this->STREAMBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(p_params->streambuffer_segment_size_frames);
	this->STREAMBUFFER_N_SEGMENTS = _get_closest_power2_ceil(p_params->streambuffer_n_segments);
	this->AUDIODELAY_BUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->delay_buffer_size_frames);
	this->AUDIODELAY_N_FF_DELAYS = p_params->delay_n_ff_delays;
	this->AUDIODELAY_N_FB_DELAYS = p_params->delay_n_fb_delays;

	return true;
}

bool AudioPB::initialize(void)
{
	audiodelay_init_params_t delay_params;

	if(this->status > 0) return true;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->SAMPLE_RATE)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid sample rate.");
		return false;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid number of channels.");
		return false;
	}

	if(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid stream buffer segment size.");
		return false;
	}

	if(this->STREAMBUFFER_N_SEGMENTS < this->STREAMBUFFER_N_SEGMENTS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid stream buffer segment count.");
		return false;
	}

	if(this->AUDIOBUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid audio buffer size. (audio buffer size smaller than stream buffer segment size).");
		return false;
	}

	if(this->AUDIODELAY_BUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid delay buffer size. (delay buffer size smaller than stream buffer segment size).");
		return false;
	}

	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = (this->AUDIOBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SEGMENT_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SIZE_FRAMES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->STREAMBUFFER_N_SEGMENTS);
	this->STREAMBUFFER_SIZE_SAMPLES = (this->STREAMBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->INPUTBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->FILE_BYTES_PER_SAMPLE);

	this->AUDIODELAY_BUFFER_N_SEGMENTS = (this->AUDIODELAY_BUFFER_SIZE_FRAMES)/(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);

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

	delay_params.buffer_size_frames = this->AUDIODELAY_BUFFER_SIZE_FRAMES;
	delay_params.buffer_n_segments = this->AUDIODELAY_BUFFER_N_SEGMENTS;
	delay_params.n_channels = this->N_CHANNELS;
	delay_params.n_ff_delays = this->AUDIODELAY_N_FF_DELAYS;
	delay_params.n_fb_delays = this->AUDIODELAY_N_FB_DELAYS;

	if(this->p_delay == NULL)
	{
		this->p_delay = new AudioDelay(&delay_params);
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

void AudioPB::pausePlayback(void)
{
	if(this->status == this->STATUS_RUNNING)
	{
		this->status = this->STATUS_PAUSED;
		snd_pcm_drain(this->audiodev.p_device);
	}

	return;
}

void AudioPB::resumePlayback(void)
{
	if(this->status == this->STATUS_PAUSED)
	{
		snd_pcm_prepare(this->audiodev.p_device);
		this->status = this->STATUS_RUNNING;
	}

	return;
}

void AudioPB::stopPlayback(void)
{
	if(this->status > this->STATUS_READY) this->status = this->STATUS_STOPPED;

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

	if(this->audiodevlist.p_entries != NULL)
	{
		free(this->audiodevlist.p_entries);
		this->audiodevlist.p_entries = NULL;
	}

	this->audiodevlist.n_entries = 0u;

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
				if((p_devioid == NULL) || _cstr_char_compare(p_devioid, "Output")) this->audiodevlist.n_entries++;

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

	if(!this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: no playback devices found.");
		return false;
	}

	this->audiodevlist.p_entries = (audiodevicelist_entry_t*) malloc((this->audiodevlist.n_entries)*sizeof(audiodevicelist_entry_t));
	if(this->audiodevlist.p_entries == NULL)
	{
		this->status = this->STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: failed to allocate heap memory.");
		return false;
	}

	audiodevicelist_byteindex = 0u;
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
						this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
						this->status = this->STATUS_ERROR_AUDIOHW;
						return false;
					}

					p_audiodevicelist_entry_name = ((audiodevicelist_entry_t*) (((uintptr_t) this->audiodevlist.p_entries) + audiodevicelist_byteindex))->name;
					p_audiodevicelist_entry_desc = ((audiodevicelist_entry_t*) (((uintptr_t) this->audiodevlist.p_entries) + audiodevicelist_byteindex))->desc;

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
	return (ssize_t) this->audiodevlist.n_entries;
}

const audiodevicelist_entry_t* AudioPB::getAudioDeviceListEntry(size_t index)
{
	if(this->audiodevlist.p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: audio device list is not loaded.");
		return NULL;
	}

	if(index >= this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: given index is out of bounds.");
		return NULL;
	}

	return (const audiodevicelist_entry_t*) (((uintptr_t) this->audiodevlist.p_entries) + index*sizeof(audiodevicelist_entry_t));
}

bool AudioPB::chooseDevice(const char *name)
{
	return this->chooseDevice(name, true);
}

bool AudioPB::chooseDevice(size_t index)
{
	return this->chooseDevice(index, true);
}

bool AudioPB::chooseDefaultDevice(void)
{
	return this->chooseDefaultDevice(true);
}

bool AudioPB::chooseDevice(const char *name, bool enable_resampling)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(name == NULL)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: given name parameter is NULL.");
		return false;
	}

	this->audiodev.device_name = name;
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

bool AudioPB::chooseDevice(size_t index, bool enable_resampling)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(this->audiodevlist.p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: audio device list is not loaded.");
		return false;
	}

	if(index >= this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: given index is out of bounds.");
		return false;
	}

	this->audiodev.device_name = this->audiodevlist.p_entries[index].name;
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

bool AudioPB::chooseDefaultDevice(bool enable_resampling)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	this->audiodev.device_name = "default";
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

__offset_t AudioPB::getAudioDataSizeFrames(void)
{
	__offset_t _file_bytes_per_frame;

	if(this->status < 1) return -1;

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	return (this->AUDIO_DATA_END - this->AUDIO_DATA_BEGIN)/_file_bytes_per_frame;
}

__offset_t AudioPB::getAudioDataPositionFrames(void)
{
	__offset_t _file_bytes_per_frame;

	if(this->status < 1) return -1;

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	return (this->filein_pos - this->AUDIO_DATA_BEGIN)/_file_bytes_per_frame;
}

bool AudioPB::setAudioDataPositionFrames(__offset_t position)
{
	__offset_t _file_bytes_per_frame;
	__offset_t _file_data_size_frames;

	if(this->status < 1) return false;

	_file_data_size_frames = this->getAudioDataSizeFrames();
	if(_file_data_size_frames < 0) return false;

	if((position < 0) || (position >= _file_data_size_frames))
	{
		this->err_msg = __TEXT("AudioPB::setAudioDataPositionFrames: Error: invalid position value.");
		return false;
	}

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	this->filein_pos = this->AUDIO_DATA_BEGIN + (position*_file_bytes_per_frame);
	return true;
}

float AudioPB::delayGetDryInputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getDryInputAmplitude();
}

float AudioPB::delayGetOutputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getOutputAmplitude();
}

bool AudioPB::delaySetDryInputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setDryInputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::delaySetOutputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setOutputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int32_t AudioPB::delayGetFFDelay(size_t n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return -1;

	if(!this->p_delay->getFFParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return -1;
	}

	return (int32_t) _params.delay;
}

float AudioPB::delayGetFFAmplitude(size_t n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return 0.0f;

	if(!this->p_delay->getFFParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return 0.0f;
	}

	return _params.amp;
}

bool AudioPB::delaySetFFDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::delaySetFFAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int32_t AudioPB::delayGetFBDelay(size_t n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return -1;

	if(!this->p_delay->getFBParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return -1;
	}

	return (int32_t) _params.delay;
}

float AudioPB::delayGetFBAmplitude(size_t n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return 0.0f;

	if(!this->p_delay->getFBParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return 0.0f;
	}

	return _params.amp;
}

bool AudioPB::delaySetFBDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::delaySetFBAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::delayResetFFParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFFParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioPB::delayResetFBParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFBParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int AudioPB::getStatus(void)
{
	return this->status;
}

__string AudioPB::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (__TEXT("Error: AudioPB object not initialized\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

void AudioPB::deinitialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	if(this->audiodevlist.p_entries != NULL)
	{
		free(this->audiodevlist.p_entries);
		this->audiodevlist.p_entries = NULL;
	}

	this->audiodevlist.n_entries = 0u;

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

bool AudioPB::audio_hw_init(void)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames = 0u;
	int n_ret = -1;

	this->audio_hw_deinit(); /*clear any previous instance of audio hw.*/

	/*OPEN AUDIO DEVICE*/

	n_ret = snd_pcm_open(&(this->audiodev.p_device), this->audiodev.device_name.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(n_ret < 0)
	{
		this->audiodev.p_device = NULL;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_open failed.");
		return false;
	}

	/*ALLOCATE DEVICE PARAMS OBJ*/

	n_ret = snd_pcm_hw_params_malloc(&p_hwparams);
	if((n_ret < 0) || (p_hwparams == NULL))
	{
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_malloc failed.");
		return false;
	}

	/*PRELOAD DEVICE PARAMETERS*/

	n_ret = snd_pcm_hw_params_any(this->audiodev.p_device, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_any failed.");
		return false;
	}

	/*ENABLE/DISABLE DEVICE RESAMPLING*/
	/*
	 * 0 = Disable (better performance)
	 * 1 = Enable (better compatibility)
	 */

	n_ret = snd_pcm_hw_params_set_rate_resample(this->audiodev.p_device, p_hwparams, this->audiodev.enable_resampling);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_rate_resample failed.");
		return false;
	}

	/*SET DEVICE ACCESS*/

	n_ret = snd_pcm_hw_params_set_access(this->audiodev.p_device, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_access failed.");
		return false;
	}

	/*SET DEVICE FORMAT*/

	n_ret = snd_pcm_hw_params_set_format(this->audiodev.p_device, p_hwparams, (snd_pcm_format_t) this->AUDIODEV_FORMAT);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_format failed.");
		return false;
	}

	/*SET DEVICE CHANNELS*/

	n_ret = snd_pcm_hw_params_set_channels(this->audiodev.p_device, p_hwparams, (unsigned int) this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_channels failed.");
		return false;
	}

	/*SET DEVICE SAMPLING RATE*/

	n_ret = snd_pcm_hw_params_set_rate(this->audiodev.p_device, p_hwparams, (unsigned int) this->SAMPLE_RATE, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_rate failed.");
		return false;
	}

	/*SET DEVICE BUFFER SIZE*/

	n_frames = (snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES;
	n_ret = snd_pcm_hw_params_set_buffer_size_near(this->audiodev.p_device, p_hwparams, &n_frames);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_buffer_size_near failed.");
		return false;
	}

	/*SET DEVICE BUFFER SEGMENT SIZE (PERIOD SIZE)*/

	n_ret = snd_pcm_hw_params_set_period_size(this->audiodev.p_device, p_hwparams, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_period_size failed.");
		return false;
	}

	/*APPLY SETTINGS TO DEVICE*/

	n_ret = snd_pcm_hw_params(this->audiodev.p_device, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params failed.");
		return false;
	}

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

void AudioPB::audio_hw_deinit(void)
{
	if(this->audiodev.p_device == NULL) return;

	snd_pcm_drop(this->audiodev.p_device);
	snd_pcm_hw_free(this->audiodev.p_device);
	snd_pcm_close(this->audiodev.p_device);

	this->audiodev.p_device = NULL;
	return;
}

bool AudioPB::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	this->p_inputbuffer = malloc(this->INPUTBUFFER_SIZE_BYTES);
	this->p_streambuffer = malloc(this->STREAMBUFFER_SIZE_BYTES);

	if(this->p_inputbuffer == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_streambuffer == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_inputbuffer, 0, this->INPUTBUFFER_SIZE_BYTES);
	memset(this->p_streambuffer, 0, this->STREAMBUFFER_SIZE_BYTES);

	return true;
}

void AudioPB::buffer_free(void)
{
	if(this->p_inputbuffer != NULL)
	{
		free(this->p_inputbuffer);
		this->p_inputbuffer = NULL;
	}

	if(this->p_streambuffer != NULL)
	{
		free(this->p_streambuffer);
		this->p_streambuffer = NULL;
	}

	return;
}

void AudioPB::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();

	snd_pcm_drain(this->audiodev.p_device);
	return;
}

void AudioPB::playback_init(void)
{
	this->streambuffer_nseg_playout = 0u;
	this->delaybuffer_nseg = 0u;

	this->filein_pos = this->AUDIO_DATA_BEGIN;

	this->p_delay->setDryInputAmplitude(1.0f);
	this->p_delay->setOutputAmplitude(1.0f);
	this->p_delay->resetFFParams();
	this->p_delay->resetFBParams();

	this->status = this->STATUS_RUNNING;
	return;
}

void AudioPB::playback_loop(void)
{
	while(true)
	{
		if((this->status == this->STATUS_STOPPED) || (this->status < 1)) break;

		if(this->status == this->STATUS_PAUSED)
		{
			delay_ms(1);
			continue;
		}

		this->buffer_play();

		this->delaybuffer_loadin();
		this->p_delay->runDSP(this->delaybuffer_nseg);
		this->delaybuffer_loadout();
		this->streambuffer_nseg_update();
		this->delaybuffer_nseg_update();

		snd_pcm_wait(this->audiodev.p_device, -1);
	}

	return;
}

void AudioPB::streambuffer_nseg_update(void)
{
	this->streambuffer_nseg_playout++;
	this->streambuffer_nseg_playout %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

void AudioPB::delaybuffer_nseg_update(void)
{
	this->delaybuffer_nseg++;
	this->delaybuffer_nseg %= this->AUDIODELAY_BUFFER_N_SEGMENTS;

	return;
}

void AudioPB::buffer_play(void)
{
	void *p_playout = NULL;
	ssize_t n_ret;

	if(this->status != this->STATUS_RUNNING) return;

	p_playout = (void*) (((uintptr_t) (this->p_streambuffer)) + (this->streambuffer_nseg_playout)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	n_ret = (ssize_t) snd_pcm_writei(this->audiodev.p_device, p_playout, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret < 0)
	{
		if(n_ret == -EPIPE)
		{
			n_ret = (ssize_t) snd_pcm_prepare(this->audiodev.p_device);
			if(n_ret < 0) app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_prepare failed."));
		}
		else app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_writei failed."));
	}

	return;
}

