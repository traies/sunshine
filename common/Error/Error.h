#pragma once

#include <string>

namespace sun
{
	class Error
	{
	public:
		Error();
		Error(std::string message);
		~Error();
		std::string Message();
	protected:
		std::string _message;
	};
}

