
#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;

#include "Processor.h"
#include "Session.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		class Server {

		private: // MEMBERS

			tcp::acceptor _acceptor;
			tcp::socket _socket;

		public: // CONSTRUCTORS

			Server(io_service &io_service, const tcp::endpoint &endpoint);

		private: // METHODS

			void listen();
		};
	}
}