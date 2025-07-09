/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 1.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioRTDelay.hpp"
#include "cstrdef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <iostream>

AudioRTDelay::AudioRTDelay(const audiortdelay_init_params_t *p_params)
{
	this->setInitParameters(p_params);
}

bool AudioRTDelay::setInitParameters(const audiortdelay_init_params_t *p_params)
{
	if(this->status == this->STATUS_INITIALIZED) return false;

	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL) return false;
	if(p_params->file_dir == NULL) return false;
	if(p_params->audio_dev_desc == NULL) return false;

	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIO_DEV_DESC = p_params->audio_dev_desc;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = (size_t) p_params->n_channels;

	this->BUFFER_SIZE_FRAMES = p_params->buffer_size_frames;
	this->P_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->P_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	this->P_FF_PARAMS_SIZE = (this->P_FF_PARAMS_LENGTH)*sizeof(audiortdelay_fx_params_t);
	this->P_FB_PARAMS_SIZE = (this->P_FB_PARAMS_LENGTH)*sizeof(audiortdelay_fx_params_t);

	return true;
}

bool AudioRTDelay::initialize(void)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(this->BUFFER_SIZE_FRAMES < this->BUFFER_SIZE_FRAMES_MIN)
	{
		this->err_msg = "AudioRTDelay::initialize: Error: BUFFER SIZE FRAMES value is invalid.";
		return false;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->err_msg = "AudioRTDelay::initialize: Error: N CHANNELS value is invalid.";
		return false;
	}

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = "AudioRTDelay::initialize: Error: file could not be opened.";
		return false;
	}

	if(!this->audio_hw_init())
	{
		this->filein_close();
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = "AudioRTDelay::initialize: Error: audio hardware init failed.";
		return false;
	}

	if(this->BUFFER_N_SEGMENTS < this->BUFFER_N_SEGMENTS_MIN)
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->err_msg = "AudioRTDelay::initialize: Error: BUFFER N SEGMENTS value is invalid.";
		return false;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = "AudioPB::initialize: Error: memory allocate failed.";
		return false;
	}

	this->status = this->STATUS_INITIALIZED;
	return true;
}

bool AudioRTDelay::runPlayback(void)
{
	if(this->status < 1) return false;

	std::cout << "Playback started\n\n";
	this->playback_proc();
	std::cout << "Playback finished\n\n";

	this->wait_all_threads();
	this->stop_all_threads();

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return true;
}

std::string AudioRTDelay::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return ("Error: AudioRTDelay object not initialized\nExtended error message: " + this->err_msg);

	return this->err_msg;
}

bool AudioRTDelay::filein_open(void)
{
	this->filein_close(); /*Close any existing open handles*/

	this->h_filein = open(this->FILEIN_DIR.c_str(), O_RDONLY);
	if(this->h_filein < 0) return false;

	this->filein_size = __LSEEK(this->h_filein, 0, SEEK_END);
	return true;
}

void AudioRTDelay::filein_close(void)
{
	if(this->h_filein < 0) return;

	close(this->h_filein);
	this->h_filein = -1;
	this->filein_size = 0;

	return;
}

void AudioRTDelay::audio_hw_deinit(void)
{
	if(this->p_audiodev == NULL) return;

	snd_pcm_drain(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);

	this->p_audiodev = NULL;
	return;
}

void AudioRTDelay::wait_all_threads(void)
{
	cppthread_wait(&(this->loadthread));
	cppthread_wait(&(this->playthread));
	cppthread_wait(&(this->userthread));

	return;
}

void AudioRTDelay::stop_all_threads(void)
{
	cppthread_stop(&(this->loadthread));
	cppthread_stop(&(this->playthread));
	cppthread_stop(&(this->userthread));

	return;
}

