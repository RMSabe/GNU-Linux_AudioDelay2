/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2 (Audio Stream Version)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"

#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "AudioStream.hpp"
#include "AudioStream_i16.hpp"
#include "AudioStream_i24.hpp"

#define __AUDIO_STREAMBUFFER_N_SEGMENTS 4U
#define __AUDIO_DELAY_BUFFER_SIZE_FRAMES 65536U
#define __AUDIO_DELAY_N_FFCH 4U
#define __AUDIO_DELAY_N_FBCH 4U

#define __STREAM_I16 1
#define __STREAM_I24 2

static __attribute__((__aligned__(PTR_SIZE_BITS))) AudioStream *p_audio = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) audiostream_params_t stream_params;
static __attribute__((__aligned__(PTR_SIZE_BITS))) const char *audiodevdesc_in = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) const char *audiodevdesc_out = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) std::thread audiothread;
static __attribute__((__aligned__(PTR_SIZE_BITS))) __string usercmd = __TEXT("");
static __attribute__((__aligned__(32))) int audio_format = -1;

bool enable_resampling = false;

static void app_deinit(void);
static void runtime_loop(void);

static bool mainarg_parse_sampling_rate(const char *text);
static bool mainarg_parse_bit_depth(const char *text);
static bool mainarg_parse_n_channels(const char *text);
static bool mainarg_parse_enable_resampling(const char *text);

static void cmdui_cmd_decode(void);
static void cmdui_print_help_text(void);
static void cmdui_print_current_params(void);
static ssize_t cmdui_parse_nfx(const __tchar_t *text_nfx, ssize_t max);
static ssize_t cmdui_parse_ndelay(const __tchar_t *text_ndelay);
static bool cmdui_parse_famp(const __tchar_t *text_famp, float *p_famp);

static void audiothread_proc(void);

int main(int argc, char **argv)
{
	if(argc < 7)
	{
		__STDCOUT__ << __TEXT("Error: missing arguments\nThis executable requires 6 arguments: <input device id> <output device id> <sampling rate> <bit depth> <number of channels> <enable device resampling (0/1)>\nThey must be in that order\n");
		return -1;
	}

	if(!mainarg_parse_sampling_rate(argv[3]))
	{
		__STDCOUT__ << __TEXT("Error: bad sampling rate\n");
		return -1;
	}

	if(!mainarg_parse_bit_depth(argv[4]))
	{
		__STDCOUT__ << __TEXT("Error: bad bit depth\n");
		return -1;
	}

	if(!mainarg_parse_n_channels(argv[5]))
	{
		__STDCOUT__ << __TEXT("Error: bad number of channels\n");
		return -1;
	}

	if(!mainarg_parse_enable_resampling(argv[6]))
	{
		__STDCOUT__ << __TEXT("Error: bad enable resampling flag\n");
	}

	audiodevdesc_in = argv[1];
	audiodevdesc_out = argv[2];

	stream_params.audiobuffer_size_frames = _get_closest_power2_ceil(stream_params.sample_rate)/4u;
	stream_params.streambuffer_segment_size_frames = stream_params.audiobuffer_size_frames/4u;
	stream_params.streambuffer_n_segments = __AUDIO_STREAMBUFFER_N_SEGMENTS;
	stream_params.delay_buffer_size_frames = __AUDIO_DELAY_BUFFER_SIZE_FRAMES;
	stream_params.delay_n_ff_delays = __AUDIO_DELAY_N_FFCH;
	stream_params.delay_n_fb_delays = __AUDIO_DELAY_N_FBCH;

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
		__STDCOUT__ << __TEXT("Error: audio object instance failed.\n");
		goto _l_main_error;
	}

	p_audio->chooseDevice(audiodevdesc_in, enable_resampling, false);
	p_audio->chooseDevice(audiodevdesc_out, enable_resampling, true);

	if(!p_audio->initialize())
	{
		__STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;
		goto _l_main_error;
	}

	audiothread = std::thread(&audiothread_proc);

	runtime_loop();

	app_deinit();
	return 0;

