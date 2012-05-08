// LocalStealthSession has all the routines which are required to control the debugger
// and communicate with the injected dll when a standard debugger (local as opposed to remote)
// is used. The class receives a parameter which represents the actual debugger being used.

#pragma once

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <iostream>
#include <HideDebugger/ntdll.h>
#include <NCodeHook/NCodeHookInstantiation.h>
#include "ResourceItem.h"
#include <set>
#include "CommonStealthSession.h"

namespace uberstealth {

template <typename EngineT, typename LoggerT>
class LocalStealthSession :
	public CommonStealthSession<LoggerT>,
	public boost::noncopyable {
public:
	LocalStealthSession(const boost::filesystem::path& profilePath) :
		CommonStealthSession<LoggerT>(profilePath),
		rtlDispatchExceptionAddr_(0),
		ntContinueCallAddr_(0) {}
	
	virtual void handleDebuggerStart(unsigned int processID, uintptr_t baseAddress) {
		CommonStealthSession::handleDebuggerStart(processID, baseAddress);
		if (currentProfile_.getEnableDbgStartEnabled()) {
			initSEHMonitoring();
			localStealth();
		}
	}

	virtual void handleDebuggerAttach(unsigned int processId) {
		CommonStealthSession::handleDebuggerAttach(processId);
		if (currentProfile_.getEnableDbgAttachEnabled()) {
			initSEHMonitoring();
			localStealth();
		}
	}

	virtual void handleDebuggerExit() {
		cleanupSEHMonitoring();
		CommonStealthSession::handleDebuggerExit();
	}

	// SEH logging and halting of the debuggee is implemented via breakpoints so we need to handle this event accordingly.
	virtual void handleBreakpoint(unsigned int threadID, uintptr_t address)	{
		std::set<BPHit>::const_iterator sehCit = sehHandlerBps_.find(BPHit(threadID, address));
		std::set<BPHit>::const_iterator postSEHCit = postSEHBps_.find(BPHit(threadID, address));
		if (address == getRtlDispatchExceptionAddr() &&
			(currentProfile_.getHaltInSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled())) {
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadID);
			if (hThread == INVALID_HANDLE_VALUE) {
				throw std::runtime_error("Unable to open thread while trying to determine top level SEH handler.");
			}

			CONTEXT context;
			context.ContextFlags = CONTEXT_SEGMENTS;
			if (!GetThreadContext(hThread, &context)) {
				CloseHandle(hThread);
				throw std::runtime_error("Unable to retrieve context of thread while trying to determine top level SEH handler.");
			}

			LDT_ENTRY ldtEntry;
			if (!GetThreadSelectorEntry(hThread, context.SegFs, &ldtEntry))	{
				CloseHandle(hThread);
				throw std::runtime_error("Unable to translate thread segment selector while trying to determine top level SEH handler.");
			}
			CloseHandle(hThread);
			uintptr_t fsBase = (uintptr_t)(ldtEntry.HighWord.Bytes.BaseHi << 24 | ldtEntry.HighWord.Bytes.BaseMid << 16 | ldtEntry.BaseLow);
			uintptr_t sehChain = 0;
			uintptr_t sehHandler = 0;
			// Read pointer to top level SEH record.
			if (ReadProcessMemory(getProcessHandle(), (LPCVOID)fsBase, &sehChain, sizeof(sehChain), NULL) &&
				ReadProcessMemory(getProcessHandle(), (LPCVOID)(sehChain + sizeof(uintptr_t)), &sehHandler, sizeof(sehHandler), NULL)) {
				if (!engine_.setBreakpoint(sehHandler)) {
					throw std::runtime_error("Error while setting breakpoint at top-level SEH handler in RtlDispatchException.");
				}
				sehHandlerBps_.insert(BPHit(threadID, sehHandler));
			} else {
				throw std::runtime_error("Error while reading memory to determine top-level SEH handler in RtlDispatchException.");
			}
			engine_.continueProcess();
		}
		else if (address == getNtContinueCallAddr() &&
				(currentProfile_.getHaltAfterSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled())) {
			// The first parameter on the stack is a pointer to the CONTEXT structure.
			HANDLE hThread = OpenThread(THREAD_GET_CONTEXT, FALSE, threadID);
			if (hThread == INVALID_HANDLE_VALUE) {
				throw std::runtime_error("Unable to open thread while trying to determine modified instruction pointer after SEH.");
			}

			CONTEXT context;
			context.ContextFlags = CONTEXT_ALL;
			if (!GetThreadContext(hThread, &context)) {
				CloseHandle(hThread);
				throw std::runtime_error("Unable to retrieve context of thread while trying to determine modified instruction pointer after SEH.");
			}
			CloseHandle(hThread);

			uintptr_t contextAddr;
			CONTEXT sehContext;
			if (ReadProcessMemory(getProcessHandle(), (LPCVOID)context.Esp, &contextAddr, sizeof(contextAddr), NULL) &&
				ReadProcessMemory(getProcessHandle(), (LPCVOID)contextAddr, &sehContext, sizeof(sehContext), NULL)) {
				if (!engine_.setBreakpoint(sehContext.Eip)) {
					throw std::runtime_error("Error while setting breakpoint at modified instruction pointer after SEH.");
				}
				postSEHBps_.insert(BPHit(threadID, sehContext.Eip));
			} else {
				throw std::runtime_error("Unable to get stack value while trying to determine modified instruction pointer after SEH.");
			}
			engine_.continueProcess();
		} else if (sehCit != sehHandlerBps_.end()) {
			engine_.removeBreakpoint(address);
			sehHandlerBps_.erase(sehCit);
			if (currentProfile_.getLogSEHEnabled()) {
				logger_.logString("uberstealth: debugger reached top-level SEH handler at 0x%X\n", address);
				if (!currentProfile_.getHaltInSEHHandlerEnabled()) {
					engine_.continueProcess();
				}
			}
			if (currentProfile_.getHaltInSEHHandlerEnabled()) {
				logger_.logString("uberstealth: debugger has been halted in top-level SEH handler\n");
			}
		} else if (postSEHCit != postSEHBps_.end())	{		
			engine_.removeBreakpoint(address);
			postSEHBps_.erase(postSEHCit);
			if (currentProfile_.getLogSEHEnabled()) {
				logger_.logString("uberstealth: debugger reached new location after the SEH handler (possibly) modified EIP at 0x%X.\n", address);
				if (!currentProfile_.getHaltAfterSEHHandlerEnabled()) {
					engine_.continueProcess();
				}
			}
			if (currentProfile_.getHaltAfterSEHHandlerEnabled()) {
				logger_.logString("uberstealth: debugger has been halted at instruction pointer after it was (possibly) modified by SEH handler.\n");
			}
		}
	}

