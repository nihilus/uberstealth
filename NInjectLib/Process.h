#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

class Process {
public:
	Process(DWORD processID);
	Process::Process(const Process& instance);
	Process& Process::operator=(const Process& instance);
	~Process();
	
	LPVOID allocMem(DWORD size) const;
	LPVOID allocMem(DWORD size, DWORD allocationType) const;
	LPVOID allocMem(DWORD size, LPVOID desiredAddress, DWORD allocationType) const;
	bool freeMem(LPVOID address) const;
	void writeMemory(LPVOID address, LPCVOID data, DWORD size) const;
	void readMemory(LPVOID address, LPVOID buffer, DWORD size) const;
	MEMORY_BASIC_INFORMATION queryMemory(LPVOID address) const;
	DWORD protectMemory(LPVOID address, SIZE_T size, DWORD protect) const;
	bool startThread(LPVOID address, LPVOID param);
	void waitForThread();
	std::vector<MODULEENTRY32> Process::getModules() const;
	uintptr_t getImageBase(HANDLE hThread) const;
	uintptr_t getImageBase() const;

private:
	bool duplicateHandle(HANDLE hSrc, HANDLE* hDest);
	void throwSysError(const char* msg, DWORD lastError) const;

	HANDLE _hProcess;
	HANDLE _hThread;
	DWORD _processID;
};

class ProcessHandleException : public std::runtime_error {
public:
	ProcessHandleException::ProcessHandleException(const std::string& msg) : std::runtime_error(msg) {};
};

class ProcessMemoryException : public std::runtime_error {
public:
	ProcessMemoryException::ProcessMemoryException(const std::string& msg, LPVOID address) : std::runtime_error(msg), _address(address) {};
	LPVOID getAddress() { return _address; };
private:
	LPVOID _address;
};

class MemoryAccessException : public std::runtime_error {
public:
	MemoryAccessException::MemoryAccessException(const std::string& msg) : std::runtime_error(msg) {};	
};

class MemoryAllocationException : public std::runtime_error {
public:
	MemoryAllocationException::MemoryAllocationException(const std::string& msg) : std::runtime_error(msg) {};	
};

class MemoryQueryException : public std::runtime_error {
public:
	MemoryQueryException::MemoryQueryException(const std::string& msg) : std::runtime_error(msg) {};	
};

class MemoryProtectException : public ProcessMemoryException {
public:
	MemoryProtectException::MemoryProtectException(const std::string& msg, LPVOID address) : ProcessMemoryException(msg, address) {};	
};