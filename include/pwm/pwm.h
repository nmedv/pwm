#ifndef PWM_H
#define PWM_H

#include <cstdint>
#include <string>
#include <map>
#include <vector>


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
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

PACK(
struct pwm_file {
	char		sigP;
	char		sigW;
	uint8_t		salt[8];
	uint32_t	cipher_len;
	uint8_t		cipher;
};)


class PwmFile
{
public:
	pwm_file *data;

	PwmFile();
	int Load(const char* fileName);
	int Save(const char* fileName);
	void Resize(size_t size);

private:
	std::vector<char> dataRaw;
};


class PwmFileHandler
{
public:
	// PwmFileHandler();
	//~PwmFileHandler();

	static void Serialize(std::vector<uint8_t> &out, std::map<std::string, std::string> &in);
	static void Deserialize(std::vector<uint8_t> &in, std::map<std::string, std::string> &out);
	static int Encrypt(std::vector<uint8_t> &in, PwmFile *file, const char *password);
	static int Decrypt(std::vector<uint8_t> &out, PwmFile *file, const char *password);
};


class Pwm
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


#endif