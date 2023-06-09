
#pragma once

#include <cstdint>

#include "utils/streams.h"
#include "crypto/Crypto.h"

using ecrp::io::be_ptr_istream;
using ecrp::crypto::b120;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class TransactionOutput {

		public: // MEMBERS

			uint64_t amount;
			b120 address;

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

		};
	}
}