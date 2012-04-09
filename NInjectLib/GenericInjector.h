#pragma once

// Abstract base class for injection of code and data into a remote process.
// Derived classes need to implement methods that generate specific code and data payloads.

#include <iostream>
#include "Process.h"

struct INJECT_DATAPAYLOAD {
	void* data;
	size_t size;
};

struct INJECT_CODEPAYLOAD {
	void* code;
	size_t size;
};

class GenericInjector {
public:
	GenericInjector(const Process& process);
	virtual ~GenericInjector();

	void doInjection(INJECT_DATAPAYLOAD dataPayload, INJECT_CODEPAYLOAD codePayload);
	void* getAddrOfData() const;
	void* getAddrOfCode() const;

private:
	void freeMem();
	Process _process;
	void* _injectedData;
	void* _injectedCode;
};
