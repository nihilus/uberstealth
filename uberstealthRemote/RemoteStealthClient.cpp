#include "RemoteStealthClient.h"
#include <boost/make_shared.hpp>

uberstealth::RemoteStealthClient::RemoteStealthClient(boost::asio::io_service& ioService, boost::asio::ip::tcp::resolver::iterator endPointIterator) :
	ioService_(ioService),
	endPointIterator_(endPointIterator) {}

void uberstealth::RemoteStealthClient::sendData(const RSProtocolItem& item) {
	connection_->syncWrite(item);
	RSProtocolResponse response;
	connection_->syncRead(response);
	if (!response.success) {
		throw std::runtime_error("Error while performing remote command: " + response.error);
	}
}

void uberstealth::RemoteStealthClient::connect() {
	connection_ = boost::make_shared<RemoteStealthConnection>(boost::ref(ioService_));	
	connection_->socket().connect(*endPointIterator_);
}