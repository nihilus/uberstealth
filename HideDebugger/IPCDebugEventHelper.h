#pragma once

/**
* Helper functions to construct ipc objects for both, reader and writer components of the debug event mechanism.
**/

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/shared_ptr.hpp>

namespace hidedebugger
{
	typedef boost::shared_ptr<boost::interprocess::managed_shared_memory> managed_shared_memory_ptr;

	managed_shared_memory_ptr createAntiDebugEventIPC();

	managed_shared_memory_ptr openAntiDebugEventIPC(unsigned int processId);
}