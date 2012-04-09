#include "GenericInjector.h"

GenericInjector::GenericInjector(const Process& process) :
	_process(process),
	_injectedData(NULL),
	_injectedCode(NULL) {}

GenericInjector::~GenericInjector() {
	freeMem();	
}

void GenericInjector::freeMem() {
	if (_injectedData != NULL) _process.freeMem(_injectedData);
	if (_injectedCode != NULL) _process.freeMem(_injectedCode);
	_injectedData = _injectedCode = NULL;
}

// Inject code + data into remote process and pass data as argument to the injected code.
void GenericInjector::doInjection(INJECT_DATAPAYLOAD dataPayload, INJECT_CODEPAYLOAD codePayload) {
	freeMem();
	// allocate memory in target process and write data struct
	_injectedData = _process.allocMem(dataPayload.size);
	_process.writeMemory(_injectedData, dataPayload.data, dataPayload.size);

	// inject code payload
	_injectedCode = _process.allocMem(codePayload.size);
	_process.writeMemory(_injectedCode, codePayload.code, codePayload.size);
	_process.startThread(_injectedCode, _injectedData);
	// wait for thread to finish
	_process.waitForThread();
}

void* GenericInjector::getAddrOfData() const {
	return _injectedData;
}

void* GenericInjector::getAddrOfCode() const {
	return _injectedCode;
}