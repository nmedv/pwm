
struct pwm_args
{
	int		info;

	int		remove;
	char*	name;
	char*	password;
	char*	source;
	int		force;

	char*	mpsw;
};

struct pwm_data
{
	short			sig;	// 0x5057
	unsigned char*	salt;	// 8 byte buffer
};

/**
 * @brief Argument parser
 * @param argc main() argc
 * @param argv main() argv
 * @param pwm_args Pointer to arguments struct. Should be initialized
 * to 0
 * @return 1 if succesfull, 0 otherwise
 */ 
int pwm_parse_args(int argc, char* argv[], struct pwm_args* pwm_args);

int pwm_read_data(const char* filename, const char* name);