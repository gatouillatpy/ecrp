
#include <iostream>

#include "Response.h"
#include "Session.h"
#include "../utils/compression.h"

using std::endl;
using boost::asio::mutable_buffers_1;
using boost::asio::buffer;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		const string Response::STATUS_SUCCESS("success");
		const string Response::STATUS_FAILURE("failure");

		Response::Response(shared_ptr<Session> target) : _target(target) {
			_data = 0;
		}

		Response::Response(const Response &other) : _target(other._target) {
			_length = other._length;
			_data = other._data;
			_content << other._content.str();
		}

		Response::~Response() {
			if (_target) {
				_target.reset();
			}
		}

		mutable_buffers_1 Response::getBuffer() {
			if (!_data) {
				string compressedOutput = compress(_content.str());

				_length = 4 + compressedOutput.size();
				_data = (byte *)malloc(_length);

				*(int *)(_data + 0) = compressedOutput.size();
				memcpy(_data + 4, compressedOutput.c_str(), compressedOutput.size());
			}

			return buffer(_data, _length);
		}

		void Response::init(int id) {
			_content << "{" << endl;

			if (id > -1) {
				_content << "\t" << "\"id\": " << id << "," << endl;
			}
		}

		void Response::send() {
			_content << "}" << endl;

			_target->send(*this);
		}
	}
}