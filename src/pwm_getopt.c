#include <string.h>

#define BADCH	(int)'?'
#define BADARG	(int)':'
#define NOOPT	(int)'*'
#define EMSG	(char*)""

#define no_argument 0
#define required_argument 1
#define optional_argument 2

//=============================================================================

typedef struct
{
	const char* name;
	int has_arg;
	int* flag;
	int val;
} option;

int optind = 1;
int argind = 1;
int optopt;
int optreset;
char *optarg;


//=============================================================================

int getopt(int argc, char * const argv[], const char *optstring)
{
	static char *place = EMSG;	/* option letter processing */
	const char *oli;	/* option letter list index */

	if (optreset || !*place)	/* update scanning pointer */
	{
		optreset = 0;
		
		if (optind >= argc)	/* no more args */
		{
			place = EMSG;
			return (-1);
		}
		
		if (*(place = argv[optind]) != '-')	/* "-" not found */
		{
			argind = optind++;
			place = EMSG;
			return (NOOPT);
		}

		if (place[1] && *++place == '-')	/* found "--" */
		{
			++optind;
			place = EMSG;
			return (-1);
		}
	}

	/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
		!(oli = strchr(optstring, optopt)) )
	{
		/*
		*	if the user didn't specify '-' as an option,
		*	assume it means -1.
		*/
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;

		return (BADCH);
	}

	if (*++oli != ':')	/* don't need argument */
	{
		optarg = 0;
		if (!*place)
			++optind;
	}
	else	/* need an argument */
	{
		if (*place)	/* no white space */
		{
			optarg = place;
			place = EMSG;
			++optind;
		}
		else if (argc <= ++optind)	/* no arg */
		{
			place = EMSG;
			if (*optstring == ':')
				return (BADARG);
			return (BADCH);
		}
		else	/* white space */
		{
			optarg = argv[optind];
			place = EMSG;
			++optind;
		}
	}

	return (optopt);	/* dump back option letter */
}


//=============================================================================

int getopt_long(
	int argc,
	char *const argv[],
	const char *optstring,
	const option *longopts,
	int *longindex)
{
	static char *place = EMSG;	/* option letter processing */
	char *oli;					/* option letter list index */
	optopt = 0;

	if (!*place)
	{							/* update scanning pointer */
		if (optind >= argc)
		{
			place = EMSG;
			return -1;
		}

		if (*(place = argv[optind]) != '-')
		{
			argind = optind++;
			place = EMSG;
			return (NOOPT);
		}

		if (!*++place)
		{
			/* treat "-" as not being an option */
			place = EMSG;
			return -1;
		}

		if (place[0] == '-' && place[1] == '\0')
		{
			/* found "--", treat it as end of options */
			++optind;
			place = EMSG;
			return -1;
		}

		if (place[0] == '-' && place[1])
		{
			/* long option */
			int i;
			size_t namelen;

			namelen = strcspn(++place, "=");
			for (i = 0; longopts[i].name != 0; i++)
			{
				if (strlen(longopts[i].name) == namelen
					&& strncmp(place, longopts[i].name, namelen) == 0)
				{
					int has_arg = longopts[i].has_arg;

					if (has_arg != no_argument)
					{
						if (place[namelen] == '=')
							optarg = place + namelen + 1;
						else if (optind < argc - 1 &&
							has_arg == required_argument)
						{
							optind++;
							optarg = argv[optind];
						}
						else
						{
							if (optstring[0] == ':')
								return BADARG;

							place = EMSG;
							optind++;

							if (has_arg == required_argument)
								return BADCH;
							optarg = 0;
						}
					}
					else
					{
						optarg = 0;
						if (place[namelen] != 0)
						{
							// error?
						}
					}

					optind++;

					if (longindex)
						*longindex = i;

					place = EMSG;

					if (longopts[i].flag == 0)
						return longopts[i].val;
					else
					{
						*longopts[i].flag = longopts[i].val;
						return 0;
					}
				}
			}

			place = EMSG;
			optind++;
			return BADCH;
		}
	}

	/* short option */
	optopt = (int) *place++;

	oli = strchr(optstring, optopt);
	if (!oli)
	{
		if (!*place)
			++optind;
		return BADCH;
	}

	if (oli[1] != ':')
	{							/* don't need argument */
		optarg = 0;
		if (!*place)
			++optind;
	}
	else
	{							/* need an argument */
		if (*place)				/* no white space */
			optarg = place;
		else if (argc <= ++optind)
		{						/* no arg */
			place = EMSG;
			if (*optstring == ':')
				return BADARG;
			return BADCH;
		}
		else
			/* white space */
			optarg = argv[optind];
		place = EMSG;
		++optind;
	}
	return optopt;
}


//=============================================================================