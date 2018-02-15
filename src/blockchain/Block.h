
#pragma once

#include <vector>

using std::vector;

#include "utils/streams.h"
#include "crypto/Crypto.h"
#include "Transaction.h"

using ecrp::io::be_ptr_istream;
using ecrp::crypto::b256;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class Block {

		private: // CONSTANTS

			static const uint16_t MIN_COMPATIBLE_VERSION = 1;
			static const uint16_t CURRENT_VERSION = 1;

		private: // MEMBERS

			uint16_t _version;
			uint32_t _timestamp;
			uint32_t _target;
			uint64_t _nonce;
			b256 _rootHash;
			vector<Transaction*> _transactions;

		public: // CONSTRUCTORS

			Block();
			Block(uint32_t timestamp);

			virtual ~Block();

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

			void addTransaction(Transaction* t);

		};
	}
}