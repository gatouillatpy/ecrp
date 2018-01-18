
#include <atomic>
#include <iostream>

#include <boost/optional.hpp>

#include "GeodisHandler.h"
#include "server/Processor.h"
#include "geodis/Registry.h"
#include "errors/Error.h"
#include "utils/utils.h"

using ecrp::geodis::Registry;

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		const double GeodisHandler::MIN_LAT = -90.0;
		const double GeodisHandler::MAX_LAT = +90.0;
		const double GeodisHandler::MIN_LON = -180.0;
		const double GeodisHandler::MAX_LON = +180.0;

		const double GeodisHandler::LAT_RATIO = (double)UINT32_MAX / (MAX_LAT - MIN_LAT);
		const double GeodisHandler::LON_RATIO = (double)UINT32_MAX / (MAX_LON - MIN_LON);
		const double GeodisHandler::LAT_FACTOR = 1.0 / LAT_RATIO;
		const double GeodisHandler::LON_FACTOR = 1.0 / LON_RATIO;

		const int GeodisHandler::MIN_LEVEL = -32768;
		const int GeodisHandler::MAX_LEVEL = +32767;

		FullPoint GeodisHandler::getFullPointFromRequest(Processor* pProcessor, Request *pRequest, Response &res) {
			auto requestContent = pRequest->getContent();
			auto lat = requestContent.get_optional<double>("lat");
			auto lon = requestContent.get_optional<double>("lon");
			auto level = requestContent.get_optional<int>("level");
			auto key = requestContent.get_optional<string>("key");
			auto Group = requestContent.get_optional<string>("Group");

			if (!lat || !lon || !level || !key || !Group) {
				pProcessor->sendError(pRequest, res, Error::MISSING_ARGUMENT);

				return INVALID_POINT;
			}

			double fLat = lat.get();
			double fLon = lon.get();

			if (fLat < MIN_LAT || fLon < MIN_LON || fLat > MAX_LAT || fLon > MAX_LON) {
				pProcessor->sendError(pRequest, res, Error::INVALID_COORDINATES);

				return INVALID_POINT;
			}

			int nLevel = level.get();

			if (nLevel < MIN_LEVEL || nLevel > MAX_LEVEL) {
				pProcessor->sendError(pRequest, res, Error::INVALID_LEVEL);

				return INVALID_POINT;
			}

			uint64_t decodedKey = retrieveKey(key.get());

			if (decodedKey == 0) {
				pProcessor->sendError(pRequest, res, Error::INVALID_KEY);

				return INVALID_POINT;
			}

			FullPoint valid_point;
			valid_point.lat = (uint32_t)(LAT_RATIO * (fLat - MIN_LAT));
			valid_point.lon = (uint32_t)(LON_RATIO * (fLon - MIN_LON));
			valid_point.level = (uint16_t)(nLevel - MIN_LEVEL);
			valid_point.key = decodedKey;
			valid_point.groupId = Registry::getOrCreateGroup(Group.get())->getId();

			return valid_point;
		}
	}
}