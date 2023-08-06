#ifndef PWM_CONSOLE_H
#define PWM_CONSOLE_H

#define INPUT_BUFFER_SIZE 4096

#ifdef __cplusplus
extern "C" {
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Get argv with UTF-8 encoding */
char** mbargv(void);

char* fgetmbs(char* buffer, int bufferCount, FILE* stream);

void setstdinecho(bool enable);

#ifdef __cplusplus
}
#endif

#endif // !PWM_CONSOLE_H