
#pragma once

#include <string>

using std::string;

//----------------------------------------------------------------------

namespace ecrp {
    
    class Error {
        
        public: // CONSTANTS

            static const int UNSPECIFIED_REQUEST_ID     = 0x0001;
            static const int UNSPECIFIED_REQUEST_TYPE   = 0x0011;
            static const int UNKNOWN_REQUEST_TYPE       = 0x0012;
			static const int INVALID_REQUEST_TYPE		= 0x0013;
			static const int MISSING_ARGUMENT           = 0x0021;
            static const int INVALID_COORDINATES        = 0x0031;
            static const int INVALID_LEVEL              = 0x0032;
            static const int INVALID_KEY                = 0x0033;
            static const int POINT_NOT_FOUND            = 0x0041;

        public: // METHODS

            static const string getMessage(int code);
    };
}