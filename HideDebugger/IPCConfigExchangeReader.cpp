#include "IPCConfigExchangeReader.h"

using namespace ipc;

IPCConfigExchangeReader::IPCConfigExchangeReader()
{
	using namespace boost::interprocess;
	segment_ = managed_shared_memory(open_read_only, ipc::getSegmentName().c_str());
}

std::string IPCConfigExchangeReader::getProfileFile()
{
	std::pair<char*, size_t> dataPtr = segment_.find<char>(ipc::ConfigFileDataStr);
	return std::string(dataPtr.first);
}

unsigned int IPCConfigExchangeReader::getIDAProcessID()
{
	std::pair<unsigned int*, size_t> dataPtr = segment_.find<unsigned int>(ipc::IDAProcessIDStr);
	return *(dataPtr.first);
}

ipc::IPCPEHeaderData IPCConfigExchangeReader::getIPCPEHeaderData()
{
	std::pair<ipc::IPCPEHeaderData*, size_t> dataPtr = segment_.find<ipc::IPCPEHeaderData>(ipc::PEHeaderDataStr);
	return *dataPtr.first;
}

bool ipc::IPCConfigExchangeReader::isPERestoreRequired()
{
	std::pair<bool*, size_t> dataPtr = segment_.find<bool>(ipc::PERestoreRequiredStr);
	return *(dataPtr.first);
}