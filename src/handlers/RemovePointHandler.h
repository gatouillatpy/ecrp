
#pragma once

#include "GeodisHandler.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		class RemovePointHandler : public GeodisHandler {

		public: // METHODS

			virtual const string& getType() { return "removePoints"; };
			virtual bool process(Processor* pProcessor, Request *pRequest, Response &res);
		};
	}
}