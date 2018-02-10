
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

		template<size_t n> struct generic_blob { byte b[n]; };

		template<size_t n> generic_blob<n> from_string(generic_blob<n> t, const string &s) {
			for (int i(0); i < n; ++i) {
				t.b[i] = (byte)stoul(s.substr(2 * i, 2), 0, 16);
			}
			return t;
		}

		template<size_t n> string to_string(const generic_blob<n> &t) {
			stringstream ss;
			ss << std::hex;
			for (int i(0); i < n; ++i) {
				ss << (int)t.b[i];
			}
			return ss.str();
		}

		template<size_t n> generic_blob<n> shake256(const void* inputData, size_t inputSize, generic_blob<n> outputData) {
			gcry_md_hd_t hd;
			gpg_error_t err;
			gcry_md_open(&hd, GCRY_MD_SHAKE256, 0);
			gcry_md_write(hd, inputData, inputSize);
			err = gcry_md_extract(hd, GCRY_MD_SHAKE256, &outputData, n);
			gcry_md_close(hd);
			return outputData;
		}

		typedef generic_blob<16> b128;
		typedef generic_blob<32> b256;
		typedef generic_blob<57> b456;
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

		b256 sha256(void* inputData, size_t inputSize);

		PrivateKey generateKey(const void* secretData, size_t secretSize, bool isRaw);
		PrivateKey generateKey();
		DerivativeKey deriveKey(const PrivateKey& sourceKey, uint32_t kValue);
		Signature signData(void* inputData, size_t inputSize, const PrivateKey& privateKey);
		bool verifyData(void* inputData, size_t inputSize, const Signature& inputSignature, const PublicKey& publicKey);
		b512 lockKey(const PrivateKey& key, const string& password);
		PrivateKey unlockKey(const b512& encryptedSecret, const string& password);
	}
}