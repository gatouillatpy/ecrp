
#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//----------------------------------------------------------------------

namespace geodis
{
	class session;

	class request
	{
	public: // CONSTANTS

		static const short MAX_LENGTH = 2048;

		static const std::string ADD_POINT;
		static const std::string REMOVE_POINT;
		static const std::string FIND_POINTS;

	private: // MEMBERS

		short _length;
		char _data[MAX_LENGTH];

		std::shared_ptr<session> _target;

		boost::property_tree::ptree _content;
		bool _contentHasBeenDeserialized;

	public: // CONSTRUCTORS

		request();

		virtual ~request();

	public: // METHODS

		bool isLengthValid() { return _length < MAX_LENGTH; }

		boost::asio::mutable_buffers_1 getLengthBuffer() { return boost::asio::buffer(&_length, sizeof(_length)); }
		boost::asio::mutable_buffers_1 getDataBuffer() { return boost::asio::buffer(_data, _length); }

		const boost::property_tree::ptree& getContent();

		void setTarget(std::shared_ptr<session> target);
		std::shared_ptr<session> getTarget() { return _target; }
	};
	
	typedef std::deque<request*> request_queue;
}
