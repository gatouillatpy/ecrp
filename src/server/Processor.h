
#pragma once

#include <boost/thread/thread.hpp>

using boost::thread;
using boost::mutex;

#include "Handler.h"
//#include "Session.h"
#include "Request.h"
#include "Response.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Processor {

		public: // STATIC METHODS

			static void attachHandler(Handler *pHandler);
			static void enqueueRequest(Request *pRequest);

		private: // MEMBERS

			int _id;

			thread _worker;

			mutex _syncMutex;

			bool _isAlive;

		public: // CONSTRUCTORS

			Processor();

			virtual ~Processor();

		public: // METHODS

			int getId() {
				return _id;
			};

			bool sendError(Request *pRequest, Response &res, int errCode);
			bool sendAcknowledge(Request *pRequest, Response &res);

		private: // METHODS

			bool process(Request *pRequest);

			void workerLoop();
		};
	}
}