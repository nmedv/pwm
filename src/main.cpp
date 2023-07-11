#include <cstdio>
#include <memory>

#include <pwm/pwm.h>


int pwm_main(pwm_args& args)
{
	if (args.info)
		return 1;

	char* password = (char*)calloc(0, PWM_MAX_MASTER_PASSWORD_SIZE);
	pwm_passw_input(password);

	auto pwm = Pwm();

	if (!pwm.load(args.source, password)) {
		if (file.signature != 0)
		{
			fprintf(stderr, "pwm: error: file corrupted: '%s'\n", args.source);
			return 0;
		}
		
		if (args.remove || !args.value)
		{
			fprintf(stderr, "pwm: error: can't open file: '%s'\n", args.source);
			return 0;
		}
			
		std::map<std::string, std::string> entries;
		entries[std::string(args.name)] = std::string(args.value);
		
		pwm_save(entries, password, file);
		pwm_write(file, args.source);
		free(password);

		return 1;
	}

	char* password = (char*)calloc(0, PWM_MAX_MASTER_PASSWORD_SIZE);
	pwm_passw_input(password);

	std::map<std::string, std::string> entries;
	if (!pwm_load(file, password, entries))
	{
		fprintf(stderr, "pwm: error: wrong password or data corrupted");
		return 0;
	}

	free(password);

	return 1;
}


int main(int argc, char* argv[])
{
	pwm_args args = {};
	if (!pwm_parse_args(argc, argv, args))
		return 1;

	pwm_main(args);

	return 0;
}