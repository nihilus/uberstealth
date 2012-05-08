#pragma once

namespace uberstealth {

class StealthSession {
public:
	virtual void handleDebuggerStart(unsigned int processId, uintptr_t baseAddress) =0;
	virtual void handleDebuggerAttach(unsigned int processId) =0;
	virtual void handleDebuggerExit() =0;
	virtual void handleBreakpoint(unsigned int threadId, uintptr_t address) =0;
	virtual void handleException(unsigned int exceptionCode) =0;
};

}