#define PWM_CORE_EXPORT
#include <pwm/error.h>

#include <stdarg.h>
#include <stdio.h>

static pwm_error err;

PWM_SHARED int PwmSetError(int code, const char *format, ...)
{
	err.code = code;

	if (!format)
		return 0;

	va_list ap;
	va_start(ap, format);
	vsnprintf(err.str, PWM_ERROR_MAX_STR_SIZE, format, ap);
	va_end(ap);

	return 0;
}

PWM_SHARED const pwm_error* PwmGetError(void)
{
	return &err;
}