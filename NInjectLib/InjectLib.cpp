#include "InjectLib.h"

#pragma pack(push, 1)

struct LOADLIB_DATA {
	void* loadLibrary;
	char fileName[MAX_PATH];
	// The dll handle will be filled by the code in the remote process so we can read it back.
	HMODULE hDll;
};

struct UNLOADLIB_DATA {
	void* freeLibrary;
	HMODULE hDll;
	int errorFlag;
};

#pragma pack(pop)

InjectLibrary::InjectLibrary(const std::string& fileName, const Process& process) 
	: _fileName(fileName),
	_process(process) {
	if (fileName.length() >= MAX_PATH) throw std::runtime_error("Invalid library - filename too long!");
}

InjectLibrary::InjectLibrary(HMODULE hRemoteLib, const Process& process)
	: _process(process),
	_hDll(hRemoteLib) {
}

INJECT_DATAPAYLOAD InjectLibrary::createLoadLibData() {
	LOADLIB_DATA* tmpData = (LOADLIB_DATA*)malloc(sizeof(LOADLIB_DATA));
	strcpy_s(tmpData->fileName, MAX_PATH, _fileName.c_str());
	// We always use ANSI version of APIs.
#ifdef UNICODE
	tmpData->loadLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
#else
	tmpData->loadLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
#endif
	
	INJECT_DATAPAYLOAD dataPayload;
	dataPayload.data = tmpData;
	dataPayload.size = sizeof(LOADLIB_DATA);
	return dataPayload;
}

#pragma warning(disable : 4731 4740) 

INJECT_CODEPAYLOAD InjectLibrary::createLoadLibCode() {
	size_t s, e;
	void* source;

	__asm
	{
		mov s, offset _start
		mov e, offset _end
		mov source, offset _start
		jmp _end // we only want to copy this code - not execute it so jump over it
	_start:
		// create standard stack frame
		push ebp
		mov ebp, esp
		push esi
		push edi

		mov esi, [ebp+8] // get first param, ESI now points to strcut
		mov edi, [esi] // get loadlibrary pointer
		lea ecx, [esi+4] // get addres of dll filename
		push ecx
		call edi // call loadlibrary
		lea ecx, [esi+MAX_PATH+4] // let EBX point to the handle entry in our struct
		mov [ecx], eax // save handle

		xor eax, eax
		pop edi
		pop esi
		pop ebp
		ret
	_end:
	}

	INJECT_CODEPAYLOAD tmpPayload;
	tmpPayload.size = e - s;
	tmpPayload.code = malloc(tmpPayload.size);
	// Copying code from code section should always work.
	memcpy(tmpPayload.code, source, tmpPayload.size);
	return tmpPayload;
}

#pragma warning(default : 4731 4740)

// Inject code into remote process to load the library.
bool InjectLibrary::injectLib() {
	GenericInjector injector(_process);
	INJECT_DATAPAYLOAD data = createLoadLibData();
	INJECT_CODEPAYLOAD code = createLoadLibCode();
	injector.doInjection(data, code);
	free(data.data);
	free(code.code);

	// Read back dll handle.
	char* readAddress = (char*)injector.getAddrOfData();
	readAddress += MAX_PATH + sizeof(HANDLE);
	HMODULE hRemoteDll;
	_process.readMemory(readAddress, (void*)&hRemoteDll, sizeof(HMODULE));
	_hDll = hRemoteDll;

	return (_hDll == 0 ? false : true);
}

// Unload library from remote process.
bool InjectLibrary::unloadLib() {
	GenericInjector injector(_process);
	INJECT_CODEPAYLOAD code = createUnloadLibCode();
	INJECT_DATAPAYLOAD data = createUnloadLibData();
	bool errorFlag = false;
	injector.doInjection(data, code);

	// Read back return value from freelibrary call.
	char* readAddress = (char*)injector.getAddrOfData();
	readAddress += sizeof(void*) + sizeof(HMODULE);
	int error;
	_process.readMemory(readAddress, &error, sizeof(int));
	errorFlag = (error != 0);
	free(code.code);
	free(data.data);
	return errorFlag;
}

// Create data struct with necessary information to unload lib from remote process.
INJECT_DATAPAYLOAD InjectLibrary::createUnloadLibData() {
	UNLOADLIB_DATA* tmpData = (UNLOADLIB_DATA*)malloc(sizeof(UNLOADLIB_DATA));
#ifdef UNICODE
	tmpData->freeLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
#else
	tmpData->freeLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "FreeLibrary");
#endif
	tmpData->hDll = _hDll;

	INJECT_DATAPAYLOAD dataPayload;
	dataPayload.data = tmpData;
	dataPayload.size = sizeof(UNLOADLIB_DATA);
	return dataPayload;
}

#pragma warning(disable : 4731 4740) 

// Create code to unload the lib from the remote process.
INJECT_CODEPAYLOAD InjectLibrary::createUnloadLibCode() {
	size_t s, e;
	void* source;

	__asm
	{
		mov s, offset _start
		mov e, offset _end
		mov source, offset _start
		jmp _end // We only want to copy this code - not execute it so jump over it.
	_start:
		// Create standard stack frame.
		push ebp
		mov ebp, esp
		push esi

		mov esi, [ebp+8] // Get first param, ESI now points to struct.
		mov eax, [esi] // Get freelibrary pointer.
		mov ecx, [esi+4] // Get module handle.
		push ecx
		call eax // Call freelibrary.
		lea ecx, [esi+8] // Let EBX point to the errorflag field.
		mov [ecx], eax

		xor eax, eax
		pop esi
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

const Process& InjectLibrary::getProcess() const {
	return _process;
}

HMODULE InjectLibrary::getDllHandle() const {
	return _hDll;
}