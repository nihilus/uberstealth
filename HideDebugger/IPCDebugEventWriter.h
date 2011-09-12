#pragma once

#include <iostream>
#include "IPCDebugEventHelper.h"

enum AntiDebuggingEvent { PreSEHException };

namespace hidedebugger
{
	/*
	* Defines the single-instance IPC object which is used to inform the IDAStealth plugin
	* about the invocation of an anti-debugging technique in a specific thread.
	**/
	class IPCDebugEventWriter
	{
	public:

		static IPCDebugEventWriter& getInstance()
		{
			static IPCDebugEventWriter instance;
			return instance;
		}
		
		~IPCDebugEventWriter();
		bool issueDebugEvent(AntiDebuggingEvent event, uintptr_t returnAddress);

	private:
	
		IPCDebugEventWriter();
		managed_shared_memory_ptr sharedMem_;
	};
}