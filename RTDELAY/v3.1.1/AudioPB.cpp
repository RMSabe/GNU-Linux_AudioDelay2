/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include "cstrdef.h"

#include <string.h>
#include <poll.h>

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

	if(p_params->audio_dev_desc == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: p_params: given audio_dev_desc value is invalid.");
		return false;
	}

	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIODEV_DESC = p_params->audio_dev_desc;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = (size_t) p_params->sample_rate;
	this->N_CHANNELS = (size_t) p_params->n_channels;

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
		this->status = this->STATUS_ERROR_MEMALLOC;
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
			this->status = this->STATUS_ERROR_MEMALLOC;
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

	__STDCOUT__ << __TEXT("Playback started\n\n");

	this->userthread = std::thread(&AudioPB::userthread_proc, this);

	this->playback_proc();
	cppthread_wait(&(this->userthread));

	__STDCOUT__ << __TEXT("Playback finished\n\n");

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return true;
}

__string AudioPB::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (__TEXT("Error: AudioPB object not initialized\nExtended error message: ") + this->err_msg);

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

	snd_pcm_drop(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);

	this->p_audiodev = NULL;
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

void AudioPB::cmdui_cmd_decode(void)
{
	const __tchar_t *cmd = NULL;
	const __tchar_t *errinv = __TEXT("Error: invalid command entered\n");
	const __tchar_t *paramupdated = __TEXT("Parameter updated\n");

	ssize_t c_index = 0;
	ssize_t n_fx = 0;
	ssize_t n_delay = 0;
	float f_amp = 0.0f;

	this->user_cmd = str_tolower(this->user_cmd);
	cmd = this->user_cmd.c_str();

	if(cstr_compare(__TEXT("stop"), cmd))
	{
		this->stop_playback = true;
		return;
	}

	if(cstr_compare(__TEXT("params"), cmd))
	{
		this->cmdui_print_current_params();
		return;
	}

	if(cstr_compare(__TEXT("help"), cmd) || cstr_compare(__TEXT("--help"), cmd))
	{
		this->cmdui_print_help_text();
		return;
	}

	if(cstr_compare(__TEXT("reset"), cmd))
	{
		this->p_delay->resetFFParams();
		this->p_delay->resetFBParams();
		return;
	}

	if(cstr_compare(__TEXT("resetff"), cmd))
	{
		this->p_delay->resetFFParams();
		return;
	}

	if(cstr_compare(__TEXT("resetfb"), cmd))
	{
		this->p_delay->resetFBParams();
		return;
	}

	if(cstr_compare_upto_char(__TEXT("setdryamp"), cmd, ':', true))
	{
		if(!this->cmdui_parse_famp(&cmd[10], &f_amp)) return;

		this->p_delay->setDryInputAmplitude(f_amp);
		__STDCOUT__ << paramupdated;
		return;
	}

	if(cstr_compare_upto_char(__TEXT("setoutamp"), cmd, ':', true))
	{
		if(!this->cmdui_parse_famp(&cmd[10], &f_amp)) return;

		this->p_delay->setOutputAmplitude(f_amp);
		__STDCOUT__ << paramupdated;
		return;
	}

	if(cstr_compare_upto_len(__TEXT("setffdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0)
		{
			__STDCOUT__ << errinv;
			return;
		}

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);
		
		n_fx = this->cmdui_parse_nfx(textbuf, (ssize_t) this->AUDIORTDELAY_N_FF_DELAYS);
		if(n_fx < 0) return;

		n_delay = this->cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(this->p_delay->setFFDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) __STDCOUT__ << paramupdated;
		else __STDCOUT__ << this->p_delay->getLastErrorMessage() << std::endl;

		return;
	}

	if(cstr_compare_upto_len(__TEXT("setffamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0)
		{
			__STDCOUT__ << errinv;
			return;
		}

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = this->cmdui_parse_nfx(textbuf, (ssize_t) this->AUDIORTDELAY_N_FF_DELAYS);
		if(n_fx < 0) return;

		if(!this->cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(this->p_delay->setFFAmplitude((size_t) (n_fx - 1), f_amp)) __STDCOUT__ << paramupdated;
		else __STDCOUT__ << this->p_delay->getLastErrorMessage() << std::endl;

		return;
	}

	if(cstr_compare_upto_len(__TEXT("setfbdelay"), cmd, 10u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0)
		{
			__STDCOUT__ << errinv;
			return;
		}

		cstr_copy_upto_char(&cmd[10], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = this->cmdui_parse_nfx(textbuf, (ssize_t) this->AUDIORTDELAY_N_FB_DELAYS);
		if(n_fx < 0) return;

		n_delay = this->cmdui_parse_ndelay(&cmd[c_index + 1]);
		if(n_delay < 0) return;

		if(this->p_delay->setFBDelay((size_t) (n_fx - 1), (uint32_t) n_delay)) __STDCOUT__ << paramupdated;
		else __STDCOUT__ << this->p_delay->getLastErrorMessage() << std::endl;

		return;
	}

	if(cstr_compare_upto_len(__TEXT("setfbamp"), cmd, 8u, false))
	{
		c_index = cstr_locatechar(cmd, ':');
		if(c_index < 0)
		{
			__STDCOUT__ << errinv;
			return;
		}

		cstr_copy_upto_char(&cmd[8], textbuf, TEXTBUF_SIZE_CHARS, ':', true);

		n_fx = this->cmdui_parse_nfx(textbuf, (ssize_t) this->AUDIORTDELAY_N_FB_DELAYS);
		if(n_fx < 0) return;

		if(!this->cmdui_parse_famp(&cmd[c_index + 1], &f_amp)) return;

		if(this->p_delay->setFBAmplitude((size_t) (n_fx - 1), f_amp)) __STDCOUT__ << paramupdated;
		else __STDCOUT__ << this->p_delay->getLastErrorMessage() << std::endl;

		return;
	}

	__STDCOUT__ << errinv;
	return;
}

void AudioPB::cmdui_print_help_text(void)
{
	__STDCOUT__ << __TEXT("User Command List:\n\n");
	__STDCOUT__ << __TEXT("\"help\" or \"--help\" : print this list\n");
	__STDCOUT__ << __TEXT("\"params\" : print current delay parameters\n");
	__STDCOUT__ << __TEXT("\"reset\" to reset all feedforward and feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"resetff\" to reset all feedforward delay channels\n");
	__STDCOUT__ << __TEXT("\"resetfb\" to reset all feedback delay channels\n");
	__STDCOUT__ << __TEXT("\"setdryamp:<number>\" set the amplitude for the dry input signal in the delay mix (does not affect feedforward delay)\n");
	__STDCOUT__ << __TEXT("\"setoutamp:<number>\" set the mix output amplitude.\n");
	__STDCOUT__ << __TEXT("\"setffdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedforward delay channel \"x\" (valid x values are 1 to ") << __TOSTRING(this->AUDIORTDELAY_N_FF_DELAYS) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setffamp<x>:<number>\" : \"number\" set the amplitude of feedforward delay channel \"x\" (valid x values are 1 to ") << __TOSTRING(this->AUDIORTDELAY_N_FF_DELAYS) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbdelay<x>:<number>\" : \"number\" set delay time (in number of samples) of feedback delay channel \"x\" (valid x values are 1 to ") << __TOSTRING(this->AUDIORTDELAY_N_FB_DELAYS) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"setfbamp<x>:<number>\" : \"number\" set the amplitude of feedback delay channel \"x\" (valid x values are 1 to ") << __TOSTRING(this->AUDIORTDELAY_N_FB_DELAYS) << __TEXT(")\n");
	__STDCOUT__ << __TEXT("\"stop\" : stop playback and quit application\n\n");

	__STDCOUT__ << __TEXT("Example commands:\n\"setffdelay1:220\"\n\"setffamp1:-0.3\"\n\n");

	__STDCOUT__ << __TEXT("Remember: feedback delays are an infinite impulse response (IIR) circuit. Be very careful when setting it, as improper values can cause the signal to clip endlessly, causing a very loud and unpleasant noise.\n\n");
	return;
}

void AudioPB::cmdui_print_current_params(void)
{
	size_t n_fx = 0u;
	float f_amp = 0.0f;
	audiortdelay_fx_params_t fx_params;

	__STDCOUT__ << __TEXT("Current Parameters:\n\n");

	f_amp = this->p_delay->getDryInputAmplitude();
	__STDCOUT__ << __TEXT("Dry signal amplitude: ") << __TOSTRING(f_amp) << std::endl;

	f_amp = this->p_delay->getOutputAmplitude();
	__STDCOUT__ << __TEXT("Output amplitude: ") << __TOSTRING(f_amp) << __TEXT("\n\n");

	n_fx = 0u;
	while(n_fx < this->AUDIORTDELAY_N_FF_DELAYS)
	{
		this->p_delay->getFFParams(n_fx, &fx_params);

		__STDCOUT__ << __TEXT("Feedforward delay ") << __TOSTRING(n_fx + 1u) << __TEXT(":\n");
		__STDCOUT__ << __TEXT("Delay time (number of samples): ") << __TOSTRING(fx_params.delay) << std::endl;
		__STDCOUT__ << __TEXT("Delay amplitude: ") << __TOSTRING(fx_params.amp) << __TEXT("\n\n");
		n_fx++;
	}

	n_fx = 0u;
	while(n_fx < this->AUDIORTDELAY_N_FB_DELAYS)
	{
		this->p_delay->getFBParams(n_fx, &fx_params);

		__STDCOUT__ << __TEXT("Feedback delay ") << __TOSTRING(n_fx + 1u) << __TEXT(":\n");
		__STDCOUT__ << __TEXT("Delay time (number of samples): ") << __TOSTRING(fx_params.delay) << std::endl;
		__STDCOUT__ << __TEXT("Delay amplitude: ") << __TOSTRING(fx_params.amp) << __TEXT("\n\n");
		n_fx++;
	}

	return;
}

ssize_t AudioPB::cmdui_parse_nfx(const __tchar_t *p_text_nfx, ssize_t max)
{
	int val = 0;

	if(p_text_nfx == NULL) return -1;
	if(max < 1) return -1;

	try
	{
		val = std::stoi(p_text_nfx);
	}
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid delay channel entered\n");
		return -1;
	}

	if((val < 1) || (val > max))
	{
		__STDCOUT__ << __TEXT("Error: invalid delay channel entered\n");
		return -1;
	}

	return (ssize_t) val;
}

ssize_t AudioPB::cmdui_parse_ndelay(const __tchar_t *p_text_ndelay)
{
	int n_delay = 0;

	if(p_text_ndelay == NULL) return -1;

	try
	{
		n_delay = std::stoi(p_text_ndelay);
	}
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid delay value entered\n");
		return -1;
	}
	
	if(n_delay < 0)
	{
		__STDCOUT__ << __TEXT("Error: invalid delay value entered\n");
		return -1;
	}

	if(((size_t) n_delay) >= this->AUDIORTDELAY_BUFFER_SIZE_FRAMES)
	{
		__STDCOUT__ << __TEXT("Error: entered delay value is too big\n");
		return -1;
	}

	return (ssize_t) n_delay;
}

bool AudioPB::cmdui_parse_famp(const __tchar_t *p_text_famp, float *p_famp)
{
	float f_amp = 0.0f;

	if(p_text_famp == NULL) return false;
	if(p_famp == NULL) return false;

	try
	{
		f_amp = std::stof(p_text_famp);
	}
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid amplitude value entered\n");
		return false;
	}

	*p_famp = f_amp;
	return true;
}

void AudioPB::userthread_proc(void)
{
	int n_ret = 0;
	struct pollfd poll_userinput;

	memset(&poll_userinput, 0, sizeof(struct pollfd));

	poll_userinput.fd = STDIN_FILENO;
	poll_userinput.events = POLLIN;

	this->cmdui_print_help_text();

	while(!this->stop_playback)
	{
		n_ret = poll(&poll_userinput, 1, 1);

		if(n_ret > 0)
		{
			this->user_cmd = __TEXT("");
			__STDCIN__ >> this->user_cmd;
			this->cmdui_cmd_decode();
		}
	}

	return;
}

