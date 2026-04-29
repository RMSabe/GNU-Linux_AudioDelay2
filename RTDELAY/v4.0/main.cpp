/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"

#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define RTDELAY_BUFFER_SIZE_FRAMES 65536U
#define RTDELAY_N_FFCH 4U
#define RTDELAY_N_FBCH 4U

#define PB_I16 1
#define PB_I24 2

static __attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;
static __attribute__((__aligned__(PTR_SIZE_BITS))) const char *audio_dev_desc = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) std::thread audiothread;
static __attribute__((__aligned__(PTR_SIZE_BITS))) __string usercmd = __TEXT("");

static __attribute__((__aligned__(32))) int h_filein = -1;

static void app_deinit(void);
static void runtime_loop(void);

static bool filein_ext_check(void);
static bool filein_open(void);
static void filein_close(void);

static int filein_get_params(void);
static bool compare_signature(const char *auth, const uint8_t *buf);

static void cmdui_cmd_decode(void);
static void cmdui_print_help_text(void);
static void cmdui_print_current_params(void);
static ssize_t cmdui_parse_nfx(const __tchar_t *text_nfx, ssize_t max);
static ssize_t cmdui_parse_ndelay(const __tchar_t *text_ndelay);
static bool cmdui_parse_famp(const __tchar_t *text_famp, float *p_famp);
static __offset_t cmdui_parse_position(const __tchar_t *text_pos);

static void audiothread_proc(void);

int main(int argc, char **argv)
{
	int n_ret = 0;

	if(argc < 3)
	{
		__STDCOUT__ << __TEXT("Error: missing arguments\nThis executable requires 2 arguments: <output device id> <input file directory>\nThey must be in that order\n");
		return -1;
	}

	audio_dev_desc = argv[1];
	pb_params.file_dir = argv[2];
	pb_params.rtdelay_buffer_size_frames = RTDELAY_BUFFER_SIZE_FRAMES;
	pb_params.rtdelay_n_ff_delays = RTDELAY_N_FFCH;
	pb_params.rtdelay_n_fb_delays = RTDELAY_N_FBCH;

	if(!filein_ext_check())
	{
		__STDCOUT__ << __TEXT("Error: bad file extension\n");
		goto _l_main_error;
	}

	if(!filein_open())
	{
		__STDCOUT__ << __TEXT("Error: failed to open file\n");
		goto _l_main_error;
	}

	n_ret = filein_get_params();
	if(n_ret < 0) goto _l_main_error;

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
		__STDCOUT__ << __TEXT("Error: audio object instance failed.\n");
		goto _l_main_error;
	}

	p_audio->chooseDevice(audio_dev_desc);

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

	__STDCOUT__ << __TEXT("Playback started\n");

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

	__STDCOUT__ << __TEXT("Playback finished\n");
	return;
}

static bool filein_ext_check(void)
{
	size_t len = 0u;

	if(pb_params.file_dir == NULL) return false;

	cstr_copy_char_to_tchar(pb_params.file_dir, textbuf, TEXTBUF_SIZE_CHARS);

	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);

	len = (size_t) cstr_getlength(textbuf);

	if(len >= 5u)
		if(cstr_compare(__TEXT(".wav"), &textbuf[len - 4u])) return true;

	while(true)
	{
		__STDCOUT__ << __TEXT("WARNING: The given file name does not have a \".wav\" extension. Might be incompatible with the application\nDo you wish to continue? (yes/no): ");

		usercmd = __TEXT("");
		__STDCIN__ >> usercmd;

		usercmd = str_tolower(usercmd);

		if(!usercmd.compare(__TEXT("yes"))) return true;
		if(!usercmd.compare(__TEXT("no"))) return false;

		__STDCOUT__ << __TEXT("Error: invalid command entered\n");
	}

	return false;
}

static bool filein_open(void)
{
	if(pb_params.file_dir == NULL) return false;

	filein_close();

	h_filein = open(pb_params.file_dir, O_RDONLY);

	return (h_filein >= 0);
}

static void filein_close(void)
{
	if(h_filein < 0) return;

	close(h_filein);
	h_filein = -1;
	return;
}

