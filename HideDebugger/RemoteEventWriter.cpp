#include "ObjectTextSerialization.h"
#include "RemoteEventWriter.h"

using namespace uberstealth;

uberstealth::RemoteEventWriter::RemoteEventWriter() :
	mq_(boost::interprocess::open_only, genRemoteEventName().c_str())
{
}

void uberstealth::RemoteEventWriter::sendEvent(const RemoteEventData& /*data*/) const
{
	using namespace std;

	//serialize(data, mq_);
	//mq_.send()
}