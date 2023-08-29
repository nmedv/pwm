#ifndef PWM_H
#define PWM_H

#include "getopt.h"
#include "error.h"

#include <string>
#include <map>
#include <vector>

#include "shared.h"


struct pwm_args
{
	int		info;
	int		remove;
	char	*name;
	char	*value;
	char	*source;
	int		force;
};

#ifdef __GNUC__
#define PACK_START
#define PACK_END   __attribute__((__packed__));
#endif

#ifdef _MSC_VER
#define PACK_START __pragma(pack(push, 1))
#define PACK_END   ; __pragma(pack(pop))
#endif

PACK_START struct pwm_file {
	char      sigP;
	char      sigW;
	uint8_t   salt[8];
	uint32_t  cipher_len;
	uint8_t   cipher;
} PACK_END

enum PwmFileType : uint8_t
{
	PWM_FILE_TABLE,
	PWM_FILE_TEXT
};


class PWM_SHARED PwmFile
{
public:
	pwm_file* data;

	PwmFile();
	int Load(const char *fileName);
	int Save(const char *fileName);
	void Resize(size_t size);

private:
	std::vector<char> dataRaw;
};

PWM_SHARED void PwmSerialize(std::vector<uint8_t> &out, std::map<std::string, std::string> &in);
PWM_SHARED void PwmDeserialize(std::vector<uint8_t> &in, std::map<std::string, std::string> &out);
PWM_SHARED int PwmEncrypt(std::vector<uint8_t> &in, PwmFile *file, const char *password);
PWM_SHARED int PwmDecrypt(std::vector<uint8_t> &out, PwmFile *file, const char *password);


class PWM_SHARED Pwm
{
public:
	Pwm(void);
	Pwm(const char *src, const char *password);
	//~Pwm(void);

	// Decrypt data from pwm file with user password
	int Load(const char *src, const char *password);

	// Check if file is loaded or not
	bool Loaded(void);

	// Set new entry value
	int Set(std::string &name, std::string &value);

	// Check if the entry name exists in pwm data
	int Has(std::string &name);

	// Remove entry
	int Remove(std::string &name);

	// Get all data
	const std::map<std::string, std::string>* Get(void);

	// Get value of entry
	const std::string* Get(std::string &name);

	// Encrypt data with user password and save to pwm file
	int Save(const char *fname, const char *password);

private:
	bool is_loaded;
	bool has_changes;
	std::vector<uint8_t> data;
	std::map<std::string, std::string> entries;

	PwmFile file;
};


PWM_SHARED Pwm* CreatePwmInstance(const char *src, const char *password);


#endif