	// Handle an exception which occurred in the debuggee.
	virtual void handleException(unsigned int exceptionCode) {
		engine_.setExceptionOption(exceptionCode, currentProfile_.getPassUnknownExceptionsEnabled());
	}

private:
	typedef boost::tuple<unsigned int, uintptr_t> BPHit;

	ResourceItem getRDTSCDriverResource() { return ResourceItem(getModuleHandle(), IDR_RDTSC, "DRV"); }
	ResourceItem getStealthDriverResource() { return ResourceItem(getModuleHandle(), IDR_STEALTH, "DRV"); }

	// Nasty trick to get the module handle of our plugin.
	static void arbitraryAddress() {};
	HMODULE getModuleHandle() const	{
		HMODULE hModule;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&arbitraryAddress, &hModule);
		return hModule;
	}

	std::string getStealthDllPath() {
		boost::filesystem::path retVal;
		wchar_t idaExe[MAX_PATH];
		if (GetModuleFileName(NULL, idaExe, MAX_PATH)) {
			boost::filesystem::path p(idaExe);
			p.remove_leaf();
			p /= "plugins";
			p /= "HideDebugger.dll";
			return p.string();
		}
		throw std::runtime_error("Unable to retrieve path of HideDebugger.dll.");
	}

	uintptr_t getRtlDispatchExceptionAddr() const {
		if (rtlDispatchExceptionAddr_) {
			return rtlDispatchExceptionAddr_;
		}

		// We need to disassemble the beginning of KiUserExceptionDispatcher to get RtlDispatchException.
		_DecodedInst instructions[20];
		unsigned int instructionCount = 0;
		HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
		unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
		if (distorm_decode(0, codePtr, 20, Decode32Bits, instructions, 20, &instructionCount) == DECRES_SUCCESS) {
			for (int i=0; i<(int)instructionCount; ++i) {
				if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0) {
					uintptr_t callOffset = 0;
					sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
					rtlDispatchExceptionAddr_ = (uintptr_t)codePtr + callOffset;
					return rtlDispatchExceptionAddr_;
				}
			}
		}
		throw std::runtime_error("Unable to locate RtlDispatchException in KiUserExceptionDispatcher.");
	}

	// Returns the address of the call to NtContinue inside KiUserExceptionDispatcher.
	uintptr_t getNtContinueCallAddr() const	{
		if (ntContinueCallAddr_) {
			return ntContinueCallAddr_;
		}

		_DecodedInst instructions[25];
		unsigned int instructionCount = 0;
		HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
		unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
		uintptr_t ntContinueAddr = (uintptr_t)GetProcAddress(hNtDll, "NtContinue");
		_DecodeResult result = distorm_decode(0, codePtr, 40, Decode32Bits, instructions, 25, &instructionCount);
		if (result == DECRES_SUCCESS || result == DECRES_MEMORYERR) {
			for (int i=0; i<(int)instructionCount; ++i) {
				if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0) {
					uintptr_t callOffset = 0;
					sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
					uintptr_t callDestination = (uintptr_t)codePtr + callOffset;
					if (callDestination == ntContinueAddr) {
						ntContinueCallAddr_ = (uintptr_t)codePtr + (uintptr_t)instructions[i].offset;
						return ntContinueCallAddr_;
					}
				}
			}
		}
		throw std::runtime_error("Unable to locate call to NtContinue in KiUserExceptionDispatcher.");
	}

	// Init mechanism to stop at SEH handler / stop at EIP after SEH.
	void initSEHMonitoring() const {
		if (currentProfile_.getHaltInSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled()) {
			engine_.setBreakpoint(getRtlDispatchExceptionAddr());
		}
		if (currentProfile_.getHaltAfterSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled()) {
			engine_.setBreakpoint(getNtContinueCallAddr());
		}
	}

	void cleanupSEHMonitoring() const {
		if (currentProfile_.getHaltInSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled()) {
			engine_.removeBreakpoint(getRtlDispatchExceptionAddr());
		}
		if (currentProfile_.getHaltAfterSEHHandlerEnabled() || currentProfile_.getLogSEHEnabled()) {
			engine_.removeBreakpoint(getNtContinueCallAddr());
		}
	}

	// Hooks local function from ntdll in order to prevent special handling of DBG_PRINTEXCEPTION_C
	// by WaitForDebugEvent loop in the debugger.
	static NTSTATUS NTAPI DbgUiConvStateChngStructHook(PDBGUI_WAIT_STATE_CHANGE WaitStateChange, LPDEBUG_EVENT DebugEvent) {
		__try {
			if (WaitStateChange->StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_C) {
				DebugEvent->dwProcessId = (DWORD)WaitStateChange->AppClientId.UniqueProcess;
				DebugEvent->dwThreadId = (DWORD)WaitStateChange->AppClientId.UniqueThread;
				DebugEvent->dwDebugEventCode = DbgReplyPending;
				return 0;
			}
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			return 0;
		}

		return origDbgUiConvStateChngStruct(WaitStateChange, DebugEvent);
	}

	// Replaces the whole code section of ntdll.dll before attaching to the debuggee.
	static BOOL WINAPI DebugActiveProcessHook(DWORD dwProcessId) {
		#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue) )
		
		HANDLE hNtDll = GetModuleHandle(L"ntdll.dll");
		if (hNtDll != INVALID_HANDLE_VALUE)	{
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hNtDll;
			PIMAGE_NT_HEADERS pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDosHeader, pDosHeader->e_lfanew);
			LPVOID base = (LPVOID)(pNTHeader->OptionalHeader.BaseOfCode + pNTHeader->OptionalHeader.ImageBase);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (hProcess) {
				DWORD oldProtect;
				DWORD codeSize = pNTHeader->OptionalHeader.SizeOfCode;
				if (VirtualProtectEx(hProcess, base, codeSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
					WriteProcessMemory(hProcess, base, base, codeSize, NULL);
					VirtualProtectEx(hProcess, base, codeSize, oldProtect, NULL);
				}
				CloseHandle(hProcess);
			}
		}
		return origDebugActiveProcess(dwProcessId);
	}

	// Add stealth hooks to the *debugger process*.
	void localStealth()	{
		if (currentProfile_.getDbgPrintExceptionEnabled()) {
			origDbgUiConvStateChngStruct = nCodeHook_.createHookByName("ntdll.dll", "DbgUiConvertStateChangeStructure", DbgUiConvStateChngStructHook);
		}
		if (currentProfile_.getKillAntiAttachEnabled()) {
			origDebugActiveProcess = nCodeHook_.createHookByName("kernel32.dll", "DebugActiveProcess", DebugActiveProcessHook);
		}
	}

	static DbgUiConvertStateChangeStructureFPtr origDbgUiConvStateChngStruct;
	static DebugActiveProcessFPtr origDebugActiveProcess;
	
	mutable uintptr_t rtlDispatchExceptionAddr_;
	mutable uintptr_t ntContinueCallAddr_;
	std::set<BPHit> sehHandlerBps_;
	std::set<BPHit> postSEHBps_;
	NCodeHookIA32 nCodeHook_;
	EngineT engine_;
};

template <typename EngineT, typename LoggerT>
DbgUiConvertStateChangeStructureFPtr uberstealth::LocalStealthSession<EngineT, LoggerT>::origDbgUiConvStateChngStruct;

template <typename EngineT, typename LoggerT>
DebugActiveProcessFPtr uberstealth::LocalStealthSession<EngineT, LoggerT>::origDebugActiveProcess;
}