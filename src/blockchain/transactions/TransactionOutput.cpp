
#include <stdexcept>

using std::runtime_error;

#include "TransactionOutput.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		void TransactionOutput::deserialize(be_ptr_istream& stream) {
			stream >> amount;
			stream >> address;
		}
	}
}