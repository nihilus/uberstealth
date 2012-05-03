#include "IPCConfigExchangeWriter.h"

uberstealth::IPCConfigExchangeWriter::IPCConfigExchangeWriter(unsigned int processId) :
	processID_(processId) {
	std::string name = getSegmentName(processId);
	try	{
		boost::interprocess::shared_memory_object::remove(name.c_str());
		segment_ = boost::interprocess::managed_shared_memory(boost::interprocess::create_only, name.c_str(), SegmentSize);
		segment_.construct<char>(ConfigFileDataStr)[ConfigDataSegmentSize](0);
		segment_.construct<unsigned int>(IDAProcessIDStr)[1](0);
		segment_.construct<IPCPEHeaderData>(PEHeaderDataStr)[1]();
		segment_.construct<bool>(PERestoreRequiredStr)[1](false);
	} catch (const std::exception&) {
		boost::interprocess::shared_memory_object::remove(name.c_str());
		throw;
	}
}

uberstealth::IPCConfigExchangeWriter::~IPCConfigExchangeWriter() {
	boost::interprocess::shared_memory_object::remove(getSegmentName(processID_).c_str());
}

void uberstealth::IPCConfigExchangeWriter::setProfileFile(const std::string& configFile) {
	std::pair<char*, size_t> segmentData = segment_.find<char>(ConfigFileDataStr);
	strcpy_s(segmentData.first, segmentData.second, configFile.c_str());
}

void uberstealth::IPCConfigExchangeWriter::setIDAProcessID(unsigned int processId) {
	std::pair<unsigned int*, size_t> segmentData = segment_.find<unsigned int>(IDAProcessIDStr);
	*(segmentData.first) = processId;
}

void uberstealth::IPCConfigExchangeWriter::setIPCPEHeaderData(const IPCPEHeaderData& headerData) {
	std::pair<IPCPEHeaderData*, size_t> segmentData = segment_.find<IPCPEHeaderData>(PEHeaderDataStr);
	*segmentData.first = headerData;
}

void uberstealth::IPCConfigExchangeWriter::setPERestoreRequired(bool required) {
	std::pair<bool*, size_t> segmentData = segment_.find<bool>(PERestoreRequiredStr);
	*(segmentData.first) = required;
}