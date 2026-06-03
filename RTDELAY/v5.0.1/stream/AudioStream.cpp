/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioStream.hpp"
#include "cstrdef.h"
#include "delay.h"

#include <stdlib.h>
#include <string.h>

AudioStream::AudioStream(const audiostream_params_t *p_params)
{
	this->setParameters(p_params);
}

bool AudioStream::setParameters(const audiostream_params_t *p_params)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioStream::setParameters: Error: AudioStream object is already initialized.");
		return false;
	}

	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioStream::setParameters: Error: given p_params object pointer is NULL.");
		return false;
	}

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

bool AudioStream::initialize(void)
{
	audiodelay_init_params_t delay_params;

	if(this->status > 0) return true;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->SAMPLE_RATE)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid sample rate.");
		return false;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid number of channels.");
		return false;
	}

	if(this->STREAMBUFFER_N_SEGMENTS < this->STREAMBUFFER_N_SEGMENTS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid stream buffer segment count.");
		return false;
	}

	if(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid stream buffer segment size.");
		return false;
	}

	if(this->AUDIOBUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid audio buffer size. (audio buffer is smaller than stream buffer segment).");
		return false;
	}

	if(this->AUDIODELAY_BUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioStream::initialize: Error: invalid delay buffer size. (delay buffer is smaller than stream buffer segment).");
		return false;
	}

	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = (this->AUDIOBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SEGMENT_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SIZE_FRAMES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->STREAMBUFFER_N_SEGMENTS);
	this->STREAMBUFFER_SIZE_SAMPLES = (this->STREAMBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->AUDIODELAY_BUFFER_N_SEGMENTS = (this->AUDIODELAY_BUFFER_SIZE_FRAMES)/(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);

	if(!this->audio_hw_init(&(this->p_audiodev_in), this->AUDIODEVDESC_IN.c_str(), SND_PCM_STREAM_CAPTURE, 0))
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg += __TEXT("\nAudioStream::initialize: Error: input audio device init failed.");
		return false;
	}

	if(!this->audio_hw_init(&(this->p_audiodev_out), this->AUDIODEVDESC_OUT.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK))
	{
		this->audio_hw_deinit(&(this->p_audiodev_in));
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg += __TEXT("\nAudioStream::initialize: Error: output audio device init failed.");
		return false;
	}

	if(!this->buffer_alloc())
	{
		this->audio_hw_deinit(&(this->p_audiodev_in));
		this->audio_hw_deinit(&(this->p_audiodev_out));
		this->status = this->STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioStream::initialize: Error: memory allocate failed.");
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
			this->audio_hw_deinit(&(this->p_audiodev_in));
			this->audio_hw_deinit(&(this->p_audiodev_out));
			this->buffer_free();
			this->status = this->STATUS_ERROR_MEMORY;
			this->err_msg = __TEXT("AudioStream::initialize: Error: failed to create delay object.");
			return false;
		}
	}
	else this->p_delay->setInitParameters(&delay_params);

	if(!this->p_delay->initialize())
	{
		this->audio_hw_deinit(&(this->p_audiodev_in));
		this->audio_hw_deinit(&(this->p_audiodev_out));
		this->buffer_free();

		this->status = this->STATUS_ERROR_GENERIC;
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	this->status = this->STATUS_READY;
	return true;
}

bool AudioStream::runStream(void)
{
	if(this->status != this->STATUS_READY)
	{
		this->err_msg = __TEXT("AudioStream::runStream: Error: AudioStream object is either not initialized or already running.");
		return false;
	}

	this->stream_proc();

	this->audio_hw_deinit(&(this->p_audiodev_in));
	this->audio_hw_deinit(&(this->p_audiodev_out));
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return true;
}

void AudioStream::pauseStream(void)
{
	if(this->status == this->STATUS_RUNNING)
	{
		this->status = this->STATUS_PAUSED;
		snd_pcm_drain(this->p_audiodev_in);
		snd_pcm_drain(this->p_audiodev_out);
	}

	return;
}

void AudioStream::resumeStream(void)
{
	if(this->status == this->STATUS_PAUSED)
	{
		memset(this->p_streambuffer_in, 0, this->STREAMBUFFER_SIZE_BYTES);
		memset(this->p_streambuffer_out, 0, this->STREAMBUFFER_SIZE_BYTES);

		snd_pcm_prepare(this->p_audiodev_in);
		snd_pcm_prepare(this->p_audiodev_out);

		this->status = this->STATUS_RUNNING;
	}

	return;
}

