
#include <iostream>

#include "FindPointsHandler.h"
#include "server/Processor.h"
#include "geodis/Registry.h"
#include "errors/Error.h"
#include "utils/utils.h"

using std::cout;
using std::endl;
using std::vector;
using std::setprecision;
using ecrp::geodis::Registry;

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		const size_t FindPointsHandler::MAX_POINTS_PER_REQUEST = 200;

		bool FindPointsHandler::process(Processor* pProcessor, Request *pRequest, Response &res) {
			cout << "findPoints from Processor[" << pProcessor->getId() << "]" << endl;

			auto minLat = pRequest->getContent().get_optional<double>("minLat");
			auto minLon = pRequest->getContent().get_optional<double>("minLon");
			auto maxLat = pRequest->getContent().get_optional<double>("maxLat");
			auto maxLon = pRequest->getContent().get_optional<double>("maxLon");
			auto Group = pRequest->getContent().get_optional<string>("Group");

			if (!minLat || !minLon || !maxLat || !maxLon || !Group) {
				return pProcessor->sendError(pRequest, res, Error::MISSING_ARGUMENT);
			}

			double fMinLat = minLat.get();
			double fMinLon = minLon.get();
			double fMaxLat = maxLat.get();
			double fMaxLon = maxLon.get();

			if (fMinLat < MIN_LAT || fMinLon < MIN_LON || fMaxLat > MAX_LAT || fMaxLon > MAX_LON || fMinLat > fMaxLat || fMinLon > fMaxLon) {
				return pProcessor->sendError(pRequest, res, Error::INVALID_COORDINATES);
			}

			uint32_t _minLat = (uint32_t)(LAT_RATIO * (fMinLat - MIN_LAT));
			uint32_t _minLon = (uint32_t)(LON_RATIO * (fMinLon - MIN_LON));
			uint32_t _maxLat = (uint32_t)(LAT_RATIO * (fMaxLat - MIN_LAT));
			uint32_t _maxLon = (uint32_t)(LON_RATIO * (fMaxLon - MIN_LON));
			uint16_t _groupId = Registry::getOrCreateGroup(Group.get())->getId();

			vector<FullPoint> points;
			points.reserve(MAX_POINTS_PER_REQUEST);

			auto pGroup = Registry::getGroupById(_groupId);

			if (pGroup != 0) {
				auto za = pGroup->beginIterator();
				auto zb = pGroup->endIterator();

				for (auto z = za; z != zb; z++) {
					auto pTree = z->second;

					pTree->findPoints(points, MAX_POINTS_PER_REQUEST, _minLat, _minLon, _maxLat, _maxLon);
				}
			}

			ostringstream &s = res.getContent();

			s << "\t" << "\"status\": " << "\"" << Response::STATUS_SUCCESS << "\"" << "," << endl;
			s << "\t" << "\"points\": " << endl;
			s << "\t" << "[" << endl;
			s << setprecision(9);

			auto ta = points.begin();
			auto tb = points.end();

			for (auto t = ta; t != tb;) {
				auto p = *t; t++;

				s << "\t\t" << "{" << endl;
				s << "\t\t\t" << "\"lat\": " << (double)(LAT_FACTOR * p.lat + MIN_LAT) << "," << endl;
				s << "\t\t\t" << "\"lon\": " << (double)(LON_FACTOR * p.lon + MIN_LON) << "," << endl;
				s << "\t\t\t" << "\"level\": " << (int)(p.level + MIN_LEVEL) << "," << endl;
				s << "\t\t\t" << "\"key\": " << "\"" << formatKey(p.key) << "\"" << endl;
				s << "\t\t" << "}";

				if (t != tb) {
					s << ",";
				}

				s << endl;
			}

			s << "\t" << "]" << endl;

			res.send();

			delete pRequest;

			return true;
		}
	}
}