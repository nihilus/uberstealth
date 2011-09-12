#include "IPCDebugEventHelper.h"
#include <sstream>
#include <Windows.h>

namespace hidedebugger
{
	const char* SegmentName = "AntiDebugEventIPC";
	const size_t SegmentSize = sizeof(int) + sizeof(uintptr_t);

	std::string getSegmentName(unsigned int processId)
	{
		std::ostringstream oss;
		oss << SegmentName << processId;
		return oss.str();
	}
}

hidedebugger::managed_shared_memory_ptr hidedebugger::createAntiDebugEventIPC()
{
	using namespace boost::interprocess;
	return managed_shared_memory_ptr(new managed_shared_memory(create_only, getSegmentName(GetCurrentProcessId()).c_str(), SegmentSize));
}

hidedebugger::managed_shared_memory_ptr hidedebugger::openAntiDebugEventIPC(unsigned int processId)
{
	using namespace boost::interprocess;
	return managed_shared_memory_ptr(new managed_shared_memory(open_only, getSegmentName(processId).c_str()));	
}