
#include <atomic>
#include <iostream>

#include <boost/optional.hpp>

#include "processor.h"
#include "registry.h"
#include "tree.h"
#include "error.h"

//----------------------------------------------------------------------

namespace geodis
{
	const size_t MAX_POINTS_PER_REQUEST = 200;

	static std::atomic_int NEXT_PROCESSOR_ID(0);

	static request_queue _queue;
	static boost::mutex _queueMutex;

	static boost::condition_variable _syncPoint;

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

	processor::processor() : _worker(&processor::workerLoop, this)
	{
		_id = NEXT_PROCESSOR_ID++;
	}

	processor::~processor()
	{
	}

	void processor::enqueueRequest(request* pRequest)
	{
		_queueMutex.lock();
		_queue.push_back(pRequest);
		_syncPoint.notify_one();
		_queueMutex.unlock();
	}

	bool processor::sendError(request* pRequest, response& res, int errCode)
	{
		std::ostringstream& s = res.getContent();

		s << "\t" << "\"status\": " << "\"" << response::STATUS_FAILURE << "\"" << "," << std::endl;
		s << "\t" << "\"errCode\": " << errCode << "," << std::endl;
		s << "\t" << "\"errMessage\": " << "\"" << error::getMessage(errCode) << "\"" << std::endl;

		res.send();

		delete pRequest;

		return false;
	}

	bool processor::sendAcknowledge(request* pRequest, response& res)
	{
		std::ostringstream& s = res.getContent();

		s << "\t" << "\"status\": " << "\"" << response::STATUS_SUCCESS << "\"" << std::endl;

		res.send();

		delete pRequest;

		return true;
	}

	full_point processor::getFullPointFromRequest(request* pRequest, response& res)
	{
		auto lat = pRequest->getContent().get_optional<double>("lat");
		auto lon = pRequest->getContent().get_optional<double>("lon");
		auto level = pRequest->getContent().get_optional<int>("level");
		auto key = pRequest->getContent().get_optional<std::string>("key");
		auto group = pRequest->getContent().get_optional<std::string>("group");

		if (!lat || !lon || !level || !key || !group)
		{
			sendError(pRequest, res, error::MISSING_ARGUMENT);

			return invalid_point;
		}

		double fLat = lat.get();
		double fLon = lon.get();

		if (fLat < MIN_LAT || fLon < MIN_LON || fLat > MAX_LAT || fLon > MAX_LON)
		{
			sendError(pRequest, res, error::INVALID_COORDINATES);

			return invalid_point;
		}

		int nLevel = level.get();

		if (nLevel < MIN_LEVEL || nLevel > MAX_LEVEL)
		{
			sendError(pRequest, res, error::INVALID_LEVEL);

			return invalid_point;
		}

		uint64_t decodedKey = retrieveKey(key.get());

		if (decodedKey == 0)
		{
			sendError(pRequest, res, error::INVALID_KEY);

			return invalid_point;
		}

		full_point valid_point;
		valid_point.lat = (uint32_t)(LAT_RATIO * (fLat - MIN_LAT));
		valid_point.lon = (uint32_t)(LON_RATIO * (fLon - MIN_LON));
		valid_point.level = (uint16_t)(nLevel - MIN_LEVEL);
		valid_point.key = decodedKey;
		valid_point.groupId = registry::getOrCreateGroup(group.get())->getId();

		return valid_point;
	}

	bool processor::addPoint(request* pRequest, response& res)
	{
		std::cout << "addPoint from processor[" << _id << "]\n";

		full_point fp = getFullPointFromRequest(pRequest, res);

		if (fp != invalid_point)
		{
			auto p = registry::getOrSpawnTree(fp.level, fp.groupId);

			p->addPoint(fp.lat, fp.lon, fp.key);

			return sendAcknowledge(pRequest, res);
		}

		return false;
	}

	bool processor::removePoint(request* pRequest, response& res)
	{
		std::cout << "removePoint from processor[" << _id << "]\n";

		full_point fp = getFullPointFromRequest(pRequest, res);

		if (fp != invalid_point)
		{
			bool ok = false;

			auto pGroup = registry::getGroupById(fp.groupId);

			if (pGroup != 0)
			{
				auto pTree = pGroup->getTreeByLevel(fp.level);

				if (pTree != 0)
					ok = pTree->removePoint(fp.lat, fp.lon, fp.key);
			}

			if (ok)
				return sendAcknowledge(pRequest, res);
			else
				return sendError(pRequest, res, error::POINT_NOT_FOUND);
		}

		return false;
	}