_l_main_error:

	app_deinit();
	return -1;
}

static void app_deinit(void)
{
	cppthread_stop(&audiothread);

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) __STDCOUT__ << __TEXT("PROCESS EXIT CALLED\n") << exit_msg << std::endl;

	exit(exit_code);

	while(true) delay_ms(16);
}

static void runtime_loop(void)
{
	int n_ret;
	int audio_status;
	struct pollfd poll_userinput;

	memset(&poll_userinput, 0, sizeof(struct pollfd));

	poll_userinput.fd = STDIN_FILENO;
	poll_userinput.events = POLLIN;

	__STDCOUT__ << __TEXT("Stream started\n");

	cmdui_print_help_text();

	audio_status = p_audio->getStatus();

	while(true)
	{
		if((audio_status < 1) || (audio_status == p_audio->STATUS_STOPPED)) break;

		n_ret = poll(&poll_userinput, 1, 1);
		if(n_ret > 0)
		{
			usercmd = __TEXT("");
			__STDCIN__ >> usercmd;
			cmdui_cmd_decode();
		}

		audio_status = p_audio->getStatus();
	}

	cppthread_wait(&audiothread);

	delay_ms(1024);
	__STDCOUT__ << __TEXT("Stream finished\n");
	return;
}

static bool mainarg_parse_sampling_rate(const char *text)
{
	int _sr;

	if(text == NULL) return false;

	try { _sr = std::stoi(text); }
	catch(...) { return false; }

	if(_sr <= 0) return false;

	stream_params.sample_rate = (size_t) _sr;
	return true;
}

static bool mainarg_parse_bit_depth(const char *text)
{
	int _bd;

	if(text == NULL) return false;

	try { _bd = std::stoi(text); }
	catch(...) { return false; }

	switch(_bd)
	{
		case 16:
			audio_format = __STREAM_I16;
			return true;

		case 24:
			audio_format = __STREAM_I24;
			return true;
	}

	return false;
}

static bool mainarg_parse_n_channels(const char *text)
{
	int _nc;

	if(text == NULL) return false;

	try { _nc = std::stoi(text); }
	catch(...) { return false; }

	if(_nc < 1) return false;

	stream_params.n_channels = (size_t) _nc;
	return true;
}

static bool mainarg_parse_enable_resampling(const char *text)
{
	int _er;

	if(text == NULL) return false;

	try { _er = std::stoi(text); }
	catch(...) { return false; }

	/*Since "bool" type is "int8_t" and "int" is "int32_t", I don't consider a good idea to simply do "enable_resampling = (bool) _er;". This could cause bugs in some compilers.*/

	if(_er) enable_resampling = true;
	else enable_resampling = false;

	return true;
}

