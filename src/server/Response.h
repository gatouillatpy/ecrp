
#pragma once

#include <deque>

#include <boost/asio.hpp>

using std::string;
using std::shared_ptr;
using std::ostringstream;
using std::deque;
using boost::asio::mutable_buffers_1;

#include "../utils/utils.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Session;

		class Response {
		public: // CONSTANTS

			static const string STATUS_SUCCESS;
			static const string STATUS_FAILURE;

		private: // MEMBERS

			int _length;
			byte *_data;

			shared_ptr<Session> _target;

			ostringstream _content;

		public: // CONSTRUCTORS

			Response(shared_ptr<Session> target);
			Response(const Response &other);

			virtual ~Response();

		public: // METHODS

			mutable_buffers_1 getBuffer();

			void init(int id);
			void send();

			ostringstream &getContent() {
				return _content;
			}
		};

		typedef deque<Response> response_queue;
	}
}