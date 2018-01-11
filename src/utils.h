
#pragma once

#include <string>

using std::string;

typedef unsigned char byte;

//----------------------------------------------------------------------

namespace ecrp {
    uint64_t retrieveKey(const string &key);
    string formatKey(uint64_t key);
}