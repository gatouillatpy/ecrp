
#pragma once

#include <string>

//----------------------------------------------------------------------

namespace ecrp {
    
    struct point { // 16 packed bytes
    
        public: // MEMBERS

            uint32_t lat;
            uint32_t lon;

            uint64_t key;

        public: // CONSTRUCTORS

            point() : lat(0), lon(0), key(0) {}
            point(uint32_t _lat, uint32_t _lon, uint64_t _key) : lat(_lat), lon(_lon), key(_key) {}
    };

    class full_point : public point {
        public: // MEMBERS

            uint16_t level;
            uint16_t groupId;

        public: // CONSTRUCTORS

            full_point() : point(), level(0), groupId(0) {}
            full_point(uint32_t _lat, uint32_t _lon, uint16_t _level, uint64_t _key, uint16_t _groupId) : point(_lat, _lon, _key), level(_level), groupId(_groupId) {}

        public: // OPERATORS

            const bool operator==(const full_point &other);
            const bool operator!=(const full_point &other);
    };

    static const full_point invalid_point;
}