// RemoteStealthSession represents the stealth session in IDA when the remote debugger is used.
// It is used instead of LocalStealthSession and will contact the remote server to perform
// the corresponding actions.

#pragma once

#include "IDAEngine.h"
#include "StealthSession.h"
#include <uberstealthRemote/RemoteStealthClient.h>
#include <uberstealthRemote/RemoteStealthConnection.h>

namespace uberstealth {

class RemoteStealthSession : public StealthSession<uberstealth::IDAEngine>
{
public:
	RemoteStealthSession() : resolver_(boost::asio::ip::tcp::resolver(ioService_)) {}
	void handleDbgAttach(unsigned int processID, const std::string& configFile, const std::string profile);
	void handleProcessStart(unsigned int processID, uintptr_t baseAddress, const std::string& configFile, const std::string profile);
	void handleProcessExit();

	// TODO: implement.
	void handleBreakPoint(unsigned int /*threadID*/, uintptr_t /*address*/) {}
	void handleException(unsigned int /*exceptionCode*/) {}

private:
	typedef boost::shared_ptr<remotestealth::RemoteStealthClient> RemoteStealthClient_Ptr;

	std::string readConfigFile(const std::string& fileName) const;
	void sendRemoteCommand(const remotestealth::RSProtocolItem& item);
	void connectToServer();

	RemoteStealthClient_Ptr client_;
	boost::asio::io_service ioService_;
	boost::asio::ip::tcp::resolver resolver_;
};

}