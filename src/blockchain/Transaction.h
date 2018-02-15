
#pragma once

#include "utils/streams.h"
#include "crypto/Crypto.h"

using ecrp::io::be_ptr_istream;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class Transaction {

		protected: // CONSTANTS

			static const uint16_t MIN_COMPATIBLE_VERSION = 1;
			static const uint16_t CURRENT_VERSION = 1;

		protected: // MEMBERS

			uint16_t _version;
			uint8_t _type;

		public: // CONSTRUCTORS

			Transaction();
			Transaction(uint8_t type);

			virtual ~Transaction();

		public: // METHODS

			virtual void deserialize(be_ptr_istream& stream);

		};
	}
}