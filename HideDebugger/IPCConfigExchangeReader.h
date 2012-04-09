// Used by the injected dll to read the original PE header, the path to the debugger profile
// as well as the process id of the debugger.

#pragma once

#include <iostream>
#pragma warning(disable : 4189)
#include <boost/interprocess/managed_shared_memory.hpp>
#pragma warning(default : 4189)
#include "uberstealth/IPCConfigExchangeCommon.h"

namespace uberstealth {
	class IPCConfigExchangeReader {
	public:
		IPCConfigExchangeReader();
		std::string getProfileFile();
		std::string getProfile();
		unsigned int getDebuggerProcessID();
		uberstealth::IPCPEHeaderData getIPCPEHeaderData();
		bool isPERestoreRequired();

	private:
		boost::interprocess::managed_shared_memory _segment;
	};
}