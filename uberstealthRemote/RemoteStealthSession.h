#pragma once

// used in the remote stealth scenario on the server side

#include <iostream>
#include <IDAStealth/StealthSession.h>
#include <IDAStealth/ResourceItem.h>
#include "RemoteStealthProtocol.h"

namespace remotestealth
{
	class RemoteStealthSession : public idastealth::StealthSession
	{
	public:

		RemoteStealthSession();
		~RemoteStealthSession() {}

		void handleDbgAttach(const RSProtocolItem& item);
		void handleProcessStart(const RSProtocolItem& item);

	private:

		std::string stealthDll_;
		
		void logString(const std::string& str);
		ResourceItem getRDTSCDriverResource();
		ResourceItem getStealthDriverResource();
		std::string getStealthDllPath();
		std::string serializeConfig(const std::string& configStr);
	};
}