
#pragma once

#include <string>

#if (defined(_MSC_VER) && (_MSC_VER > 600))
#include "../zlib-1.2.8/zlib.h"
#else
#include "zlib-1.2.8/zlib.h"
#endif

//----------------------------------------------------------------------

namespace geodis
{
	std::string compress(const std::string& str, int compressionlevel = Z_BEST_COMPRESSION);
	std::string decompress(char* data, size_t length);
}