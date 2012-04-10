// Represents the stealth session on the server side if "remote stealth" is used.

#pragma once

#include <iostream>
#include "RemoteStealthProtocol.h"
#include <uberstealth/ResourceItem.h>

namespace remotestealth {

	class RemoteStealthSession {
	public:
		RemoteStealthSession();
		void handleDbgAttach(const RSProtocolItem& item);
		void handleProcessStart(const RSProtocolItem& item);

	private:		
		void logString(const std::string& str);
		ResourceItem getRDTSCDriverResource();
		ResourceItem getStealthDriverResource();
		std::string getStealthDllPath();
		std::string serializeConfig(const std::string& configStr);

		std::string _stealthDll;
	};
}