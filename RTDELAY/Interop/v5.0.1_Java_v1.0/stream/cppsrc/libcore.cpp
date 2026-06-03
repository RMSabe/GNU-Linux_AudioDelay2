/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1 (Audio Stream Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"

#include <stdlib.h>
#include <string.h>

#define __STREAM_I16 1
#define __STREAM_I24 2

__attribute__((__aligned__(PTR_SIZE_BITS))) size_t __AUDIO_STREAMBUFFER_N_SEGMENTS = 0u;
__attribute__((__aligned__(PTR_SIZE_BITS))) size_t __AUDIO_DELAY_BUFFER_SIZE_FRAMES = 0u;
__attribute__((__aligned__(PTR_SIZE_BITS))) size_t __AUDIO_DELAY_N_FFCH = 0u;
__attribute__((__aligned__(PTR_SIZE_BITS))) size_t __AUDIO_DELAY_N_FBCH = 0u;

__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");
__attribute__((__aligned__(PTR_SIZE_BITS))) size_t audio_samplerate = 0u;
__attribute__((__aligned__(PTR_SIZE_BITS))) size_t audio_nchannels = 0u;
__attribute__((__aligned__(32))) int audio_format = -1;

static __attribute__((__aligned__(PTR_SIZE_BITS))) AudioStream *p_audio = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) audiostream_params_t stream_params;

bool core_init(void)
{
	/*No initialization required for this version*/
	return true;
}

void core_deinit(void)
{
	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	core_deinit();

	exit(exit_code);

	while(true) delay_ms(16);
}

bool createaudioobject(void)
{
	stream_params.sample_rate = audio_samplerate;
	stream_params.n_channels = audio_nchannels;
	stream_params.audiobuffer_size_frames = _get_closest_power2_ceil(stream_params.sample_rate)/4u;
	stream_params.streambuffer_segment_size_frames = stream_params.audiobuffer_size_frames/4u;
	stream_params.streambuffer_n_segments = __AUDIO_STREAMBUFFER_N_SEGMENTS;
	stream_params.delay_buffer_size_frames = __AUDIO_DELAY_BUFFER_SIZE_FRAMES;
	stream_params.delay_n_ff_delays = __AUDIO_DELAY_N_FFCH;
	stream_params.delay_n_fb_delays = __AUDIO_DELAY_N_FBCH;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	switch(audio_format)
	{
		case __STREAM_I16:
			p_audio = new AudioStream_i16(&stream_params);
			break;

		case __STREAM_I24:
			p_audio = new AudioStream_i24(&stream_params);
			break;
	}

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_createaudioobject: Error: audio object instance failed.");
		return false;
	}

	return true;
}

bool load_audiodevicelist(bool output)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_load_audiodevicelist: Error: no audio object instance.");
		return false;
	}

	if(!p_audio->loadAudioDeviceList(output))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t audiodevicelist_get_entry_count(bool output)
{
	ssize_t _ssize;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_audiodevicelist_get_entry_count: Error: no audio object instance.");
		return -1;
	}

	_ssize = p_audio->getAudioDeviceListEntryCount(output);
	if(_ssize < 0) err_msg = p_audio->getLastErrorMessage();

	return _ssize;
}

const char* audiodevicelist_get_entry_info(size_t index, bool output, bool info)
{
	const audiodevicelist_entry_t *p_entry = NULL;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_audiodevicelist_get_entry_info: Error: no audio object instance.");
		return NULL;
	}

	p_entry = p_audio->getAudioDeviceListEntry(index, output);
	if(p_entry == NULL)
	{
		err_msg = p_audio->getLastErrorMessage();
		return NULL;
	}

	if(info) return p_entry->desc;
	
	return p_entry->name;
}

bool choose_device(size_t index, bool defaultdev, bool output)
{
	bool b_ret;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_choose_device: Error: no audio object instance.");
		return false;
	}

	if(defaultdev) b_ret = p_audio->chooseDefaultDevice(output);
	else b_ret = p_audio->chooseDevice(index, output);

	if(!b_ret) err_msg = p_audio->getLastErrorMessage();

	return b_ret;
}

bool audioobject_init(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_audioobject_init: Error: no audio object instance.");
		return false;
	}

	if(!p_audio->initialize())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

int audioobject_getstatus(void)
{
	if(p_audio == NULL) return -1;

	return p_audio->getStatus();
}

bool run_stream(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_run_stream: Error: no audio object instance.");
		return false;
	}

	if(!p_audio->runStream())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	delete p_audio;
	p_audio = NULL;

	return true;
}

void pause_stream(void)
{
	if(p_audio != NULL) p_audio->pauseStream();

	return;
}

void resume_stream(void)
{
	if(p_audio != NULL) p_audio->resumeStream();

	return;
}

void stop_stream(void)
{
	if(p_audio != NULL) p_audio->stopStream();

	return;
}

float delay_getDryInputAmplitude(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getDryInputAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_audio->delayGetDryInputAmplitude();
}

bool delay_setDryInputAmplitude(float amp)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setDryInputAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetDryInputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float delay_getOutputAmplitude(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getOutputAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_audio->delayGetOutputAmplitude();
}

bool delay_setOutputAmplitude(float amp)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setOutputAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetOutputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t delay_getFFDelay(size_t n_fx)
{
	int32_t _ndelay;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getFFDelay: Error: audio object not ready.");
		return -1;
	}

	_ndelay = p_audio->delayGetFFDelay(n_fx);
	if(_ndelay < 0)
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) _ndelay;
}

bool delay_setFFDelay(size_t n_fx, uint32_t delay)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setFFDelay: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetFFDelay(n_fx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float delay_getFFAmplitude(size_t n_fx)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getFFAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_audio->delayGetFFAmplitude(n_fx);
}

bool delay_setFFAmplitude(size_t n_fx, float amp)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setFFAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetFFAmplitude(n_fx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t delay_getFBDelay(size_t n_fx)
{
	int32_t _ndelay;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getFBDelay: Error: audio object not ready.");
		return -1;
	}

	_ndelay = p_audio->delayGetFBDelay(n_fx);
	if(_ndelay < 0)
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) _ndelay;
}

bool delay_setFBDelay(size_t n_fx, uint32_t delay)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setFBDelay: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetFBDelay(n_fx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float delay_getFBAmplitude(size_t n_fx)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_getFBAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_audio->delayGetFBAmplitude(n_fx);
}

bool delay_setFBAmplitude(size_t n_fx, float amp)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_setFBAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delaySetFBAmplitude(n_fx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool delay_resetFFParams(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_resetFFParams: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delayResetFFParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool delay_resetFBParams(void)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_delay_resetFBParams: Error: audio object not ready.");
		return false;
	}

	if(!p_audio->delayResetFBParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

