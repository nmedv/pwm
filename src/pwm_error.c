#include <stdarg.h>
#include <pwm/pwm_error.h>
#include <stdio.h>

static char* errorBuffer;
static size_t errorBufferCount;

int pwm_error(const char* format, ...)
{
	if (!format)
		return 0;

	va_list ap;
	va_start(ap, format);
	int res = vsnprintf(errorBuffer, errorBuffer, format, ap);
	va_end(ap);
}