
#pragma once

#include <deque>

#include <boost/asio.hpp>

#include "utils.h"

//----------------------------------------------------------------------

namespace geodis
{
	class session;

	class response
	{
	public: // CONSTANTS

		static const std::string STATUS_SUCCESS;
		static const std::string STATUS_FAILURE;

	private: // MEMBERS

		int _length;
		byte* _data;

		std::shared_ptr<session> _target;

		std::ostringstream _content;

	public: // CONSTRUCTORS

		response(std::shared_ptr<session> target);
		response(const response& other);

		virtual ~response();

	public: // METHODS

		boost::asio::mutable_buffers_1 getBuffer();

		void init(int id);
		void send();

		std::ostringstream& getContent() { return _content; }
	};
	
	typedef std::deque<response> response_queue;
}
