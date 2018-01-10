
#pragma once

#include <string>

typedef unsigned char byte;

//----------------------------------------------------------------------

namespace geodis
{
	uint64_t retrieveKey(const std::string& key);
	std::string formatKey(uint64_t key);
}