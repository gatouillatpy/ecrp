
#pragma once

#include <stdint.h>

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class TransactionType {

		public: // CONSTANTS

			static const uint8_t BASIC = 1;
			static const uint8_t FEE = 2;
			static const uint8_t REWARD = 3;

		};
	}
}