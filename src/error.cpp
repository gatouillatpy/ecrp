
#include "error.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    const string error::getMessage(int code) {
        
        switch (code) {
            case error::UNSPECIFIED_REQUEST_ID:
                return "Unspecified request id.";
            case error::UNSPECIFIED_REQUEST_TYPE:
                return "Unspecified request type.";
            case error::UNKNOWN_REQUEST_TYPE:
                return "Unknown request type.";
            case error::MISSING_ARGUMENT:
                return "Missing request argument.";
            case error::INVALID_COORDINATES:
                return "Invalid coordinates.";
            case error::INVALID_LEVEL:
                return "Invalid level.";
            case error::INVALID_KEY:
                return "Invalid key.";
            case error::POINT_NOT_FOUND:
                return "Point not found.";
            default:
                return "Unknown error.";
        }
    }
}