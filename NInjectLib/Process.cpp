#include "Process.h"
#include <iostream>
#include <sstream>
#include <vector>

Process::Process(DWORD processID) :
	_hThread(INVALID_HANDLE_VALUE),
	_hProcess(INVALID_HANDLE_VALUE),
	_processID(processID) {
	_hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processID);
	if (_hProcess == NULL)
	{
		DWORD lastErr = GetLastError();
		std::stringstream ss;
		ss << "Failed to get appropriate process access rights for process id: " << processID
		   << ", system error code: " << lastErr;
		throw ProcessHandleException(ss.str());
	}
}

Process::Process(const Process& instance) {
	this->_hProcess = this->_hThread = INVALID_HANDLE_VALUE;
	if (!duplicateHandle(instance._hProcess, &this->_hProcess)
		|| !duplicateHandle(instance._hThread, &this->_hThread)) throw ProcessHandleException("Failed to duplicate handle!");
	this->_processID = instance._processID;
}

Process& Process::operator=(const Process& instance) {
	if (!duplicateHandle(instance._hProcess, &this->_hProcess)
		|| !duplicateHandle(instance._hThread, &this->_hThread)) throw ProcessHandleException("Failed to duplicate handle!");
	this->_processID = instance._processID;
	return *this;
}

Process::~Process() {
	if (_hProcess != INVALID_HANDLE_VALUE) CloseHandle(_hProcess);
	if (_hThread != INVALID_HANDLE_VALUE) CloseHandle(_hThread);
}

bool Process::duplicateHandle(HANDLE hSrc, HANDLE* hDest) {
	if (hSrc == INVALID_HANDLE_VALUE) return true;
	return (DuplicateHandle(GetCurrentProcess(), 
						   hSrc, 
						   GetCurrentProcess(), 
						   hDest,
						   0,
						   FALSE,
						   DUPLICATE_SAME_ACCESS) == TRUE ? true : false);
}

void Process::writeMemory(LPVOID address, LPCVOID data, DWORD size) const {
	SIZE_T written = 0;
	WriteProcessMemory(_hProcess, address, data, size, &written);
	if (written != size) throw MemoryAccessException("Write memory failed!");
}

void Process::readMemory(LPVOID address, LPVOID buffer, DWORD size) const {
	SIZE_T read = 0;
	ReadProcessMemory(_hProcess, address, buffer, size, &read);
	if (read != size) throw MemoryAccessException("Read memory failed!");
}

MEMORY_BASIC_INFORMATION Process::queryMemory(LPVOID address) const {
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T retVal = VirtualQueryEx(_hProcess, address, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
	if (retVal == 0) throw MemoryQueryException("Unable to query memory");
	return mbi;
}

DWORD Process::protectMemory(LPVOID address, SIZE_T size, DWORD protect) const {
	DWORD oldProtect;
	BOOL retVal = VirtualProtectEx(_hProcess, address, size, protect, &oldProtect);
	if (retVal == FALSE) throw MemoryProtectException("Unable to set memory protection", address);
	return oldProtect;
}

bool Process::startThread(LPVOID address, LPVOID param) {
	_hThread = CreateRemoteThread(_hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, param, 0, NULL);
	if (_hThread != INVALID_HANDLE_VALUE) SetThreadPriority(_hThread, THREAD_PRIORITY_TIME_CRITICAL);
	return (_hThread != NULL);
}

// wait for remote thread to exit and close its handle
void Process::waitForThread() {
	if (_hThread == NULL) throw std::runtime_error("Invalid thread handle");
	WaitForSingleObject(_hThread, INFINITE);
	CloseHandle(_hThread);
	_hThread = NULL;
}

LPVOID Process::allocMem(DWORD size) const {
	return allocMem(size, MEM_RESERVE | MEM_COMMIT);	
}

LPVOID Process::allocMem(DWORD size, DWORD allocationType) const {
	return allocMem(size, NULL, allocationType);
}

LPVOID Process::allocMem(DWORD size, LPVOID desiredAddress, DWORD allocationType) const {
	LPVOID addr = VirtualAllocEx(_hProcess, desiredAddress, size, allocationType, PAGE_EXECUTE_READWRITE);
	if (addr == NULL) throw MemoryAllocationException("Failed to allocate memory");
	return addr;
}

bool Process::freeMem(LPVOID address) const {
	return (VirtualFreeEx(_hProcess, address, 0, MEM_RELEASE) != 0);
}

// Note: this method does not work in process start event.
std::vector<MODULEENTRY32> Process::getModules() const {
	std::vector<MODULEENTRY32> result;
	HANDLE hModulesSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _processID);
	if (hModulesSnap == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		std::ostringstream oss;
		oss << "Unable to create modules snapshot, last error: " << err << std::endl;
		throw std::runtime_error(oss.str());
	}

	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	if(!Module32First(hModulesSnap, &me32)) {
		DWORD err = GetLastError();
		std::ostringstream oss;
		oss << "Unable to enumerate modules from snapshot, last error: " << err << std::endl;
		CloseHandle(hModulesSnap);
		throw std::runtime_error(oss.str());
	}

	bool found = false;
	do {
		result.push_back(me32);
	} while(!found && Module32Next(hModulesSnap, &me32));

	CloseHandle(hModulesSnap);
	return result;
}

