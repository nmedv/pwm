#ifndef PWM_CRYPTO_H
#define PWM_CRYPTO_H

#include <stdint.h>

// We use AES with 256 bit key (16 byte cipher block).
// So we need at least n + 16 (including \0) bytes to
// store out cipher data.
//   n - size of plain data (in bytes)
#define AES256_ENCRYPT_SIZE(n) n + 16
#define AES256_DECRYPT_SIZE(n) n + 16

#define B64_ENCODE_SIZE(n) (n + 3) & ~3
#define B64_DECODE_SIZE(n) n * 3 / 4

#ifdef __cplusplus
extern "C" {
#endif


int aes_encrypt(
	const uint8_t* data,
	int ndata,
	uint8_t* cipher_data,
	const uint8_t* key,
	const uint8_t* iv
);

int aes_decrypt(
	const uint8_t* cipher_data,
	int ncipher_data,
	uint8_t* data,
	const uint8_t* key,
	const uint8_t* iv
);

int b64encode(const uint8_t* data, int ndata, uint8_t* data_encoded);

int b64d_size(int ndata_encoded);

int b64decode(const uint8_t* data_encoded, int ndata_encoded, uint8_t* data);

int gensalt(uint8_t* buf, int num);

int genkeyiv(const char* pass, uint8_t* salt, size_t saltlen, uint8_t* buf);


#ifdef __cplusplus
}
#endif

#endif