	bool processor::findPoints(request* pRequest, response& res)
	{
		std::cout << "findPoints from processor[" << _id << "]\n";

		auto minLat = pRequest->getContent().get_optional<double>("minLat");
		auto minLon = pRequest->getContent().get_optional<double>("minLon");
		auto maxLat = pRequest->getContent().get_optional<double>("maxLat");
		auto maxLon = pRequest->getContent().get_optional<double>("maxLon");
		auto group = pRequest->getContent().get_optional<std::string>("group");

		if (!minLat || !minLon || !maxLat || !maxLon || !group)
			return sendError(pRequest, res, error::MISSING_ARGUMENT);

		double fMinLat = minLat.get();
		double fMinLon = minLon.get();
		double fMaxLat = maxLat.get();
		double fMaxLon = maxLon.get();

		if (fMinLat < MIN_LAT || fMinLon < MIN_LON || fMaxLat > MAX_LAT || fMaxLon > MAX_LON || fMinLat > fMaxLat || fMinLon > fMaxLon)
			return sendError(pRequest, res, error::INVALID_COORDINATES);

		uint32_t _minLat = (uint32_t)(LAT_RATIO * (fMinLat - MIN_LAT));
		uint32_t _minLon = (uint32_t)(LON_RATIO * (fMinLon - MIN_LON));
		uint32_t _maxLat = (uint32_t)(LAT_RATIO * (fMaxLat - MIN_LAT));
		uint32_t _maxLon = (uint32_t)(LON_RATIO * (fMaxLon - MIN_LON));
		uint16_t _groupId = registry::getOrCreateGroup(group.get())->getId();

		std::vector<full_point> points;
		points.reserve(MAX_POINTS_PER_REQUEST);

		auto pGroup = registry::getGroupById(_groupId);

		if (pGroup != 0)
		{
			auto za = pGroup->beginIterator();
			auto zb = pGroup->endIterator();

			for (auto z = za; z != zb; z++)
			{
				auto pTree = z->second;

				pTree->findPoints(points, MAX_POINTS_PER_REQUEST, _minLat, _minLon, _maxLat, _maxLon);
			}
		}

		std::ostringstream& s = res.getContent();

		s << "\t" << "\"status\": " << "\"" << response::STATUS_SUCCESS << "\"" << "," << std::endl;
		s << "\t" << "\"points\": " << std::endl;
		s << "\t" << "[" << std::endl;
		s << std::setprecision(9);

		auto ta = points.begin();
		auto tb = points.end();

		for (auto t = ta; t != tb;)
		{
			auto p = *t; t++;

			s << "\t\t" << "{" << std::endl;
			s << "\t\t\t" << "\"lat\": " << (double)(LAT_FACTOR * p.lat + MIN_LAT) << "," << std::endl;
			s << "\t\t\t" << "\"lon\": " << (double)(LON_FACTOR * p.lon + MIN_LON) << "," << std::endl;
			s << "\t\t\t" << "\"level\": " << (int)(p.level + MIN_LEVEL) << "," << std::endl;
			s << "\t\t\t" << "\"key\": " << "\"" << formatKey(p.key) << "\"" << std::endl;
			s << "\t\t" << "}";

			if (t != tb) s << ",";

			s << std::endl;
		}

		s << "\t" << "]" << std::endl;

		res.send();

		delete pRequest;

		return true;
	}

	bool processor::process(request* pRequest)
	{
		response res(pRequest->getTarget());

		auto id = pRequest->getContent().get_optional<int>("id");

		if (id)
		{
			res.init(id.get());

			auto type = pRequest->getContent().get_optional<std::string>("type");

			if (type)
			{
				if (type.get().compare(request::ADD_POINT) == 0)
					return addPoint(pRequest, res);

				if (type.get().compare(request::REMOVE_POINT) == 0)
					return removePoint(pRequest, res);

				if (type.get().compare(request::FIND_POINTS) == 0)
					return findPoints(pRequest, res);
			}
			else
			{
				return sendError(pRequest, res, error::UNSPECIFIED_REQUEST_TYPE);
			}

			return sendError(pRequest, res, error::UNKNOWN_REQUEST_TYPE);
		}
		else
		{
			res.init(-1);
			
			return sendError(pRequest, res, error::UNSPECIFIED_REQUEST_ID);
		}
	}

	void processor::workerLoop()
	{
		_isAlive = true;

		boost::unique_lock<boost::mutex> syncLock(_syncMutex);

		while (_isAlive)
		{
			_queueMutex.lock();

			while (_queue.empty())
			{
				_queueMutex.unlock();
				_syncPoint.wait(syncLock);
				_queueMutex.lock();
			}

			request* pRequest = _queue.front();
			_queue.pop_front();
			_queueMutex.unlock();

			process(pRequest);
		}
	}
}
