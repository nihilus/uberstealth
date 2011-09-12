// Defines the protocol which is used by the plugin and the remote server to communicate.

#pragma once

#pragma warning(disable : 4512)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(default : 4512)
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>

namespace remotestealth
{
	enum ProcessEvent { ProcessStart, ProcessAttach, ProcessExit };

	struct RSProtocolItem
	{
		RSProtocolItem(unsigned int pID, uintptr_t base, ProcessEvent pEvent,
			const std::string& configStr, const std::string currentProfile) :
			processID(pID),
			baseAddress(base),
			procEvent(pEvent),
			serializedConfigFile(configStr),
			profile(currentProfile) {}

		RSProtocolItem() :
			processID(0),
			baseAddress(0),
			procEvent(ProcessStart),
			remoteEventPort(0) {}

		unsigned int processID;
		uintptr_t baseAddress;
		ProcessEvent procEvent;
		std::string serializedConfigFile;
		std::string profile;
		std::string remoteEventIP;
		int remoteEventPort;

		template <typename Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & processID;
			ar & baseAddress;
			ar & procEvent;
			ar & serializedConfigFile;
			ar & profile;
		}
	};

	// Response sent from the remote side to indicate success / error of an operation
	struct RSProtocolResponse 
	{
		RSProtocolResponse(bool succ, const std::string& err) :
			success(succ),
			error(err) {}
		RSProtocolResponse() : success(false) {}

		template <typename Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & success;
			ar & error;			
		}

		bool success;
		std::string error;
	};
}