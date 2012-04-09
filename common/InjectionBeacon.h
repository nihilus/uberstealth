// The injection beacon is used to test whether the HideDebugger.dll should initialize all the stealth measures.
// If a process loads the dll manually via LoadLibrary (e.g. OllyDbg), the dll cannot access the shared memory
// which is used to communicate between debugger and debuggee. Thus, all functionality must be disabled.

#pragma once

#include <Windows.h>
#include <iostream>
#include <boost/noncopyable.hpp>

namespace uberstealth {

class InjectionBeacon : public boost::noncopyable {
public:
	InjectionBeacon() :
		hMutex_(NULL) {}
	InjectionBeacon(uintptr_t processId);
	~InjectionBeacon();
	bool queryBeacon();

private:
	HANDLE hMutex_;
};

}