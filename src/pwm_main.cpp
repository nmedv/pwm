#include <pwm/pwm.h>
#include <pwm/pwm_getopt.h>
#include <pwm/pwm_error.h>
#include <pwm/pwm_console.h>

#define PWM_UTF8

#ifdef PWM_UTF8
#include <io.h>
#include <fcntl.h>
#include <locale.h>
#endif

static const char* pwm_version = "pwm version 0.1 beta";
static const char* pwm_usage = "pwm name [value] [options]";
static const char* pwm_args_help[] = {
	"Password manager\n",

	"options:",
	"  -h, --help       Print help and exit",
	"  -v, --version    Print version and exit",
	"  -r, --remove     Remove password  (default=off)",
	"  -s, --source     Data file  (default=\"data.pw\")",
	"  -f, --force      Suppres warnings  (default=off)",
	0
};
static const char* options = ":hvrs:f";
static const option long_options[] =
{
	{"help",	0, 0, 'h'},
	{"version",	0, 0, 'v'},
	{"remove",	0, 0, 'r'},
	{"source",	1, 0, 's'},
	{"force",	0, 0, 'f'},
	{0,			0, 0,  0 }
};


#ifdef _DEBUG
static void _dumpmem(uint8_t* mem, int size, const char* title)
{
	if (title)
		printf("%s:", title);
	else
	{
		char format[32];
		int addrsz = sizeof(mem) * 2 + 2;
		sprintf(format, "Dump at %%#0%dx:", addrsz);
		printf(format, mem);
	}

	for (int i = 0; i < size; i++)
	{
		if (i % 16 == 0)
			fputs("\n  ", stdout);

		printf("%02X ", (uint8_t)mem[i]);
	}

	fputs("\n", stdout);
}
#endif


static int PwmParseArgs(int argc, char* argv[], pwm_args& pwm_args)
{
	char opt;
	int longindex;
	int i = 0;

	while ((opt = getopt_long(argc, argv, options, long_options, &longindex)) != -1) {
		switch (opt) {
		case 'h':
			printf("usage: %s\n\n", pwm_usage);
			while (pwm_args_help[i] != 0)
				puts(pwm_args_help[i++]);
			pwm_args.info = 1;
			return 1;
		case 'v':
			puts(pwm_version);
			pwm_args.info = 1;
			return 1;
		case 'r': pwm_args.remove = 1; break;
		case 's': pwm_args.source = optarg; break;
		case 'f': pwm_args.force = 1; break;
		case '?':
			if (optopt)
				return PwmSetError(PWM_BAD_ARGS, "unknown option '-%c'", optopt);
			else
				return PwmSetError(PWM_BAD_ARGS, "unknown option \"%s\"", argv[optind - 1]);
		case ':':
			return PwmSetError(PWM_BAD_ARGS, "option '-%c' requires an argument", optopt);
		case '*':
			if (!pwm_args.name)
				pwm_args.name = argv[argind];
			else if (!pwm_args.value)
				pwm_args.value = argv[argind];
			else
				return PwmSetError(PWM_BAD_ARGS, "unknown no-option argument '-%s'", argv[argind]);
			break;
		default: return 0;
		}
	}

	if (!pwm_args.name)
		return PwmSetError(PWM_BAD_ARGS, "'name' argument is required", pwm_usage);

	if (!pwm_args.source)
		pwm_args.source = (char*)"data.pw";

	return 1;
}


static inline int PwmPrintError()
{
	fprintf(stderr, "pwm: error: %s\n", PwmGetError()->str);
	return 0;
}


static inline void PwmPasswordInput(char* out)
{
	setstdinecho(false);
	fputs("Enter the password: ", stdout);

#ifdef PWM_UTF8
	fgetmbs(out, 32, stdin);
#else
	fgets(out, 32, stdin);
#endif

	fputs("\n", stdout);
	setstdinecho(true);
}


int PwmMain(int argc, char** argv)
{
#ifdef PWM_UTF8
	char** _argv = mbargv();
#else
	char** _argv = argv;
#endif

	pwm_args args = {};
	if (!PwmParseArgs(argc, _argv, args))
		return PwmPrintError();
	if (args.info)
		return 1;

	std::string name = std::string(args.name);
	std::string value;
	if (args.value)
		value = std::string(args.value);

	char* password = new char[32];
	PwmPasswordInput(password);
#ifdef _DEBUG
	_dumpmem((uint8_t*)password, 32, "password");
#endif

	int result;
	Pwm pwm = Pwm(args.source, password);
	if (pwm.Loaded())
	{
		if (args.remove)
			result = pwm.Remove(name);
		else if (args.value)
			result = pwm.Set(name, value);
		else
		{
			if (!strcmp(args.name, "*"))
			{
				for (auto pair : *pwm.Get())
					printf("%s: %s\n", pair.first.c_str(), pair.second.c_str());

				result = 1;
			}
			else
			{
				const std::string* value = pwm.Get(name);
				if (!value)
					result = 0;
				else
					result = printf("%s: %s", args.name, value->c_str());
			}
		}
	}
	else if (PwmGetError()->code != PWM_BAD_DECRYPT && !args.remove && args.value)
		result = pwm.Set(name, value);
	else
		result = -1;

	if (result <= 0)
		PwmPrintError();

	if (result >= 0)
	{
		result = pwm.Save(args.source, password);
		if (!result)
			PwmPrintError();
	}
	else
		result = 0;
	
	delete password;

#ifdef PWM_UTF8
	mbargv_free(argc, _argv);
#endif

	return result;
}


int main(int argc, char* argv[])
{
#ifdef PWM_UTF8
	// Set stdout and stderr encoding to UTF-8
	setlocale(LC_CTYPE, ".UTF8");

	fflush(stdin);
	_setmode(_fileno(stdin), _O_WTEXT);
#endif

	if (!PwmMain(argc, argv))
		return PwmGetError()->code;

	return 0;
}