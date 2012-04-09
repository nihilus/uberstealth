#include "InjectionBeacon.h"
#include <string>
#include <sstream>
#include "StringHelper.h"

namespace {

std::string generateMutexName(unsigned int processId) {
	std::ostringstream oss;
	oss << "uberstealth" << processId;
	return oss.str();
}

std::string generateMutexName() {
	return generateMutexName(GetCurrentProcessId());
}

}

uberstealth::InjectionBeacon::~InjectionBeacon() {
	if (hMutex_) CloseHandle(hMutex_);
}

bool uberstealth::InjectionBeacon::queryBeacon() {
	HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, uberstealth::StringToUnicode(generateMutexName()));
	if (hMutex) {
		CloseHandle(hMutex);
		return true;
	}
	else return false;
}

uberstealth::InjectionBeacon::InjectionBeacon(uintptr_t processId) {
	hMutex_ = CreateMutex(NULL, TRUE, StringToUnicode(generateMutexName(processId)));
	if (!hMutex_) throw std::runtime_error(std::string("Unable to create injection beacon mutex: ") + generateMutexName(processId));
}