#include "IPCConfigExchangeReader.h"

using namespace uberstealth;

IPCConfigExchangeReader::IPCConfigExchangeReader() {
	using namespace boost::interprocess;
	_segment = managed_shared_memory(open_read_only, uberstealth::getSegmentName().c_str());
}

std::string IPCConfigExchangeReader::getProfileFile() {
	std::pair<char*, size_t> dataPtr = _segment.find<char>(uberstealth::ConfigFileDataStr);
	return std::string(dataPtr.first);
}

unsigned int IPCConfigExchangeReader::getDebuggerProcessID() {
	std::pair<unsigned int*, size_t> dataPtr = _segment.find<unsigned int>(uberstealth::IDAProcessIDStr);
	return *(dataPtr.first);
}

uberstealth::IPCPEHeaderData IPCConfigExchangeReader::getIPCPEHeaderData() {
	std::pair<uberstealth::IPCPEHeaderData*, size_t> dataPtr = _segment.find<uberstealth::IPCPEHeaderData>(uberstealth::PEHeaderDataStr);
	return *dataPtr.first;
}

bool uberstealth::IPCConfigExchangeReader::isPERestoreRequired() {
	std::pair<bool*, size_t> dataPtr = _segment.find<bool>(uberstealth::PERestoreRequiredStr);
	return *(dataPtr.first);
}