
#include <iostream>

#include "response.h"
#include "session.h"
#include "compression.h"

using std::endl;
using boost::asio::mutable_buffers_1;
using boost::asio::buffer;

//----------------------------------------------------------------------

namespace ecrp {
    
    const string response::STATUS_SUCCESS("success");
    const string response::STATUS_FAILURE("failure");

    response::response(shared_ptr<session> target) : _target(target) {
        _data = 0;
    }

    response::response(const response &other) : _target(other._target) {
        _length = other._length;
        _data = other._data;
        _content << other._content.str();
    }

    response::~response() {
        if (_target) {
            _target.reset();
        }
    }

    mutable_buffers_1 response::getBuffer() {
        if (!_data) {
            string compressedOutput = compress(_content.str());

            _length = 4 + compressedOutput.size();
            _data = (byte *)malloc(_length);

            *(int *)(_data + 0) = compressedOutput.size();
            memcpy(_data + 4, compressedOutput.c_str(), compressedOutput.size());
        }

        return buffer(_data, _length);
    }

    void response::init(int id) {
        _content << "{" << endl;

        if (id > -1) {
            _content << "\t" << "\"id\": " << id << "," << endl;
        }
    }

    void response::send() {
        _content << "}" << endl;

        _target->send(*this);
    }
}