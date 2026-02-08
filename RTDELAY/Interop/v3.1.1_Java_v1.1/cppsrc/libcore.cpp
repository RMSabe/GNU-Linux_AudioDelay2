/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.1)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"

#include <stdlib.h>
#include <string.h>

#define PB_I16 1
#define PB_I24 2

extern bool filein_open(procctx_t *p_procctx);
extern void filein_close(procctx_t *p_procctx);
extern int filein_get_params(procctx_t *p_procctx);
extern bool compare_signature(const char *auth, const uint8_t *buf);

procctx_t* procctx_alloc(void)
{
	procctx_t *p_procctx = NULL;

	p_procctx = (procctx_t*) malloc(sizeof(procctx_t));
	if(p_procctx == NULL) return NULL;

	p_procctx->p_fileindir = new std::string("");
	p_procctx->p_audiodevdesc = new std::string("");
	p_procctx->p_errmsg = new __string(__TEXT(""));

	if(p_procctx->p_fileindir == NULL)
	{
		procctx_free(p_procctx);
		return NULL;
	}

	if(p_procctx->p_audiodevdesc == NULL)
	{
		procctx_free(p_procctx);
		return NULL;
	}

	if(p_procctx->p_errmsg == NULL)
	{
		procctx_free(p_procctx);
		return NULL;
	}

	p_procctx->p_audio = NULL;
	p_procctx->p_jvm = NULL;
	p_procctx->h_filein = -1;

	return p_procctx;
}

bool procctx_free(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_fileindir != NULL)
	{
		delete p_procctx->p_fileindir;
		p_procctx->p_fileindir = NULL;
	}

	if(p_procctx->p_audiodevdesc != NULL)
	{
		delete p_procctx->p_audiodevdesc;
		p_procctx->p_audiodevdesc = NULL;
	}

	if(p_procctx->p_errmsg != NULL)
	{
		delete p_procctx->p_errmsg;
		p_procctx->p_errmsg = NULL;
	}

	free(p_procctx);
	return true;
}

bool core_init(procctx_t *p_procctx)
{
	/*No initialization required for this version*/
	return true;
}

void core_deinit(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	filein_close(p_procctx);

	if(p_procctx->p_audio != NULL)
	{
		delete p_procctx->p_audio;
		p_procctx->p_audio = NULL;
	}

	return;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	exit(exit_code);

	while(true) delay_ms(16);
}

const __tchar_t* get_last_errmsg(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return NULL;

	return p_procctx->p_errmsg->c_str();
}

bool set_filein_dir(procctx_t *p_procctx, const char *fdir)
{
	if(p_procctx == NULL) return false;

	if(fdir == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("Error: invalid parameter.");
		return false;
	}

	*(p_procctx->p_fileindir) = fdir;
	return true;
}

bool set_audiodev_desc(procctx_t *p_procctx, const char *desc)
{
	if(p_procctx == NULL) return false;

	if(desc == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("Error: invalid parameter.");
		return false;
	}

	*(p_procctx->p_audiodevdesc) = desc;
	return true;
}

bool playback_init(procctx_t *p_procctx)
{
	int n_ret;

	if(p_procctx == NULL) return false;

	if(!filein_open(p_procctx))
	{
		*(p_procctx->p_errmsg) = __TEXT("core_playback_init: Error: could not open input file.");
		return false;
	}

	n_ret = filein_get_params(p_procctx);
	if(n_ret < 0) return false;

	p_procctx->pb_params.file_dir = p_procctx->p_fileindir->c_str();
	p_procctx->pb_params.audio_dev_desc = p_procctx->p_audiodevdesc->c_str();
	p_procctx->pb_params.rtdelay_buffer_size_frames = RTDELAY_BUFFER_SIZE_FRAMES;
	p_procctx->pb_params.rtdelay_n_ff_delays = RTDELAY_N_FF_DELAYS;
	p_procctx->pb_params.rtdelay_n_fb_delays = RTDELAY_N_FB_DELAYS;

	if(p_procctx->p_audio != NULL)
	{
		delete p_procctx->p_audio;
		p_procctx->p_audio = NULL;
	}

	switch(n_ret)
	{
		case PB_I16:
			p_procctx->p_audio = new AudioPB_i16(&(p_procctx->pb_params));
			break;

		case PB_I24:
			p_procctx->p_audio = new AudioPB_i24(&(p_procctx->pb_params));
			break;
	}

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_playback_init: Error: audio object instance failed.");
		return false;
	}

	if(!p_procctx->p_audio->initialize())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();

		delete p_procctx->p_audio;
		p_procctx->p_audio = NULL;

		return false;
	}

	return true;
}

bool playback_run(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_playback_run: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->runPlayback())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	delete p_procctx->p_audio;
	p_procctx->p_audio = NULL;

	return true;
}

void stop_playback(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	if(p_procctx->p_audio != NULL) p_procctx->p_audio->stopPlayback();

	return;
}

