#include "IDAStealthRemote.h"
#include <boost/filesystem.hpp>
#include "../uberstealth/version.h"

int _tmain(int argc, char* argv[]) {
	try {		
		std::cout << REMOTESTEALTH_INFO_STRING << std::endl << std::endl;

		if (!boost::filesystem::exists("HideDebugger.dll")) {
			std::cout << "HideDebugger.dll not found...terminating" << std::endl;
			return 1;
		}

		unsigned short port = 4242;
		if (argc == 2) port = boost::lexical_cast<unsigned short>(argv[1]);
		else std::cout << "Usage: IDAStealthRemote.exe <port>" << std::endl
				  << "Using standard port " << port << std::endl;
		std::cout << "Starting server..." << std::endl;

		boost::asio::io_service ioService;
		uberstealth::RemoteStealthServer server(ioService, port);
		server.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}