static void cmdui_cmd_decode(void)
{
	const __tchar_t *cmd = NULL;
	const __tchar_t *errinv = __TEXT("Error: invalid command entered\n");
	const __tchar_t *paramupdated = __TEXT("Parameter Updated\n");

	ssize_t c_index;
	ssize_t n_fx;
	ssize_t n_delay;
	float f_amp;

	usercmd = str_tolower(usercmd);
	cmd = usercmd.c_str();

	if(cstr_compare(__TEXT("stop"), cmd))
	{
		p_audio->stopStream();
		return;
	}

	if(cstr_compare(__TEXT("pause"), cmd))
	{
		p_audio->pauseStream();
		__STDCOUT__ << __TEXT("Stream Paused\n");
		return;
	}

	if(cstr_compare(__TEXT("run"), cmd))
	{
		p_audio->resumeStream();
		__STDCOUT__ << __TEXT("Stream Resumed\n");
		return;
	}

	if(cstr_compare(__TEXT("params"), cmd))
	{
		cmdui_print_current_params();
		return;
	}

	if(cstr_compare(__TEXT("help"), cmd) || cstr_compare(__TEXT("--help"), cmd))
	{
		cmdui_print_help_text();
		return;
	}

	if(cstr_compare(__TEXT("reset"), cmd))
	{
		p_audio->delayResetFFParams();
		p_audio->delayResetFBParams();

		return;
	}

	if(cstr_compare(__TEXT("resetff"), cmd))
	{
		p_audio->delayResetFFParams();
		return;
	}

	if(cstr_compare(__TEXT("resetfb"), cmd))
	{
		p_audio->delayResetFBParams();
		return;
	}

	if(cstr_compare_upto_char(__TEXT("setdryamp"), cmd, ':', true))
	{
		if(!cmdui_parse_famp(&cmd[10], &f_amp)) return;

		if(p_audio->delaySetDryInputAmplitude(f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_char(__TEXT("setoutamp"), cmd, ':', true))
	{
		if(!cmdui_parse_famp(&cmd[10], &f_amp)) return;

		if(p_audio->delaySetOutputAmplitude(f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setffdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) __AUDIO_DELAY_N_FFCH);
		if(n_fx < 0) return;

		n_delay = cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(p_audio->delaySetFFDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setffamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) __AUDIO_DELAY_N_FFCH);
		if(n_fx < 0) return;

		if(!cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(p_audio->delaySetFFAmplitude((size_t) (n_fx - 1), f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setfbdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) __AUDIO_DELAY_N_FBCH);
		if(n_fx < 0) return;

		n_delay = cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(p_audio->delaySetFBDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setfbamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) __AUDIO_DELAY_N_FBCH);
		if(n_fx < 0) return;

		if(!cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(p_audio->delaySetFBAmplitude((size_t) (n_fx - 1), f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

_l_cmdui_cmd_decode_return_errinv:

	__STDCOUT__ << errinv;
	return;

_l_cmdui_cmd_decode_return_paramupdated:

	__STDCOUT__ << paramupdated;
	return;

_l_cmdui_cmd_decode_return_erraudio:

	__STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;
	return;
}

static void cmdui_print_help_text(void)
{
	__STDCOUT__ << __TEXT("User Command List:\n\n");
	__STDCOUT__ << __TEXT("\"help\" or \"--help\" : print this list\n");
	__STDCOUT__ << __TEXT("\"params\" : print current delay parameters\n");
	__STDCOUT__ << __TEXT("\"stop\" : stop stream and quit application\n");
	__STDCOUT__ << __TEXT("\"pause\" : pause stream (when running)\n");
	__STDCOUT__ << __TEXT("\"run\" : resume stream (when paused)\n");
	__STDCOUT__ << __TEXT("\"reset\" : reset all feedforward and feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"resetff\" : reset all feedforward delay channels\n");
	__STDCOUT__ << __TEXT("\"resetfb\" : reset all feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"setdryamp:<number>\" : set the amplitude for the dry input signal in the delay mix (does not affect feedforward delay)\n");
	__STDCOUT__ << __TEXT("\"setoutamp:<number>\" : set the mix output amplitude (will affect feedback delay)\n");
	__STDCOUT__ << __TEXT("\"setffdelay<x>:<number>\" : set the delay time (in number of samples) for a specific feedforward delay channel. \"number\" is the delay time and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(__AUDIO_DELAY_N_FFCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setffamp<x>:<number>\" : set the amplitude for a specific feedforward delay channel. \"number\" is the amplitude and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(__AUDIO_DELAY_N_FFCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbdelay<x>:<number>\" : set the delay time (in number of samples) for a specific feedback delay channel. \"number\" is the delay time and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(__AUDIO_DELAY_N_FBCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbamp<x>:<number>\" : set the amplitude for a specific feedback delay channel. \"number\" is the amplitude and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(__AUDIO_DELAY_N_FBCH) << __TEXT(")\n\n");

	__STDCOUT__ << __TEXT("Example commands:\n\"setffdelay1:220\"\n\"setfbamp1:-0.3\"\n\n");

	__STDCOUT__ << __TEXT("Remember: feedback delays are an infinite impulse response (IIR) circuit. Be very careful when setting it, as improper values can cause the signal to clip endlessly, causing a very loud and unpleasant noise.\n\n");
	return;
}

static void cmdui_print_current_params(void)
{
	size_t n_fx;
	uint32_t n_delay;
	float f_amp;

	__STDCOUT__ << __TEXT("Current Parameters:\n\n");

	f_amp = p_audio->delayGetDryInputAmplitude();
	__STDCOUT__ << __TEXT("Dry Input Amplitude: ") << __TOSTRING(f_amp);

	f_amp = p_audio->delayGetOutputAmplitude();
	__STDCOUT__ << __TEXT("\nOutput Amplitude: ") << __TOSTRING(f_amp) << __TEXT("\n\n");

	n_fx = 0u;
	while(n_fx < __AUDIO_DELAY_N_FFCH)
	{
		n_delay = (uint32_t) p_audio->delayGetFFDelay(n_fx);
		f_amp = p_audio->delayGetFFAmplitude(n_fx);

		__STDCOUT__ << __TEXT("FF Delay Channel ") << __TOSTRING(n_fx + 1u) << __TEXT(":\t");
		__STDCOUT__ << __TEXT("Delay Time (number of samples): ") << __TOSTRING(n_delay);
		__STDCOUT__ << __TEXT("\tDelay Amplitude: ") << __TOSTRING(f_amp) << std::endl;

		n_fx++;
	}

	__STDCOUT__ << __TEXT("\n");

	n_fx = 0u;
	while(n_fx < __AUDIO_DELAY_N_FBCH)
	{
		n_delay = (uint32_t) p_audio->delayGetFBDelay(n_fx);
		f_amp = p_audio->delayGetFBAmplitude(n_fx);

		__STDCOUT__ << __TEXT("FB Delay Channel ") << __TOSTRING(n_fx + 1u) << __TEXT(":\t");
		__STDCOUT__ << __TEXT("Delay Time (number of samples): ") << __TOSTRING(n_delay);
		__STDCOUT__ << __TEXT("\tDelay Amplitude: ") << __TOSTRING(f_amp) << std::endl;

		n_fx++;
	}

	__STDCOUT__ << std::endl;
	return;
}

static ssize_t cmdui_parse_nfx(const __tchar_t *text_nfx, ssize_t max)
{
	long val;

	if(text_nfx == NULL) return -1;
	if(max < 1) return -1;

	try{ val = std::stol(text_nfx); }
	catch(...) 
	{ 
		__STDCOUT__ << __TEXT("Error: invalid delay channel entered\n");
		return -1;
	}

	if((val < 1l) || (val > max))
	{
		__STDCOUT__ << __TEXT("Error: invalid delay channel entered\n");
		return -1;
	}

	return (ssize_t) val;
}

static ssize_t cmdui_parse_ndelay(const __tchar_t *text_ndelay)
{
	long n_delay;

	if(text_ndelay == NULL) return -1;

	try { n_delay = std::stol(text_ndelay); }
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid delay value entered\n");
		return -1;
	}

	if(n_delay < 0l)
	{
		__STDCOUT__ << __TEXT("Error: invalid delay value entered\n");
		return -1;
	}

	if(n_delay >= ((long) __AUDIO_DELAY_BUFFER_SIZE_FRAMES))
	{
		__STDCOUT__ << __TEXT("Error: entered delay value is too big\n");
		return -1;
	}

	return (ssize_t) n_delay;
}

static bool cmdui_parse_famp(const __tchar_t *text_famp, float *p_famp)
{
	float f_amp;

	if(text_famp == NULL) return false;
	if(p_famp == NULL) return false;

	try { f_amp = std::stof(text_famp); }
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid amplitude value entered\n");
		return false;
	}

	*p_famp = f_amp;
	return true;
}

static void audiothread_proc(void)
{
	p_audio->runStream();
	return;
}

