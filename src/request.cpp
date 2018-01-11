
#include <iostream>

#include "request.h"
#include "session.h"
#include "compression.h"

using std::istringstream;
using boost::property_tree::ptree;
using boost::property_tree::read_json;

//----------------------------------------------------------------------

namespace ecrp {
    const string request::ADD_POINT("add");
    const string request::REMOVE_POINT("remove");
    const string request::FIND_POINTS("find");

    request::request() {
        _contentHasBeenDeserialized = false;
    }

    request::~request() {
        if (_target) {
            _target.reset();
        }
    }

    void request::setTarget(shared_ptr<session> target) {
        if (_target) {
            _target.reset();
        }

        if (target) {
            _target = target;
        }
    }

    const ptree &request::getContent() {
        if (!_contentHasBeenDeserialized) {
            istringstream stream(decompress(_data, _length));
            read_json(stream, _content);

            _contentHasBeenDeserialized = true;
        }

        return _content;
    }
}