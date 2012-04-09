#pragma once

// Class which allows to transfer all needed configuration data to the debugge via IPC.

#pragma warning(disable : 4189)
#include <boost/interprocess/managed_shared_memory.hpp>
#pragma warning(default : 4189)
#include <iostream>
#include "IPCConfigExchangeCommon.h"

namespace uberstealth
{
	class IPCConfigExchangeWriter
	{
	public:

		IPCConfigExchangeWriter(unsigned int processID);
		~IPCConfigExchangeWriter() {}
		void setProfileFile(const std::string& configFile);
		void setIDAProcessID(unsigned int processID);
		void setIPCPEHeaderData(const IPCPEHeaderData& headerData);
		void setPERestoreRequired(bool required);
		void remove();

	private:

		boost::interprocess::managed_shared_memory segment_;
		unsigned int processID_;
	};
}