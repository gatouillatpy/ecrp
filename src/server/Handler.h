
#pragma once

#include "Request.h"
#include "Response.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Processor;

		class Handler {

		public: // METHODS

			virtual const string& getType() = 0;
			virtual bool process(Processor* pProcessor, Request *pRequest, Response &res) = 0;

			virtual int getBinaryType() {
				return getType().c_str()[0]; // TODO: user Adler32 or something...
			};
		};
	}
}