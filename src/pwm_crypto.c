#include <string.h>
#include <stdint.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>


int aes_encrypt(
	const uint8_t* data,
	int ndata,
	uint8_t* cipher_data,
	const uint8_t* key,
	const uint8_t* iv)
{
	int len;
	int cipher_len;

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx || !EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	if (!cipher_data)	// Get sufficient size if out is NULL
	{
		int sufficient_size = ndata + 
			EVP_CIPHER_CTX_get_block_size(ctx) - 1;
		EVP_CIPHER_CTX_free(ctx);
		return sufficient_size;
	}

	if (!EVP_EncryptUpdate(ctx, cipher_data, &len, data, ndata))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	cipher_len = len;

	if (!EVP_EncryptFinal_ex(ctx, cipher_data + len, &len))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	cipher_len += len;

	EVP_CIPHER_CTX_free(ctx);

	return cipher_len;
}

int aes_decrypt(
	const uint8_t* cipher_data,
	int ncipher_data,
	uint8_t* data,
	const uint8_t* key,
	const uint8_t* iv)
{
	int len;
	int plain_len;

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx ||
		!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key, iv))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	if (!data)	// Get sufficient size if out is NULL
	{
		int sufficient_size = ncipher_data + 
			EVP_CIPHER_CTX_get_block_size(ctx);
		EVP_CIPHER_CTX_free(ctx);
		return sufficient_size;
	}

	if (!EVP_DecryptUpdate(ctx, data, &len, cipher_data, ncipher_data))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	plain_len = len;

	if (!EVP_DecryptFinal_ex(ctx, data + len, &len))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	plain_len += len;

	EVP_CIPHER_CTX_free(ctx);

	return plain_len;
}

int b64encode(const uint8_t* data, int ndata, uint8_t* data_encoded)
{
	int len;
	int encode_len;

	EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
	if (!ctx)
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	EVP_EncodeInit(ctx);

	if (!EVP_EncodeUpdate(ctx, data_encoded, &len, data, ndata))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	encode_len = len;
	EVP_EncodeFinal(ctx, data_encoded, &len);
	encode_len += len;

	EVP_ENCODE_CTX_free(ctx);

	return encode_len;
}

int b64decode(const uint8_t* data_encoded, int ndata_encoded, uint8_t* data)
{
	int len;
	int decode_len;

	EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
	if (!ctx)
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	EVP_EncodeInit(ctx);

	if (!EVP_DecodeUpdate(ctx, data, &len, data_encoded, ndata_encoded))
	{
		ERR_print_errors_fp(stderr);
		return 0;
	}

	decode_len = len;
	EVP_EncodeFinal(ctx, data, &len);
	decode_len += len;

	EVP_ENCODE_CTX_free(ctx);

	return decode_len;
}

int RAND_bytes(uint8_t* buf, int num);
int gensalt(uint8_t* buf, int num)
{
	return RAND_bytes(buf, num);
}

int genkeyiv(const char* pass, uint8_t* salt, size_t saltlen, uint8_t* buf)
{
	return PKCS5_PBKDF2_HMAC_SHA1(pass, 0, salt, saltlen, 1000, 24, buf);
}