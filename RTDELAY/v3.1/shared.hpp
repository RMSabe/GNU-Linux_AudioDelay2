/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef SHARED_HPP
#define SHARED_HPP

#include "globldef.h"
#include <iostream>

#ifdef __TEXTFORMAT_USE_WCHAR
#define __STDCIN__ std::wcin
#define __STDCOUT__ std::wcout
#else
#define __STDCIN__ std::cin
#define __STDCOUT__ std::cout
#endif

extern void app_exit(int exit_code, const __tchar_t *exit_msg) __attribute__((__noreturn__));

#endif /*SHARED_HPP*/

