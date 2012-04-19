#include "IPCConfigExchangeReader.h"

uberstealth::IPCConfigExchangeReader::IPCConfigExchangeReader() {
	segment_ = boost::interprocess::managed_shared_memory(boost::interprocess::open_read_only, uberstealth::getSegmentName().c_str());
}

std::string uberstealth::IPCConfigExchangeReader::getProfileFile() {
	std::pair<char*, size_t> dataPtr = segment_.find<char>(uberstealth::ConfigFileDataStr);
	return std::string(dataPtr.first);
}

unsigned int uberstealth::IPCConfigExchangeReader::getDebuggerProcessID() {
	std::pair<unsigned int*, size_t> dataPtr = segment_.find<unsigned int>(uberstealth::IDAProcessIDStr);
	return *(dataPtr.first);
}

uberstealth::IPCPEHeaderData uberstealth::IPCConfigExchangeReader::getIPCPEHeaderData() {
	std::pair<uberstealth::IPCPEHeaderData*, size_t> dataPtr = segment_.find<uberstealth::IPCPEHeaderData>(uberstealth::PEHeaderDataStr);
	return *dataPtr.first;
}

bool uberstealth::IPCConfigExchangeReader::isPERestoreRequired() {
	std::pair<bool*, size_t> dataPtr = segment_.find<bool>(uberstealth::PERestoreRequiredStr);
	return *(dataPtr.first);
}