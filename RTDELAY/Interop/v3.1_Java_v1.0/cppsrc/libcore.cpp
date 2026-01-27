/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"

#include <stdlib.h>
#include <string.h>

#include "AudioRTDelay.hpp"

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define PB_I16 1
#define PB_I24 2

__attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio = NULL;
__attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;

__attribute__((__aligned__(PTR_SIZE_BITS))) std::string filein_dir = "";
__attribute__((__aligned__(PTR_SIZE_BITS))) std::string audiodev_desc = "";
__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");

__attribute__((__aligned__(32))) int h_filein = -1;

extern bool filein_open(void);
extern void filein_close(void);
extern int filein_get_params(void);
extern bool compare_signature(const char *auth, const uint8_t *buf);

bool core_init(void)
{
	return true;
}

void core_deinit(void)
{
	filein_close();

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

	if(exit_msg != NULL)
	{
		err_msg = __TEXT("PROCESS EXIT CALLED: ");
		err_msg += exit_msg;
	}

	libjni_trigger_exitProcess(exit_code);

	/*If JRE Exit Process fails, try exiting process through SYSTEM CALL*/
	exit(exit_code);

	while(true) delay_ms(16);
}

const __tchar_t* get_last_errmsg(void)
{
	return err_msg.c_str();
}

bool set_filein_dir(const char *fdir)
{
	if(fdir == NULL)
	{
		err_msg = __TEXT("Error: invalid parameter.");
		return false;
	}

	filein_dir = fdir;
	return true;
}

bool set_audiodev_desc(const char *desc)
{
	if(desc == NULL)
	{
		err_msg = __TEXT("Error: invalid parameter.");
		return false;
	}

	audiodev_desc = desc;
	return true;
}

bool playback_init(void)
{
	int n_ret;

	if(!filein_open())
	{
		err_msg = __TEXT("core_playback_init: Error: could not open input file.");
		return false;
	}

	n_ret = filein_get_params();
	if(n_ret < 0) return false;

	pb_params.file_dir = filein_dir.c_str();
	pb_params.audio_dev_desc = audiodev_desc.c_str();
	pb_params.rtdelay_buffer_size_frames = RTDELAY_BUFFER_SIZE_FRAMES;
	pb_params.rtdelay_n_ff_delays = RTDELAY_N_FF_DELAYS;
	pb_params.rtdelay_n_fb_delays = RTDELAY_N_FB_DELAYS;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	switch(n_ret)
	{
		case PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio == NULL)
	{
		err_msg = __TEXT("core_playback_init: Error: audio object instance failed.");
		return false;
	}

	if(!p_audio->initialize())
	{
		err_msg = p_audio->getLastErrorMessage();

		delete p_audio;
		p_audio = NULL;

		return false;
	}

	return true;
}

bool playback_run(void)
{
	if(p_audio == NULL) return false;

	if(!p_audio->runPlayback())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	delete p_audio;
	p_audio = NULL;

	return true;
}

void stop_playback(void)
{
	if(p_audio != NULL) p_audio->stopPlayback();

	return;
}

float rtdelay_getDryInputAmplitude(void)
{
	if(p_audio == NULL) return 0.0f;

	return p_audio->rtdelayGetDryInputAmplitude();
}

bool rtdelay_setDryInputAmplitude(float amp)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetDryInputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getOutputAmplitude(void)
{
	if(p_audio == NULL) return 0.0f;

	return p_audio->rtdelayGetOutputAmplitude();
}

bool rtdelay_setOutputAmplitude(float amp)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetOutputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t rtdelay_getFFDelay(size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_audio == NULL) return -1;

	if(!p_audio->rtdelayGetFFParams(n_fx, &params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) params.delay;
}

bool rtdelay_setFFDelay(size_t n_fx, uint32_t delay)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetFFDelay(n_fx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getFFAmplitude(size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_audio == NULL) return 0.0f;

	if(!p_audio->rtdelayGetFFParams(n_fx, &params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return params.amp;
}

bool rtdelay_setFFAmplitude(size_t n_fx, float amp)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetFFAmplitude(n_fx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t rtdelay_getFBDelay(size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_audio == NULL) return -1;

	if(!p_audio->rtdelayGetFBParams(n_fx, &params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) params.delay;
}

bool rtdelay_setFBDelay(size_t n_fx, uint32_t delay)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetFBDelay(n_fx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getFBAmplitude(size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_audio == NULL) return 0.0f;

	if(!p_audio->rtdelayGetFBParams(n_fx, &params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return params.amp;
}

bool rtdelay_setFBAmplitude(size_t n_fx, float amp)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelaySetFBAmplitude(n_fx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool rtdelay_resetFFParams(void)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelayResetFFParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool rtdelay_resetFBParams(void)
{
	if(p_audio == NULL) return false;

	if(!p_audio->rtdelayResetFBParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool filein_open(void)
{
	filein_close();

	h_filein = open(filein_dir.c_str(), O_RDONLY);
	return (h_filein >= 0);
}

void filein_close(void)
{
	if(h_filein < 0) return;

	close(h_filein);
	h_filein = -1;
	return;
}

int filein_get_params(void)
{
	const uintptr_t BUFFER_SIZE = 8192u;
	uint8_t *p_headerinfo = NULL;

	uintptr_t buffer_index;

	uint32_t u32;
	uint16_t u16;

	uint16_t bit_depth;

	p_headerinfo = (uint8_t*) malloc(BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		err_msg = __TEXT("filein_get_params: Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	memset(p_headerinfo, 0, BUFFER_SIZE);

	__LSEEK(h_filein, 0, SEEK_SET);
	read(h_filein, p_headerinfo, BUFFER_SIZE);
	filein_close();

	if(!compare_signature("RIFF", p_headerinfo))
	{
		err_msg = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const uint8_t*) (((uintptr_t) p_headerinfo) + 8u)))
	{
		err_msg = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			err_msg = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const uint8_t*) (((uintptr_t) p_headerinfo) + buffer_index))) break;

		u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
		buffer_index += (uintptr_t) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		err_msg = __TEXT("filein_get_params: Error: broken file header.");
		goto _l_filein_get_params_error;
	}

	u16 = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 8u));
	if(u16 != 1u)
	{
		err_msg = __TEXT("filein_get_params: Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = (size_t) *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 10u));
	pb_params.sample_rate = (size_t) *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 12u));

	bit_depth = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 22u));

	u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
	buffer_index += (uintptr_t) (u32 + 8u);

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			err_msg = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const uint8_t*) (((uintptr_t) p_headerinfo) + buffer_index))) break;

		u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
		buffer_index += (uintptr_t) (u32 + 8u);
	}

	u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));

	pb_params.audio_data_begin = (__offset_t) (buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((__offset_t) u32);

	free(p_headerinfo);
	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	err_msg = __TEXT("filein_get_params: Error: audio format not supported.");

_l_filein_get_params_error:

	filein_close();
	if(p_headerinfo != NULL) free(p_headerinfo);
	return -1;
}

bool compare_signature(const char *auth, const uint8_t *buf)
{
	size_t _index;

	if(auth == NULL) return false;
	if(buf == NULL) return false;

	for(_index = 0u; _index < 4u; _index++) if(auth[_index] != ((char) buf[_index])) return false;

	return true;
}

