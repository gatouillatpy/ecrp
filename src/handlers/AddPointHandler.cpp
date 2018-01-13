
#include <iostream>

#include "AddPointHandler.h"
#include "server/Processor.h"
#include "geodis/Registry.h"

using std::cout;
using std::endl;
using ecrp::geodis::Registry;

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		bool AddPointHandler::process(Processor* pProcessor, Request *pRequest, Response &res) {
			cout << "addPoint from Processor[" << pProcessor->getId() << "]" << endl;

			FullPoint fp = getFullPointFromRequest(pProcessor, pRequest, res);

			if (fp != INVALID_POINT) {
				auto p = Registry::getOrSpawnTree(fp.level, fp.groupId);

				p->addPoint(fp.lat, fp.lon, fp.key);

				return pProcessor->sendAcknowledge(pRequest, res);
			}

			return false;
		}
	}
}