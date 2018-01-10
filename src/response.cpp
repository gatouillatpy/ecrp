
#include <iostream>

#include "response.h"
#include "session.h"
#include "compression.h"

//----------------------------------------------------------------------

namespace geodis
{
	const std::string response::STATUS_SUCCESS("success");
	const std::string response::STATUS_FAILURE("failure");

	response::response(std::shared_ptr<session> target) : _target(target)
	{
		_data = 0;
	}

	response::response(const response& other) : _target(other._target)
	{
		_length = other._length;
		_data = other._data;
		_content << other._content.str();
	}

	response::~response()
	{
		if (_target)
			_target.reset();
	}

	boost::asio::mutable_buffers_1 response::getBuffer()
	{
		if (!_data)
		{
			std::string compressedOutput = compress(_content.str());

			_length = 4 + compressedOutput.size();
			_data = (byte*)malloc(_length);

			*(int*)(_data + 0) = compressedOutput.size();
			memcpy(_data + 4, compressedOutput.c_str(), compressedOutput.size());
		}

		return boost::asio::buffer(_data, _length);
	}

	void response::init(int id)
	{
		_content << "{" << std::endl;
		
		if (id > -1)
			_content << "\t" << "\"id\": " << id << "," << std::endl;
	}

	void response::send()
	{
		_content << "}" << std::endl;

		_target->send(*this);
	}
}
