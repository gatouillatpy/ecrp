
#pragma once

#include <string>

using std::string;

#include "zlib.h"

//----------------------------------------------------------------------

namespace ecrp {
	string compress(const string& str, int compressionlevel = Z_BEST_COMPRESSION);
	string decompress(char* data, size_t length);
}