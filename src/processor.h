
#pragma once

#include <boost/thread/thread.hpp>

#include "session.h"
#include "request.h"
#include "response.h"
#include "point.h"

//----------------------------------------------------------------------

namespace geodis
{
	class processor
	{
	public: // STATIC METHODS

		static void enqueueRequest(request* req);

	private: // MEMBERS

		int _id;

		boost::thread _worker;

		boost::mutex _syncMutex;

		bool _isAlive;

	public: // CONSTRUCTORS

		processor();

		virtual ~processor();

	private: // METHODS

		full_point getFullPointFromRequest(request* pRequest, response& res);

		bool sendError(request* pRequest, response& res, int errCode);
		bool sendAcknowledge(request* pRequest, response& res);

		bool addPoint(request* pRequest, response& res);
		bool removePoint(request* pRequest, response& res);
		bool findPoints(request* pRequest, response& res);

		bool process(request* pRequest);

		void workerLoop();
	};
}