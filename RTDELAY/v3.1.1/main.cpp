/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include "shared.hpp"

#include <string.h>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define PB_I16 1
#define PB_I24 2

__attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio = NULL;
__attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;

int h_filein = -1;

extern void app_deinit(void);

extern bool filein_ext_check(void);
extern bool filein_open(void);
extern void filein_close(void);

extern int filein_get_params(void);
extern bool compare_signature(const char *auth, const uint8_t *buf);

int main(int argc, char **argv)
{
	int n_ret = 0;

	if(argc < 3)
	{
		__STDCOUT__ << __TEXT("Error: missing arguments\nThis executable requires 2 arguments: <output device id> <input file directory>\nThey must be in that order\n");
		return -1;
	}

	pb_params.audio_dev_desc = argv[1];
	pb_params.file_dir = argv[2];

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

	if(!p_audio->initialize())
	{
		__STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;
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

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) __STDCOUT__ << __TEXT("PROCESS EXIT CALLED\n") << exit_msg << std::endl;

	exit(exit_code);

	while(true) delay_ms(16);
}

bool filein_ext_check(void)
{
	__string user_cmd = __TEXT("");
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

		user_cmd = __TEXT("");
		__STDCIN__ >> user_cmd;

		user_cmd = str_tolower(user_cmd);

		if(!user_cmd.compare(__TEXT("yes"))) return true;
		if(!user_cmd.compare(__TEXT("no"))) return false;

		__STDCOUT__ << __TEXT("Error: invalid command entered\n");
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

bool compare_signature(const char *auth, const uint8_t *buf)
{
	if(auth == NULL) return false;
	if(buf == NULL) return false;

	if(auth[0] != ((char) buf[0])) return false;
	if(auth[1] != ((char) buf[1])) return false;
	if(auth[2] != ((char) buf[2])) return false;
	if(auth[3] != ((char) buf[3])) return false;

	return true;
}

