// Represents the stealth session on the server side if "remote stealth" is used.

#pragma once

#include <iostream>
#include "RemoteStealthProtocol.h"
#include "RemoteStealthLogger.h"
#include <uberstealth/ResourceItem.h>
#include <uberstealth/StealthSession.h>

namespace uberstealth {

	class RemoteStealthSession : public StealthSession<RemoteStealthLogger> {
	public:
		RemoteStealthSession(const boost::filesystem::path& configFile);
		virtual void handleBreakPoint(unsigned int threadID, uintptr_t address);
		virtual void handleException(unsigned int exceptionCode);

	private:
		virtual ResourceItem getRDTSCDriverResource();
		virtual ResourceItem getStealthDriverResource();
		virtual std::string getStealthDllPath();
		std::string serializeConfig(const std::string& configStr);
		std::string stealthDll_;
	};
}