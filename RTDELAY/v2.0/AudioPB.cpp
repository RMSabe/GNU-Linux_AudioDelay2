/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 2.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include "cstrdef.h"

#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <iostream>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

bool AudioPB::setParameters(const audiopb_params_t *p_params)
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
	this->N_CHANNELS = p_params->n_channels;

	return true;
}

bool AudioPB::initialize(void)
{
	audiortdelay_init_params_t delay_params;

	if(this->status == this->STATUS_INITIALIZED) return true;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = "AudioPB::initialize: Error: file could not be opened.";
		return false;
	}

	if(!this->audio_hw_init())
	{
		this->filein_close();
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = "AudioPB::initialize: Error: audio hardware init failed.";
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

	delay_params.buffer_size_frames = this->AUDIORTDELAY_BUFFER_SIZE_FRAMES;
	delay_params.buffer_n_segments = this->BUFFER_N_SEGMENTS;
	delay_params.n_channels = (size_t) this->N_CHANNELS;
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
			this->err_msg = "AudioPB::initialize: Error: failed to allocate delay object.";
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

	this->status = this->STATUS_INITIALIZED;
	return true;
}

bool AudioPB::runPlayback(void)
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

std::string AudioPB::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return ("Error: AudioPB object not initialized\nExtended error message: " + this->err_msg);

	return this->err_msg;
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

	snd_pcm_drain(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);

	this->p_audiodev = NULL;
	return;
}

void AudioPB::wait_all_threads(void)
{
	cppthread_wait(&(this->loadthread));
	cppthread_wait(&(this->playthread));
	cppthread_wait(&(this->userthread));

	return;
}

void AudioPB::stop_all_threads(void)
{
	cppthread_stop(&(this->loadthread));
	cppthread_stop(&(this->playthread));
	cppthread_stop(&(this->userthread));

	return;
}

void AudioPB::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();
	return;
}

void AudioPB::playback_init(void)
{
	this->n_segment = 0u;
	this->stop = false;
	this->filein_pos = this->AUDIO_DATA_BEGIN;
	this->buffer_remap();

	this->userthread = std::thread(&AudioPB::userthread_proc, this);
	return;
}

