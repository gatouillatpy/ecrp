
#pragma once

#include <string>
#include <sstream>

using std::string;
using std::stringstream;
using std::stoul;

#include "utils/byte.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

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
		typedef generic_blob<32> b256;
		typedef generic_blob<57> b456; // TODO: 57
		typedef generic_blob<64> b512;

		struct PublicKey {
			b456 q;

			virtual ~PublicKey() {};
		};

		struct PrivateKey : PublicKey {
			b456 d; // TODO: make sure this is stored securely

			virtual ~PrivateKey() {};
		};

		struct DerivativeKey : PrivateKey {
			uint32_t k;

			DerivativeKey() : k(0) {}
			virtual ~DerivativeKey() {};
		};

		struct Signature {
			b456 r;
			b456 s;
		};

		b256 sha256(const void* pInputData, size_t inputSize);

		void generateKey(PrivateKey* pOutput, const void* pSecretData, size_t secretSize, bool isRaw);
		PrivateKey* generateKey();
		DerivativeKey* deriveKey(const PrivateKey* pSourceKey, uint32_t kValue);
		Signature* signData(void* pInputData, size_t inputSize, const PrivateKey* pPrivateKey);
		bool verifyData(void* pInputData, size_t inputSize, const Signature* pInputSignature, const PublicKey* pPublicKey);
		b512* lockKey(const PrivateKey* pKey, const string& password);
		PrivateKey* unlockKey(const b512* encryptedSecret, const string& password);
	}
}