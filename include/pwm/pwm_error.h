#ifndef PWM_ERROR
#define PWM_ERROR

typedef struct pwm_error
{
    int error; /* This is a numeric value corresponding to the current error */
    char* str;
    size_t len;
} pwm_error;

#ifdef __cplusplus
extern "C" {
#endif

int pwm_error(const char* format, ...);



#ifdef __cplusplus
}
#endif

#endif