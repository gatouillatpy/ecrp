
#pragma once

#include <string>
#include <sstream>

using std::string;
using std::stringstream;

#include "utils/byte.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		template<size_t s> struct generic_blob { byte b[s]; };

		template<size_t s> string to_string(const generic_blob<s> &t) {
			stringstream ss;
			ss << std::hex;
			for (int i(0); i < s; ++i) {
				ss << (int)t.b[i];
			}
			return ss.str();
		}

		typedef generic_blob<16> b128;
		typedef generic_blob<32> b256;
		typedef generic_blob<57> b456;

		struct PublicKey {
			b456 q;
			uint32_t k;

			PublicKey() : k(0) {}
		};

		struct PrivateKey : PublicKey {
			b456 d; // TODO: make sure this is stored securely
		};

		struct Signature {
			b456 r;
			b456 s;
		};

		b256 sha256(void* inputData, size_t inputSize);

		PrivateKey generateKey();
		PrivateKey deriveKey(const PrivateKey& sourceKey);
		Signature signData(void* inputData, size_t inputSize, const PrivateKey& privateKey);
		bool verifyData(void* inputData, size_t inputSize, const Signature& inputSignature, const PublicKey& publicKey);

	}
}