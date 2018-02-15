
#pragma once

#include <string>

using std::string;

#include "byte.h"

#define CLEAR_BYTES(j) \
	for (int i(0); i < j; ++i) { \
		b[i] = 0; \
	}

#define SAFE_SET_BYTE(n, k, d, r) \
	if (n >= k) { \
		b[n - k] = (d >> r) & 0xFF; \
	} else { \
		b[n - k] = 0; \
	}

#define SAFE_GET_BYTE(n, k, d, r) \
	if (n >= k) { \
		d |= (uint64_t)b[n - k] << r; \
	}

//----------------------------------------------------------------------

namespace ecrp {
	template<size_t n> struct generic_uintV {
		byte b[n];

		generic_uintV() {
			clear();
		}

		generic_uintV(uint8_t d) {
			setValue(d);
		}

		generic_uintV(uint16_t d) {
			setValue(d);
		}

		generic_uintV(uint32_t d) {
			setValue(d);
		}

		generic_uintV(uint64_t d) {
			setValue(d);
		}

		void clear() {
			CLEAR_BYTES(n);
		}

		void setValue(uint8_t d) {
			SAFE_SET_BYTE(n, 1, d, 0);
			CLEAR_BYTES(n - 1);
		}

		void setValue(uint16_t d) {
			SAFE_SET_BYTE(n, 1, d, 0);
			SAFE_SET_BYTE(n, 2, d, 8);
			CLEAR_BYTES(n - 2);
		}

		void setValue(uint32_t d) {
			SAFE_SET_BYTE(n, 1, d, 0);
			SAFE_SET_BYTE(n, 2, d, 8);
			SAFE_SET_BYTE(n, 3, d, 16);
			SAFE_SET_BYTE(n, 4, d, 24);
			CLEAR_BYTES(n - 4);
		}

		void setValue(uint64_t d) {
			SAFE_SET_BYTE(n, 1, d, 0);
			SAFE_SET_BYTE(n, 2, d, 8);
			SAFE_SET_BYTE(n, 3, d, 16);
			SAFE_SET_BYTE(n, 4, d, 24);
			SAFE_SET_BYTE(n, 5, d, 32);
			SAFE_SET_BYTE(n, 6, d, 40);
			SAFE_SET_BYTE(n, 7, d, 48);
			SAFE_SET_BYTE(n, 8, d, 56);
			CLEAR_BYTES(n - 8);
		}

		uint64_t getValue() {
			uint64_t d = 0;
			SAFE_GET_BYTE(n, 8, d, 56);
			SAFE_GET_BYTE(n, 7, d, 48);
			SAFE_GET_BYTE(n, 6, d, 40);
			SAFE_GET_BYTE(n, 5, d, 32);
			SAFE_GET_BYTE(n, 4, d, 24);
			SAFE_GET_BYTE(n, 3, d, 16);
			SAFE_GET_BYTE(n, 2, d, 8);
			SAFE_GET_BYTE(n, 1, d, 0);
			return d;
		}

		string toString() {
			stringstream ss;
			ss << std::hex;
			for (int i(0); i < n; ++i) {
				ss << (int)b[i];
			}
			return ss.str();
		}
	};

	typedef generic_uintV<2> uintV1l_t;
}