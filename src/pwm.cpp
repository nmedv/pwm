#include <pwm/pwm.h>
#include <pwm/pwm_error.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <fstream>


/*
	We use AES with 256 bit key (16 byte cipher block).
	So we need at least n + 16 (including \0) bytes to
	store out cipher data.
	  n - size of plain data (in bytes)
*/
#define AES256_ENCRYPT_SIZE(n) n + 16
#define AES256_DECRYPT_SIZE(n) n + 16

#define PWM_HEADER_SIZE reinterpret_cast<size_t>(&(reinterpret_cast<pwm_file*>(0)->cipher))


PwmFile::PwmFile()
{
	Resize(PWM_HEADER_SIZE);
	data->sigP = 'P';
	data->sigW = 'W';
}


int PwmFile::Load(const char *fileName)
{
	// Open file
	std::ifstream file(fileName, std::ios::binary);
	if (!file.is_open())
		return PwmSetError(PWM_NO_FILE, "can't open file \"%s\"", fileName);

	// Read file header
	Resize(PWM_HEADER_SIZE);
	file.read(reinterpret_cast<char*>(data), PWM_HEADER_SIZE);
	if (file.bad() || data->sigP != 'P' || data->sigW != 'W') // Check signature
	{
		file.close();
		return PwmSetError(PWM_BAD_FILE, "\"%s\" is not pwm data file", fileName);
	}

	// Read cipher data
	Resize(PWM_HEADER_SIZE + data->cipher_len);
	file.read(reinterpret_cast<char*>(&data->cipher), data->cipher_len);
	int success = file.good();
	file.close();

	if (!success)
		return PwmSetError(PWM_BAD_FILE, "error while reading file \"%s\": data corrupted", fileName);

	return 1;
}


int PwmFile::Save(const char *fileName)
{
	// Open file (or create file)
	std::ofstream file(fileName, std::ios::binary);

	// Write data to file
	file.write(reinterpret_cast<char*>(data), PWM_HEADER_SIZE + data->cipher_len);
	int success = file.good();
	file.close();

	if (!success)
		return PwmSetError(PWM_BAD_FILE, "error while writing file \"%s\"", fileName);

	return 1;
}


void PwmFile::Resize(size_t size)
{
	dataRaw.resize(size);
	data = reinterpret_cast<pwm_file*>(dataRaw.data());
}


//PwmFileHandler::PwmFileHandler()
//{ }


/*
	Pwm data table format:

	Offset                      Size (in bytes)        Header name
	-----------------------------------------------------------------
	0                           4                      Table size
	4                           1                      Key #1 size
	5                           1                      Value #1 size
	6                           1                      Key #2 size
	7                           1                      Value #2 size
	...                         ...                    ...
	Table size - 2              1                      Key #n size
	Table size - 1              1                      Value #n size
	Table size                  Key size #1            Key #1
	Table size + Key size #1    Value size #1          Value #1
	...                         ...                    ...
	...                         Key #n size            Key #n
	...                         Value #n size          Value #n
*/


void PwmFileHandler::Serialize(std::vector<uint8_t> &out, std::map<std::string, std::string> &in)
{
	// Find out the serialize size
	size_t serializeSize = in.size() * 4 + sizeof(uint32_t);
	for (auto const& pair : in)
	{
		serializeSize += pair.first.size();
		serializeSize += pair.second.size();
	}

	out.resize(serializeSize);

	size_t tableSize = in.size() * 2;
	*reinterpret_cast<uint32_t*>(out.data()) = static_cast<uint32_t>(tableSize);
	uint8_t* table = out.data() + sizeof(uint32_t);
	uint8_t* data_ptr = table + tableSize;

	for (auto const& pair : in)
	{
		*table++ = static_cast<uint8_t>(pair.first.size());
		*table++ = static_cast<uint8_t>(pair.second.size());

		strcpy((char*)data_ptr, pair.first.c_str());
		data_ptr += pair.first.size() + 1;
		strcpy((char*)data_ptr, pair.second.c_str());
		data_ptr += pair.second.size() + 1;
	}
}


void PwmFileHandler::Deserialize(std::vector<uint8_t> &in, std::map<std::string, std::string> &out)
{
	char* name;
	char* value;
	uint32_t tableSize = *reinterpret_cast<uint32_t*>(in.data());
	uint8_t* table = in.data() + sizeof(uint32_t);
	uint8_t* data_ptr = table + tableSize;

	for (uint32_t i = 0; i < tableSize; i += 2)
	{
		name = (char*)data_ptr;
		data_ptr += table[i] + 1;
		value = (char*)data_ptr;
		data_ptr += table[i + 1] + 1;
		out[std::string(name, table[i])] = std::string(value, table[i + 1]);
	}
}


