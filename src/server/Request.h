
#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using std::string;
using std::shared_ptr;
using std::deque;
using boost::property_tree::ptree;
using boost::asio::mutable_buffers_1;
using boost::asio::buffer;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Session;

		class Request {
		public: // CONSTANTS

			static const short MAX_LENGTH = 2048;

		private: // MEMBERS

			short _length;
			char _data[MAX_LENGTH];

			shared_ptr<Session> _target;

			ptree _content;
			bool _contentHasBeenDeserialized;

		public: // CONSTRUCTORS

			Request();

			virtual ~Request();

		public: // METHODS

			bool isLengthValid() {
				return _length < MAX_LENGTH;
			}

			mutable_buffers_1 getLengthBuffer() {
				return buffer(&_length, sizeof(_length));
			}
			mutable_buffers_1 getDataBuffer() {
				return buffer(_data, _length);
			}

			const ptree &getContent();

			void setTarget(shared_ptr<Session> target);
			shared_ptr<Session> getTarget() {
				return _target;
			}
		};

		typedef deque<Request *> request_queue;
	}
}