// The OllyDbg specific functions required to implement uberstealth.

#pragma once

#include <iostream>

namespace uberstealth {

class OllyEngine
{
public:
	bool setBreakpoint(uintptr_t address) const;
	bool removeBreakpoint(uintptr_t address) const;
	void setExceptionOption(unsigned int exceptionCode, bool ignore) const;
	bool continueProcess() const;
	void logString(const char* str, ...) const;
};

}