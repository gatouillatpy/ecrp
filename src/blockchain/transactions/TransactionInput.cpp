
#include <stdexcept>

using std::runtime_error;

#include "TransactionInput.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		void TransactionInput::deserialize(be_ptr_istream& stream) {
			stream >> source;
			stream >> sourceOutputId;
			stream >> signatureR;
			stream >> signatureS;
			stream >> publicKey;
		}
	}
}