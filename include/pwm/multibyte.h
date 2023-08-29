#ifdef WIN32

#ifndef PWM_MULTIBYTE_H
#define PWM_MULTIBYTE_H

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Use null terminated strings for the correct operation of this function!!! */
#define SIZE_IN_WCHAR(str) \
	MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0)


/* Use null terminated strings for the correct operation of this function!!! */
#define SIZE_IN_CHAR(str) \
	WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0)


/* Can write a terminal zero if it fits */
#define WCHAR_TO_MULTIBYTE(str, buffer, newSz) \
	WideCharToMultiByte(CP_UTF8, 0, str, -1, buffer, newSz, 0, 0)


/* Can write a terminal zero if it fits */
#define MULTIBYTE_TO_WCHAR(str, wstr, wstrl) \
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, wstrl)

/* Get argv with UTF-8 encoding */
char** mbargv      (void);
void   mbargv_free (int argc, char** argv);
char*  fgetmbs     (char* buffer, int bufferCount, FILE* stream);

void multibyte_init();

#ifdef __cplusplus
}
#endif

#endif

#endif