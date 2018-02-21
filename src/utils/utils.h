
#pragma once

#include <string>
#include <stdexcept>

using std::string;
using std::runtime_error;

#define readBuffer(t, p, q) \
	if (p + sizeof(t) > q) { \
		throw runtime_error("Cannot read past the buffer size limit."); \
	} \
	memcpy(&t, p, sizeof(t)); \
	p += sizeof(t);

//----------------------------------------------------------------------

namespace ecrp {
    uint64_t retrieveKey(const string &key);
    string formatKey(uint64_t key);
	uint32_t getUnixTimestampUTC();
	uint64_t getTimestampUTC();
}