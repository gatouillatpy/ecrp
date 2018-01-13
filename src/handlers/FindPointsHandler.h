
#pragma once

#include "GeodisHandler.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		class FindPointsHandler : public GeodisHandler {

		public: // CONSTANTS

			static const size_t MAX_POINTS_PER_REQUEST;

		public: // METHODS

			virtual const string& getType() { return "findPoints"; };
			virtual bool process(Processor* pProcessor, Request *pRequest, Response &res);
		};
	}
}