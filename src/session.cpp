
#include <iostream>

#include "session.h"
#include "processor.h"

//----------------------------------------------------------------------

namespace geodis
{
	session::session(tcp::socket socket) : _socket(std::move(socket))
	{
	}

	session::~session()
	{
		std::cout << "Diconnected.\n";
	}

	void session::start()
	{
		boost::asio::ip::tcp::endpoint ep = _socket.remote_endpoint();

		std::cout << "Connected to " << ep.address().to_string() << ":" << ep.port() << ".\n";

		read();
	}

	void session::send(response& res)
	{
		boost::asio::async_write(_socket, res.getBuffer(), [this](boost::system::error_code errcode, size_t)
		{
			if (errcode)
				std::cerr << "ERROR: Something bad happened.\n";
		});
	}

	void session::read()
	{
		request* pRequest = new request();
		pRequest->setTarget(shared_from_this());

		boost::asio::async_read(_socket, pRequest->getLengthBuffer(), [this, pRequest](boost::system::error_code errcode, size_t)
		{
			if (!errcode)
			{
				if (pRequest->isLengthValid())
					readContent(pRequest);
				else
					std::cerr << "ERROR: The received message is too large.\n";
			}
			else
			{
				if (errcode.value() != boost::asio::error::eof)
					std::cerr << "ERROR: Something bad happened.\n";

				pRequest->setTarget(nullptr);
			}
		});
	}

	void session::readContent(request* pRequest)
	{
		boost::asio::async_read(_socket, pRequest->getDataBuffer(), [this, pRequest](boost::system::error_code errcode, size_t)
		{
			if (!errcode)
			{
				processor::enqueueRequest(pRequest);

				read();
			}
			else
			{
				if (errcode.value() != boost::asio::error::eof)
					std::cerr << "ERROR: Something bad happened.\n";

				pRequest->setTarget(nullptr);
			}
		});
	}
}
