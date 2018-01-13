
#include <atomic>
#include <unordered_map>
#include <iostream>

#include <boost/optional.hpp>

#include "Processor.h"
#include "errors/Error.h"

using std::atomic_int;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::setprecision;
using boost::mutex;
using boost::condition_variable;
using boost::unique_lock;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		static atomic_int NEXT_PROCESSOR_ID(0);

		static unordered_map<string, Handler*> _handlers;

		static request_queue _queue;
		static mutex _queueMutex;

		static condition_variable _syncPoint;

		void Processor::attachHandler(Handler *pHandler) {
			_handlers[pHandler->getType()] = pHandler;
		}

		void Processor::enqueueRequest(Request *pRequest) {
			_queueMutex.lock();
			_queue.push_back(pRequest);
			_syncPoint.notify_one();
			_queueMutex.unlock();
		}

		Processor::Processor() : _worker(&Processor::workerLoop, this) {
			_id = NEXT_PROCESSOR_ID++;
		}

		Processor::~Processor() {
		}

		bool Processor::sendError(Request *pRequest, Response &res, int errCode) {
			ostringstream &s = res.getContent();

			s << "\t" << "\"status\": " << "\"" << Response::STATUS_FAILURE << "\"" << "," << endl;
			s << "\t" << "\"errCode\": " << errCode << "," << endl;
			s << "\t" << "\"errMessage\": " << "\"" << Error::getMessage(errCode) << "\"" << endl;

			res.send();

			delete pRequest;

			return false;
		}

		bool Processor::sendAcknowledge(Request *pRequest, Response &res) {
			ostringstream &s = res.getContent();

			s << "\t" << "\"status\": " << "\"" << Response::STATUS_SUCCESS << "\"" << endl;

			res.send();

			delete pRequest;

			return true;
		}

		bool Processor::process(Request *pRequest) {
			Response res(pRequest->getTarget());

			auto id = pRequest->getContent().get_optional<int>("id");

			if (id) {
				res.init(id.get());

				auto type = pRequest->getContent().get_optional<string>("type");

				if (type) {
					auto i = _handlers.find(type.get());
					if (i != _handlers.end()) {
						(*i).second->process(this, pRequest, res);
					} else {
						return sendError(pRequest, res, Error::INVALID_REQUEST_TYPE);
					}
				} else {
					return sendError(pRequest, res, Error::UNSPECIFIED_REQUEST_TYPE);
				}

				return sendError(pRequest, res, Error::UNKNOWN_REQUEST_TYPE);
			} else {
				res.init(-1);

				return sendError(pRequest, res, Error::UNSPECIFIED_REQUEST_ID);
			}
		}

		void Processor::workerLoop() {
			_isAlive = true;

			unique_lock<mutex> syncLock(_syncMutex);

			while (_isAlive) {
				_queueMutex.lock();

				while (_queue.empty()) {
					_queueMutex.unlock();
					_syncPoint.wait(syncLock);
					_queueMutex.lock();
				}

				Request *pRequest = _queue.front();
				_queue.pop_front();
				_queueMutex.unlock();

				process(pRequest);
			}
		}
	}
}