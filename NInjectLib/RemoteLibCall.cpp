#include "RemoteLibCall.h"
#include <tchar.h>

// TODO(jan.newger@newgre.net): this code uses malloc, free, is not exception safe and should be fixed!

namespace {

const int MaxFuncNameLength = 42;

}

#pragma pack(push, 1)

// This struct holds all data needed to perform a remote call.
struct REMOTELIB_DATA {
	void* getProcAddress;
	HMODULE hDll;
	int functionNr;
	int errorFlag;
	char functionName[MaxFuncNameLength];
};

#pragma pack(pop)

RemoteLibCall::RemoteLibCall(const InjectLibrary& library, const Process& process)
	: process_(process) {
	hDll_ = library.getDllHandle();
	if (hDll_ == NULL) {
		throw std::runtime_error("Invalid dll handle.");
	}
}

// Preform remote call by function name.
bool RemoteLibCall::remoteCall(const std::string& functionName) {
	if (functionName.length() >= MaxFuncNameLength) {
		throw std::runtime_error("Function name too long.");
	}
	functionName_ = functionName;
	functionNumber_ = -1;
	return remoteCall();
}

// Perform remote call by ordinal.
bool RemoteLibCall::remoteCall(unsigned int exportNumber) {
	functionNumber_ = exportNumber;
	functionName_ = "";
	return remoteCall();
}

// Create specific data/code payloads and use GenericInjector to start the remote code.
// Returns true iff the remote call succeeded.
bool RemoteLibCall::remoteCall() {
	GenericInjector injector(process_);
	INJECT_DATAPAYLOAD data = createDataPayload();
	INJECT_CODEPAYLOAD code = createCodePayload();
	bool noError = false;
	injector.doInjection(data, code);
	unsigned int* errorFlagAddr = (unsigned int*)injector.getAddrOfData();
	errorFlagAddr += 12;
	unsigned int errorFlag;
	process_.readMemory(errorFlagAddr, &errorFlag, sizeof(unsigned int));
	noError = (errorFlag != 0);

	free(data.data);
	free(code.code);
	return noError;
}

INJECT_DATAPAYLOAD RemoteLibCall::createDataPayload() {
	REMOTELIB_DATA* tmpData = (REMOTELIB_DATA*)malloc(sizeof(REMOTELIB_DATA));
	if (functionNumber_ == -1) {
		strcpy_s(tmpData->functionName, MaxFuncNameLength, functionName_.c_str());
		tmpData->functionNr = -1;
	} else {
		tmpData->functionNr = functionNumber_;
	}

	// The remote process needs the virtual address of GetProcAddress and our dll handle.
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	tmpData->getProcAddress = GetProcAddress(hKernel32, "GetProcAddress");
	// Save handle of the dll we want to call.
	tmpData->hDll = hDll_;

	INJECT_DATAPAYLOAD injectData;
	injectData.data = tmpData;
	injectData.size = sizeof(REMOTELIB_DATA);
	return injectData;
}

#pragma warning(disable : 4731 4740) 

// Create code to perform remote call.
INJECT_CODEPAYLOAD RemoteLibCall::createCodePayload() {
	size_t s, e;
	void* source;

	__asm {
		mov s, offset _start
		mov e, offset _end
		mov source, offset _start
		jmp _end			// We only want to copy this code - not execute it so jump over it.
	_start:
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		
		mov esi, [ebp+8]	// Get first param, ESI now points to struct.
		mov edi, [esi]		// Get GetProcaddress pointer.
		mov ebx, [esi+8]	// Get functionNr.
		mov ecx, [esi+4]	// Get module handle.
		cmp ebx, 0xFFFFFFFF
		jnz _byOrdinal
		lea ebx, [esi+16]	// Get pointer to function name.
		
	_byOrdinal:
		push ebx
		push ecx
		call edi			// Call GetProcaddress.
		test eax, eax		// Could we resolve the function?
		jz _error
		call eax			// Call exported function.
		mov eax, 1
		jmp _finished

	_error:
		xor eax, eax

	_finished:
		mov [esi+12], eax	// Save error flag.
		pop edi
		pop esi
		pop ebx
		pop ebp
		ret
	_end:
	}

	INJECT_CODEPAYLOAD tmpPayload;
	tmpPayload.size = e - s;
	tmpPayload.code = malloc(tmpPayload.size);
	memcpy(tmpPayload.code, source, tmpPayload.size);
	return tmpPayload;
}

#pragma warning(default: 4731 4740)