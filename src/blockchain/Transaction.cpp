
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
		}

		void Transaction::deserialize(be_ptr_istream& stream) {
			stream >> _version;
			stream >> _type;

			if (_version < MIN_COMPATIBLE_VERSION || _version > CURRENT_VERSION) {
				throw runtime_error("Incompatible ecrp::blockchain::Transaction version '" + std::to_string(_version) + "'.");
			}
		}
	}
}