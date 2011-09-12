#ifdef IDASTEALTH

#include "HideDebugger/HideDebuggerProfile.h"
#include "IDACommon.h"
#include "uberstealthRemote/RemoteStealthProtocol.h"
#include "RemoteStealthSession.h"

void uberstealth::RemoteStealthSession::handleDbgAttach(unsigned int processID, const std::string& configFile, const std::string profile)
{
	connectToServer();
	sendRemoteCommand(remotestealth::RSProtocolItem(processID, 0, remotestealth::ProcessAttach, readConfigFile(configFile), profile));
}

void uberstealth::RemoteStealthSession::sendRemoteCommand(const remotestealth::RSProtocolItem& item)
{
	using namespace boost::asio;

	try
	{
		client_->sendData(item);
	}
	catch (const std::exception& e)
	{
		engine_.logString("Error while sending remote command: %s\n", e.what());
	}
}

void uberstealth::RemoteStealthSession::connectToServer()
{
	using namespace boost::asio;

	try
	{
		// get host for remote debugging
		qstring host;
		get_process_options(NULL, NULL, NULL, &host, NULL, NULL);

		std::ostringstream oss;
		oss << currentProfile_.getRemoteTCPPortValue();

		ip::tcp::resolver::query query(host.c_str(), oss.str());
		ip::tcp::resolver::iterator iterator = resolver_.resolve(query);

		client_ = RemoteStealthClient_Ptr(new remotestealth::RemoteStealthClient(ioService_, iterator));
		client_->connect();
	}
	catch (const std::exception& e)
	{
		engine_.logString("Error while connecting: %s\n", e.what());
	}
}

std::string uberstealth::RemoteStealthSession::readConfigFile(const std::string& fileName) const
{
	std::ifstream ifs(fileName.c_str());
	std::ostringstream oss;
	oss << ifs.rdbuf();
	return oss.str();
}

#endif