void AudioStream::stopStream(void)
{
	if(this->status > this->STATUS_READY) this->status = this->STATUS_STOPPED;

	return;
}

bool AudioStream::loadAudioDeviceList(bool output)
{
	void **pp_hints = NULL;
	char *p_devname = NULL;
	char *p_devdesc = NULL;
	char *p_devioid = NULL;
	char *p_audiodevicelist_entry_name = NULL;
	char *p_audiodevicelist_entry_desc = NULL;

	audiodevicelist_t *p_devlist = NULL;

	size_t n_hint;
	size_t n_sndctl;
	size_t n_sndctl_count;
	size_t audiodevicelist_byteindex;

	int n_ret;
	int _i32_sndctlindex;

	bool ioid_matched;

	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: cannot run method. AudioStream object already initialized.");
		return false;
	}

	if(output) p_devlist = &(this->audiodevlist_out);
	else p_devlist = &(this->audiodevlist_in);

	if(p_devlist->p_entries != NULL)
	{
		free(p_devlist->p_entries);
		p_devlist->p_entries = NULL;
	}

	p_devlist->n_entries = 0u;

	n_sndctl_count = 0u;
	_i32_sndctlindex = -1;

	while(true)
	{
		n_ret = snd_card_next(&_i32_sndctlindex);
		if(n_ret < 0)
		{
			this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_card_next failed.");
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
			this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
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
				this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				this->status = this->STATUS_ERROR_AUDIOHW;
				return false;
			}

			/*Consider only hardware interface devices ("hw:..." "plughw:...")*/

			if(_cstr_char_compare_upto_char(p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(p_devname, "plughw", ':', true))
			{
				p_devioid = snd_device_name_get_hint((const char*) pp_hints[n_hint], "IOID");

				if(p_devioid == NULL) ioid_matched = true;
				else if(output) ioid_matched = _cstr_char_compare(p_devioid, "Output");
				else ioid_matched = _cstr_char_compare(p_devioid, "Input");

				if(ioid_matched) p_devlist->n_entries++;

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

	if(!p_devlist->n_entries)
	{
		this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: no devices found.");
		return false;
	}

	p_devlist->p_entries = (audiodevicelist_entry_t*) malloc((p_devlist->n_entries)*sizeof(audiodevicelist_entry_t));
	if(p_devlist->p_entries == NULL)
	{
		this->status = this->STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: failed to allocate heap memory.");
		return false;
	}

	audiodevicelist_byteindex = 0u;
	n_sndctl = 0u;

	while(n_sndctl < n_sndctl_count)
	{
		n_ret = snd_device_name_hint((int) n_sndctl, "pcm", &pp_hints);
		if((n_ret < 0) || (pp_hints == NULL))
		{
			this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
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
				this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				this->status = this->STATUS_ERROR_AUDIOHW;
				return false;
			}

			if(_cstr_char_compare_upto_char(p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(p_devname, "plughw", ':', true))
			{
				p_devioid = snd_device_name_get_hint((const char*) pp_hints[n_hint], "IOID");

				if(p_devioid == NULL) ioid_matched = true;
				else if(output) ioid_matched = _cstr_char_compare(p_devioid, "Output");
				else ioid_matched = _cstr_char_compare(p_devioid, "Input");

				if(ioid_matched)
				{
					p_devdesc = snd_device_name_get_hint((const char*) pp_hints[n_hint], "DESC");
					if(p_devdesc == NULL)
					{
						if(p_devioid != NULL) free(p_devioid);
						free(p_devname);
						snd_device_name_free_hint(pp_hints);
						this->err_msg = __TEXT("AudioStream::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
						this->status = this->STATUS_ERROR_AUDIOHW;
						return false;
					}

					p_audiodevicelist_entry_name = ((audiodevicelist_entry_t*) (((uintptr_t) p_devlist->p_entries) + audiodevicelist_byteindex))->name;
					p_audiodevicelist_entry_desc = ((audiodevicelist_entry_t*) (((uintptr_t) p_devlist->p_entries) + audiodevicelist_byteindex))->desc;

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

ssize_t AudioStream::getAudioDeviceListEntryCount(bool output)
{
	if(output) return (ssize_t) this->audiodevlist_out.n_entries;

	return (ssize_t) this->audiodevlist_in.n_entries;
}

const audiodevicelist_entry_t* AudioStream::getAudioDeviceListEntry(size_t index, bool output)
{
	audiodevicelist_t *p_devlist = NULL;

	if(output) p_devlist = &(this->audiodevlist_out);
	else p_devlist = &(this->audiodevlist_in);

	if(p_devlist->p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioStream::getAudioDeviceListEntry: Error: audio device list is not loaded.");
		return NULL;
	}

	if(index >= p_devlist->n_entries)
	{
		this->err_msg = __TEXT("AudioStream::getAudioDeviceListEntry: Error: given index is out of bounds.");
		return NULL;
	}

	return (const audiodevicelist_entry_t*) (((uintptr_t) p_devlist->p_entries) + index*sizeof(audiodevicelist_entry_t));
}

bool AudioStream::chooseDevice(const char *name, bool output)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioStream::chooseDevice: Error: cannot run method. AudioStream object already initialized.");
		return false;
	}

	if(name == NULL)
	{
		this->err_msg = __TEXT("AudioStream::chooseDevice: Error: given name parameter is NULL.");
		return false;
	}

	if(output) this->AUDIODEVDESC_OUT = name;
	else this->AUDIODEVDESC_IN = name;

	return true;
}

bool AudioStream::chooseDevice(size_t index, bool output)
{
	std::string *p_audiodevdesc = NULL;
	audiodevicelist_t *p_devlist = NULL;

	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioStream::chooseDevice: Error: cannot run method. AudioStream object already initialized.");
		return false;
	}

	if(output)
	{
		p_audiodevdesc = &(this->AUDIODEVDESC_OUT);
		p_devlist = &(this->audiodevlist_out);
	}
	else
	{
		p_audiodevdesc = &(this->AUDIODEVDESC_IN);
		p_devlist = &(this->audiodevlist_in);
	}

	if(p_devlist->p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioStream::chooseDevice: Error: audio device list is not loaded.");
		return false;
	}

	if(index >= p_devlist->n_entries)
	{
		this->err_msg = __TEXT("AudioStream::chooseDevice: Error: given index is out of bounds.");
		return false;
	}

	(*p_audiodevdesc) = p_devlist->p_entries[index].name;
	return true;
}

bool AudioStream::chooseDefaultDevice(bool output)
{
	if(this->status > 0)
	{
		this->err_msg = __TEXT("AudioStream::chooseDefaultDevice: Error: cannot run method. AudioStream object already initialized.");
		return false;
	}

	if(output) this->AUDIODEVDESC_OUT = "default";
	else this->AUDIODEVDESC_IN = "default";

	return true;
}

float AudioStream::delayGetDryInputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getDryInputAmplitude();
}

float AudioStream::delayGetOutputAmplitude(void)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getOutputAmplitude();
}

bool AudioStream::delaySetDryInputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setDryInputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioStream::delaySetOutputAmplitude(float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setOutputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int32_t AudioStream::delayGetFFDelay(size_t n_fx)
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

float AudioStream::delayGetFFAmplitude(size_t n_fx)
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

bool AudioStream::delaySetFFDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioStream::delaySetFFAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFFAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int32_t AudioStream::delayGetFBDelay(size_t n_fx)
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

float AudioStream::delayGetFBAmplitude(size_t n_fx)
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

bool AudioStream::delaySetFBDelay(size_t n_fx, uint32_t delay)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioStream::delaySetFBAmplitude(size_t n_fx, float amp)
{
	if(this->status < 1) return false;

	if(!this->p_delay->setFBAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioStream::delayResetFFParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFFParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

bool AudioStream::delayResetFBParams(void)
{
	if(this->status < 1) return false;

	if(!this->p_delay->resetFBParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return false;
	}

	return true;
}

int AudioStream::getStatus(void)
{
	return this->status;
}

__string AudioStream::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (__TEXT("Error: AudioStream object not initialized\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

void AudioStream::deinitialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;

	cppthread_stop(&(this->capture_thread));

	this->audio_hw_deinit(&(this->p_audiodev_in));
	this->audio_hw_deinit(&(this->p_audiodev_out));
	this->buffer_free();

	if(this->audiodevlist_in.p_entries != NULL)
	{
		free(this->audiodevlist_in.p_entries);
		this->audiodevlist_in.p_entries = NULL;
	}

	this->audiodevlist_in.n_entries = 0u;

	if(this->audiodevlist_out.p_entries != NULL)
	{
		free(this->audiodevlist_out.p_entries);
		this->audiodevlist_out.p_entries = NULL;
	}

	this->audiodevlist_out.n_entries = 0u;

	if(this->p_delay != NULL)
	{
		delete this->p_delay;
		this->p_delay = NULL;
	}

	return;
}

bool AudioStream::audio_hw_init(snd_pcm_t **pp_audiodev, const char *audiodev_desc, int stream_mode, int nonblock)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames;
	int n_ret;

	if(pp_audiodev == NULL) return false;
	if(audiodev_desc == NULL) return false;

	this->audio_hw_deinit(pp_audiodev); /*clear any previous instance of audio hw.*/

	/*OPEN AUDIO DEVICE*/

	n_ret = snd_pcm_open(pp_audiodev, audiodev_desc, (snd_pcm_stream_t) stream_mode, nonblock);
	if(n_ret < 0)
	{
		*pp_audiodev = NULL;
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_open failed.");
		return false;
	}

	/*ALLOCATE DEVICE PARAMS OBJ*/

	n_ret = snd_pcm_hw_params_malloc(&p_hwparams);
	if((n_ret < 0) || (p_hwparams == NULL))
	{
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_malloc failed.");
		return false;
	}

	/*PRELOAD DEVICE PARAMETERS*/

	n_ret = snd_pcm_hw_params_any(*pp_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_any failed.");
		return false;
	}

	/*ENABLE/DISABLE DEVICE RESAMPLING*/
	/*
	 * 0 = Disable (better performance)
	 * 1 = Enable (better compatibility)
	 */

	n_ret = snd_pcm_hw_params_set_rate_resample(*pp_audiodev, p_hwparams, 1u);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_rate_resample failed.");
		return false;
	}

	/*SET DEVICE ACCESS*/

	n_ret = snd_pcm_hw_params_set_access(*pp_audiodev, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_access failed.");
		return false;
	}

	/*SET DEVICE FORMAT*/

	n_ret = snd_pcm_hw_params_set_format(*pp_audiodev, p_hwparams, (snd_pcm_format_t) this->AUDIODEV_FORMAT);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_format failed.");
		return false;
	}

	/*SET DEVICE CHANNELS*/

	n_ret = snd_pcm_hw_params_set_channels(*pp_audiodev, p_hwparams, (unsigned int) this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_channels failed.");
		return false;
	}

	/*SET DEVICE SAMPLING RATE*/

	n_ret = snd_pcm_hw_params_set_rate(*pp_audiodev, p_hwparams, (unsigned int) this->SAMPLE_RATE, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_rate failed.");
		return false;
	}

	/*SET DEVICE BUFFER SIZE*/

	n_frames = (snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES;
	n_ret = snd_pcm_hw_params_set_buffer_size_near(*pp_audiodev, p_hwparams, &n_frames);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_buffer_size_near failed.");
		return false;
	}

	/*SET DEVICE BUFFER SEGMENT SIZE (PERIOD SIZE)*/

	n_ret = snd_pcm_hw_params_set_period_size(*pp_audiodev, p_hwparams, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params_set_period_size failed.");
		return false;
	}

	/*APPLY SETTINGS TO DEVICE*/

	n_ret = snd_pcm_hw_params(*pp_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit(pp_audiodev);
		this->err_msg = __TEXT("AudioStream::audio_hw_init: Error: snd_pcm_hw_params failed.");
		return false;
	}

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

void AudioStream::audio_hw_deinit(snd_pcm_t **pp_audiodev)
{
	if(pp_audiodev == NULL) return;
	if(*pp_audiodev == NULL) return;

	snd_pcm_drop(*pp_audiodev);
	snd_pcm_hw_free(*pp_audiodev);
	snd_pcm_close(*pp_audiodev);

	*pp_audiodev = NULL;
	return;
}

bool AudioStream::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	this->p_streambuffer_in = malloc(this->STREAMBUFFER_SIZE_BYTES);
	this->p_streambuffer_out = malloc(this->STREAMBUFFER_SIZE_BYTES);

	if(this->p_streambuffer_in == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_streambuffer_out == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_streambuffer_in, 0, this->STREAMBUFFER_SIZE_BYTES);
	memset(this->p_streambuffer_out, 0, this->STREAMBUFFER_SIZE_BYTES);

	return true;
}

void AudioStream::buffer_free(void)
{
	if(this->p_streambuffer_in != NULL)
	{
		free(this->p_streambuffer_in);
		this->p_streambuffer_in = NULL;
	}

	if(this->p_streambuffer_out != NULL)
	{
		free(this->p_streambuffer_out);
		this->p_streambuffer_out = NULL;
	}

	return;
}

void AudioStream::stream_proc(void)
{
	this->stream_init();

	this->capture_thread = std::thread(&AudioStream::capture_thread_proc, this);

	this->playback_dsp_thread_proc();

	cppthread_wait(&(this->capture_thread));

	snd_pcm_drain(this->p_audiodev_in);
	snd_pcm_drain(this->p_audiodev_out);

	return;
}

void AudioStream::stream_init(void)
{
	this->streambuffer_in_nseg = 0u;
	this->streambuffer_out_nseg = 0u;

	this->delaybuffer_nseg = 0u;

	this->p_delay->setDryInputAmplitude(1.0f);
	this->p_delay->setOutputAmplitude(1.0f);
	this->p_delay->resetFFParams();
	this->p_delay->resetFBParams();

	this->status = this->STATUS_RUNNING;
	return;
}

void AudioStream::streambuffer_in_nseg_update(void)
{
	this->streambuffer_in_nseg++;
	this->streambuffer_in_nseg %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

void AudioStream::streambuffer_out_nseg_update(void)
{
	this->streambuffer_out_nseg++;
	this->streambuffer_out_nseg %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

void AudioStream::delaybuffer_nseg_update(void)
{
	this->delaybuffer_nseg++;
	this->delaybuffer_nseg %= this->AUDIODELAY_BUFFER_N_SEGMENTS;

	return;
}

void AudioStream::streambuffer_in_capture(void)
{
	void *p_in = NULL;
	ssize_t n_ret;

	if(this->status != this->STATUS_RUNNING) return;

	p_in = (void*) (((uintptr_t) (this->p_streambuffer_in)) + (this->streambuffer_in_nseg)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	n_ret = (ssize_t) snd_pcm_readi(this->p_audiodev_in, p_in, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret < 0)
	{
		if(n_ret == -EPIPE)
		{
			n_ret = (ssize_t) snd_pcm_prepare(this->p_audiodev_in);
			if(n_ret < 0) app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioStream::streambuffer_in_capture: Error: snd_pcm_prepare failed."));
		}
		else app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioStream::streambuffer_in_capture: Error: snd_pcm_readi failed."));
	}

	return;
}

void AudioStream::streambuffer_out_playback(void)
{
	void *p_out = NULL;
	ssize_t n_ret;

	if(this->status != this->STATUS_RUNNING) return;

	p_out = (void*) (((uintptr_t) (this->p_streambuffer_out)) + (this->streambuffer_out_nseg)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	n_ret = (ssize_t) snd_pcm_writei(this->p_audiodev_out, p_out, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret < 0)
	{
		if(n_ret == -EPIPE)
		{
			n_ret = (ssize_t) snd_pcm_prepare(this->p_audiodev_out);
			if(n_ret < 0) app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioStream::streambuffer_out_playback: Error: snd_pcm_prepare failed."));
		}
		else app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioStream::streambuffer_out_playback: Error: snd_pcm_writei failed."));
	}

	return;
}

void AudioStream::capture_thread_proc(void)
{
	while(true)
	{
		if((this->status == this->STATUS_STOPPED) || (this->status < 1)) break;

		if(this->status == this->STATUS_PAUSED)
		{
			delay_ms(1);
			continue;
		}

		this->streambuffer_in_capture();
		this->streambuffer_in_nseg_update();
	}

	return;
}

void AudioStream::playback_dsp_thread_proc(void)
{
	while(true)
	{
		if((this->status == this->STATUS_STOPPED) || (this->status < 1)) break;

		if(this->status == this->STATUS_PAUSED)
		{
			delay_ms(1);
			continue;
		}

		this->streambuffer_out_playback();

		this->delaybuffer_loadin();
		this->p_delay->runDSP(this->delaybuffer_nseg);
		this->delaybuffer_loadout();

		this->streambuffer_out_nseg_update();
		this->delaybuffer_nseg_update();

		snd_pcm_wait(this->p_audiodev_out, -1);
	}

	return;
}

