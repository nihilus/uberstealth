// RemoteStealthSession represents the stealth session in IDA when the remote debugger is used.
// It is used instead of LocalStealthSession and will contact the remote server to perform
// the corresponding actions.

#pragma once

#include "IDAEngine.h"
#include "StealthSession.h"
#include <uberstealthRemote/RemoteStealthClient.h>
#include <uberstealthRemote/RemoteStealthConnection.h>

namespace uberstealth {

class RemoteStealthSession : public StealthSession<IDAEngine>
{
public:
	RemoteStealthSession(ProfileHelper* profileHelper) :
		StealthSession<IDAEngine>(profileHelper),
		_resolver(boost::asio::ip::tcp::resolver(_ioService)) {};
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

	RemoteStealthClient_Ptr _client;
	boost::asio::io_service _ioService;
	boost::asio::ip::tcp::resolver _resolver;
};

}