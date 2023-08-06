#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>


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


#define INPUT_BUFFER_SIZE 4096


char** mbargv(void)
{
	int argc;
	wchar_t** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	char** argvU8 = (char**)malloc(argc);
	for (int i = 0; i < argc; i++)
	{
		int argSize = SIZE_IN_CHAR(argvW[i]);
		argvU8[i] = (char*)malloc(argSize);
		WCHAR_TO_MULTIBYTE(argvW[i], argvU8[i], argSize);
	}
	LocalFree(argvW);

	return argvU8;
}


char* fgetmbs(char* buffer, int bufferCount, FILE* stream)
{
	if (!buffer) return 0;

	int len = bufferCount - 1;
	wchar_t wbuf[INPUT_BUFFER_SIZE] = { 0 };
	
	char* res = (char*)fgetws(wbuf, len, stream);
	if (!res) return res;

	memset(buffer, 0, bufferCount);
	if (!WCHAR_TO_MULTIBYTE(wbuf, buffer, len))
		return 0;
	
	return buffer;
}


void setstdinecho(bool enable)
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);

	if (!enable)
		mode &= ~ENABLE_ECHO_INPUT;
	else
		mode |= ENABLE_ECHO_INPUT;

	SetConsoleMode(hStdin, mode);
}