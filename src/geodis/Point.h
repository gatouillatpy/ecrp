
#pragma once

#include <string>

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		struct Point { // 16 packed bytes

		public: // MEMBERS

			uint32_t lat;
			uint32_t lon;

			uint64_t key;

		public: // CONSTRUCTORS

			Point() : lat(0), lon(0), key(0) {}
			Point(uint32_t _lat, uint32_t _lon, uint64_t _key) : lat(_lat), lon(_lon), key(_key) {}
		};

		class FullPoint : public Point {

		public: // MEMBERS

			uint16_t level;
			uint16_t groupId;

		public: // CONSTRUCTORS

			FullPoint() : Point(), level(0), groupId(0) {}
			FullPoint(uint32_t _lat, uint32_t _lon, uint16_t _level, uint64_t _key, uint16_t _groupId) : Point(_lat, _lon, _key), level(_level), groupId(_groupId) {}

		public: // OPERATORS

			const bool operator==(const FullPoint &other);
			const bool operator!=(const FullPoint &other);
		};
	}
}