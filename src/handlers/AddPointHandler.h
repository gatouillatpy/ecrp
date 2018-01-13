
#pragma once

#include "GeodisHandler.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		class AddPointHandler : public GeodisHandler {

		public: // METHODS

			virtual const string& getType() { return "addPoint"; };
			virtual bool process(Processor* pProcessor, Request *pRequest, Response &res);
		};
	}
}