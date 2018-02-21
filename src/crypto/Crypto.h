
#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <type_traits>
#include <gcrypt.h>

using std::string;
using std::stringstream;
using std::stoul;

#include "utils/byte.h"
#include "errors/Error.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		static const char aesSalt[] = ":$Z=n;d4[Yx1(8<ZyF,S/etF>Rj@f5[s";
		static const char aesIV[] = "a~/:U2v@9wDC]z,6";

		extern const char format_E168_generateKey[];
		extern const char format_E168_generateKey_withSecret[];
		extern const char format_E168_signData_privateKey[];
		extern const char format_E168_verifyData_publicKey[];
		extern const char format_E168_verifyData_signature[];
		extern const char format_E168_data[];
		extern const char format_Ed25519_generateKey[];
		extern const char format_Ed25519_generateKey_withSecret[];
		extern const char format_Ed25519_signData_privateKey[];
		extern const char format_Ed25519_verifyData_publicKey[];
		extern const char format_Ed25519_verifyData_signature[];
		extern const char format_Ed25519_data[];
		extern const char format_Ed448_generateKey[];
		extern const char format_Ed448_generateKey_withSecret[];
		extern const char format_Ed448_signData_privateKey[];
		extern const char format_Ed448_verifyData_publicKey[];
		extern const char format_Ed448_verifyData_signature[];
		extern const char format_Ed448_data[];

		template<class bXXX> inline const char* format_generateKey() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_generateKey;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_generateKey;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_generateKey;
			}
		}
		template<class bXXX> inline const char* format_generateKey_withSecret() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_generateKey_withSecret;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_generateKey_withSecret;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_generateKey_withSecret;
			}
		}
		template<class bXXX> inline const char* format_signData_privateKey() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_signData_privateKey;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_signData_privateKey;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_signData_privateKey;
			}
		}
		template<class bXXX> inline const char* format_verifyData_publicKey() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_verifyData_publicKey;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_verifyData_publicKey;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_verifyData_publicKey;
			}
		}
		template<class bXXX> inline const char* format_verifyData_signature() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_verifyData_signature;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_verifyData_signature;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_verifyData_signature;
			}
		}
		template<class bXXX> inline const char* format_data() {
			if (std::is_same<bXXX,b176>::value) {
				return format_E168_data;
			} else if (std::is_same<bXXX,b256>::value) {
				return format_Ed25519_data;
			} else if (std::is_same<bXXX,b456>::value) {
				return format_Ed448_data;
			}
		}

		template<size_t n> struct generic_blob {
			byte b[n];

			generic_blob() {
				for (int i(0); i < n; ++i) {
					b[i] = 0;
				}
			}

			generic_blob(const string& s) {
				for (int i(0); i < n; ++i) {
					b[i] = (byte)stoul(s.substr(2 * i, 2), 0, 16);
				}
			}

			template<size_t z> bool equals(const generic_blob<z>& other) {
				if (z == n) {
					for (int i(0); i < n; ++i) {
						if (b[i] != other.b[i]) {
							return false;
						}
					}
					return true;
				}
				return false;
			}

			template<size_t z> bool equalsRegardlessOfSize(const generic_blob<z>& other) {
				size_t w = min(n, z);

				for (int i(0); i < w; ++i) {
					if (b[i] != other.b[i]) {
						return false;
					}
				}

				return true;
			}

			string toString() {
				stringstream ss;
				for (int i(0); i < n; ++i) {
					ss << std::setw(2) << std::setfill('0') << std::hex << (int)b[i];
				}
				return ss.str();
			}
		};

		template<size_t n> generic_blob<n> shake256(const void* pInputData, size_t inputSize, const generic_blob<n>& outputData) {
			gcry_md_hd_t hd;
			gpg_error_t err;
			gcry_md_open(&hd, GCRY_MD_SHAKE256, 0);
			gcry_md_write(hd, pInputData, inputSize);
			err = gcry_md_extract(hd, GCRY_MD_SHAKE256, (void*)&outputData, n);
			if (err) {
				throw Error("Extracting hash failed: %s", gcry_strerror(err));
			}
			gcry_md_close(hd);
			return outputData;
		}

		typedef generic_blob<15> b120;
		typedef generic_blob<22> b176;
		typedef generic_blob<32> b256;
		typedef generic_blob<57> b456;
		typedef generic_blob<64> b512;

		template<class bXXX> struct PublicKey {
			bXXX q;

			virtual ~PublicKey() {};
		};

		template<class bXXX> struct PrivateKey : PublicKey<bXXX> {
			bXXX d; // TODO: make sure this is stored securely

			virtual ~PrivateKey() {};
		};

		template<class bXXX> struct DerivativeKey : PrivateKey<bXXX> {
			uint32_t k;

			DerivativeKey() : k(0) {}
			virtual ~DerivativeKey() {};
		};

		template<class bXXX> struct Signature {
			bXXX r;
			bXXX s;
		};

		typedef PublicKey<b176> FastPublicKey;
		typedef PrivateKey<b176> FastPrivateKey;
		typedef DerivativeKey<b176> FastDerivativeKey;
		typedef Signature<b176> FastSignature;

		typedef PublicKey<b256> BalancedPublicKey;
		typedef PrivateKey<b256> BalancedPrivateKey;
		typedef DerivativeKey<b256> BalancedDerivativeKey;
		typedef Signature<b256> BalancedSignature;

		typedef PublicKey<b456> StrongPublicKey;
		typedef PrivateKey<b456> StrongPrivateKey;
		typedef DerivativeKey<b456> StrongDerivativeKey;
		typedef Signature<b456> StrongSignature;

		b256 sha256(const void* pInputData, size_t inputSize);

		template<class bXXX> void generateKey(PrivateKey<bXXX>* pOutput, const void* pSecretData, size_t secretSize, bool isRaw) {
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			gcry_sexp_t key_spec;
			if (pSecretData && secretSize) {
				if (isRaw) {
					err = gcry_sexp_build(&key_spec, NULL, format_generateKey_withSecret<bXXX>(), secretSize, pSecretData);
				}
				else {
					b456 d = shake256(pSecretData, secretSize, b456());
					err = gcry_sexp_build(&key_spec, NULL, format_generateKey_withSecret<bXXX>(), sizeof(d), &d);
				}
			}
			else {
				err = gcry_sexp_build(&key_spec, NULL, format_generateKey<bXXX>());
			}
			if (err) {
				throw Error("Creating S-expression failed: %d", err);
			}

			gcry_sexp_t key_pair;
			err = gcry_pk_genkey(&key_pair, key_spec);
			gcry_sexp_release(key_spec);
			if (err) {
				throw Error("Creating ECC key failed: %d", err);
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

		template<class bXXX> PrivateKey<bXXX>* generateKey() {
			PrivateKey<bXXX> output;
			generateKey(&output, NULL, 0, true);
			return new PrivateKey<bXXX>(output);
		}

		template<class bXXX> DerivativeKey<bXXX>* deriveKey(const PrivateKey<bXXX>* pSourceKey, uint32_t kValue) {
			size_t secretSize = sizeof(pSourceKey->d) + sizeof(kValue);
			std::unique_ptr<byte> secretData(new byte[secretSize]);
			memcpy(secretData.get(), &pSourceKey->d, sizeof(pSourceKey->d));
			memcpy(secretData.get() + sizeof(pSourceKey->d), &kValue, sizeof(kValue));
			DerivativeKey<bXXX> output;
			generateKey(&output, secretData.get(), secretSize, false);
			memcpy(&output.d, &pSourceKey->d, sizeof(pSourceKey->d));
			output.k = kValue;
			return new DerivativeKey<bXXX>(output);
		}

		template<class bXXX> Signature<bXXX>* signData(void* pInputData, size_t inputSize, const PrivateKey<bXXX>* pPrivateKey) {
			Signature<bXXX> output;
			void* buffer;
			uint32_t bufferSize;
			gpg_error_t err;

			gcry_sexp_t private_key;
			if (typeid(*pPrivateKey) == typeid(DerivativeKey<bXXX>)) {
				const DerivativeKey<bXXX>* pDerivativeKey = (const DerivativeKey<bXXX>*)(pPrivateKey);
				size_t slen = sizeof(pDerivativeKey->d) + sizeof(pDerivativeKey->k);
				std::unique_ptr<byte> sbuf(new byte[slen]);
				memcpy(sbuf.get(), &pDerivativeKey->d, sizeof(pDerivativeKey->d));
				memcpy(sbuf.get() + sizeof(pDerivativeKey->d), &pDerivativeKey->k, sizeof(pDerivativeKey->k));
				b456 d = shake256(sbuf.get(), slen, b456());
				err = gcry_sexp_build(&private_key, NULL, format_signData_privateKey<bXXX>(), sizeof(pDerivativeKey->q), &pDerivativeKey->q, sizeof(d), &d);
			}
			else {
				err = gcry_sexp_build(&private_key, NULL, format_signData_privateKey<bXXX>(), sizeof(pPrivateKey->q), &pPrivateKey->q, sizeof(pPrivateKey->d), &pPrivateKey->d);
			}
			if (err) {
				throw Error("Loading private key failed: %d", err);
			}

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, format_data<bXXX>(), inputSize, pInputData);
			if (err) {
				gcry_sexp_release(private_key);
				throw Error("Loading data failed: %d", err);
			}

			gcry_sexp_t signature;
			err = gcry_pk_sign(&signature, data, private_key);
			gcry_sexp_release(private_key);
			gcry_sexp_release(data);
			if (err) {
				throw Error("Signing data failed: %d", err);
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
			return new Signature<bXXX>(output);
		}

		template<class bXXX> bool verifyData(void* pInputData, size_t inputSize, const Signature<bXXX>* pInputSignature, const PublicKey<bXXX>* pPublicKey) {
			gpg_error_t err;

			gcry_sexp_t public_key;
			err = gcry_sexp_build(&public_key, NULL, format_verifyData_publicKey<bXXX>(), sizeof(pPublicKey->q), &pPublicKey->q);
			if (err) {
				throw Error("Loading public key failed: %d", err);
			}

			gcry_sexp_t signature;
			err = gcry_sexp_build(&signature, NULL, format_verifyData_signature<bXXX>(), sizeof(pInputSignature->r), &pInputSignature->r, sizeof(pInputSignature->s), &pInputSignature->s);
			if (err) {
				gcry_sexp_release(public_key);
				throw Error("Loading signature failed: %d", err);
			}

			gcry_sexp_t data;
			err = gcry_sexp_build(&data, NULL, format_data<bXXX>(), inputSize, pInputData);
			if (err) {
				gcry_sexp_release(signature);
				gcry_sexp_release(public_key);
				throw Error("Loading data failed: %d", err);
			}

			err = gcry_pk_verify(signature, data, public_key);
			gcry_sexp_release(signature);
			gcry_sexp_release(data);
			gcry_sexp_release(public_key);
			if (err) {
				throw Error("Verifying data failed: %d", err);
			}

			return true;
		}

		template<class bXXX> b512* lockKey(const PrivateKey<bXXX>* pKey, const string& password) {
			gpg_error_t err;

			size_t slen = password.size() + sizeof(aesSalt) - 1;
			std::unique_ptr<byte> sbuf(new byte[slen]);
			memcpy(sbuf.get(), password.c_str(), password.size());
			memcpy(sbuf.get() + password.size(), aesSalt, sizeof(aesSalt) - 1);
			b256 hash = sha256(sbuf.get(), slen);

			gcry_cipher_hd_t handle;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				throw Error("Initializing cipher algorithm failed: %d", err);
			}

			err = gcry_cipher_setkey(handle, &hash, sizeof(hash));
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher key failed: %d", err);
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher IV failed: %d", err);
			}

			b512 input;
			memset(&input, 0, sizeof(input));
			memcpy(&input, &pKey->d, sizeof(pKey->d));

			b512 output;
			err = gcry_cipher_encrypt(handle, &output, sizeof(output), &input, sizeof(input));
			gcry_cipher_close(handle);
			if (err) {
				throw Error("Encrypting data failed: %d", err);
			}
			return new b512(output);
		}

		template<class bXXX> PrivateKey<bXXX>* unlockKey(const b512* pInput, const string& password) {
			gpg_error_t err;

			size_t slen = password.size() + sizeof(aesSalt) - 1;
			std::unique_ptr<byte> sbuf(new byte[slen]);
			memcpy(sbuf.get(), password.c_str(), password.size());
			memcpy(sbuf.get() + password.size(), aesSalt, sizeof(aesSalt) - 1);
			b256 hash = sha256(sbuf.get(), slen);

			gcry_cipher_hd_t handle;
			err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_ECB, 0);
			if (err) {
				throw Error("Initializing cipher algorithm failed: %d", err);
			}

			err = gcry_cipher_setkey(handle, &hash, sizeof(hash));
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher key failed: %d", err);
			}

			err = gcry_cipher_setiv(handle, aesIV, sizeof(aesIV) - 1);
			if (err) {
				gcry_cipher_close(handle);
				throw Error("Setting cipher IV failed: %d", err);
			}

			b512 secret;
			err = gcry_cipher_decrypt(handle, &secret, sizeof(secret), pInput, sizeof(*pInput));
			gcry_cipher_close(handle);
			if (err) {
				throw Error("Decrypting data failed: %d", err);
			}

			b456 d;
			memcpy(&d, &secret, sizeof(d));

			PrivateKey<bXXX> output;
			generateKey(&output, &d, sizeof(d), true);
			return new PrivateKey<bXXX>(output);
		}
	}
}