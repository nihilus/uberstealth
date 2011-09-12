#include "IPCDebugEventWriter.h"

hidedebugger::IPCDebugEventWriter::IPCDebugEventWriter()
{
	sharedMem_ = createAntiDebugEventIPC();
}

bool hidedebugger::IPCDebugEventWriter::issueDebugEvent(AntiDebuggingEvent event, uintptr_t returnAddress)
{
	event;
	returnAddress;
	return false;		
}

hidedebugger::IPCDebugEventWriter::~IPCDebugEventWriter()
{
	// remove managed_shared_memory
}