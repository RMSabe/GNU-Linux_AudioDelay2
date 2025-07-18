/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 2.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define PB_I16 1
#define PB_I24 2

AudioPB *p_audio = NULL;

audiopb_params_t pb_params;

int h_filein = -1;

extern void app_deinit(void);

extern bool filein_check_ext(void);
extern bool filein_open(void);
extern void filein_close(void);

extern int filein_get_params(void);
extern bool compare_signature(const char *auth, const char *buf, size_t offset);

int main(int argc, char **argv)
{
	int n_ret = 0;

	if(argc < 3)
	{
		std::cout << "Error: missing arguments\nThis executable requires 2 arguments: <output device id> <input file directory>\nThey must be in that order\n";
		return 1;
	}

	pb_params.audio_dev_desc = argv[1];
	pb_params.file_dir = argv[2];

	if(!filein_check_ext())
	{
		std::cout << "Error: file format is not supported\n";
		goto _l_main_error;
	}

	if(!filein_open())
	{
		std::cout << "Error: failed to open file\n";
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
		std::cout << "Error: audio object instance failed.\n";
		goto _l_main_error;
	}

	if(!p_audio->initialize())
	{
		std::cout << p_audio->getLastErrorMessage() << std::endl;
		goto _l_main_error;
	}

	p_audio->runPlayback();

	app_deinit();
	return 0;

_l_main_error:
	app_deinit();
	return 1;
}

void app_deinit(void)
{
	filein_close();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const char *exit_msg)
{
	if(exit_msg != NULL) std::cout << "PROCESS EXIT CALLED\n" << exit_msg << std::endl;

	app_deinit();
	exit(exit_code);

	while(true) delay_ms(1);
}

bool filein_check_ext(void)
{
	std::string user_cmd = "";
	size_t len = 0u;

	if(pb_params.file_dir == NULL) return false;

	snprintf(textbuf, TEXTBUF_SIZE_CHARS, "%s", pb_params.file_dir);
	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);

	len = (size_t) cstr_getlength(textbuf);

	if(len >= 5u)
		if(cstr_compare(".wav", &textbuf[len - 4u])) return true;

	while(true)
	{
		std::cout << "The given file name does not seem to have a .wav extension. Might be incompatible with the application\nDo you wish to continue? (YES/NO): ";

		user_cmd = "";
		std::cin >> user_cmd;

		user_cmd = str_tolower(user_cmd);

		if(!user_cmd.compare("yes")) return true;
		if(!user_cmd.compare("no")) return false;

		std::cout << "Error: invalid command entered\n";
	}

	return false;
}

bool filein_open(void)
{
	if(pb_params.file_dir == NULL) return false;

	filein_close();

	h_filein = open(pb_params.file_dir, O_RDONLY);

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
	const size_t BUFFER_SIZE = 4096u;
	size_t bytepos = 0u;

	uint8_t *p_headerinfo = NULL;
	uint16_t *p_u16 = NULL;
	uint32_t *p_u32 = NULL;

	uint16_t bit_depth = 0u;

	p_headerinfo = (uint8_t*) malloc(BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	memset(p_headerinfo, 0, BUFFER_SIZE);

	__LSEEK(h_filein, 0, SEEK_SET);
	read(h_filein, p_headerinfo, BUFFER_SIZE);
	filein_close();

	if(!compare_signature("RIFF", (const char*) p_headerinfo, 0u))
	{
		snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const char*) p_headerinfo, 8u))
	{
		snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	bytepos = 12u;

	while(true)
	{
		if(bytepos > (BUFFER_SIZE - 8u))
		{
			snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: subchunk \"fmt \" not found.\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const char*) p_headerinfo, bytepos)) break;

		p_u32 = (uint32_t*) &p_headerinfo[bytepos + 4u];

		bytepos += (size_t) (*p_u32 + 8u);
	}

	p_u16 = (uint16_t*) &p_headerinfo[bytepos + 8u];

	if(p_u16[0] != 1u)
	{
		snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = p_u16[1];

	p_u32 = (uint32_t*) &p_headerinfo[bytepos + 12u];

	pb_params.sample_rate = *p_u32;

	p_u16 = (uint16_t*) &p_headerinfo[bytepos + 22u];

	bit_depth = *p_u16;

	p_u32 = (uint32_t*) &p_headerinfo[bytepos + 4u];

	bytepos += (size_t) (*p_u32 + 8u);

	while(true)
	{
		if(bytepos > (BUFFER_SIZE - 8u))
		{
			snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: subchunk \"data\" not found.\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const char*) p_headerinfo, bytepos)) break;

		p_u32 = (uint32_t*) &p_headerinfo[bytepos + 4u];

		bytepos += (size_t) (*p_u32 + 8u);
	}

	p_u32 = (uint32_t*) &p_headerinfo[bytepos + 4u];

	pb_params.audio_data_begin = (__offset) (bytepos + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((__offset) *p_u32);

	free(p_headerinfo);

	p_headerinfo = NULL;
	p_u16 = NULL;
	p_u32 = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	snprintf(textbuf, TEXTBUF_SIZE_CHARS, "filein_get_params: Error: audio format not supported.");

_l_filein_get_params_error:
	std::cout << textbuf << std::endl;
	if(p_headerinfo != NULL) free(p_headerinfo);
	return -1;
}

bool compare_signature(const char *auth, const char *buf, size_t offset)
{
	if(auth == NULL) return false;
	if(buf == NULL) return false;

	if(auth[0] != buf[offset]) return false;
	if(auth[1] != buf[offset + 1u]) return false;
	if(auth[2] != buf[offset + 2u]) return false;
	if(auth[3] != buf[offset + 3u]) return false;

	return true;
}

