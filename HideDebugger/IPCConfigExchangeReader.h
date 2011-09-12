#pragma once

#pragma warning(disable : 4189)
#include <boost/interprocess/managed_shared_memory.hpp>
#pragma warning(default : 4189)
#include <iostream>
#include "uberstealth/IPCConfigExchangeCommon.h"

namespace ipc
{
	class IPCConfigExchangeReader
	{
	public:
		IPCConfigExchangeReader();
		~IPCConfigExchangeReader() {}

		std::string getProfileFile();
		std::string getProfile();
		unsigned int getIDAProcessID();
		ipc::IPCPEHeaderData getIPCPEHeaderData();
		bool isPERestoreRequired();

	private:
		boost::interprocess::managed_shared_memory segment_;
	};
}