#include "RemoteEventWriter.h"
#include "ObjectTextSerialization.h"

uberstealth::RemoteEventWriter::RemoteEventWriter() :
	mq_(boost::interprocess::open_only, genRemoteEventName().c_str()) {}

void uberstealth::RemoteEventWriter::sendEvent(const RemoteEventData& /*data*/) const {
	//serialize(data, mq_);
	//mq_.send()
}