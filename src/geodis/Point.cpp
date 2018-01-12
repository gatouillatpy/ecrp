
#include "Point.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		const bool FullPoint::operator==(const FullPoint &other) {
			return key == other.key;
		}

		const bool FullPoint::operator!=(const FullPoint &other) {
			return !(*this == other);
		}
	}
}