static int filein_get_params(void)
{
	const uintptr_t BUFFER_SIZE = 8192u;
	uintptr_t buffer_index = 0u;

	uint8_t *p_headerinfo = NULL;

	uint16_t u16 = 0u;
	uint32_t u32 = 0u;

	uint16_t bit_depth = 0u;

	p_headerinfo = (uint8_t*) malloc(BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: memory allocate failed.\n");
		goto _l_filein_get_params_error;
	}

	memset(p_headerinfo, 0, BUFFER_SIZE);

	__LSEEK(h_filein, 0, SEEK_SET);
	read(h_filein, p_headerinfo, BUFFER_SIZE);
	filein_close();

	if(!compare_signature("RIFF", p_headerinfo))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: file format not supported.\n");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const uint8_t*) (((uintptr_t) p_headerinfo) + 8u)))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: file format not supported.\n");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			__STDCOUT__ << __TEXT("filein_get_params: Error: broken header (subchunk \"fmt \" not found).\nFile probably corrupted.\n");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const uint8_t*) (((uintptr_t) p_headerinfo) + buffer_index))) break;

		u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));

		buffer_index += (uintptr_t) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: broken header (error on subchunk \"fmt \").\nFile probably corrupted.\n");
		goto _l_filein_get_params_error;
	}

	u16 = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 8u));

	if(u16 != 1u)
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: audio encoding format not supported.\n");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 10u));
	pb_params.sample_rate = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 12u));
	bit_depth = *((uint16_t*) (((uintptr_t) p_headerinfo) + buffer_index + 22u));

	u32 = *((uint32_t*) (((uintptr_t) p_headerinfo) + buffer_index + 4u));

	buffer_index += (uintptr_t) (u32 + 8u);

	while(true)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			__STDCOUT__ << __TEXT("filein_get_params: Error: broken header (subchunk \"data\" not found).\nFile probably corrupted.\n");
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

	__STDCOUT__ << __TEXT("filein_get_params: Error: audio format not supported.\n");

_l_filein_get_params_error:

	filein_close();
	if(p_headerinfo != NULL) free(p_headerinfo);
	return -1;
}

static bool compare_signature(const char *auth, const uint8_t *buf)
{
	size_t nchar;

	if(auth == NULL) return false;
	if(buf == NULL) return false;

	for(nchar = 0u; nchar < 4u; nchar++) if(auth[nchar] != ((char) buf[nchar])) return false;

	return true;
}

