#ifndef PWM_H
#define PWM_H

#include <cstdint>
#include <string>
#include <map>

#define PWM_MAX_MASTER_PASSWORD_SIZE 32
#define PWM_DEFAULT_SALT_SIZE 8
#define PWM_MAX_NAME_LEN 32
#define PWM_MAX_VALUE_LEN 64


typedef struct pwm_args
{
	int		info;
	int		remove;
	char*	name;
	char*	value;
	char*	source;
	int		force;
} pwm_args;


// Parse command line args
int pwm_parse_args(int argc, char* argv[], pwm_args& pwm_args);

// Password input without keyboard echo
void pwm_passw_input(char* out);


class Pwm
{
public:
	Pwm(void);
	Pwm(const char* src, const char* password);
	~Pwm(void);

	// Decrypt data from pwm file with user password
	int load(const char* src, const char* password);

	// Encrypt data with user password and save to pwm file
	int save(const char* fname, const char* password);

private:
	int read(const char* src);
	int write(const char* fname);
	void serialize(void);
	void deserialize(void);

	std::map<std::string, std::string> entries;
	uint8_t saltSize;
	uint8_t* salt;
	uint32_t eSize;
	uint8_t* eData;
	uint8_t* data;
	size_t dataSize;
	uint8_t* keyIv;
};


#endif