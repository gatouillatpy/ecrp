
#include <memory>
#include <gcrypt.h>

#include "Crypto.h"
#include "errors/Error.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		const char aesSalt[] = ":$Z=n;d4[Yx1(8<ZyF,S/etF>Rj@f5[s";
		const char aesIV[] = "a~/:U2v@9wDC]z,6";

		static void show_sexp(const char* prefix, gcry_sexp_t a) {
			fputs(prefix, stderr);
			size_t size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
			char* buf = (char*)malloc(size);
			gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
			printf("%.*s", (int)size, buf);
		}

		static void hash(int algo, const void* pInputData, size_t inputSize, void* pOutputData, size_t outputSize) {
			gcry_md_hd_t hd;
			gpg_error_t err;
			err = gcry_md_open(&hd, algo, 0);
			if (err) {
				throw Error("Initializing hash algorithm failed: %s", gcry_strerror(err));
			}
			gcry_md_write(hd, pInputData, inputSize);
			byte* hash = gcry_md_read(hd, algo);
			if (hash == NULL) {
				gcry_md_close(hd);
				throw Error("Reading hash failed.");
			}
			memcpy(pOutputData, hash, outputSize);
			gcry_md_close(hd);
		}

		b256 sha256(const void* pInputData, size_t inputSize) {
			b256 outputData;
			hash(GCRY_MD_SHA256, pInputData, inputSize, &outputData, sizeof(outputData));
			return outputData;
		}

		void generateKey(PrivateKey* pOutput, const void* pSecretData, size_t secretSize, bool isRaw) {
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			gcry_sexp_t key_spec;
			if (pSecretData && secretSize) {
				if (isRaw) {
					err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)(secret %b)))", secretSize, pSecretData);
				} else {
					b456 d = shake256(pSecretData, secretSize, b456());
					err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)(secret %b)))", sizeof(d), &d);
				}
			} else {
				err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)))");
			}
			if (err) {
				throw Error("Creating S-expression failed: %s", gcry_strerror(err));
			}

			gcry_sexp_t key_pair;
			err = gcry_pk_genkey(&key_pair, key_spec);
			gcry_sexp_release(key_spec);
			if (err) {
				throw Error("Creating ECC key failed: %s", gcry_strerror(err));
			}

			gcry_sexp_t private_key;
			private_key = gcry_sexp_find_token(key_pair, "private-key", 0);
			gcry_sexp_release(key_pair);
			if (!private_key) {
				throw Error("Private part missing in key.");
			}

			gcry_sexp_t q_component;
			q_component = gcry_sexp_find_token(private_key, "q", 0);
			if (!q_component) {
				gcry_sexp_release(private_key);
				throw Error("Q component missing from the private key.");
			}
			buffer = (void*)gcry_sexp_nth_data(q_component, 1, &bufferSize);
			memcpy(&pOutput->q, buffer, sizeof(pOutput->q));
			gcry_sexp_release(q_component);

			gcry_sexp_t d_component;
			d_component = gcry_sexp_find_token(private_key, "d", 0);
			if (!d_component) {
				gcry_sexp_release(private_key);
				throw Error("D component missing from the private key.");
			}
			buffer = (void*)gcry_sexp_nth_data(d_component, 1, &bufferSize);
			memcpy(&pOutput->d, buffer, sizeof(pOutput->d));
			gcry_sexp_release(d_component);

			gcry_sexp_release(private_key);
		}

		PrivateKey* generateKey() {
			PrivateKey output;
			generateKey(&output, NULL, 0, true);
			return new PrivateKey(output);
		}

		DerivativeKey* deriveKey(const PrivateKey* pSourceKey, uint32_t kValue) {
			size_t secretSize = sizeof(pSourceKey->d) + sizeof(kValue);
			std::unique_ptr<byte> secretData(new byte[secretSize]);
			memcpy(secretData.get(), &pSourceKey->d, sizeof(pSourceKey->d));
			memcpy(secretData.get() + sizeof(pSourceKey->d), &kValue, sizeof(kValue));
			DerivativeKey output;
			generateKey(&output, secretData.get(), secretSize, false);
			memcpy(&output.d, &pSourceKey->d, sizeof(pSourceKey->d));
			output.k = kValue;
			return new DerivativeKey(output);
		}

		Signature* signData(void* pInputData, size_t inputSize, const PrivateKey* pPrivateKey) {
			Signature output;
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			static const char private_key_format[] =
				"(private-key\n"
				"  (ecc\n"
				"    (curve Ed448)\n"
				"      (flags eddsa)\n"
				"      (q %b)\n"
				"      (d %b)\n"
				"  )\n"
				")\n";

			gcry_sexp_t private_key;
			if (typeid(*pPrivateKey) == typeid(DerivativeKey)) {
				const DerivativeKey* pDerivativeKey = (const DerivativeKey*)(pPrivateKey);
				size_t slen = sizeof(pDerivativeKey->d) + sizeof(pDerivativeKey->k);
				std::unique_ptr<byte> sbuf(new byte[slen]);
				memcpy(sbuf.get(), &pDerivativeKey->d, sizeof(pDerivativeKey->d));
				memcpy(sbuf.get() + sizeof(pDerivativeKey->d), &pDerivativeKey->k, sizeof(pDerivativeKey->k));
				b456 d = shake256(sbuf.get(), slen, b456());
				err = gcry_sexp_build(&private_key, NULL, private_key_format, sizeof(pDerivativeKey->q), &pDerivativeKey->q, sizeof(d), &d);
			} else {
				err = gcry_sexp_build(&private_key, NULL, private_key_format, sizeof(pPrivateKey->q), &pPrivateKey->q, sizeof(pPrivateKey->d), &pPrivateKey->d);
			}
			if (err) {
				throw Error("Loading private key failed: %s", gcry_strerror(err));
			}

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, pInputData);
			if (err) {
				gcry_sexp_release(private_key);
				throw Error("Loading data failed: %s", gcry_strerror(err));
			}

			gcry_sexp_t signature;
			err = gcry_pk_sign(&signature, data, private_key);
			gcry_sexp_release(private_key);
			gcry_sexp_release(data);
			if (err) {
				throw Error("Signing data failed: %s", gcry_strerror(err));
			}

			gcry_sexp_t r_component;
			r_component = gcry_sexp_find_token(signature, "r", 0);
			if (!r_component) {
				gcry_sexp_release(signature);
				throw Error("R component missing from the private key.");
			}
			buffer = (void*)gcry_sexp_nth_data(r_component, 1, &bufferSize);
			memcpy(&output.r, buffer, sizeof(output.r));
			gcry_sexp_release(r_component);

			gcry_sexp_t s_component;
			s_component = gcry_sexp_find_token(signature, "s", 0);
			if (!s_component) {
				gcry_sexp_release(signature);
				throw Error("S component missing from the private key.");
			}
			buffer = (void*)gcry_sexp_nth_data(s_component, 1, &bufferSize);
			memcpy(&output.s, buffer, sizeof(output.s));
			gcry_sexp_release(s_component);

			gcry_sexp_release(signature);
			return new Signature(output);
		}

		bool verifyData(void* pInputData, size_t inputSize, const Signature* pInputSignature, const PublicKey* pPublicKey) {
			gpg_error_t err;

			static const char public_key_format[] =
				"(public-key\n"
				"  (ecc\n"
				"    (curve Ed448)\n"
				"      (flags eddsa)\n"
				"      (q %b)\n"
				"  )\n"
				")\n";

			gcry_sexp_t public_key;
			err = gcry_sexp_build(&public_key, NULL, public_key_format, sizeof(pPublicKey->q), &pPublicKey->q);
			if (err) {
				throw Error("Loading public key failed: %s", gcry_strerror(err));
			}

			static const char signature_format[] =
				"(signature\n"
				"  (sig-val\n"
				"    (eddsa\n"
				"      (r %b)\n"
				"      (s %b)\n"
				"    )\n"
				"  )\n"
				")\n";

			gcry_sexp_t signature;
			err = gcry_sexp_build(&signature, NULL, signature_format, sizeof(pInputSignature->r), &pInputSignature->r, sizeof(pInputSignature->s), &pInputSignature->s);
			if (err) {
				gcry_sexp_release(public_key);
				throw Error("Loading signature failed: %s", gcry_strerror(err));
			}

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, pInputData);
			if (err) {
				gcry_sexp_release(signature);
				gcry_sexp_release(public_key);
				throw Error("Loading data failed: %s", gcry_strerror(err));
			}

			err = gcry_pk_verify(signature, data, public_key);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);
			gcry_sexp_release(public_key);
			if (err) {
				throw Error("Verifying data failed: %s", gcry_strerror(err));
			}

			return true;
		}

		b512* lockKey(const PrivateKey* pKey, const string& password) {
			gpg_error_t err;

			size_t slen = password.size() + sizeof(aesSalt) - 1;
			std::unique_ptr<byte> sbuf(new byte[slen]);
			memcpy(sbuf.get(), password.c_str(), password.size());
			memcpy(sbuf.get() + password.size(), aesSalt, sizeof(aesSalt) - 1);
			b256 hash = sha256(sbuf.get(), slen);

			gcry_cipher_hd_t handle;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				throw Error("Initializing cipher algorithm failed: %s", gcry_strerror(err));
			}

			err = gcry_cipher_setkey(handle, &hash, sizeof(hash));
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher key failed: %s", gcry_strerror(err));
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher IV failed: %s", gcry_strerror(err));
			}

			b512 input;
			memset(&input, 0, sizeof(input));
			memcpy(&input, &pKey->d, sizeof(pKey->d));

			b512 output;
			err = gcry_cipher_encrypt(handle, &output, sizeof(output), &input, sizeof(input));
			gcry_cipher_close(handle);
			if (err) {
				throw Error("Encrypting data failed: %s", gcry_strerror(err));
			}
			return new b512(output);
		}

		PrivateKey* unlockKey(const b512* pInput, const string& password) {
			gpg_error_t err;

			size_t slen = password.size() + sizeof(aesSalt) - 1;
			std::unique_ptr<byte> sbuf(new byte[slen]);
			memcpy(sbuf.get(), password.c_str(), password.size());
			memcpy(sbuf.get() + password.size(), aesSalt, sizeof(aesSalt) - 1);
			b256 hash = sha256(sbuf.get(), slen);

			gcry_cipher_hd_t handle;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				throw Error("Initializing cipher algorithm failed: %s", gcry_strerror(err));
			}

			err = gcry_cipher_setkey(handle, &hash, sizeof(hash));
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher key failed: %s", gcry_strerror(err));
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher IV failed: %s", gcry_strerror(err));
			}

			b512 secret;
			err = gcry_cipher_decrypt(handle, &secret, sizeof(secret), pInput, sizeof(*pInput));
			gcry_cipher_close(handle);
			if (err) {
				throw Error("Decrypting data failed: %s", gcry_strerror(err));
			}

			b456 d;
			memcpy(&d, &secret, sizeof(d));

			PrivateKey output;
			generateKey(&output, &d, sizeof(d), true);
			return new PrivateKey(output);
		}
	}
}