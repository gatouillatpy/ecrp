
#include "point.h"

//----------------------------------------------------------------------

namespace geodis
{
	const bool full_point::operator==(const full_point& other)
	{
		return key == other.key;
	}

	const bool full_point::operator!=(const full_point& other)
	{
		return !(*this == other);
	}
}
