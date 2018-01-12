
#include "Error.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    const string Error::getMessage(int code) { // TODO: refactor that with a static map
        
        switch (code) {
            case Error::UNSPECIFIED_REQUEST_ID:
                return "Unspecified request id.";
            case Error::UNSPECIFIED_REQUEST_TYPE:
                return "Unspecified request type.";
            case Error::UNKNOWN_REQUEST_TYPE:
                return "Unknown request type.";
            case Error::MISSING_ARGUMENT:
                return "Missing request argument.";
            case Error::INVALID_COORDINATES:
                return "Invalid coordinates.";
            case Error::INVALID_LEVEL:
                return "Invalid level.";
            case Error::INVALID_KEY:
                return "Invalid key.";
            case Error::POINT_NOT_FOUND:
                return "Point not found.";
            default:
                return "Unknown Error.";
        }
    }
}