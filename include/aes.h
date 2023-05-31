
// We use AES with 256 bit key (16 byte cipher block).
// So we need at least n + 16 (including \0) bytes to
// store out cipher data.
//   n - size of plain data (in bytes)
#define AES256_ENCRYPT_SIZE(n) n + 16
#define AES256_DECRYPT_SIZE(n) n + 16

#define B64_ENCODE_SIZE(n) (n + 3) & ~3
#define B64_DECODE_SIZE(n) n * 3 / 4

int AES_encrypt(
	const unsigned char* data,
	int ndata,
	unsigned char* cipher_data,
	const unsigned char* key,
	const unsigned char* iv
);

int AES_decrypt(
	const unsigned char* cipher_data,
	int ncipher_data,
	unsigned char* data,
	const unsigned char* key,
	const unsigned char* iv
);

int b64encode(
	const unsigned char* data,
	int ndata,
	unsigned char* data_encoded
);

int b64d_size(int ndata_encoded);

int b64decode(
	const unsigned char* data_encoded,
	int ndata_encoded,
	unsigned char* data
);