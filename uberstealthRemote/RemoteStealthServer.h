// Implements the IDAStealth remote server.

#pragma once

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "RemoteStealthConnection.h"
#include "RemoteStealthProtocol.h"

namespace uberstealth {
	class RemoteStealthServer {
	public:
		RemoteStealthServer(boost::asio::io_service& ioService, unsigned short port);
		void run();

	private:
		void session(boost::shared_ptr<RemoteStealthConnection> connection);

		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::io_service& ioService_;
		uberstealth::RSProtocolItem protocolItem_;
	};
}