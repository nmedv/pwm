#ifndef PWM_GETOPT_H
#define PWM_GETOPT_H

#ifdef __cplusplus
extern "C" {
#endif

extern	int		optind;		/* index into parent argv vector */
extern	int		argind;		/* index into parent argv vector (no-option) */
extern	int		optopt;		/* character checked for validity */
extern	int		optreset;	/* reset getopt  */
extern	char*	optarg;		/* argument associated with option */

typedef struct
{
	const char* name;
	int has_arg;
	int* flag;
	int val;
} option;

int getopt(int argc, char * const argv[], const char *optstring);

int getopt_long(
	int argc,
	char *const argv[],
	const char *optstring,
	const option *longopts,
	int *longindex);


#ifdef __cplusplus
}
#endif

#endif