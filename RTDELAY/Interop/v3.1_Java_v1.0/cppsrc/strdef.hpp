/*
 * Some functions and definitions to ease development on GNU-Linux systems.
 * Version 2.0
 *
 * config.h is the macro settings file. Used to define behavior and variables.
 * globldef.h is the global definition file. It should be the first file included in all other source files.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 *
 * GitHub Repository: https://github.com/RMSabe/GNU-Linux_Lib
 */

#ifndef STRDEF_HPP
#define STRDEF_HPP

#include "globldef.h"
#include <string>

#ifdef __TEXTFORMAT_USE_WCHAR
typedef std::wstring __string;
#define __TOSTRING(val) std::to_wstring(val)
#else
typedef std::string __string;
#define __TOSTRING(val) std::to_string(val)
#endif

/*
 * str_tolower() & str_toupper()
 * converts all the characters from an input string to lower-case or upper-case.
 *
 * returns the converted string.
 * WARNING: Might not work too well with wstrings.
 */

extern __string str_tolower(__string input);
extern __string str_toupper(__string input);

#endif /*STRDEF_HPP*/

