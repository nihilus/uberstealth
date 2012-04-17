#include "RemoteStealthSession.h"
#include <Windows.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include "RemoteStealthConnection.h"
#include "resource.h"
#include <uberstealth/ResourceItem.h>

namespace {

const std::string StealthDllFileName = "HideDebugger.dll";

}

// TODO(jan.newger@newgre.net): wrap profile path string into a "profile file" class which will cleanup its temp file when destructed.
uberstealth::RemoteStealthSession::RemoteStealthSession(const std::string& configFile) :
	StealthSession(configFile)
	{
	wchar_t buffer[MAX_PATH];
	if (!GetModuleFileName(NULL, buffer, MAX_PATH)) {
		throw std::runtime_error("Unable to determine location of executable for current process.");
	}
	boost::filesystem::path p = buffer;
	p.remove_leaf();
	p /= StealthDllFileName;
	stealthDll_ = p.string();
}

ResourceItem uberstealth::RemoteStealthSession::getRDTSCDriverResource() {
	return ResourceItem(GetModuleHandle(NULL), IDR_RDTSC, "DRV");
}

ResourceItem uberstealth::RemoteStealthSession::getStealthDriverResource() {
	return ResourceItem(GetModuleHandle(NULL), IDR_STEALTH, "DRV");
}

std::string uberstealth::RemoteStealthSession::getStealthDllPath() {
	return stealthDll_;
}

void uberstealth::RemoteStealthSession::handleBreakPoint(unsigned int threadID, uintptr_t address) {
	// TODO(jan.newger@newgre.net): implement.
}

void uberstealth::RemoteStealthSession::handleException(unsigned int exceptionCode) {
	// TODO(jan.newger@newgre.net): implement.
}