#ifndef PWM_ERROR
#define PWM_ERROR

#define PWM_ERROR_MAX_STR_SIZE  256
#define PWM_ERROR_STACK_SIZE    16

enum EPwmError {
	PWM_BAD_ARGS = 2,
	PWM_NO_FILE,
	PWM_BAD_FILE,
	PWM_BAD_DECRYPT,
	PWM_BAD_NAME,
	PWM_BAD_DATA,
	PWM_BAD_ENCRYPT
};

typedef struct {
    int     code;
    char    str[PWM_ERROR_MAX_STR_SIZE];
} pwm_error;

#ifdef __cplusplus
extern "C" {
#endif

int PwmSetError(int code, const char *format, ...);
const pwm_error* PwmGetError(void);

#ifdef __cplusplus
}
#endif

#endif