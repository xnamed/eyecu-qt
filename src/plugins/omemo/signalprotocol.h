#ifndef LIBSIGNALPROTOCOL_H
#define LIBSIGNALPROTOCOL_H

#include <thirdparty/libsignal-protocol-c/signal_protocol.h>

extern "C" {
#include "sha2.h"
#include "hmac_sha2.h"
}

class SignalProtocol
{
public:
	static SignalProtocol *instance();

protected:
	/**
	 * Callback for a secure random number generator.
	 * This function shall fill the provided buffer with random bytes.
	 *
	 * @param data pointer to the output buffer
	 * @param len size of the output buffer
	 * @return 0 on success, negative on failure
	 */
	static int randomFunc(uint8_t *AData, size_t ALen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall initialize an HMAC context with the provided key.
	 *
	 * @param AHmacContext private HMAC context pointer
	 * @param AKey pointer to the key
	 * @param AKeyLen length of the key
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall update the HMAC context with the provided data
	 *
	 * @param hmac_context private HMAC context pointer
	 * @param data pointer to the data
	 * @param data_len length of the data
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall finalize an HMAC calculation and populate the output
	 * buffer with the result.
	 *
	 * @param AHmacContext private HMAC context pointer
	 * @param AOutput buffer to be allocated and populated with the result
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall free the private context allocated in
	 * hmac_sha256_init_func.
	 *
	 * @param hmac_context private HMAC context pointer
	 */
	static void hmacSha256CleanupFunc(void *AHmacContext, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall initialize a digest context.
	 *
	 * @param digest_context private digest context pointer
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestInitFunc(void **ADigestContext, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall update the digest context with the provided data.
	 *
	 * @param digest_context private digest context pointer
	 * @param data pointer to the data
	 * @param data_len length of the data
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData, size_t ADataLen, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall finalize the digest calculation, populate the
	 * output buffer with the result, and prepare the context for reuse.
	 *
	 * @param digest_context private digest context pointer
	 * @param output buffer to be allocated and populated with the result
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall free the private context allocated in
	 * sha512_digest_init_func.
	 *
	 * @param digest_context private digest context pointer
	 */
	static void sha512DigestCleanupFunc(void *ADigestContext, void *AUserData);

	/**
	 * Callback for an AES encryption implementation.
	 *
	 * @param output buffer to be allocated and populated with the ciphertext
	 * @param cipher specific cipher variant to use, either SG_CIPHER_AES_CTR_NOPADDING or SG_CIPHER_AES_CBC_PKCS5
	 * @param key the encryption key
	 * @param key_len length of the encryption key
	 * @param iv the initialization vector
	 * @param iv_len length of the initialization vector
	 * @param plaintext the plaintext to encrypt
	 * @param plaintext_len length of the plaintext
	 * @return 0 on success, negative on failure
	 */
	static int encryptFunc(signal_buffer **AOutput, int ACipher,
						   const uint8_t *AKey, size_t AKeyLen,
						   const uint8_t *AIv, size_t AIvLen,
						   const uint8_t *APlaintext, size_t APlaintextLen,
						   void *AUserData);

	/**
	 * Callback for an AES decryption implementation.
	 *
	 * @param output buffer to be allocated and populated with the plaintext
	 * @param cipher specific cipher variant to use, either SG_CIPHER_AES_CTR_NOPADDING or SG_CIPHER_AES_CBC_PKCS5
	 * @param key the encryption key
	 * @param key_len length of the encryption key
	 * @param iv the initialization vector
	 * @param iv_len length of the initialization vector
	 * @param ciphertext the ciphertext to decrypt
	 * @param ciphertext_len length of the ciphertext
	 * @return 0 on success, negative on failure
	 */
	static int decryptFunc(signal_buffer **AOutput,
						   int ACipher,
						   const uint8_t *AKey, size_t AKeyLen,
						   const uint8_t *AIv, size_t AIvLen,
						   const uint8_t *ACiphertext, size_t ACiphertextLen,
						   void *AUserData);

private:
	SignalProtocol();

	// signal-protocol
	signal_context		*FGlobalContext;

	//SHA2
	unsigned char digest[32];

	static SignalProtocol *FInstance;	
};

#endif // LIBSIGNALPROTOCOL_H