
#include <stdexcept>

using std::runtime_error;

#include "Transaction.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		Transaction::Transaction() {
		}

		Transaction::Transaction(uint8_t type) {
			_type = type;
		}

		Transaction::~Transaction() {
			for (uint16_t i = 0; i < _inputs.size(); ++i) {
				delete _inputs[i];
			}
			for (uint16_t i = 0; i < _outputs.size(); ++i) {
				delete _outputs[i];
			}
			_inputs.clear();
			_outputs.clear();
		}

		void Transaction::deserialize(be_ptr_istream& stream) {
			stream >> _version;
			stream >> _type;

			if (_version < MIN_COMPATIBLE_VERSION || _version > CURRENT_VERSION) {
				throw runtime_error("Incompatible ecrp::blockchain::Transaction version '" + std::to_string(_version) + "'.");
			}

			uint16_t inputCount;
			stream >> inputCount;
			for (uint16_t i = 0; i < inputCount; ++i) {
				TransactionInput* t = new TransactionInput();
				t->deserialize(stream);
				_inputs.push_back(t);
			}

			uint16_t outputCount;
			stream >> outputCount;
			for (uint16_t i = 0; i < outputCount; ++i) {
				TransactionOutput* t = new TransactionOutput();
				t->deserialize(stream);
				_outputs.push_back(t);
			}
		}

		void Transaction::addInput(TransactionInput* i) {
			_inputs.push_back(i);
		}

		void Transaction::addOutput(TransactionOutput* o) {
			_outputs.push_back(o);
		}
	}
}