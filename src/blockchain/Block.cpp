
#include <stdexcept>

using std::runtime_error;

#include "Block.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		Block::Block() {
		}

		Block::Block(uint32_t timestamp) {
			_timestamp = timestamp;
			_target = 0;
			_nonce = 0;
			memset(&_rootHash, 0, sizeof(_rootHash)); // TODO: create a dedicated function for that
		}

		Block::~Block() {
			for (uint16_t i = 0; i < _transactions.size(); ++i) {
				delete _transactions[i];
			}
			_transactions.clear();
		}

		void Block::deserialize(be_ptr_istream& stream) {
			stream >> _version;

			if (_version < MIN_COMPATIBLE_VERSION || _version > CURRENT_VERSION) {
				throw runtime_error("Incompatible ecrp::blockchain::Block version '" + std::to_string(_version) + "'.");
			}

			stream >> _timestamp;
			stream >> _target;
			stream >> _nonce;
			stream >> _rootHash;

			uint16_t transactionCount;
			stream >> transactionCount;
			for (uint16_t i = 0; i < transactionCount; ++i) {
				Transaction* t = new Transaction();
				t->deserialize(stream);
				_transactions.push_back(t);
			}
		}

		void Block::addTransaction(Transaction* t) {
			_transactions.push_back(t);
		}
	}
}