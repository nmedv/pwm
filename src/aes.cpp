#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>


int AES_encrypt(
	const unsigned char* data,
	int ndata,
	unsigned char* cipher_data,
	const unsigned char* key,
	const unsigned char* iv)
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


int AES_decrypt(
	const unsigned char* cipher_data,
	int ncipher_data,
	unsigned char* data,
	const unsigned char* key,
	const unsigned char* iv)
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


int b64encode(
	const unsigned char* data,
	int ndata,
	unsigned char* data_encoded)
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


int b64decode(
	const unsigned char* data_encoded,
	int ndata_encoded,
	unsigned char* data)
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