int PwmFileHandler::Encrypt(std::vector<uint8_t> &in, PwmFile *file, const char *password)
{
	// Generate sault
	RAND_bytes(file->data->salt, 8);

	// Generate key and iv from password and sault
	uint8_t* key = new uint8_t[24];
	uint8_t* iv = key + 16;
	PKCS5_PBKDF2_HMAC_SHA1(password, (int)strlen(password), file->data->salt, 8, 1000, 24, key);

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx || !EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	file->Resize(PWM_HEADER_SIZE + AES256_ENCRYPT_SIZE(in.size()));
	int len;

	if (!EVP_EncryptUpdate(ctx, &file->data->cipher, &len, in.data(), (int)in.size()))
	{
		// ERR_print_errors_fp(stderr);
		return 0;
	}

	file->data->cipher_len = len;

	if (!EVP_EncryptFinal_ex(ctx, &file->data->cipher + len, &len))
	{
		// ERR_print_errors_fp(stderr);
		return 0;
	}

	file->data->cipher_len += len;

	EVP_CIPHER_CTX_free(ctx);
	delete key;

	return 1;
}


int PwmFileHandler::Decrypt(std::vector<uint8_t> &out, PwmFile *file, const char *password)
{
	// Generate key and iv from password and sault
	uint8_t* key = new uint8_t[24];
	uint8_t* iv = key + 16;
	PKCS5_PBKDF2_HMAC_SHA1(password, (int)strlen(password), file->data->salt, 8, 1000, 24, key);

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx || !EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	out.resize(AES256_DECRYPT_SIZE(file->data->cipher_len));
	int len;
	int plain_len;

	if (!EVP_DecryptUpdate(ctx, out.data(), &len, &file->data->cipher, file->data->cipher_len))
	{
		// ERR_print_errors_fp(stderr);
		return 0;
	}

	plain_len = len;

	if (!EVP_DecryptFinal_ex(ctx, out.data() + len, &len))
	{
		// ERR_print_errors_fp(stderr);
		return 0;
	}

	plain_len += len;
	out.resize(plain_len);

	EVP_CIPHER_CTX_free(ctx);
	delete key;

	return 1;
}


Pwm::Pwm(void)
	: is_loaded(false), has_changes(false)
{ }


Pwm::Pwm(const char *src, const char *password) : Pwm()
{
	Load(src, password);
}


int Pwm::Load(const char *src, const char *password)
{
	// Read file
	if (!file.Load(src))
		return 0;

	// Decrypt data
	if (!PwmFileHandler::Decrypt(data, &file, password))
		return PwmSetError(PWM_BAD_DECRYPT, "wrong password or data corrupted");

	// Deserialize data
	PwmFileHandler::Deserialize(data, entries);

	is_loaded = true;
	return 1;
}


bool Pwm::Loaded(void)
{
	return is_loaded;
}


int Pwm::Save(const char *fname, const char *password)
{
	// Can't save empty data
	if (entries.size() == 0)
		return PwmSetError(PWM_BAD_DATA, "no data to save");

	// Create new file
	if (!is_loaded)
		file = PwmFile();

	// Serialize data
	if (has_changes)
		PwmFileHandler::Serialize(data, entries);

	// Encrypt data
	if (!PwmFileHandler::Encrypt(data, &file, password))
		return PwmSetError(PWM_BAD_ENCRYPT, "can't encrypt data");

	// Save data to file
	int res = file.Save(fname);

	is_loaded = false;
	has_changes = false;

	if (!res)
		return 0;

	return 1;
}


int Pwm::Set(std::string &name, std::string &value)
{
	if (name.size() > 256 || value.size() > 256)
		return PwmSetError(PWM_BAD_NAME, "string value is too large");

	if (Has(name))
		return PwmSetError(PWM_BAD_NAME, "entry with name \"%s\" already exists", name.c_str());
	else
	{
		entries[name] = value;
		has_changes = true;
		return 1;
	}
}


int Pwm::Has(std::string &name)
{
	return !(entries.find(name) == entries.end());
}


int Pwm::Remove(std::string &name)
{
	if (entries.size() == 1)
		return PwmSetError(PWM_BAD_NAME, "can't remove last entry with name \"%s\"", name.c_str());

	if (!entries.erase(name))
		return PwmSetError(PWM_BAD_NAME, "can't remove entry with name \"%s\"", name.c_str());
	else
	{
		has_changes = true;
		return 1;
	}
}


const std::map<std::string, std::string>* Pwm::Get(void)
{
	return &entries;
}


const std::string* Pwm::Get(std::string &name)
{
	if (Has(name))
		return &entries[name];
	else
	{
		PwmSetError(PWM_BAD_NAME, "can't find entry with name \"%s\"", name.c_str());
		return reinterpret_cast<std::string*>(0);
	}		
}