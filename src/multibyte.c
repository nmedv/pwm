#include <pwm/multibyte.h>

#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>

#define INPUT_BUFFER_SIZE 4096

char** mbargv(void)
{
	int argc;
	wchar_t** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	char** argvU8 = (char**)malloc(argc * sizeof(char*));
	for (int i = 0; i < argc; i++)
	{
		int argSize = SIZE_IN_CHAR(argvW[i]);
		argvU8[i] = (char*)malloc(argSize);
		WCHAR_TO_MULTIBYTE(argvW[i], argvU8[i], argSize);
	}
	LocalFree(argvW);

	return argvU8;
}


void mbargv_free(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		free(argv[i]);

	free(argv);
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


void multibyte_init()
{
	// Set stdout and stderr encoding to UTF-8
	setlocale(LC_CTYPE, ".UTF8");

	fflush(stdin);
	_setmode(_fileno(stdin), _O_WTEXT);
}