// The IDA specific functions required to implement uberstealth.

#pragma once

#include <iostream>
#include <Windows.h>

namespace uberstealth {

class IDAEngine
{
public:
	IDAEngine();
	bool setBreakpoint(uintptr_t address) const;
	bool removeBreakpoint(uintptr_t address) const;
	void setExceptionOption(unsigned int exceptionCode, bool ignore) const;
	bool continueProcess() const;
	void logString(const char* str, ...) const;

private:
	void showExceptionDialog(bool ignoreException) const;
	HWND hIDAWnd_;
	unsigned int idaMainThread_;
};

}