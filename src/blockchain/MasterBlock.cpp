
#include <stdexcept>

using std::runtime_error;

#include "MasterBlock.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		MasterBlock::MasterBlock() {
		}

		MasterBlock::MasterBlock(uint32_t id, uint32_t timestamp) {
			_id = id;
			_timestamp = timestamp;
			_target = 0;
			_nonce = 0;
			memset(&_previousHash, 0, sizeof(_previousHash)); // TODO: create a dedicated function for that
			memset(&_masterHash, 0, sizeof(_masterHash));
		}

		MasterBlock::~MasterBlock() {
			for (uint16_t i = 0; i < _blocks.size(); ++i) {
				delete _blocks[i];
			}
			_blocks.clear();
		}

		void MasterBlock::deserialize(be_ptr_istream& stream) {
			stream >> _id;
			stream >> _version;

			if (_version < MIN_COMPATIBLE_VERSION || _version > CURRENT_VERSION) {
				throw runtime_error("Incompatible ecrp::blockchain::MasterBlock version '" + std::to_string(_version) + "'.");
			}

			stream >> _timestamp;
			stream >> _target;
			stream >> _nonce;
			stream >> _previousHash;
			stream >> _masterHash;

			uint16_t blockCount;
			stream >> blockCount;
			for (uint16_t i = 0; i < blockCount; ++i) {
				Block* t = new Block();
				t->deserialize(stream);
				_blocks.push_back(t);
			}
		}

		void MasterBlock::addBlock(Block* b) {
			_blocks.push_back(b);
		}
	}
}