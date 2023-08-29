#define PWM_CORE_EXPORT
#include <pwm/error.h>
// #include <pwm/pwm.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <fstream>
#include <vector>
#include <map>


#ifdef _DEBUG

static void _dumpmem(uint8_t* mem, int size, const char* title)
{
	if (title)
		printf("%s (%d bytes):", title, size);
	else
	{
		char format[32];
		int addrsz = sizeof(mem) * 2 + 2;
		sprintf(format, "Dump at %%#0%dx (%d bytes):", addrsz, size);
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

#define DUMP(mem, sz, title) _dumpmem(mem, sz, title)
#define OSSL_PRINTERROR ERR_print_errors_fp(stderr);
#else
#define DUMP(mem, sz, title)
#define OSSL_PRINTERROR
#endif


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


/*
	We use AES with 256 bit key (16 byte cipher block).
	So we need at least n + 16 (including \0) bytes to
	store out cipher data.
	  n - size of plain data (in bytes)
*/
#define AES256_ENCRYPT_SIZE(n) n + 16
#define AES256_DECRYPT_SIZE(n) n + 16

#define PWM_HEADER_SIZE reinterpret_cast<size_t>(&(reinterpret_cast<pwm_file*>(0)->cipher))

class PWM_SHARED PwmFile
{
public:
	pwm_file* data;

	PwmFile()
	{
		Resize(PWM_HEADER_SIZE);
		data->sigP = 'P';
		data->sigW = 'W';
	}


	int Load(const char *fileName)
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


	int Save(const char *fileName)
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


	void Resize(size_t size)
	{
		dataRaw.resize(size);
		data = reinterpret_cast<pwm_file*>(dataRaw.data());
	}

private:
	std::vector<char> dataRaw;
};


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


PWM_SHARED void PwmSerialize(std::vector<uint8_t> &out, std::map<std::string, std::string> &in)
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


PWM_SHARED void PwmDeserialize(std::vector<uint8_t> &in, std::map<std::string, std::string> &out)
{
	uint8_t* data = in.data();
	char *name, *value;
	
	uint32_t tableSize = *reinterpret_cast<uint32_t*>(data);
	uint8_t* table = data + sizeof(uint32_t);
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


PWM_SHARED int PwmEncrypt(std::vector<uint8_t> &in, PwmFile *file, const char *password)
{
	// Generate sault
	RAND_bytes(file->data->salt, 8);

	// Generate key and iv from password and sault
	uint8_t* key = new uint8_t[48];
	uint8_t* iv = key + 32;
	PKCS5_PBKDF2_HMAC_SHA1(password, (int)strlen(password), file->data->salt, 8, 1000, 48, key);
	DUMP(key, 32, "Key");
	DUMP(iv, 16, "IV");

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx || !EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		OSSL_PRINTERROR
		return 0;
	}

	file->Resize(PWM_HEADER_SIZE + AES256_ENCRYPT_SIZE(in.size()));
	int len;
	
	DUMP(in.data(), in.size(), "in");
	if (!EVP_EncryptUpdate(ctx, &file->data->cipher, &len, in.data(), (int)in.size()))
	{
		OSSL_PRINTERROR
		return 0;
	}

	file->data->cipher_len = len;

	if (!EVP_EncryptFinal_ex(ctx, &file->data->cipher + len, &len))
	{
		OSSL_PRINTERROR
		return 0;
	}

	file->data->cipher_len += len;

	DUMP(&file->data->cipher, file->data->cipher_len, "out");

	EVP_CIPHER_CTX_free(ctx);
	delete key;

	return 1;
}


PWM_SHARED int PwmDecrypt(std::vector<uint8_t> &out, PwmFile *file, const char *password)
{
	// Generate key and iv from password and sault
	uint8_t* key = new uint8_t[48];
	uint8_t* iv = key + 32;
	PKCS5_PBKDF2_HMAC_SHA1(password, (int)strlen(password), file->data->salt, 8, 1000, 48, key);
	DUMP(key, 32, "Key");
	DUMP(iv, 16, "IV");

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx || !EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		OSSL_PRINTERROR
		return 0;
	}

	out.resize(AES256_DECRYPT_SIZE(file->data->cipher_len));
	int len;
	int plain_len;

	DUMP(&file->data->cipher, file->data->cipher_len, "in");

	if (!EVP_DecryptUpdate(ctx, out.data(), &len, &file->data->cipher, file->data->cipher_len))
	{
		OSSL_PRINTERROR
		return 0;
	}

	plain_len = len;

	if (!EVP_DecryptFinal_ex(ctx, out.data() + len, &len))
	{
		OSSL_PRINTERROR
		return 0;
	}

	plain_len += len;
	out.resize(plain_len);

	DUMP(out.data(), out.size(), "out");

	EVP_CIPHER_CTX_free(ctx);
	delete key;

	return 1;
}

class PWM_SHARED Pwm
{
	bool is_loaded;
	bool has_changes;
	std::vector<uint8_t> data;
	std::map<std::string, std::string> entries;
	PwmFile file;

public:
	Pwm(void)
		: is_loaded(false), has_changes(false)
	{ }


	Pwm(const char *src, const char *password) : Pwm()
	{
		Load(src, password);
	}


	int Load(const char *src, const char *password)
	{
		// Read file
		if (!file.Load(src))
			return 0;

		// Decrypt data
		if (!PwmDecrypt(data, &file, password))
			return PwmSetError(PWM_BAD_DECRYPT, "wrong password or data corrupted");

		// Deserialize data
		PwmDeserialize(data, entries);

		is_loaded = true;
		return 1;
	}


	bool Loaded(void)
	{
		return is_loaded;
	}


	int Save(const char *fname, const char *password)
	{
		// Can't save empty data
		if (entries.size() == 0)
			return PwmSetError(PWM_BAD_DATA, "no data to save");

		// Create new file
		if (!is_loaded)
			file = PwmFile();

		// Serialize data
		if (has_changes)
			PwmSerialize(data, entries);

		// Encrypt data
		if (!PwmEncrypt(data, &file, password))
			return PwmSetError(PWM_BAD_ENCRYPT, "can't encrypt data");

		// Save data to file
		int res = file.Save(fname);

		is_loaded = false;
		has_changes = false;

		if (!res)
			return 0;

		return 1;
	}


	int Set(std::string &name, std::string &value)
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


	int Has(std::string &name)
	{
		return !(entries.find(name) == entries.end());
	}


	int Remove(std::string &name)
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


	const std::map<std::string, std::string>* Get(void)
	{
		return &entries;
	}


	const std::string* Get(std::string &name)
	{
		if (Has(name))
			return &entries[name];
		else
		{
			PwmSetError(PWM_BAD_NAME, "can't find entry with name \"%s\"", name.c_str());
			return reinterpret_cast<std::string*>(0);
		}		
	}
};

PWM_SHARED Pwm* CreatePwmInstance(const char *src, const char *password)
{
	return new Pwm(src, password);
}