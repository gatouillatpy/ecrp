
#include <atomic>
#include <iostream>

#include <boost/optional.hpp>

#include "Processor.h"
#include "../geodis/Registry.h"
#include "../errors/Error.h"

using std::atomic_int;
using std::cout;
using std::endl;
using std::vector;
using std::setprecision;
using boost::mutex;
using boost::condition_variable;
using boost::unique_lock;
using ecrp::geodis::Registry;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		const size_t MAX_POINTS_PER_REQUEST = 200;

		static atomic_int NEXT_PROCESSOR_ID(0);

		static request_queue _queue;
		static mutex _queueMutex;

		static condition_variable _syncPoint;

		static double MIN_LAT(-90.0);
		static double MAX_LAT(+90.0);
		static double MIN_LON(-180.0);
		static double MAX_LON(+180.0);

		static double LAT_RATIO((double)UINT32_MAX / (MAX_LAT - MIN_LAT));
		static double LON_RATIO((double)UINT32_MAX / (MAX_LON - MIN_LON));
		static double LAT_FACTOR(1.0 / LAT_RATIO);
		static double LON_FACTOR(1.0 / LON_RATIO);

		static int MIN_LEVEL(-32768);
		static int MAX_LEVEL(+32767);

		Processor::Processor() : _worker(&Processor::workerLoop, this) {
			_id = NEXT_PROCESSOR_ID++;
		}

		Processor::~Processor() {
		}

		void Processor::enqueueRequest(Request *pRequest) {
			_queueMutex.lock();
			_queue.push_back(pRequest);
			_syncPoint.notify_one();
			_queueMutex.unlock();
		}

		bool Processor::sendError(Request *pRequest, Response &res, int errCode) {
			ostringstream &s = res.getContent();

			s << "\t" << "\"status\": " << "\"" << Response::STATUS_FAILURE << "\"" << "," << endl;
			s << "\t" << "\"errCode\": " << errCode << "," << endl;
			s << "\t" << "\"errMessage\": " << "\"" << Error::getMessage(errCode) << "\"" << endl;

			res.send();

			delete pRequest;

			return false;
		}

		bool Processor::sendAcknowledge(Request *pRequest, Response &res) {
			ostringstream &s = res.getContent();

			s << "\t" << "\"status\": " << "\"" << Response::STATUS_SUCCESS << "\"" << endl;

			res.send();

			delete pRequest;

			return true;
		}

		FullPoint Processor::getFullPointFromRequest(Request *pRequest, Response &res) {
			auto lat = pRequest->getContent().get_optional<double>("lat");
			auto lon = pRequest->getContent().get_optional<double>("lon");
			auto level = pRequest->getContent().get_optional<int>("level");
			auto key = pRequest->getContent().get_optional<string>("key");
			auto Group = pRequest->getContent().get_optional<string>("Group");

			if (!lat || !lon || !level || !key || !Group) {
				sendError(pRequest, res, Error::MISSING_ARGUMENT);

				return INVALID_POINT;
			}

			double fLat = lat.get();
			double fLon = lon.get();

			if (fLat < MIN_LAT || fLon < MIN_LON || fLat > MAX_LAT || fLon > MAX_LON) {
				sendError(pRequest, res, Error::INVALID_COORDINATES);

				return INVALID_POINT;
			}

			int nLevel = level.get();

			if (nLevel < MIN_LEVEL || nLevel > MAX_LEVEL) {
				sendError(pRequest, res, Error::INVALID_LEVEL);

				return INVALID_POINT;
			}

			uint64_t decodedKey = retrieveKey(key.get());

			if (decodedKey == 0) {
				sendError(pRequest, res, Error::INVALID_KEY);

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

		bool Processor::addPoint(Request *pRequest, Response &res) {
			cout << "addPoint from Processor[" << _id << "]" << endl;

			FullPoint fp = getFullPointFromRequest(pRequest, res);

			if (fp != INVALID_POINT) {
				auto p = Registry::getOrSpawnTree(fp.level, fp.groupId);

				p->addPoint(fp.lat, fp.lon, fp.key);

				return sendAcknowledge(pRequest, res);
			}

			return false;
		}

		bool Processor::removePoint(Request *pRequest, Response &res) {
			cout << "removePoint from Processor[" << _id << "]" << endl;

			FullPoint fp = getFullPointFromRequest(pRequest, res);

			if (fp != INVALID_POINT) {
				bool ok = false;

				auto pGroup = Registry::getGroupById(fp.groupId);

				if (pGroup != 0) {
					auto pTree = pGroup->getTreeByLevel(fp.level);

					if (pTree != 0) {
						ok = pTree->removePoint(fp.lat, fp.lon, fp.key);
					}
				}

				if (ok) {
					return sendAcknowledge(pRequest, res);
				}
				else {
					return sendError(pRequest, res, Error::POINT_NOT_FOUND);
				}
			}

			return false;
		}

		bool Processor::findPoints(Request *pRequest, Response &res) {
			cout << "findPoints from Processor[" << _id << "]" << endl;

			auto minLat = pRequest->getContent().get_optional<double>("minLat");
			auto minLon = pRequest->getContent().get_optional<double>("minLon");
			auto maxLat = pRequest->getContent().get_optional<double>("maxLat");
			auto maxLon = pRequest->getContent().get_optional<double>("maxLon");
			auto Group = pRequest->getContent().get_optional<string>("Group");

			if (!minLat || !minLon || !maxLat || !maxLon || !Group) {
				return sendError(pRequest, res, Error::MISSING_ARGUMENT);
			}

			double fMinLat = minLat.get();
			double fMinLon = minLon.get();
			double fMaxLat = maxLat.get();
			double fMaxLon = maxLon.get();

			if (fMinLat < MIN_LAT || fMinLon < MIN_LON || fMaxLat > MAX_LAT || fMaxLon > MAX_LON || fMinLat > fMaxLat || fMinLon > fMaxLon) {
				return sendError(pRequest, res, Error::INVALID_COORDINATES);
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

		bool Processor::process(Request *pRequest) {
			Response res(pRequest->getTarget());

			auto id = pRequest->getContent().get_optional<int>("id");

			if (id) {
				res.init(id.get());

				auto type = pRequest->getContent().get_optional<string>("type");

				if (type) {
					if (type.get().compare(Request::ADD_POINT) == 0) {
						return addPoint(pRequest, res);
					}

					if (type.get().compare(Request::REMOVE_POINT) == 0) {
						return removePoint(pRequest, res);
					}

					if (type.get().compare(Request::FIND_POINTS) == 0) {
						return findPoints(pRequest, res);
					}
				}
				else {
					return sendError(pRequest, res, Error::UNSPECIFIED_REQUEST_TYPE);
				}

				return sendError(pRequest, res, Error::UNKNOWN_REQUEST_TYPE);
			}
			else {
				res.init(-1);

				return sendError(pRequest, res, Error::UNSPECIFIED_REQUEST_ID);
			}
		}

		void Processor::workerLoop() {
			_isAlive = true;

			unique_lock<mutex> syncLock(_syncMutex);

			while (_isAlive) {
				_queueMutex.lock();

				while (_queue.empty()) {
					_queueMutex.unlock();
					_syncPoint.wait(syncLock);
					_queueMutex.lock();
				}

				Request *pRequest = _queue.front();
				_queue.pop_front();
				_queueMutex.unlock();

				process(pRequest);
			}
		}
	}
}