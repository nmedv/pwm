#include <cstdio>
#include <memory>
#include <vector>
#include <map>
#include <string.h>

#include <pwm/pwm_getopt.h>
#include <pwm/pwm_crypto.h>
#include <pwm/pwm.h>


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


int pwm_parse_args(int argc, char* argv[], pwm_args& pwm_args)
{
	char opt;
	int longindex;
	int i = 0;

	while ((opt = getopt_long(argc, argv, options,
		long_options, &longindex)) != -1) {
		switch (opt) {
		case 'h':
			printf("usage: %s\n\n", pwm_usage);
			while (pwm_args_help[i] != 0)
				printf(pwm_args_help[i++]);
			pwm_args.info = 1;
			return 1;
		case 'v':
			printf("%s\n", pwm_version);
			pwm_args.info = 1;
			return 1;
		case 'r': pwm_args.remove = 1; break;
		case 's': pwm_args.source = optarg; break;
		case 'f': pwm_args.force = 1; break;
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
			if (!pwm_args.name)
				pwm_args.name = argv[argind];
			else if (!pwm_args.value)
				pwm_args.value = argv[argind];
			else
			{
				fprintf(stderr,
					"pwm: error: unknown no-option argument '-%s'\n",
					argv[argind]);
				return 0;
			}
			break;
		default: return 0;
		}
	}

	if (!pwm_args.name)
	{
		printf("usage: %s\n", pwm_usage);
		fprintf(stderr, "pwm: error: 'name' argument is required\n");
		return 0;
	}

	if (!pwm_args.source)
		pwm_args.source = (char*)"data.pw";

	return 1;
}


extern "C" void SetStdinEcho(bool enable);
void pwm_passw_input(char* out)
{
	SetStdinEcho(false);
	printf("Enter the password: ");
	scanf("%32s", out);
	SetStdinEcho(true);
}


static size_t mapSize(const std::map<std::string, std::string>& map)
{
	// Table size
	size_t size = map.size() * 2 + sizeof(size_t);

	// Map size
	for (auto const &pair : map)
	{
		size += pair.first.size();
		size += pair.second.size();
	}

	return size;
}




Pwm::Pwm(void)
{
	saltSize = 0;
	salt = (uint8_t*)malloc(8);
	eSize = 0;
	eData = (uint8_t*)malloc(32);
	data = (uint8_t*)malloc(32);
	keyIv = (uint8_t*)malloc(24);
}


Pwm::Pwm(const char* src, const char* password)
{
	Pwm();
	load(src, password);
}


Pwm::~Pwm(void)
{
	free(salt);
	free(eData);
	free(data);
	free(keyIv);
}


int Pwm::load(const char* src, const char* password)
{
	if (!read(src))
		return 0;

	genkeyiv(password, salt, saltSize, keyIv);

	dataSize = AES256_DECRYPT_SIZE(eSize);
	data = (uint8_t*)realloc(data, dataSize);
	dataSize = aes_decrypt(eData, eSize, data, keyIv, keyIv + 16);
	if (!dataSize)
		return 0;

	deserialize();

	return 1;
}


int Pwm::save(const char* fname, const char* password)
{
	if (!saltSize)
		saltSize = 8;
	salt = (uint8_t*)realloc(salt, saltSize);

	gensalt(salt, saltSize);
	genkeyiv(password, salt, saltSize, keyIv);

	dataSize = mapSize(entries);
	data = (uint8_t*)realloc(data, dataSize);
	serialize();

	eSize = AES256_ENCRYPT_SIZE(dataSize);
	eData = (uint8_t*)realloc(eData, eSize);
	eSize = aes_encrypt(data, dataSize, eData, keyIv, keyIv + 16);

	if (!eSize || !write(fname))
		return 0;

	return 1;
}


int Pwm::read(const char* src)
{
	int filePtr = 0;
	uint16_t sig;

	FILE* file = fopen(src, "r");
	if (!file)
		return 0;

	filePtr += fread(&sig, sizeof(sig), 1, file);
	if (sig != 0x5057)
		return 0;

	filePtr += fread(&saltSize, sizeof(saltSize), 1, file);
	salt = (uint8_t*)realloc(salt, saltSize);
	filePtr += fread(salt, saltSize, 1, file);

	filePtr += fread(&eSize, sizeof(eSize), 1, file);
	eData = (uint8_t*)realloc(eData, eSize);
	filePtr += fread(eData, 1, eSize, file);

	fclose(file);

	if (filePtr != sizeof(sig) + sizeof(saltSize) + sizeof(eSize) +
		saltSize + eSize)
		return 0;

	return 1;
}


int Pwm::write(const char* fname)
{
	int filePtr = 0;
	uint16_t sig = 0x5057;

	FILE* file = fopen(fname, "w");
	if (!file)
		return 0;

	filePtr += fwrite(&sig, sizeof(sig), 1, file);
	filePtr += fwrite(&saltSize, sizeof(saltSize), 1, file);
	filePtr += fwrite(salt, saltSize, 1, file);
	filePtr += fwrite(&eSize, sizeof(eSize), 1, file);
	filePtr += fwrite(eData, eSize, 1, file);

	if (filePtr != sizeof(sig) + sizeof(saltSize) + sizeof(eSize) +
		saltSize + eSize)
		return 0;

	return 1;
}


void Pwm::serialize(void)
{
	size_t tableSize = entries.size() * 2;
	*(size_t*)data = tableSize;
	uint8_t* table = data + sizeof(size_t);
	uint8_t* data_ptr = table + tableSize;

	for (auto const& pair : entries)
	{
		*table++ = (uint8_t)pair.first.size();
		*table++ = (uint8_t)pair.second.size();

		strcpy((char*)data_ptr, pair.first.c_str());
		data_ptr += pair.first.size();
		strcpy((char*)data_ptr, pair.second.c_str());
		data_ptr += pair.second.size();
	}
}


void Pwm::deserialize(void)
{
	char* name;
	char* value;
	size_t tableSize = *(size_t*)data;
	uint8_t* table = data + sizeof(size_t);
	uint8_t* data_ptr = table + tableSize;
	
	for (size_t i = 0; i < tableSize; i += 2)
	{
		name = (char*)data_ptr;
		data_ptr += table[i];
		value = (char*)data_ptr;
		data_ptr += table[i + 1];
		entries[std::string(name, table[i])] = std::string(value, table[i + 1]);
	}
}