static void cmdui_cmd_decode(void)
{
	const __tchar_t *cmd = NULL;
	const __tchar_t *errinv = __TEXT("Error: invalid command entered\n");
	const __tchar_t *paramupdated = __TEXT("Parameter Updated\n");

	__offset_t n_off;

	ssize_t c_index;
	ssize_t n_fx;
	ssize_t n_delay;
	float f_amp;

	usercmd = str_tolower(usercmd);
	cmd = usercmd.c_str();

	if(cstr_compare(__TEXT("stop"), cmd))
	{
		p_audio->stopPlayback();
		return;
	}

	if(cstr_compare(__TEXT("pause"), cmd))
	{
		p_audio->pausePlayback();
		__STDCOUT__ << __TEXT("Playback Paused\n");
		return;
	}

	if(cstr_compare(__TEXT("play"), cmd))
	{
		p_audio->resumePlayback();
		__STDCOUT__ << __TEXT("Playback Resumed\n");
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

	if(cstr_compare(__TEXT("getsize"), cmd))
	{
		n_off = p_audio->getAudioDataSizeFrames();
		if(n_off < 0) goto _l_cmdui_cmd_decode_return_erraudio;

		__STDCOUT__ << __TEXT("Data Size: ") << __TOSTRING(n_off) << __TEXT(" frames\n");
		return;
	}

	if(cstr_compare(__TEXT("getpos"), cmd))
	{
		n_off = p_audio->getAudioDataPositionFrames();
		if(n_off < 0) goto _l_cmdui_cmd_decode_return_erraudio;
		else __STDCOUT__ << __TEXT("Current Data Position: ") << __TOSTRING(n_off) << __TEXT(" frames\n");

		return;
	}

	if(cstr_compare(__TEXT("reset"), cmd))
	{
		p_audio->rtdelayResetFFParams();
		p_audio->rtdelayResetFBParams();

		return;
	}

	if(cstr_compare(__TEXT("resetff"), cmd))
	{
		p_audio->rtdelayResetFFParams();
		return;
	}

	if(cstr_compare(__TEXT("resetfb"), cmd))
	{
		p_audio->rtdelayResetFBParams();
		return;
	}

	if(cstr_compare_upto_char(__TEXT("setpos"), cmd, ':', true))
	{
		n_off = cmdui_parse_position(&cmd[7]);
		if(n_off < 0) return;

		if(p_audio->setAudioDataPositionFrames(n_off)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_char(__TEXT("setdryamp"), cmd, ':', true))
	{
		if(!cmdui_parse_famp(&cmd[10], &f_amp)) return;

		if(p_audio->rtdelaySetDryInputAmplitude(f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_char(__TEXT("setoutamp"), cmd, ':', true))
	{
		if(!cmdui_parse_famp(&cmd[10], &f_amp)) return;

		if(p_audio->rtdelaySetOutputAmplitude(f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setffdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) RTDELAY_N_FFCH);
		if(n_fx < 0) return;

		n_delay = cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(p_audio->rtdelaySetFFDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setffamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) RTDELAY_N_FFCH);
		if(n_fx < 0) return;

		if(!cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(p_audio->rtdelaySetFFAmplitude((size_t) (n_fx - 1), f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setfbdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) RTDELAY_N_FBCH);
		if(n_fx < 0) return;

		n_delay = cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(p_audio->rtdelaySetFBDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) goto _l_cmdui_cmd_decode_return_paramupdated;

		goto _l_cmdui_cmd_decode_return_erraudio;
	}

	if(cstr_compare_upto_len(__TEXT("setfbamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0) goto _l_cmdui_cmd_decode_return_errinv;

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = cmdui_parse_nfx(textbuf, (ssize_t) RTDELAY_N_FBCH);
		if(n_fx < 0) return;

		if(!cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(p_audio->rtdelaySetFBAmplitude((size_t) (n_fx - 1), f_amp)) goto _l_cmdui_cmd_decode_return_paramupdated;

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
	__STDCOUT__ << __TEXT("\"stop\" : stop playback and quit application\n");
	__STDCOUT__ << __TEXT("\"pause\" : pause playback (when playing)\n");
	__STDCOUT__ << __TEXT("\"play\" : resume playback (when paused)\n");
	__STDCOUT__ << __TEXT("\"getsize\" : print the size of audio data (in number of frames)\n");
	__STDCOUT__ << __TEXT("\"getpos\" : print the current position of audio data (in number of frames)\n");
	__STDCOUT__ << __TEXT("\"setpos:<number>\" : set the current position of audio data (in number of frames)\n");
	__STDCOUT__ << __TEXT("\"reset\" : reset all feedforward and feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"resetff\" : reset all feedforward delay channels\n");
	__STDCOUT__ << __TEXT("\"resetfb\" : reset all feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"setdryamp:<number>\" : set the amplitude for the dry input signal in the delay mix (does not affect feedforward delay)\n");
	__STDCOUT__ << __TEXT("\"setoutamp:<number>\" : set the mix output amplitude (will affect feedback delay)\n");
	__STDCOUT__ << __TEXT("\"setffdelay<x>:<number>\" : set the delay time (in number of samples) for a specific feedforward delay channel. \"number\" is the delay time and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(RTDELAY_N_FFCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setffamp<x>:<number>\" : set the amplitude for a specific feedforward delay channel. \"number\" is the amplitude and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(RTDELAY_N_FFCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbdelay<x>:<number>\" : set the delay time (in number of samples) for a specific feedback delay channel. \"number\" is the delay time and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(RTDELAY_N_FBCH) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbamp<x>:<number>\" : set the amplitude for a specific feedback delay channel. \"number\" is the amplitude and \"x\" is the delay channel. (valid x values are 1 to ") << __TOSTRING(RTDELAY_N_FBCH) << __TEXT(")\n\n");

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

	f_amp = p_audio->rtdelayGetDryInputAmplitude();
	__STDCOUT__ << __TEXT("Dry Input Amplitude: ") << __TOSTRING(f_amp);

	f_amp = p_audio->rtdelayGetOutputAmplitude();
	__STDCOUT__ << __TEXT("\nOutput Amplitude: ") << __TOSTRING(f_amp) << __TEXT("\n\n");

	n_fx = 0u;
	while(n_fx < RTDELAY_N_FFCH)
	{
		n_delay = (uint32_t) p_audio->rtdelayGetFFDelay(n_fx);
		f_amp = p_audio->rtdelayGetFFAmplitude(n_fx);

		__STDCOUT__ << __TEXT("FF Delay Channel ") << __TOSTRING(n_fx + 1u) << __TEXT(":\t");
		__STDCOUT__ << __TEXT("Delay Time (number of samples): ") << __TOSTRING(n_delay);
		__STDCOUT__ << __TEXT("\tDelay Amplitude: ") << __TOSTRING(f_amp) << std::endl;

		n_fx++;
	}

	__STDCOUT__ << __TEXT("\n");

	n_fx = 0u;
	while(n_fx < RTDELAY_N_FBCH)
	{
		n_delay = (uint32_t) p_audio->rtdelayGetFBDelay(n_fx);
		f_amp = p_audio->rtdelayGetFBAmplitude(n_fx);

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

	try
	{
		val = std::stol(text_nfx);
	}
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

	try
	{
		n_delay = std::stol(text_ndelay);
	}
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

	if(n_delay >= ((long) RTDELAY_BUFFER_SIZE_FRAMES))
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

	try
	{
		f_amp = std::stof(text_famp);
	}
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid amplitude value entered\n");
		return false;
	}

	*p_famp = f_amp;
	return true;
}

static __offset_t cmdui_parse_position(const __tchar_t *text_pos)
{
	__offset_t n_pos;

	if(text_pos == NULL) return -1;

	try
	{
		n_pos = (__offset_t) std::stoll(text_pos);
	}
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid position value entered\n");
		return -1;
	}

	if(n_pos < 0)
	{
		__STDCOUT__ << __TEXT("Error: invalid position value entered\n");
		return -1;
	}

	return n_pos;
}

static void audiothread_proc(void)
{
	p_audio->runPlayback();
	return;
}

