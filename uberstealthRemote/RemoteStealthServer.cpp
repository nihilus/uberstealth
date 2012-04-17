#include <boost/filesystem.hpp>
#pragma warning(disable : 4244 4512)
#include <boost/thread.hpp>
#pragma warning(default : 4244 4512)
#include <boost/bind.hpp>
#include "RemoteStealthServer.h"
#include "RemoteStealthSession.h"
#include "TemporaryConfigFile.h"

uberstealth::RemoteStealthServer::RemoteStealthServer(boost::asio::io_service& ioService, unsigned short port) :
	acceptor_(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	ioService_(ioService) {}

void uberstealth::RemoteStealthServer::run() {
	for (;;) {
		boost::shared_ptr<RemoteStealthConnection> connection = boost::make_shared<RemoteStealthConnection>(boost::ref(ioService_));
		acceptor_.accept(connection->socket());
		std::cout << "Accepted connection from " << connection->socket().remote_endpoint().address() << std::endl;
		boost::thread t(boost::bind(&RemoteStealthServer::session, this, connection));
	}
}

void uberstealth::RemoteStealthServer::session(boost::shared_ptr<RemoteStealthConnection> connection) {
	try	{
		boost::shared_ptr<RemoteStealthSession> stealthSession;
		boost::shared_ptr<TemporaryConfigFile> configFile;

		for (;;) {
			RSProtocolItem item;
			connection->syncRead(item);

			try	{				
				switch (item.procEvent) {
				case ProcessStart:
					std::cout << "process start: process ID = " << item.processID << ", base addr = 0x" << std::hex << item.baseAddress << std::endl;
					configFile = boost::make_shared<TemporaryConfigFile>(item.serializedConfigFile);
					stealthSession = boost::make_shared<RemoteStealthSession>(configFile->getFileName());
					stealthSession->handleProcessStart(item.processID, item.baseAddress);
					break;

				case ProcessAttach:
					configFile = boost::make_shared<TemporaryConfigFile>(item.serializedConfigFile);
					stealthSession = boost::make_shared<RemoteStealthSession>(configFile->getFileName());
					stealthSession->handleDbgAttach(item.processID);
					break;

				case ProcessExit:
					stealthSession->handleProcessExit();
					break;
				}
			}
			catch (std::exception& e) {
				std::cerr << "Error while handling protocol item: " << e.what() << std::endl;
				connection->syncWrite(RSProtocolResponse(false, e.what()));
				return;
			}
			connection->syncWrite(RSProtocolResponse(true, ""));
			if (item.procEvent == ProcessExit) return;
		}
	}
	catch (const std::exception& e)	{
		std::cerr << "Error while handling connection: " << e.what() << std::endl;
	}
}