void Process::throwSysError(const char* msg, DWORD lastError) const {
	std::ostringstream oss;
	oss << msg << ", system error was: " << lastError;
	throw std::runtime_error(oss.str());
}

// This method also works if the process is suspended and not fully initialized yet.
uintptr_t Process::getImageBase(HANDLE hThread) const {
	CONTEXT context;
	context.ContextFlags = CONTEXT_SEGMENTS;
	if (!GetThreadContext(hThread, &context)) {
		throwSysError("Error while retrieving thread context to determine IBA",  GetLastError());
	}

	LDT_ENTRY ldtEntry;
	if (!GetThreadSelectorEntry(hThread, context.SegFs, &ldtEntry))	{
		throwSysError("Error while translating FS selector to virtual address",  GetLastError());
	}

	uintptr_t fsVA = (ldtEntry.HighWord.Bytes.BaseHi) << 24
		| (ldtEntry.HighWord.Bytes.BaseMid) << 16 | (ldtEntry.BaseLow);

	uintptr_t iba = 0;
	SIZE_T read;
	// Finally read image based address from PEB:[8].
	if (!(ReadProcessMemory(_hProcess, (LPCVOID)(fsVA+0x30), &iba, sizeof(uintptr_t), &read)
		&& ReadProcessMemory(_hProcess, (LPCVOID)(iba+8), &iba, sizeof(uintptr_t), &read)))	{
		throwSysError("Error while reading process memory to retrieve image base address", GetLastError());
	}
	return iba;
}

// Retrieve image base address for current process.
// This only works if the process has bee initialized, otherwise thread enumeration will fail.
// Use overloaded function instead.
uintptr_t Process::getImageBase() const {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, _processID);
	if (hSnapshot == INVALID_HANDLE_VALUE) throw std::runtime_error("Unable to create thread snapshot");

	DWORD lastError = 0;
	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);
	if (!Thread32First(hSnapshot, &threadEntry)) {
		lastError = GetLastError();
		CloseHandle(hSnapshot);
		throwSysError("Unable to get first thread from snapshot", lastError);
	}

	CloseHandle(hSnapshot);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
	if (!hThread) {
		lastError = GetLastError();
		throwSysError("Error while retrieving thread handle", lastError);
	}

	try	{
		uintptr_t iba = getImageBase(hThread);
		CloseHandle(hThread);
		return iba;
	}
	catch (const std::exception&) {
		CloseHandle(hThread);
		throw;
	}
}