
#pragma once

#include <boost/asio.hpp>

#include "request.h"
#include "response.h"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

namespace geodis
{
	class session : public std::enable_shared_from_this<session>
	{
	private: // MEMBERS

		tcp::socket _socket;

	public: // CONSTRUCTORS

		session(tcp::socket socket);

		virtual ~session();

	public: // METHODS

		void start();

		void send(response& res);

	private: // METHODS

		void read();
		void readContent(request* pRequest);
	};
}