void AudioPB::playback_loop(void)
{
	while(!this->stop)
	{
		this->playthread = std::thread(&AudioPB::playthread_proc, this);
		this->loadthread = std::thread(&AudioPB::loadthread_proc, this);
		cppthread_wait(&(this->loadthread));
		cppthread_wait(&(this->playthread));

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
	int n_ret = 0;

	n_ret = snd_pcm_writei(this->p_audiodev, this->p_playoutput, this->AUDIOBUFFER_SIZE_FRAMES);
	if(n_ret == -EPIPE) snd_pcm_prepare(this->p_audiodev);

	return;
}

void AudioPB::loadthread_proc(void)
{
	this->buffer_load_in();
	if(this->stop) return;

	this->p_delay->runDSP(this->n_segment);
	this->buffer_load_out();

	this->buffer_segment_update();
	return;
}

void AudioPB::playthread_proc(void)
{
	this->buffer_play();
	return;
}

void AudioPB::userthread_proc(void)
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

void AudioPB::cmdui_cmd_decode(void)
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

bool AudioPB::cmdui_cmd_compare(const char *auth, const char *input, size_t stop_index)
{
	if(auth == NULL) return false;
	if(input == NULL) return false;
	if(stop_index >= TEXTBUF_SIZE_CHARS) return false;

	snprintf(textbuf, TEXTBUF_SIZE_CHARS, "%s", input);

	textbuf[stop_index] = '\0';

	return cstr_compare(auth, textbuf);
}

void AudioPB::cmdui_print_help_text(void)
{
	std::cout << "User Command List:\n\n";
	std::cout << "\"help\" or \"--help\" : print this list\n";
	std::cout << "\"params\" : print current delay parameters\n";
	std::cout << "\"setffdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedforward delay channel \"x\" (valid x values are 1 to " << std::to_string(this->AUDIORTDELAY_N_FF_DELAYS) << ")\n";
	std::cout << "\"setffamp<x>:<number>\" : \"number\" set the amplitude of feedforward delay channel \"x\" (valid x values are 1 to " << std::to_string(this->AUDIORTDELAY_N_FF_DELAYS) << ")\n";
	std::cout << "\"setfbdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedback delay channel \"x\" (valid x values are 1 to " << std::to_string(this->AUDIORTDELAY_N_FB_DELAYS) << ")\n";
	std::cout << "\"setfbamp<x>:<number>\" : \"number\" set the amplitude of feedback delay channel \"x\" (valid x values are 1 to " << std::to_string(this->AUDIORTDELAY_N_FB_DELAYS) << ")\n";
	std::cout << "\"stop\" : stop playback and quit application\n\n";

	std::cout << "Example commands:\n\"setffdelay1:220\"\n\"setffamp1:0.3\"\n\n";

	std::cout << "Remember: feedback delays are an infinite impulse response (IIR) circuit. Be very careful when setting it, as improper values can cause the signal to clip endlessly, causing a very loud and unpleasant noise.\n\n";
	return;
}

void AudioPB::cmdui_print_current_params(void)
{
	size_t n_fx = 0u;
	audiortdelay_fx_params_t fx_params;

	std::cout << "Current Parameters:\n\n";

	n_fx = 0u;
	while(n_fx < this->AUDIORTDELAY_N_FF_DELAYS)
	{
		this->p_delay->getFFParams(n_fx, &fx_params);

		std::cout << "Feedforward delay " << std::to_string(n_fx + 1u) << ":\n";
		std::cout << "Delay time (number of samples): " << std::to_string(fx_params.delay) << std::endl;
		std::cout << "Delay amplitude: " << std::to_string(fx_params.amp) << "\n\n";
		n_fx++;
	}

	n_fx = 0u;
	while(n_fx < this->AUDIORTDELAY_N_FB_DELAYS)
	{
		this->p_delay->getFBParams(n_fx, &fx_params);

		std::cout << "Feedback delay " << std::to_string(n_fx + 1u) << ":\n";
		std::cout << "Delay time (number of samples): " << std::to_string(fx_params.delay) << std::endl;
		std::cout << "Delay amplitude: " << std::to_string(fx_params.amp) << "\n\n";
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

bool AudioPB::cmdui_get_nfx_from_cmd(const char *cmdargnfx, size_t *p_nfx, size_t *p_cmdargvalrelindex)
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

bool AudioPB::cmdui_attempt_update_ffdelay(const char *cmdargval, size_t n_fx)
{
	int n_delay = 0;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->AUDIORTDELAY_N_FF_DELAYS))
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

	if(!this->p_delay->setFFDelay((n_fx - 1u), (uint32_t) n_delay))
	{
		std::cout << this->p_delay->getLastErrorMessage() << std::endl;
		return false;
	}

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioPB::cmdui_attempt_update_ffamp(const char *cmdargval, size_t n_fx)
{
	float f_amp = 0.0f;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->AUDIORTDELAY_N_FF_DELAYS))
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

	if(!this->p_delay->setFFAmplitude((n_fx - 1u), f_amp))
	{
		std::cout << this->p_delay->getLastErrorMessage() << std::endl;
		return false;
	}

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioPB::cmdui_attempt_update_fbdelay(const char *cmdargval, size_t n_fx)
{
	int n_delay = 0;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->AUDIORTDELAY_N_FB_DELAYS))
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

	if(!this->p_delay->setFBDelay((n_fx - 1u), (uint32_t) n_delay))
	{
		std::cout << this->p_delay->getLastErrorMessage() << std::endl;
		return false;
	}

	std::cout << "Parameter updated\n";
	return true;
}

bool AudioPB::cmdui_attempt_update_fbamp(const char *cmdargval, size_t n_fx)
{
	float f_amp = 0.0f;

	if(cmdargval == NULL) return false;

	if((n_fx < 1u) || (n_fx > this->AUDIORTDELAY_N_FB_DELAYS))
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

	if(!this->p_delay->setFBAmplitude((n_fx - 1u), f_amp))
	{
		std::cout << this->p_delay->getLastErrorMessage() << std::endl;
		return false;
	}

	std::cout << "Parameter updated\n";
	return true;
}


