#pragma once

// Class to inject a dll into a remote process which is already running.

#include <Windows.h>
#include "GenericInjector.h"

class InjectLibrary {
public:
	InjectLibrary(const std::string& fileName, const Process& process);
	InjectLibrary(HMODULE hRemoteLib, const Process& process);
	HMODULE getDllHandle() const;
	const Process& getProcess() const;
	bool injectLib();
	bool unloadLib();

private:
	INJECT_DATAPAYLOAD createLoadLibData();
	INJECT_CODEPAYLOAD createLoadLibCode();
	INJECT_DATAPAYLOAD createUnloadLibData();
	INJECT_CODEPAYLOAD createUnloadLibCode();

	std::string _fileName;
	Process _process;
	HMODULE _hDll;
};