bool AudioRTDelay::buffer_fxparams_alloc(void)
{
	this->buffer_fxparams_free(); /*Clear any previous fxparams allocations*/

	if(this->P_FF_PARAMS_LENGTH)
	{
		this->p_ff_params = (audiortdelay_fx_params_t*) malloc(this->P_FF_PARAMS_SIZE);
		if(this->p_ff_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);
	}

	if(this->P_FB_PARAMS_LENGTH)
	{
		this->p_fb_params = (audiortdelay_fx_params_t*) malloc(this->P_FB_PARAMS_SIZE);
		if(this->p_fb_params == NULL)
		{
			this->buffer_fxparams_free();
			return false;
		}

		memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);
	}

	return true;
}

void AudioRTDelay::buffer_fxparams_free(void)
{
	if(this->p_ff_params != NULL)
	{
		free(this->p_ff_params);
		this->p_ff_params = NULL;
	}

	if(this->p_fb_params != NULL)
	{
		free(this->p_fb_params);
		this->p_fb_params = NULL;
	}

	return;
}

void AudioRTDelay::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();
	return;
}

void AudioRTDelay::playback_init(void)
{
	this->n_loadsegment = 0u;
	this->n_playsegment = this->BUFFER_N_SEGMENTS/2u;

	this->stop = false;
	this->filein_pos = this->AUDIO_DATA_BEGIN;

	memset(this->p_ff_params, 0, this->P_FF_PARAMS_SIZE);
	memset(this->p_fb_params, 0, this->P_FB_PARAMS_SIZE);

	this->userthread = std::thread(&AudioRTDelay::userthread_proc, this);
	return;
}

void AudioRTDelay::playback_loop(void)
{
	while(!this->stop)
	{
		this->playthread = std::thread(&AudioRTDelay::playthread_proc, this);
		this->loadthread = std::thread(&AudioRTDelay::loadthread_proc, this);
		cppthread_wait(&(this->loadthread));
		cppthread_wait(&(this->playthread));

		this->buffer_segment_remap();
	}

	return;
}

void AudioRTDelay::buffer_segment_remap(void)
{
	this->n_loadsegment++;
	this->n_loadsegment %= this->BUFFER_N_SEGMENTS;

	this->n_playsegment++;
	this->n_playsegment %= this->BUFFER_N_SEGMENTS;

	return;
}

void AudioRTDelay::buffer_play(void)
{
	int n_ret = 0;

	n_ret = snd_pcm_writei(this->p_audiodev, this->pp_bufferoutput_segments[this->n_playsegment], this->BUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret == -EPIPE) snd_pcm_prepare(this->p_audiodev);

	return;
}

