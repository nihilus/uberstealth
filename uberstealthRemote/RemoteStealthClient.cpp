#include <boost/bind.hpp>
#include "HideDebugger/ObjectTextSerialization.h"
#include "RemoteStealthClient.h"

remotestealth::RemoteStealthClient::RemoteStealthClient(boost::asio::io_service& ioService,
														boost::asio::ip::tcp::resolver::iterator endPointIterator) :
	ioService_(ioService)
{
	endPointIterator_ = endPointIterator;
}

void remotestealth::RemoteStealthClient::sendData(const RSProtocolItem& item)
{
	connection_->syncWrite(item);
	
	RSProtocolResponse response;
	connection_->syncRead(response);
	if (!response.success)
		throw std::runtime_error("Error while performing remote command: " + response.error);
}

void remotestealth::RemoteStealthClient::connect()
{
	connection_ = RemoteStealthConnectionPtr(new RemoteStealthConnection(ioService_));	
	connection_->socket().connect(*endPointIterator_);
}