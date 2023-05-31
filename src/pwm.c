#include <stdio.h>
#include <getopt.h>
#include <pwm.h>
#include <aes.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

//=============================================================================

static const char* pwm_version = "pwm version 0.1 beta";

static const char* pwm_usage = "pwm name [password] [options]";

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

static const struct option long_options[] =
{
	{"help",	0, 0, 'h'},
	{"version",	0, 0, 'v'},
	{"remove",	0, 0, 'r'},
	{"source",	1, 0, 's'},
	{"force",	0, 0, 'f'},
	{0,			0, 0,  0 }
};


//=============================================================================

int pwm_parse_args(int argc, char* argv[], struct pwm_args* pwm_args)
{
	char opt;
	int longindex;
	
	while((opt = getopt_long(argc, argv, options,
		long_options, &longindex)) != -1) {
	switch (opt) {
		case 'h':
			printf("usage: %s\n\n", pwm_usage);
			for (int i = 0; i < 10; i++)
				printf(pwm_args_help[i]);
			printf("\n");

			pwm_args->info = 1;

			return 1;

		case 'v':
			printf("%s\n", pwm_version);
			pwm_args->info = 1;
			
			return 1;
		
		case 'r': pwm_args->remove = 1; break;
		case 's': pwm_args->source = optarg; break;
		case 'f': pwm_args->force = 1; break;

		case '?':
			if (optopt)
				fprintf(stderr,
					"pwm: error: unknown option '-%c'\n",
					optopt);
			else
				fprintf(stderr,
					"pwm: error: unknown option \"%s\"\n",
					argv[optind - 1]);
			
			return 0;

		case ':':
			fprintf(stderr,
				"pwm: error: option '-%c' requires an argument\n",
				optopt);
			
			return 0;
		
		case '*':
			if (!pwm_args->name)
				pwm_args->name = argv[argind];
			else if (!pwm_args->password)
				pwm_args->password = argv[argind];
			else
			{
				fprintf(stderr,
					"pwm: error: unknown no-option argument '-%s'\n",
					argv[argind]);
				return 0;
			}

			break;
		
		default: return 0;
	}}

	if (!pwm_args->name)
	{
		printf("usage: %s\n", pwm_usage);
		fprintf(stderr, "pwm: error: 'name' argument is required\n");
		return 0;
	}

	if (!pwm_args->source)
		pwm_args->source = (char*)"data.pw";

	return 1;
}


//=============================================================================

static unsigned char* pwm_key_iv;
static unsigned char* pwm_data;
static int pwm_data_len;


//=============================================================================

int pwm_load(const char* src, const char* mpsw)
{
	int fptr = 0;

	// Datafile header
	short sig;				// 0x5057
	unsigned char* salt;	// 8 byte buffer

	FILE* file = fopen(src, "r");
	if (file)
	{
		fptr += fread(sig,	1, 2, file);
		if (sig != 0x5057)
		{
			fclose(file);
			return 0;
		}

		fptr += fread(salt,	1, 8, file);
		if (fptr != 10)
		{
			fclose(file);
			return 0;
		}

		int ndata_encrypted = fseek(file, 0, SEEK_END);
		fseek(file, fptr, SEEK_SET);
		unsigned char* data_encrypted = malloc(ndata_encrypted);
		fread(data_encrypted, 1, ndata_encrypted, file);

		int ndata = AES256_DECRYPT_SIZE(ndata_encrypted);
		pwm_data = malloc(ndata);

		// Generate key and iv for AES256 from password
		// ---------------------------------
		// | key (16 bytes) | iv (8 bytes) |
		// ---------------------------------
		pwm_key_iv = malloc(24);
		PKCS5_PBKDF2_HMAC_SHA1(mpsw, 0, salt, 8, 1000, 24, pwm_key_iv);

		int decrypt_len = AES_decrypt(data_encrypted, ndata_encrypted,
			pwm_data, pwm_key_iv, pwm_key_iv + 16);

		if (!decrypt_len)
		{
			fclose(file);
			fprintf(stderr, "pwm: error: wrong password or data corrupted");
			return 0;
		}
		
	}
	else // new file
	{
		salt = (unsigned char*)malloc(8);
		RAND_bytes(salt, 8);
	}

	return 1;
}