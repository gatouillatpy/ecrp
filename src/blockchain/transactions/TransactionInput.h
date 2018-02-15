
#pragma once

#include <cstdint>

#include "utils/streams.h"
#include "crypto/Crypto.h"

using ecrp::io::be_ptr_istream;
using ecrp::crypto::b120;
using ecrp::crypto::b256;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class TransactionInput {

		public: // MEMBERS

			b120 source;
			uint16_t sourceOutputId;
			b256 signatureR;
			b256 signatureS;
			b256 publicKey;

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

		};
	}
}