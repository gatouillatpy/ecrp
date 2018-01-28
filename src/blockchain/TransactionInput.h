
#pragma once

#include <cstdint>

#include "utils/streams.h"
#include "crypto/Crypto.h"

using ecrp::io::be_ptr_istream;
using ecrp::crypto::hash128;
using ecrp::crypto::hash256;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class TransactionInput {

		public: // MEMBERS

			hash128 source;
			uint16_t sourceOutputId;
			hash128 signature;
			hash256 publicKey;

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

		};
	}
}