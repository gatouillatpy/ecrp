
#include <gcrypt.h>

#include "Crypto.h"

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

		static void hash(int algo, const void* inputData, size_t inputSize, void* outputData, size_t outputSize) {
			gcry_md_hd_t hd;
			gcry_md_open(&hd, algo, 0);
			gcry_md_write(hd, inputData, inputSize);
			byte* hash = gcry_md_read(hd, algo);
			memcpy(outputData, hash, outputSize);
			gcry_md_close(hd);
		}

		b256 sha256(const void* inputData, size_t inputSize) {
			b256 outputData;
			hash(GCRY_MD_SHA256, inputData, inputSize, &outputData, sizeof(outputData));
			return outputData;
		}

		PrivateKey generateKey(const void* secretData, size_t secretSize, bool isRaw) {
			PrivateKey output;
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			gcry_sexp_t key_spec;
			if (secretData && secretSize) {
				if (isRaw) {
					err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)(secret %b)))", secretSize, secretData);
				} else {
					b456 d = shake256(secretData, secretSize, b456());
					err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)(secret %b)))", sizeof(d), &d);
				}
			} else {
				err = gcry_sexp_build(&key_spec, NULL, "(genkey (ecdsa (curve \"Ed448\")(flags eddsa)))");
			}
			if (err) {
				printf("Creating S-expression failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t key_pair;
			err = gcry_pk_genkey(&key_pair, key_spec);
			if (err) {
				printf("Creating ECC key failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t private_key;
			private_key = gcry_sexp_find_token(key_pair, "private-key", 0);
			if (!private_key) {
				printf("Private part missing in key.\n");
				return output;
			}

			gcry_sexp_t q_component;
			q_component = gcry_sexp_find_token(private_key, "q", 0);
			if (!q_component) {
				printf("Q component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(q_component, 1, &bufferSize);
			memcpy(&output.q, buffer, sizeof(output.q));
			gcry_sexp_release(q_component);

			gcry_sexp_t d_component;
			d_component = gcry_sexp_find_token(private_key, "d", 0);
			if (!d_component) {
				printf("D component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(d_component, 1, &bufferSize);
			memcpy(&output.d, buffer, sizeof(output.d));
			gcry_sexp_release(d_component);

			gcry_sexp_release(private_key);
			gcry_sexp_release(key_pair);
			gcry_sexp_release(key_spec);

			return output;
		}

		PrivateKey generateKey() {
			return generateKey(NULL, 0, true);
		}

		DerivativeKey deriveKey(const PrivateKey& sourceKey, uint32_t kValue) {
			size_t secretSize = sizeof(sourceKey.d) + sizeof(kValue);
			byte* secretData = (byte*)malloc(secretSize);
			memcpy(secretData, &sourceKey.d, sizeof(sourceKey.d));
			memcpy(secretData + sizeof(sourceKey.d), &kValue, sizeof(kValue));
			DerivativeKey output = static_cast<DerivativeKey&>(generateKey(secretData, secretSize, false));
			memcpy(&output.d, &sourceKey.d, sizeof(sourceKey.d));
			output.k = kValue;
			free(secretData);
			return output;
		}

		Signature signData(void* inputData, size_t inputSize, const PrivateKey& privateKey) {
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
			if (typeid(privateKey) == typeid(DerivativeKey)) {
				const DerivativeKey& derivativeKey = static_cast<const DerivativeKey&>(privateKey);
				size_t slen = sizeof(derivativeKey.d) + sizeof(derivativeKey.k);
				byte* sbuf = (byte*)malloc(slen);
				memcpy(sbuf, &derivativeKey.d, sizeof(derivativeKey.d));
				memcpy(sbuf + sizeof(derivativeKey.d), &derivativeKey.k, sizeof(derivativeKey.k));
				b456 d = shake256(sbuf, slen, b456());
				err = gcry_sexp_build(&private_key, NULL, private_key_format, sizeof(derivativeKey.q), &derivativeKey.q, sizeof(d), &d);
				free(sbuf);
			} else {
				err = gcry_sexp_build(&private_key, NULL, private_key_format, sizeof(privateKey.q), &privateKey.q, sizeof(privateKey.d), &privateKey.d);
			}
			if (!private_key) {
				printf("Loading private key failed: %s\n", gcry_strerror(err));
				return output;
			}

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, inputData);
			if (err) {
				printf("Loading data failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t signature;
			err = gcry_pk_sign(&signature, data, private_key);
			if (err) {
				printf("Signing data failed: %s\n", gcry_strerror(err));
				return output;
			}

			gcry_sexp_t r_component;
			r_component = gcry_sexp_find_token(signature, "r", 0);
			if (!r_component) {
				printf("R component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(r_component, 1, &bufferSize);
			memcpy(&output.r, buffer, sizeof(output.r));

			gcry_sexp_t s_component;
			s_component = gcry_sexp_find_token(signature, "s", 0);
			if (!s_component) {
				printf("S component missing from the private key.\n");
				return output;
			}
			buffer = (void*)gcry_sexp_nth_data(s_component, 1, &bufferSize);
			memcpy(&output.s, buffer, sizeof(output.s));

			gcry_sexp_release(s_component);
			gcry_sexp_release(r_component);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);
			gcry_sexp_release(private_key);

			return output;
		}

		bool verifyData(void* inputData, size_t inputSize, const Signature& inputSignature, const PublicKey& publicKey) {
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
			err = gcry_sexp_build(&public_key, NULL, public_key_format, sizeof(publicKey.q), &publicKey.q);
			if (!public_key) {
				printf("Loading public key failed: %s\n", gcry_strerror(err));
				return false;
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
			err = gcry_sexp_build(&signature, NULL, signature_format, sizeof(inputSignature.r), &inputSignature.r, sizeof(inputSignature.s), &inputSignature.s);
			if (!signature) {
				printf("Loading signature failed: %s\n", gcry_strerror(err));
				return false;
			}

			static const char data_format[] =
				"(data\n"
				"  (flags eddsa)\n"
				"  (hash-algo shake256)\n"
				"  (value %b)\n"
				")\n";

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, data_format, inputSize, inputData);
			if (err) {
				printf("Loading data failed: %s\n", gcry_strerror(err));
				return false;
			}

			err = gcry_pk_verify(signature, data, public_key);
			if (err) {
				printf("Verifying data failed: %s\n", gcry_strerror(err));
				return false;
			}

			gcry_sexp_release(public_key);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);

			return true;
		}

		b512 lockKey(const PrivateKey& key, const string& password) {
			char hash[32];

			gcry_md_hd_t hd;
			gcry_md_open(&hd, GCRY_MD_SHA256, 0);
			gcry_md_write(hd, password.c_str(), password.size());
			gcry_md_write(hd, aesSalt, sizeof(aesSalt) - 1);
			memcpy(hash, gcry_md_read(hd, GCRY_MD_SHA256), sizeof(hash));
			gcry_md_close(hd);

			gcry_cipher_hd_t handle;
			gpg_error_t err;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				printf("Error in gcry_cipher_open\n");
				return b512();
			}

			err = gcry_cipher_setkey(handle, hash, sizeof(hash));
			if (err) {
				printf("Error in gcry_cipher_setkey\n");
				return b512();
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				printf("Error in gcry_cipher_setiv\n");
				return b512();
			}

			b512 input;
			memset(&input, 0, sizeof(input));
			memcpy(&input, &key.d, sizeof(key.d));

			b512 output;
			err = gcry_cipher_encrypt(handle, &output, sizeof(output), &input, sizeof(input));
			if (err) {
				printf("Error in gcry_cipher_encrypt\n");
				return b512();
			}
			gcry_cipher_close(handle);
			return output;
		}

		PrivateKey unlockKey(const b512& input, const string& password) {
			char hash[32];

			gcry_md_hd_t hd;
			gcry_md_open(&hd, GCRY_MD_SHA256, 0);
			gcry_md_write(hd, password.c_str(), password.size());
			gcry_md_write(hd, aesSalt, sizeof(aesSalt) - 1);
			memcpy(hash, gcry_md_read(hd, GCRY_MD_SHA256), sizeof(hash));
			gcry_md_close(hd);

			gcry_cipher_hd_t handle;
			gpg_error_t err;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				printf("Error in gcry_cipher_open\n");
				return PrivateKey();
			}

			err = gcry_cipher_setkey(handle, hash, sizeof(hash));
			if (err) {
				printf("Error in gcry_cipher_setkey\n");
				return PrivateKey();
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				printf("Error in gcry_cipher_setiv\n");
				return PrivateKey();
			}

			b512 output;
			err = gcry_cipher_decrypt(handle, &output, sizeof(output), &input, sizeof(input));
			if (err) {
				printf("Error in gcry_cipher_encrypt\n");
				return PrivateKey();
			}
			gcry_cipher_close(handle);

			b456 d = b456();
			memcpy(&d, &output, sizeof(d));

			return generateKey(&d, sizeof(d), true);
		}
	}
}