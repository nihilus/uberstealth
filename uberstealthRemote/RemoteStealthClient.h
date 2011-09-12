// Implements the IDAStealth remote client.

#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include "RemoteStealthConnection.h"
#include "RemoteStealthProtocol.h"

namespace remotestealth
{
	class RemoteStealthClient
	{
	public:
		RemoteStealthClient(boost::asio::io_service& ioService,
			boost::asio::ip::tcp::resolver::iterator endPointIterator);

		void sendData(const RSProtocolItem& item);
		void connect();

	private:
		RemoteStealthClient& operator=(const RemoteStealthClient &);
		boost::asio::io_service& ioService_;
		boost::asio::ip::tcp::resolver::iterator endPointIterator_;
		RemoteStealthConnectionPtr connection_;
	};
}