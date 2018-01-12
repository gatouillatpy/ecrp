
#pragma once

#include <boost/asio.hpp>

using std::enable_shared_from_this;
using boost::asio::ip::tcp;

#include "Request.h"
#include "Response.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Session : public enable_shared_from_this<Session> {

		private: // MEMBERS

			tcp::socket _socket;

		public: // CONSTRUCTORS

			Session(tcp::socket socket);

			virtual ~Session();

		public: // METHODS

			void start();

			void send(Response &res);

		private: // METHODS

			void read();
			void readContent(Request *pRequest);
		};
	}
}