
#include <iostream>

#include "Request.h"
#include "Session.h"
#include "../utils/compression.h"

using std::istringstream;
using boost::property_tree::ptree;
using boost::property_tree::read_json;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		const string Request::ADD_POINT("add");
		const string Request::REMOVE_POINT("remove");
		const string Request::FIND_POINTS("find");

		Request::Request() {
			_contentHasBeenDeserialized = false;
		}

		Request::~Request() {
			if (_target) {
				_target.reset();
			}
		}

		void Request::setTarget(shared_ptr<Session> target) {
			if (_target) {
				_target.reset();
			}

			if (target) {
				_target = target;
			}
		}

		const ptree &Request::getContent() {
			if (!_contentHasBeenDeserialized) {
				istringstream stream(decompress(_data, _length));
				read_json(stream, _content);

				_contentHasBeenDeserialized = true;
			}

			return _content;
		}
	}
}