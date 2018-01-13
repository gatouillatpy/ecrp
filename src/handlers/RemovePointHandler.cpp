
#include <iostream>

#include "RemovePointHandler.h"
#include "server/Processor.h"
#include "geodis/Registry.h"
#include "errors/Error.h"

using std::cout;
using std::endl;
using ecrp::geodis::Registry;

//----------------------------------------------------------------------

namespace ecrp {
	namespace handlers {

		bool RemovePointHandler::process(Processor* pProcessor, Request *pRequest, Response &res) {
			cout << "removePoint from Processor[" << pProcessor->getId() << "]" << endl;

			FullPoint fp = getFullPointFromRequest(pProcessor, pRequest, res);

			if (fp != INVALID_POINT) {
				bool ok = false;

				auto pGroup = Registry::getGroupById(fp.groupId);

				if (pGroup != 0) {
					auto pTree = pGroup->getTreeByLevel(fp.level);

					if (pTree != 0) {
						ok = pTree->removePoint(fp.lat, fp.lon, fp.key);
					}
				}

				if (ok) {
					return pProcessor->sendAcknowledge(pRequest, res);
				}
				else {
					return pProcessor->sendError(pRequest, res, Error::POINT_NOT_FOUND);
				}
			}

			return false;
		}
	}
}