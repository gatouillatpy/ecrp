
#pragma once

#include <boost/thread/thread.hpp>

using boost::thread;
using boost::mutex;

#include "Session.h"
#include "Request.h"
#include "Response.h"
#include "../geodis/Point.h"

using ecrp::geodis::FullPoint;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Processor {

		public: // STATIC METHODS

			static void enqueueRequest(Request *req);

		private: // MEMBERS

			int _id;

			thread _worker;

			mutex _syncMutex;

			bool _isAlive;

		public: // CONSTRUCTORS

			Processor();

			virtual ~Processor();

		private: // METHODS

			FullPoint getFullPointFromRequest(Request *pRequest, Response &res);

			bool sendError(Request *pRequest, Response &res, int errCode);
			bool sendAcknowledge(Request *pRequest, Response &res);

			bool addPoint(Request *pRequest, Response &res); // TODO: refactor this into a Service => processors use services
			bool removePoint(Request *pRequest, Response &res);
			bool findPoints(Request *pRequest, Response &res);

			bool process(Request *pRequest);

			void workerLoop();
		};

		static const FullPoint INVALID_POINT;
	}
}