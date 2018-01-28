
#pragma once

#include <vector>

#include "utils/streams.h"
#include "crypto/Crypto.h"
#include "Block.h"

using std::vector;
using ecrp::io::be_ptr_istream;
using ecrp::crypto::b256;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class MasterBlock {

		private: // CONSTANTS

			static const uint16_t MIN_COMPATIBLE_VERSION = 1;
			static const uint16_t CURRENT_VERSION = 1;

		private: // MEMBERS

			uint32_t _id;
			uint16_t _version;
			uint32_t _timestamp;
			uint32_t _target;
			uint64_t _nonce;
			b256 _previousHash;
			b256 _masterHash;
			vector<Block*> _blocks;

		public: // CONSTRUCTORS

			MasterBlock();
			MasterBlock(uint32_t id, uint32_t timestamp);

			virtual ~MasterBlock();

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

			void addBlock(Block* b);

		};
	}
}