float rtdelay_getDryInputAmplitude(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getDryInputAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_procctx->p_audio->rtdelayGetDryInputAmplitude();
}

bool rtdelay_setDryInputAmplitude(procctx_t *p_procctx, float amp)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setDryInputAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetDryInputAmplitude(amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getOutputAmplitude(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getOutputAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	return p_procctx->p_audio->rtdelayGetOutputAmplitude();
}

bool rtdelay_setOutputAmplitude(procctx_t *p_procctx, float amp)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setOutputAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetOutputAmplitude(amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t rtdelay_getFFDelay(procctx_t *p_procctx, size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_procctx == NULL) return -1;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getFFDelay: Error: audio object not ready.");
		return -1;
	}

	if(!p_procctx->p_audio->rtdelayGetFFParams(n_fx, &params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) params.delay;
}

bool rtdelay_setFFDelay(procctx_t *p_procctx, size_t n_fx, uint32_t delay)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setFFDelay: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetFFDelay(n_fx, delay))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getFFAmplitude(procctx_t *p_procctx, size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getFFAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	if(!p_procctx->p_audio->rtdelayGetFFParams(n_fx, &params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return params.amp;
}

bool rtdelay_setFFAmplitude(procctx_t *p_procctx, size_t n_fx, float amp)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setFFAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetFFAmplitude(n_fx, amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

ssize_t rtdelay_getFBDelay(procctx_t *p_procctx, size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_procctx == NULL) return -1;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getFBDelay: Error: audio object not ready.");
		return -1;
	}

	if(!p_procctx->p_audio->rtdelayGetFBParams(n_fx, &params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return -1;
	}

	return (ssize_t) params.delay;
}

bool rtdelay_setFBDelay(procctx_t *p_procctx, size_t n_fx, uint32_t delay)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setFBDelay: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetFBDelay(n_fx, delay))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

float rtdelay_getFBAmplitude(procctx_t *p_procctx, size_t n_fx)
{
	audiortdelay_fx_params_t params;

	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_getFBAmplitude: Error: audio object not ready.");
		return 0.0f;
	}

	if(!p_procctx->p_audio->rtdelayGetFBParams(n_fx, &params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return params.amp;
}

bool rtdelay_setFBAmplitude(procctx_t *p_procctx, size_t n_fx, float amp)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_setFBAmplitude: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelaySetFBAmplitude(n_fx, amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool rtdelay_resetFFParams(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_resetFFParams: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelayResetFFParams())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool rtdelay_resetFBParams(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return false;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("core_rtdelay_resetFBParams: Error: audio object not ready.");
		return false;
	}

	if(!p_procctx->p_audio->rtdelayResetFBParams())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return false;
	}

	return true;
}

bool filein_open(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return false;

	filein_close(p_procctx);

	p_procctx->h_filein = open(p_procctx->p_fileindir->c_str(), O_RDONLY);
	return (p_procctx->h_filein >= 0);
}

void filein_close(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	if(p_procctx->h_filein < 0) return;

	close(p_procctx->h_filein);
	p_procctx->h_filein = -1;
	return;
}

int filein_get_params(procctx_t *p_procctx)
{
	const uintptr_t BUFFER_SIZE = 8192u;
	uint8_t *p_headerinfo = NULL;

	uintptr_t buffer_index;

	uint32_t u32;
	uint16_t u16;

	uint16_t bit_depth;

	if(p_procctx == NULL) return -1;

	p_headerinfo = (uint8_t*) malloc(BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	memset(p_headerinfo, 0, BUFFER_SIZE);

	__LSEEK(p_procctx->h_filein, 0, SEEK_SET);
	read(p_procctx->h_filein, p_headerinfo, BUFFER_SIZE);
	filein_close(p_procctx);

	if(!compare_signature("RIFF", p_headerinfo))
	{
		*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const uint8_t*) (((uintptr_t) p_headerinfo) + 8u)))
	{
		*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const uint8_t*) (((uintptr_t) p_headerinfo) + buffer_index))) break;

		u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
		buffer_index += (uintptr_t) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: broken file header.");
		goto _l_filein_get_params_error;
	}

	u16 = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 8u));
	if(u16 != 1u)
	{
		*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	p_procctx->pb_params.n_channels = (size_t) *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 10u));
	p_procctx->pb_params.sample_rate = (size_t) *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 12u));

	bit_depth = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 22u));

	u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
	buffer_index += (uintptr_t) (u32 + 8u);

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const uint8_t*) (((uintptr_t) p_headerinfo) + buffer_index))) break;

		u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));
		buffer_index += (uintptr_t) (u32 + 8u);
	}

	u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));

	p_procctx->pb_params.audio_data_begin = (__offset_t) (buffer_index + 8u);
	p_procctx->pb_params.audio_data_end = p_procctx->pb_params.audio_data_begin + ((__offset_t) u32);

	free(p_headerinfo);
	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	*(p_procctx->p_errmsg) = __TEXT("filein_get_params: Error: audio format not supported.");

_l_filein_get_params_error:

	filein_close(p_procctx);
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

