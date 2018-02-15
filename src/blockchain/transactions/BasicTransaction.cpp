
#include "BasicTransaction.h"
#include "blockchain/TransactionType.h"
#include "errors/Error.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		BasicTransaction::BasicTransaction() : Transaction(TransactionType::BASIC) {
		}

		BasicTransaction::~BasicTransaction() {
			for (uint16_t i = 0; i < _outputs.size(); ++i) {
				delete _outputs[i];
			}
			_outputs.clear();
		}

		void BasicTransaction::deserialize(be_ptr_istream& stream) {
			Transaction::deserialize(stream);

			_input.deserialize(stream);

			uint16_t outputCount;
			stream >> outputCount;
			for (uint16_t i = 0; i < outputCount; ++i) {
				TransactionOutput* t = new TransactionOutput();
				t->deserialize(stream);
				_outputs.push_back(t);
			}
		}

		void BasicTransaction::addOutput(TransactionOutput* o) {
			_outputs.push_back(o);
		}
	}
}