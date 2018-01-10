
#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "processor.h"
#include "session.h"

//----------------------------------------------------------------------

namespace geodis
{
	class server
	{
	private: // MEMBERS

		tcp::acceptor _acceptor;
		tcp::socket _socket;

	public: // CONSTRUCTORS

		server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint);

	private: // METHODS

		void listen();
	};
}
