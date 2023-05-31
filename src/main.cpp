#include <cstdio>
#include <string.h>
#include <pwm.h>
#include <aes.h>
#include <openssl/conf.h>
#include <openssl/evp.h>

int main(int argc, char* argv[])
{
	// pwm_args args = {};

	// int res

	// if (!pwm_parse_args(argc, argv, &args))
	// 	return 1;

	// if (!args.info)
	// {
	// 	printf("\nremove: %d\n", args.remove);
	// 	printf("name: %s\n", args.name);
	// 	printf("password: %s\n", args.password);
	// 	printf("source: %s\n", args.source);
	// 	printf("force: %d\n", args.force);
	// }

	unsigned char *plaintext =
		(unsigned char *)"The quick brown fox jumps over the lazy dog";
	int plaintext_size = strlen((char*)plaintext);

	// int decryptedtext_size = AES256_DECRYPT_SIZE(cipher_len);
	// unsigned char* decryptedtext = (unsigned char*)malloc(decryptedtext_size);
	// int decrypted_len = AES_decrypt(ciphertext, cipher_len, decryptedtext, key, iv);

	// printf("Ciphertext is:\n%s\n", (char*)ciphertext_b64);

	// cipher_len = b64decode(ciphertext_b64, cipher_len_b64, ciphertext);
	
	// decryptedtext[decrypted_len] = '\0';

	// printf("Decrypted text is:\n%s\n", decryptedtext);

	return 0;
}