#include "stdafx.h"
#include "Error.h"

using namespace sun;

Error::Error()
{
}

Error::Error(std::string message)
{
	_message = message;
}

Error::~Error()
{
}

std::string Error::Message()
{
	return _message;
}
