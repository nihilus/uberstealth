// RemoteStealthSession represents the stealth session in IDA when the remote debugger is used.
// It is used instead of LocalStealthSession and will contact the remote server to perform
// the corresponding actions.

#pragma once

#include "IDAEngine.h"
#include "StealthSession.h"
#include <uberstealthRemote/RemoteStealthClient.h>
#include <uberstealthRemote/RemoteStealthConnection.h>

namespace uberstealth {

class RemoteStealthSession : public StealthSession<IDALogger>
{
public:
	RemoteStealthSession(const std::string& profilePath) :
		StealthSession<IDALogger>(profilePath),
		resolver_(boost::asio::ip::tcp::resolver(ioService_)) {}
	void handleDbgAttach(unsigned int processID, const std::string& configFile, const std::string profile);
	void handleProcessStart(unsigned int processID, uintptr_t baseAddress, const std::string& configFile, const std::string profile);
	void handleProcessExit();

	// The remote stealth server doesn't have access to the debugging engine of IDA so these methods are implemented on the client side.
	void handleBreakPoint(unsigned int /*threadID*/, uintptr_t /*address*/) {}
	void handleException(unsigned int /*exceptionCode*/) {}

private:
	std::string readConfigFile(const std::string& fileName) const;
	void sendRemoteCommand(const uberstealth::RSProtocolItem& item);
	void connectToServer();

	boost::shared_ptr<uberstealth::RemoteStealthClient> client_;
	boost::asio::io_service ioService_;
	boost::asio::ip::tcp::resolver resolver_;
};

}