
#pragma once

#include <string>
#include <stdexcept>
#include <stdarg.h>

using std::string;

//----------------------------------------------------------------------

namespace ecrp {

    class Error : std::exception {
        
	public: // STATIC CONSTANTS

        static const int UNSPECIFIED_REQUEST_ID     = 0x0001;
        static const int UNSPECIFIED_REQUEST_TYPE   = 0x0011;
        static const int UNKNOWN_REQUEST_TYPE       = 0x0012;
		static const int INVALID_REQUEST_TYPE		= 0x0013;
		static const int MISSING_ARGUMENT           = 0x0021;
        static const int INVALID_COORDINATES        = 0x0031;
        static const int INVALID_LEVEL              = 0x0032;
        static const int INVALID_KEY                = 0x0033;
        static const int POINT_NOT_FOUND            = 0x0041;

    public: // STATIC METHODS

        static const string getMessage(int code);

	private: // MEMBERS

		char buffer[1024];

	public: // CONSTRUCTOR

		Error() {
		}

		Error(char const* format, ...) {
			va_list ap;
			va_start(ap, format);
			vsnprintf(buffer, sizeof(buffer), format, ap);
			va_end(ap);
		}

	public: // METHODS

		char const* what() const throw() { return buffer; }
	};
}