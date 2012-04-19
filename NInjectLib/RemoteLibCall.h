// Performs a procedure call to a dll which was previously injected into a remote process.

#pragma once

#include <Windows.h>
#include <iostream>
#include "InjectLib.h"
#include "GenericInjector.h"
#include "Process.h"

class RemoteLibCall {
public:
	RemoteLibCall(const InjectLibrary& library, const Process& process);
	bool remoteCall(const std::string& functionName);
	bool remoteCall(unsigned int exportNumber);

private:
	bool remoteCall();
	INJECT_DATAPAYLOAD createDataPayload();
	INJECT_CODEPAYLOAD createCodePayload();

	Process process_;
	HMODULE hDll_;
	std::string functionName_;
	int functionNumber_;
};
