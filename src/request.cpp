
#include <iostream>

#include "request.h"
#include "session.h"
#include "compression.h"

//----------------------------------------------------------------------

namespace geodis
{
	const std::string request::ADD_POINT("add");
	const std::string request::REMOVE_POINT("remove");
	const std::string request::FIND_POINTS("find");

	request::request()
	{
		_contentHasBeenDeserialized = false;
	}

	request::~request()
	{
		if (_target)
			_target.reset();
	}

	void request::setTarget(std::shared_ptr<session> target)
	{
		if (_target)
			_target.reset();

		if (target)
			_target = target;
	}

	const boost::property_tree::ptree& request::getContent()
	{
		if (!_contentHasBeenDeserialized)
		{
			std::istringstream stream(decompress(_data, _length));
			boost::property_tree::read_json(stream, _content);

			_contentHasBeenDeserialized = true;
		}

		return _content;
	}
}
