// Implements the IDAStealth remote server.

#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include "RemoteStealthConnection.h"
#include "RemoteStealthProtocol.h"
#include <vector>

namespace remotestealth {
	class RemoteStealthServer {
	public:
		RemoteStealthServer(boost::asio::io_service& ioService, unsigned short port);
		void run();

	private:
		void session(RemoteStealthConnectionPtr connection);

		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::io_service& ioService_;
		remotestealth::RSProtocolItem protocolItem_;
	};
}