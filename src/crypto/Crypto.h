
#pragma once

#include <string>
#include <sstream>

using std::string;
using std::stringstream;

#include "utils/byte.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace crypto {

		template<size_t s> struct generic_hash { byte b[s]; };

		template<size_t s> string to_string(const generic_hash<s> &t) {
			stringstream ss;
			ss << std::hex;
			for (int i(0); i < s; ++i) {
				ss << (int)t.b[i];
			}
			return ss.str();
		}

		typedef generic_hash<16> hash128;
		typedef generic_hash<32> hash256;

		hash256& sha256(void* inputData, size_t inputSize);

		void test(void* inputData, size_t inputSize);

	}
}