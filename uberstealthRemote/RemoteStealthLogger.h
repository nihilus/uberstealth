// The loger compoenent that is used on the server side if remote stealth is used.

#pragma once

#include <iostream>

namespace uberstealth {

class RemoteStealthLogger
{
public:
	void logString(const char* str, ...) const {
		char buffer[500];
		va_list arglist;	
		va_start(arglist, str);
		vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, str, arglist);
		va_end(arglist);
		std::cout << buffer << std::endl;
	}
};

}