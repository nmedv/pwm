#include <pwm/pwm.h>
#include <pwm/pwm_getopt.h>
#include <pwm/pwm_error.h>
#include <console/Console.h>


Console* console;

static const char* pwm_version = "pwm version 0.1 beta";
static const char* pwm_usage = "pwm name [value] [options]";
static const char* pwm_args_help[] = {
	"Password manager\n\n",

	"options:\n",
	"  -h, --help       Print help and exit\n",
	"  -v, --version    Print version and exit\n",
	"  -r, --remove     Remove password  (default=off)\n",
	"  -s, --source     Data file  (default=\"data.pw\")\n",
	"  -f, --force      Suppres warnings  (default=off)\n",
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


static int PwmParseArgs(int argc, char* argv[], pwm_args& pwm_args)
{
	char opt;
	int longindex;
	int i = 0;

	while ((opt = getopt_long(argc, argv, options, long_options, &longindex)) != -1) {
		switch (opt) {
		case 'h':
			console->Printf("usage: %s\n\n", pwm_usage);
			while (pwm_args_help[i] != 0)
				console->Write(pwm_args_help[i++]);
			pwm_args.info = 1;
			return 1;
		case 'v':
			console->Printf("%s\n", pwm_version);
			pwm_args.info = 1;
			return 1;
		case 'r': pwm_args.remove = 1; break;
		case 's': pwm_args.source = optarg; break;
		case 'f': pwm_args.force = 1; break;
		case '?':
			if (optopt)
				return PwmSetError(PWM_BAD_ARGS, "unknown option '-%c'\n", optopt);
			else
				return PwmSetError(PWM_BAD_ARGS, "unknown option \"%s\"\n", argv[optind - 1]);
		case ':':
			return PwmSetError(PWM_BAD_ARGS, "option '-%c' requires an argument\n", optopt);
		case '*':
			if (!pwm_args.name)
				pwm_args.name = argv[argind];
			else if (!pwm_args.value)
				pwm_args.value = argv[argind];
			else
				return PwmSetError(PWM_BAD_ARGS, "unknown no-option argument '-%s'\n", argv[argind]);
			break;
		default: return 0;
		}
	}

	if (!pwm_args.name)
	{
		console->Printf("usage: %s\n", pwm_usage);
		return PwmSetError(PWM_BAD_ARGS, "'name' argument is required\n");
	}

	if (!pwm_args.source)
		pwm_args.source = (char*)"data.pw";

	return 1;
}


static inline int PwmPrintError()
{
	console->Printf("pwm: error: %s\n", PwmGetError()->str);
	return 0;
}


static inline void PwmPasswordInput(char* out)
{
	console->SetStdinEcho(false);
	console->Write("Enter the password: ");
	console->Read(out, 32);
	console->Write("\n");
	console->SetStdinEcho(true);
}


int PwmMain(int argc)
{
	console = new Console();
	char** argvU8 = const_cast<char**>(console->GetUTF8Argv());

	pwm_args args = {};
	if (!PwmParseArgs(argc, argvU8, args))
		return PwmPrintError();
	if (args.info)
		return 1;

	std::string name = std::string(args.name);
	std::string value;
	if (args.value)
		value = std::string(args.value);

	char* password = new char[32];
	PwmPasswordInput(password);

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
					console->Printf("%s: %s\n", pair.first.c_str(), pair.second.c_str());

				result = 1;
			}
			else
			{
				value = *pwm.Get(name);
				if (value.empty())
					result = 0;
				else
					result = console->Printf("%s: %s", args.name, value.c_str());
			}
		}
	}
	else if (PwmGetError()->code != PWM_BAD_DECRYPT && !args.remove && args.value)
		result = pwm.Set(name, value);
	else
		result = -1;

	if (result <= 0)
		PwmPrintError();

	if (result >= 0 && !pwm.Save(args.source, password))
		result = PwmPrintError();
	else
		result = 0;

	delete password;
	delete console;

	return result;
}


int main(int argc, char* argv[])
{
	if (!PwmMain(argc))
		return PwmGetError()->code;

	return 0;
}