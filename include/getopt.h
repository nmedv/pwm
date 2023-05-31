#ifndef GETOPT_H
#define GETOPT_H

extern int		optind;		/* index into parent argv vector */
extern int		argind;		/* index into parent argv vector (no-option) */
extern int		optopt;		/* character checked for validity */
extern int		optreset;	/* reset getopt  */
extern char*	optarg;		/* argument associated with option */

int getopt(int argc, char * const argv[], const char *optstring);

struct option
{
	const char* name;
	int has_arg;
	int* flag;
	int val;
};

#define no_argument 0
#define required_argument 1
#define optional_argument 2

int getopt_long(
	int argc,
	char *const argv[],
	const char *optstring,
	const struct option *longopts,
	int *longindex);

#endif