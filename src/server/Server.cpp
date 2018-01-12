
#include "Server.h"

using std::make_shared;
using std::move;
using boost::asio::io_service;
using boost::system::error_code;

//----------------------------------------------------------------------

namespace ecrp {
	namespace server {

		Server::Server(io_service &ios, const tcp::endpoint &endpoint) : _acceptor(ios, endpoint), _socket(ios) {
			listen();
		}

		void Server::listen() {
			_acceptor.async_accept(_socket, [this](error_code e) {
				if (!e) {
					make_shared<Session>(move(_socket))->start();
				}

				listen();
			});
		}
	}
}