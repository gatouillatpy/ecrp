
#pragma once

#include "server/Handler.h"
#include "geodis/Point.h"

using ecrp::server::Request;
using ecrp::server::Response;
using ecrp::server::Processor;
using ecrp::server::Handler;
using ecrp::geodis::FullPoint;

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		class GeodisHandler : public Handler {

		public: // CONSTANTS

			static const double MIN_LAT;
			static const double MAX_LAT;
			static const double MIN_LON;
			static const double MAX_LON;

			static const double LAT_RATIO;
			static const double LON_RATIO;
			static const double LAT_FACTOR;
			static const double LON_FACTOR;

			static const int MIN_LEVEL;
			static const int MAX_LEVEL;

		protected: // METHODS

			FullPoint getFullPointFromRequest(Processor* pProcessor, Request *pRequest, Response &res);
		};

		static const FullPoint INVALID_POINT;
	}
}