bool AudioRTDelay::retrieve_prev_nframe(size_t curr_buf_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
{
	size_t prev_buf_nframe = 0u;
	size_t prev_nseg = 0u;
	size_t prev_seg_nframe = 0u;

	if(curr_buf_nframe >= this->BUFFER_SIZE_FRAMES) return false;
	if(n_delay >= this->BUFFER_SIZE_FRAMES) return false;

	if(n_delay > curr_buf_nframe) prev_buf_nframe = this->BUFFER_SIZE_FRAMES - (n_delay - curr_buf_nframe);
	else prev_buf_nframe = curr_buf_nframe - n_delay;

	prev_nseg = prev_buf_nframe/this->BUFFER_SEGMENT_SIZE_FRAMES;
	prev_seg_nframe = prev_buf_nframe%this->BUFFER_SEGMENT_SIZE_FRAMES;

	if(p_prev_buf_nframe != NULL) *p_prev_buf_nframe = prev_buf_nframe;
	if(p_prev_nseg != NULL) *p_prev_nseg = prev_nseg;
	if(p_prev_seg_nframe != NULL) *p_prev_seg_nframe = prev_seg_nframe;

	return true;
}

bool AudioRTDelay::retrieve_prev_nframe(size_t curr_nseg, size_t curr_seg_nframe, size_t n_delay, size_t *p_prev_buf_nframe, size_t *p_prev_nseg, size_t *p_prev_seg_nframe)
{
	size_t curr_buf_nframe = 0u;

	if(curr_nseg >= this->BUFFER_N_SEGMENTS) return false;
	if(curr_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return false;

	curr_buf_nframe = curr_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + curr_seg_nframe;

	return this->retrieve_prev_nframe(curr_buf_nframe, n_delay, p_prev_buf_nframe, p_prev_nseg, p_prev_seg_nframe);
}

void AudioRTDelay::loadthread_proc(void)
{
	this->buffer_load();
	if(this->stop) return;

	this->run_dsp();
	return;
}

void AudioRTDelay::playthread_proc(void)
{
	this->buffer_play();
	return;
}

void AudioRTDelay::userthread_proc(void)
{
	int n_ret = 0;
	struct pollfd poll_userinput;

	memset(&poll_userinput, 0, sizeof(struct pollfd));

	poll_userinput.fd = STDIN_FILENO;
	poll_userinput.events = POLLIN;

	this->cmdui_print_help_text();

	while(!this->stop)
	{
		n_ret = poll(&poll_userinput, 1, 1);

		if(n_ret > 0)
		{
			this->user_cmd = "";
			std::cin >> this->user_cmd;
			this->cmdui_cmd_decode();
		}
	}

	return;
}

/*
 * Here begins the UI portion of the code.
 * I really don't like/am not good with developing UIs, so bear with me.
 *
 * commands for setting fx parameters look something like this: setffdelay1:10000 (set ff delay channel 1 to 10000 samples)
 *
 * The delay channel is refered to as "n_fx".
 * The value is refered to as "value" or "val".
 * The command line portion relative to the "n_fx" is "cmdargnfx".
 * The command line portion relative to the "value" is "cmdargval".
 *
 * The whole command line string is refered to as "cmd" or "cmdline".
 *
 * I know it's confusing... sorry...
 */

void AudioRTDelay::cmdui_cmd_decode(void)
{
	const char *errinv = "Error: invalid command entered\n";
	const char *cmd = NULL;

	size_t n_fx = 0u;
	size_t n_cmdargvalindex = 0u;

	this->user_cmd = str_tolower(this->user_cmd);
	cmd = this->user_cmd.c_str();

	if(cstr_compare("stop", cmd))
	{
		this->stop = true;
		return;
	}

	if(cstr_compare("params", cmd))
	{
		this->cmdui_print_current_params();
		return;
	}

	if(cstr_compare("help", cmd) || cstr_compare("--help", cmd))
	{
		this->cmdui_print_help_text();
		return;
	}

	if(this->cmdui_cmd_compare("setffdelay", cmd, 10u))
	{
		if(!this->cmdui_get_nfx_from_cmd(&cmd[10u], &n_fx, &n_cmdargvalindex))
		{
			std::cout << errinv;
			return;
		}

		n_cmdargvalindex += 10u;
		this->cmdui_attempt_update_ffdelay(&cmd[n_cmdargvalindex], n_fx);
		return;
	}

	if(this->cmdui_cmd_compare("setffamp", cmd, 8u))
	{
		if(!this->cmdui_get_nfx_from_cmd(&cmd[8u], &n_fx, &n_cmdargvalindex))
		{
			std::cout << errinv;
			return;
		}

		n_cmdargvalindex += 8u;
		this->cmdui_attempt_update_ffamp(&cmd[n_cmdargvalindex], n_fx);
		return;
	}

	if(this->cmdui_cmd_compare("setfbdelay", cmd, 10u))
	{
		if(!this->cmdui_get_nfx_from_cmd(&cmd[10u], &n_fx, &n_cmdargvalindex))
		{
			std::cout << errinv;
			return;
		}

		n_cmdargvalindex += 10u;
		this->cmdui_attempt_update_fbdelay(&cmd[n_cmdargvalindex], n_fx);
		return;
	}

	if(this->cmdui_cmd_compare("setfbamp", cmd, 8u))
	{
		if(!this->cmdui_get_nfx_from_cmd(&cmd[8u], &n_fx, &n_cmdargvalindex))
		{
			std::cout << errinv;
			return;
		}

		n_cmdargvalindex += 8u;
		this->cmdui_attempt_update_fbamp(&cmd[n_cmdargvalindex], n_fx);
		return;
	}

	std::cout << errinv;
	return;
}

/*
 * cmdui_cmd_compare()
 * behaves similarly to cstr_compare, but it can be set to compare only a portion of the text rather than the whole text
 */

bool AudioRTDelay::cmdui_cmd_compare(const char *auth, const char *input, size_t stop_index)
{
	if(auth == NULL) return false;
	if(input == NULL) return false;
	if(stop_index >= TEXTBUF_SIZE_CHARS) return false;

	snprintf(textbuf, TEXTBUF_SIZE_CHARS, "%s", input);

	textbuf[stop_index] = '\0';

	return cstr_compare(auth, textbuf);
}

void AudioRTDelay::cmdui_print_help_text(void)
{
	std::cout << "User Command List:\n\n";
	std::cout << "\"help\" or \"--help\" : print this list\n";
	std::cout << "\"params\" : print current delay parameters\n";
	std::cout << "\"setffdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedforward delay channel \"x\" (valid x values are 1 to " << std::to_string(this->P_FF_PARAMS_LENGTH) << ")\n";
	std::cout << "\"setffamp<x>:<number>\" : \"number\" set the amplitude of feedforward delay channel \"x\" (valid x values are 1 to " << std::to_string(this->P_FF_PARAMS_LENGTH) << ")\n";
	std::cout << "\"setfbdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedback delay channel \"x\" (valid x values are 1 to " << std::to_string(this->P_FB_PARAMS_LENGTH) << ")\n";
	std::cout << "\"setfbamp<x>:<number>\" : \"number\" set the amplitude of feedback delay channel \"x\" (valid x values are 1 to " << std::to_string(this->P_FB_PARAMS_LENGTH) << ")\n";
	std::cout << "\"stop\" : stop playback and quit application\n\n";

	std::cout << "Example commands:\n\"setffdelay1:220\"\n\"setffamp1:0.3\"\n\n";

	std::cout << "Remember: feedback delays are an infinite impulse response (IIR) circuit. Be very careful when setting it, as improper values can cause the signal to clip endlessly, causing a very loud and unpleasant noise.\n\n";
	return;
}

void AudioRTDelay::cmdui_print_current_params(void)
{
	size_t n_fx = 0u;
	uint32_t n_delay = 0u;
	float f_amp = 0.0f;

	std::cout << "Current Parameters:\n\n";

	n_fx = 0u;
	while(n_fx < this->P_FF_PARAMS_LENGTH)
	{
		n_delay = this->p_ff_params[n_fx].delay;
		f_amp = this->p_ff_params[n_fx].amp;

		std::cout << "Feedforward delay " << std::to_string(n_fx + 1u) << ":\n";
		std::cout << "Delay time (number of samples): " << std::to_string(n_delay) << std::endl;
		std::cout << "Delay amplitude: " << std::to_string(f_amp) << "\n\n";
		n_fx++;
	}

	n_fx = 0u;
	while(n_fx < this->P_FB_PARAMS_LENGTH)
	{
		n_delay = this->p_fb_params[n_fx].delay;
		f_amp = this->p_fb_params[n_fx].amp;

		std::cout << "Feedback delay " << std::to_string(n_fx + 1u) << ":\n";
		std::cout << "Delay time (number of samples): " << std::to_string(n_delay) << std::endl;
		std::cout << "Delay amplitude: " << std::to_string(f_amp) << "\n\n";
		n_fx++;
	}

	return;
}

/*
 * cmdui_get_nfx_from_cmd:
 * Retrieve the "n_fx" value from the cmd line, and the start index for the next value string.
 *
 * Input:
 *
 * cmdargnfx: the cmd line buffer at the starting index of the "n_fx" argument.
 * Example: "setffdelay" is 10 characters long, so the "n_fx" string begins at character index 10. "cmdargnfx" should be set to "&cmdline[10]".
 *
 * Outputs:
 *
 * p_nfx: Pointer to variable that receives the "n_fx" value. Set to NULL if unused.
 *
 * p_cmdargvalrelindex: Pointer to variable that receives the relative index of "cmdargval".
 * Remember: "p_cmdargvalrelindex" receives the index RELATIVE to "cmdargnfx". To get the absolute index, "p_cmdargvalrelindex" value must be summed to "cmdargnfx" offset value.
 * Example: "setffdelay" is 10 characters long, so "cmdargnfx" will have and offset of 10 ("&cmdline[10]"). The index/offset of the "value" string ("cmdargval") should be "&cmdline[cmdargvalrelindex + 10]"
 */

bool AudioRTDelay::cmdui_get_nfx_from_cmd(const char *cmdargnfx, size_t *p_nfx, size_t *p_cmdargvalrelindex)
{
	size_t n_len = 0u;
	size_t n_char = 0u;
	int n32 = 0;

	if(cmdargnfx == NULL) return false;
	if(n_len >= TEXTBUF_SIZE_CHARS) return false;

	n_len = (size_t) cstr_getlength(cmdargnfx);

	n_char = 0u;
	while(n_char < n_len)
	{
		if(cmdargnfx[n_char] == ':') break;
		n_char++;
	}

	if(n_char >= n_len) return false;

	if(p_cmdargvalrelindex != NULL) *p_cmdargvalrelindex = (n_char + 1u);

	snprintf(textbuf, TEXTBUF_SIZE_CHARS, "%s", cmdargnfx);
	textbuf[n_char] = '\0';

	try
	{
		n32 = std::stoi(textbuf);
	}
	catch(...)
	{
		return false;
	}

	if(n32 < 0) return false;

	if(p_nfx != NULL) *p_nfx = (size_t) n32;
	return true;
}

/*
 * cmdui_attempt_update_...()
 *
 * Attempts to update the delay/amplitude value for a given feedforward/feedback channel.
 *
 * Inputs:
 * cmdargval: pointer to the "value" string.
 * n_fx: feedforward/feedback delay channel to be set.
 *
 * returns true if successful, false otherwise.
 */

bool AudioRTDelay::cmdui_attempt_update_ffdelay(const char *cmdargval, size_t n_fx)
{
	int n_delay = 0;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->P_FF_PARAMS_LENGTH))
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	try
	{
		n_delay = std::stoi(cmdargval);
	}
	catch(...)
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	if(n_delay < 0)
	{
		std::cout << "Error: invalid value entered\n";
		return false;
	}

	if(((size_t) n_delay) >= this->BUFFER_SIZE_FRAMES)
	{
		std::cout << "Error: given delay value is too big.\n";
		return false;
	}

	this->p_ff_params[n_fx - 1u].delay = (uint32_t) n_delay;

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioRTDelay::cmdui_attempt_update_ffamp(const char *cmdargval, size_t n_fx)
{
	float f_amp = 0.0f;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->P_FF_PARAMS_LENGTH))
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	try
	{
		f_amp = std::stof(cmdargval);
	}
	catch(...)
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	this->p_ff_params[n_fx - 1u].amp = f_amp;

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioRTDelay::cmdui_attempt_update_fbdelay(const char *cmdargval, size_t n_fx)
{
	int n_delay = 0;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->P_FB_PARAMS_LENGTH))
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	try
	{
		n_delay = std::stoi(cmdargval);
	}
	catch(...)
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	if(n_delay < 0)
	{
		std::cout << "Error: invalid value entered\n";
		return false;
	}

	if(((size_t) n_delay) >= this->BUFFER_SIZE_FRAMES)
	{
		std::cout << "Error: given delay value is too big.\n";
		return false;
	}

	this->p_fb_params[n_fx - 1u].delay = (uint32_t) n_delay;

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioRTDelay::cmdui_attempt_update_fbamp(const char *cmdargval, size_t n_fx)
{
	float f_amp = 0.0f;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->P_FB_PARAMS_LENGTH))
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	try
	{
		f_amp = std::stof(cmdargval);
	}
	catch(...)
	{
		std::cout << "Error: invalid command entered\n";
		return false;
	}

	this->p_fb_params[n_fx - 1u].amp = f_amp;

	std::cout << "Parameter updated\n";
	return true;
}


