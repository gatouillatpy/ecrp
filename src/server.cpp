
#include "server.h"

//----------------------------------------------------------------------

namespace geodis
{
	server::server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint) : _acceptor(io_service, endpoint), _socket(io_service)
	{
		listen();
	}

	void server::listen()
	{
		_acceptor.async_accept(_socket, [this](boost::system::error_code errcode)
		{
			if (!errcode)
				std::make_shared<session>(std::move(_socket))->start();

			listen();
		});
	}
}
