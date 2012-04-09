#pragma once

// common data structures for remote event inter process communication

#include <iostream>

namespace uberstealth
{
	struct RemoteEventData 
	{
		std::string functionName;
		unsigned int threadID;
	};

	std::string genRemoteEventName(unsigned int processID);
	std::string genRemoteEventName();
}