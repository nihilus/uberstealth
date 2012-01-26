// HideDebugger.cpp : Defines the entry point for the DLL application.
//

#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>
#include "HideDebugger.h"
#include "HideDebuggerProfile.h"
#include "HookHelper.h"
#include <iostream>
#include "../common/InjectionBeacon.h"
#include "IPCConfigExchangeReader.h"
#include <map>
#include "ThreadDebugRegisterState.h"
#include <Tlhelp32.h>

void hideDebugger();
void applyConfigFromFile(const string& configFile);
void getOSVersion();

void handleProcessAttach();
void handleThreadDetach();
void handleProcessDetach();

void acquireDebugRegistersLock();
void releaseDebugRegistersLock();
boost::shared_ptr<ThreadDebugRegisterState> getDebugRegisterState(DWORD threadId);

namespace {

	bool xpOrLater_ = false;
	bool vistaOrLater_ = false;
	bool ntQuerySysInfo_ = false;
	bool fakeParentProcess_ = false;
	bool hideIDAProcess_ = false;
	unsigned int tickDelta_ = 23;

	unsigned int IDAProcessID = 0;

	// critical section for map access from KiUserExceptionDispatcher
	CRITICAL_SECTION section_;

	// map thread ids to the corresponding debug register state
	map<DWORD, boost::shared_ptr<ThreadDebugRegisterState>> threads;

}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		handleProcessAttach();
		break;

	case DLL_THREAD_DETACH:
		handleThreadDetach();
		break;

	case DLL_PROCESS_DETACH:
		handleProcessDetach();
		break;
	}
	return TRUE;
}

void handleProcessAttach()
{
	uberstealth::InjectionBeacon injectionBeacon;
	if (injectionBeacon.queryBeacon())
	{
		initSystemAPIs();
		restoreNTHeaders();
		InitializeCriticalSection(&section_);
		getOSVersion();
		hideDebugger();
	}
}

void handleThreadDetach()
{
	uberstealth::InjectionBeacon injectionBeacon;
	if (injectionBeacon.queryBeacon())
	{
		acquireDebugRegistersLock();
		threads.erase(GetCurrentThreadId());
		releaseDebugRegistersLock();
	}
}

void handleProcessDetach()
{
	uberstealth::InjectionBeacon injectionBeacon;
	if (injectionBeacon.queryBeacon())
	{
		DeleteCriticalSection(&section_);
	}
}

void hideDebugger()
{
	try
	{
		ipc::IPCConfigExchangeReader configExchange;
		std::string configFile = configExchange.getProfileFile();
		IDAProcessID = configExchange.getIDAProcessID();
		applyConfigFromFile(configFile);
	}
	catch (const std::exception& e)
	{
		dbgPrint("Exception while trying to initialize stealth techniques: %s", e.what());
	}
}

NTSTATUS NTAPI NtQueryInformationProcessHook(HANDLE ProcessHandle, PROCESS_INFORMATION_CLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength) 
{
	if (ProcessInformationClass == ProcessDebugPort || ProcessInformationClass == ProcessDebugFlags)
	{
		// if this process acts as a debugger, we will unveil ourselves if we hide the debug port
		// see also: http://forum.tuts4you.com/index.php?showtopic=16750
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

		DWORD procInfo;
		DWORD retLen;
		// try original API with the given handle, if it works we need to hide ourselves
		// otherwise original function will fail anyway
		if (!NT_SUCCESS(origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, &procInfo, ProcessInformationLength, &retLen)))
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

		// handle is valid, so employ stealth!
		if (ReturnLength != NULL && !IsBadWritePtr(ReturnLength, sizeof(UINT_PTR)))
			*ReturnLength = 4;
		// hide debug port - if we cannot write result we will fail, so call original function
		if (IsBadWritePtr(ProcessInformation, sizeof(UINT_PTR)))
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		else
		{
			if (ProcessInformationClass == ProcessDebugFlags) *((DWORD_PTR*)ProcessInformation) = 1;
			else *((DWORD_PTR*)ProcessInformation) = 0;
			return STATUS_SUCCESS;
		}
	}
	else if (ProcessInformationClass == ProcessDebugObjectHandle)
	{
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		if (!IsBadWritePtr(ReturnLength, 4))
			*ReturnLength = 4;
		return STATUS_PORT_NOT_SET;
	}
	else if (ProcessInformationClass == ProcessBasicInformation && fakeParentProcess_)
	{
		// first we need to check if ProcessHandle is a handle to the current process
		// otherwise we need to skip this hook
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

		NTSTATUS status = origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		if (NT_SUCCESS(status))
		{
			DWORD processID = 0;
			GetWindowThreadProcessId(GetShellWindow(), &processID);
			DWORD* parentPID = (DWORD*)ProcessInformation;
			// patch parent process id of current process
			parentPID[5] = processID;
		}
		return status;
	}
	else return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
}

// hide debug object
NTSTATUS NTAPI NtQueryObjectHook(HANDLE ObjectHandle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG Length, PULONG ResultLength)
{
	if (ObjectInformationClass == ObjectAllInformation && Length > 0)
	{
		NTSTATUS status = origNtQueryObject(ObjectHandle, ObjectInformationClass, ObjectInformation, Length, ResultLength);
		if (!NT_SUCCESS(status)) return status;

		unsigned int buffer = (unsigned int)ObjectInformation;
		ULONG count = *(ULONG*)buffer;
		PUNICODE_STRING pWStr = (PUNICODE_STRING)(buffer + sizeof(ULONG));
		unsigned int address = 0;

		// walk object structure and overwrite count of DebugObject with 0
		const WCHAR* dbgObj = L"DebugObject";
		const size_t objSize = sizeof(L"DebugObject") - 2;
		for (unsigned int i=0; i<count; ++i)
		{
			if (pWStr->Length == objSize)
			{
				// consider the case that object name is not zero-terminated
				if (memcmp(pWStr->Buffer, dbgObj, objSize) == 0)
				{
					// zero out handle and object count
					unsigned int* count = (unsigned int*)(&pWStr->Buffer) + 1;
					*count = 0; ++count;
					*count = 0;
				}
			}
			address = ((unsigned int)(pWStr->Buffer) + pWStr->Length) & -4;
			address += 4; // skip alignment bytes
			pWStr = (PUNICODE_STRING)address;
		}
		return status;
	}
	else return origNtQueryObject(ObjectHandle, ObjectInformationClass, ObjectInformation, Length, ResultLength);
}

// skip KiRaiseUserExceptionDispatcher for INVALID_HANDLE exceptions
void __declspec(naked) NTAPI KiRaiseUserExceptionDispatcherHook()
{
	__asm
	{
		pushf
			push eax
			// get current exception code from PEB
			mov eax, fs:[0x18]
		mov eax, [eax+0x1A4]
		cmp eax, 0xC0000008
			jnz call_orig
			pop eax
			popf
			ret

call_orig:
		pop eax
			popf
			jmp [origKiRaiseUED]
	}
}

// hide debugger by pretending there was no receiver for the debug string
// increase stealth by always returning 1
VOID WINAPI OutputDebugStringAHook(LPCSTR lpOutputString)
{
	origOutputDebugStringA(lpOutputString);
	SetLastError(ERROR_FILE_NOT_FOUND);
	__asm mov eax, 1
}

VOID WINAPI OutputDebugStringWHook(LPCWSTR lpOutputString)
{
	origOutputDebugStringW(lpOutputString);
	SetLastError(ERROR_FILE_NOT_FOUND);
	__asm mov eax, 1
}

// search in data returned from NtQuerySystemInformation for the corresponding
// chunk of the given process
PSYSTEM_PROCESS_INFORMATION findProcessChunk(PSYSTEM_PROCESS_INFORMATION pInfo, const wstring& processName)
{
	PSYSTEM_PROCESS_INFORMATION lastPInfo = pInfo;
	while (lastPInfo->NextEntryOffset)
	{
		lastPInfo = pInfo;
		LPCWSTR str = (LPCWSTR)pInfo->Reserved2[1];
		if (str)
			if (_wcsicmp(processName.c_str(), str) == 0) return pInfo;
		pInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)pInfo + pInfo->NextEntryOffset);
	}
	return NULL;
}

// one hook for multiple stealth techniques
NTSTATUS NTAPI NtQuerySystemInformationHook(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
	// just query length?
	if (SystemInformationLength == 0) 
		return origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	if (ntQuerySysInfo_ && SystemInformationClass == SystemKernelDebuggerInformation)
	{
		NTSTATUS status = origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
		if (!NT_SUCCESS(status)) return status;

		*(unsigned short*)SystemInformation = 0x0100;
		return STATUS_SUCCESS;
	}

	if (SystemInformationClass == SystemProcessInformation)
	{
		// get original process list		
		NTSTATUS status = origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
		if (!NT_SUCCESS(status)) return status;

		// parameters are ok and we have the process list, now apply stealth techniques
		if (hideIDAProcess_)
		{
			PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
			PSYSTEM_PROCESS_INFORMATION lastPInfo = pInfo;
			// we need to walk the whole list because there might be multiple instances
			while (pInfo->NextEntryOffset)
			{
				LPCWSTR processName = (LPCWSTR)pInfo->Reserved2[1];
				// increase size of last chunk so we just skip over the IDA chunk
				if (processName)
					if (_wcsicmp(processName, L"idag.exe") == 0 || _wcsicmp(processName, L"idaw.exe") == 0)
					{
						lastPInfo->NextEntryOffset += pInfo->NextEntryOffset; 
						size_t len = wcslen(processName) * sizeof(wchar_t);
						if (len) memset((void*)processName, 0, len);
					}
					lastPInfo = pInfo;
					pInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)pInfo + pInfo->NextEntryOffset);
			}
		}

		if (fakeParentProcess_)
		{
			// first search process id of explorer
			PSYSTEM_PROCESS_INFORMATION explorerPInfo = findProcessChunk((PSYSTEM_PROCESS_INFORMATION)SystemInformation, L"explorer.exe");
			if (explorerPInfo)
			{
				PSYSTEM_PROCESS_INFORMATION ownPInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
				wstring processName = getProcessName();
				// we need to walk the whole list because there might be multiple instances running
				while (ownPInfo->NextEntryOffset)
				{
					ownPInfo = findProcessChunk(ownPInfo, processName);
					// finally replace parent process id
					if (ownPInfo) ownPInfo->ParentProcessId = explorerPInfo->UniqueProcessId;
					else break;
					ownPInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)ownPInfo + ownPInfo->NextEntryOffset);
				}
			}
		}
		return status;
	}

	// fall through for default processing
	return origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
}

NTSTATUS NTAPI NtSetInformationThreadHook(HANDLE ThreadHandle, THREAD_INFORMATION_CLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength)
{
	if (!isHandleValid(ThreadHandle)) return STATUS_INVALID_HANDLE;
	if (ThreadInformationClass == ThreadHideFromDebugger) return STATUS_SUCCESS;
	return origNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
}

bool isCSRSS(DWORD processID)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnap == INVALID_HANDLE_VALUE) return NULL;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if(!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return NULL;
	}

	bool retVal = false;
	bool found = false;
	do
	{
		std::wstring exeFile = std::wstring(pe32.szExeFile);
		std::transform(exeFile.begin(), exeFile.end(), exeFile.begin(), tolower);
		if (exeFile.find(L"csrss.exe") != std::string::npos)
		{
			found = true;
			retVal = (processID == pe32.th32ProcessID) ? true : false;
		}
	} while (!found && Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return retVal;
}

// prevent access to CSRSS process
HANDLE WINAPI OpenProcessHook(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
	if (isCSRSS(dwProcessId))
	{
		// set proper last error to increase stealth
		SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	else return origOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

// just disable it
BOOL WINAPI SwitchDesktopHook(HDESK hDesktop)
{
	if (!isHandleValid(hDesktop)) return FALSE;
	return TRUE;
}

DWORD WINAPI GetTickCountHook()
{
	static bool tickCounterInitialized = false;
	static LARGE_INTEGER counter = { 23 };

	if (!tickCounterInitialized)
	{
		QueryPerformanceCounter(&counter);
		tickCounterInitialized = true;
	}
	else
	{
		if (tickDelta_ != 0) counter.QuadPart += rand() % tickDelta_;
	}

	DWORD retVal;
	// mimic original implementation to increase stealth
	__asm
	{
		lea		edx, counter
		mov		eax, [edx]
		mov		edx, [edx+4]
		mul		edx
		shrd	eax, edx, 24
		mov		retVal, eax
	}
	return retVal;
}

BOOL WINAPI BlockInputHook(BOOL /*fBlockIt*/)
{
	return TRUE;
}

DWORD WINAPI SuspendThreadHook(HANDLE hThread)
{
	if (!isHandleValid(hThread)) return (DWORD)-1;
	return 0;
}

// convert ascii string to widechar string
wstring strToWstr(const string& str)
{
	wstring wstr(str.length() + 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], str.length());
	return wstr;
}

// check if given class name matches any of the IDA window classes
bool filterClassName(const wstring& name)
{
	static wchar_t* names[] = { L"idawindow", L"tnavbox", L"idaview", L"tgrzoom" };
	wstring tmp(name.length(), 0);
	transform(name.begin(), name.end(), tmp.begin(), tolower);
	for (int i=0; i<ARRAYSIZE(names); ++i) if (tmp.find(names[i]) != string::npos) return true;
	return false;
}

//check if given window name matches any of the IDA window captions
bool filterWndName(const wstring& name)
{
	static wchar_t* names[] = { L"ida", L"graph overview", L"idc scripts", L"disassembly", 
		L"program segmentation", L"call stack", L"general registers",
		L"breakpoint", L"structure offsets", L"database notepad", L"threads",
		L"segment translation", L"imports", L"desktopform", L"function calls",
		L"structures", L"strings window", L"functions window", L"no signature"};
	wstring tmp(name.length(), 0);
	transform(name.begin(), name.end(), tmp.begin(), tolower);
	for (int i=0; i<ARRAYSIZE(names); ++i) if (tmp.find(names[i]) != string::npos) return true;
	return false;
}

HWND WINAPI FindWindowAHook(LPCSTR lpClassName, LPCSTR lpWindowName)
{
	if (lpClassName != NULL)
		if (filterClassName(strToWstr(lpClassName))) return NULL;

	if (lpWindowName != NULL)
		if (filterWndName(strToWstr(lpWindowName))) return NULL;

	return origFindWindowA(lpClassName, lpWindowName);	
}

HWND WINAPI FindWindowWHook(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	if (lpClassName != NULL)
		if (filterClassName(lpClassName)) return NULL;

	if (lpWindowName != NULL)
		if (filterWndName(lpWindowName)) return NULL;

	return origFindWindowW(lpClassName, lpWindowName);
}

HWND WINAPI FindWindowExAHook(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow)
{
	if (lpszClass != NULL)
		if (filterClassName(strToWstr(lpszClass))) return NULL;

	if (lpszWindow != NULL)
		if (filterWndName(strToWstr(lpszWindow))) return NULL;

	return origFindWindowExA(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
}

HWND WINAPI FindWindowExWHook(HWND hWndParent, HWND hWndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow)
{
	if (lpszClass != NULL)
		if (filterClassName(lpszClass)) return NULL;

	if (lpszWindow != NULL)
		if (filterWndName(lpszWindow)) return NULL;

	return origFindWindowExW(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
}

static WNDENUMPROC enumCallback = NULL;
BOOL CALLBACK enumWndFilterCallback(HWND hwnd, LPARAM lParam)
{
	wchar_t tmp[666];
	if (GetWindowTextW(hwnd, tmp, 666))
	{
		wstring wndText(tmp);
		if (filterWndName(wndText)) return TRUE;
	}
	return enumCallback(hwnd, lParam);
}

BOOL WINAPI EnumWindowsHook(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
	enumCallback = lpEnumFunc;
	return origEnumWindows(enumWndFilterCallback, lParam);
}

// init some function addresses needed for the re-implementation of KiUserExceptionDispatcher
void getOSVersion()
{
	OSVERSIONINFO osVi;
	osVi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVi);
	xpOrLater_ = (osVi.dwMajorVersion > 5) ||
		((osVi.dwMajorVersion == 5) && (osVi.dwMinorVersion >= 1));
	vistaOrLater_ = (osVi.dwMajorVersion > 5);
}

// preserve real debug registers by saving them to std::map and overwrite debug registers
// in supplied context with the debug registers from emulation storage
void handlePreSEH(PCONTEXT context)
{
	getDebugRegisterState(GetCurrentThreadId())->handlePreSEH(context);
}

// merge original DRs back into context which was possibly modified by the exception handler
void handlePostSEH(PCONTEXT context)
{
	getDebugRegisterState(GetCurrentThreadId())->handlePostSEH(context);
}

void __declspec(naked) NTAPI KiUserExceptionDispatcherHook(PEXCEPTION_RECORD /*pExceptRec*/, PCONTEXT pContext)
{
	__asm
	{					
		push	eax								// push fake address on stack so compiler generated code works!
		push	ebp								// setup standard stack frame
		mov		ebp, esp
		sub		esp, __LOCAL_SIZE				// get required size from compiler
	}

	handlePreSEH(pContext);

	// now we have to call RtlDispatchException function and we need to be sure the stack is
	// in the same state as it would be without our hook because this function might never return!
	__asm
	{
		add		esp, __LOCAL_SIZE
		pop		ebp
		add		esp, 4							// remove fake address
		mov		ecx, [esp+4]
		mov		ebx, [esp+0]
		push	ecx
		push	ebx
		call	sysAPIs[4*RtlDispatchExcept]	// stack is now clean, call RtlDispatchException
		pop		ebx
		pop		ecx
		or		al, al
		jz		_RaiseExcept
		push	ecx
		call	handlePostSEH
		pop		ecx								// PCONTEXT
		push	0
		push	ecx
	}

	__asm
	{
		call	sysAPIs[4*NtContinue]
		// this code is probably never reached at all since ZwContinue does not return
		jmp _RtlRaiseExcept

_RaiseExcept:
		push	0
		push	ecx
		push	ebx
		call	sysAPIs[4*ZwRaiseExcept]

		// original code from KiUserExceptionDispatcher
_RtlRaiseExcept:
		add		esp, 0FFFFFFECh
		mov     [esp], eax
		mov     dword ptr [esp+4], 1
		mov     [esp+8], ebx
		mov     dword ptr [esp+10h], 0
		push    esp
		call    sysAPIs[4*RtlRaiseExcept]
		retn    8
	}	
}

NTSTATUS NTAPI NtSetContextThreadHook(HANDLE hThread, const CONTEXT* context)
{
	if (IsBadReadPtr(context, sizeof(CONTEXT)) ||
		IsBadWritePtr((LPVOID)context, sizeof(CONTEXT)))
	{
		// the original API could succeed if context is not writable (but readable)
		// however, we will fail in this case because we need to write the modified context flags
		return ERROR_NOACCESS;
	}
	else if (context->ContextFlags & CONTEXT_DEBUG_REGISTERS)
	{
		boost::shared_ptr<ThreadDebugRegisterState> state = getDebugRegisterState(handleToThreadID(hThread));
		bool callAPI = state->handleSetContext(const_cast<CONTEXT*>(context));

		if (callAPI) 
		{
			return origSetThreadContext(hThread, context);
		}
		else
		{
			return STATUS_SUCCESS;
		}
	}
	else return origSetThreadContext(hThread, context);
}

NTSTATUS NTAPI NtGetContextThreadHook(HANDLE hThread, LPCONTEXT context)
{	
	NTSTATUS status = origGetThreadContext(hThread, context);
	if (NT_SUCCESS(status))
	{
		getDebugRegisterState(handleToThreadID(hThread))->handleGetContext(context);
	}

	return status;
}

NTSTATUS NTAPI NtYieldExecutionHook()
{
	origNtYieldExecution();
	return 1;
}

NTSTATUS NTAPI RtlGetVersionHook(PRTL_OSVERSIONINFOW lpVersionInfo)
{
	// oddly enough, the original API behaves this way: if we specify an incorrect size
	// the function will still return successfully!
	// the high level wrappers are the only ones to check the supplied length, i.e. GetVersion(Ex)
	// to mimic the original API, we try to fill as much info into the buffer as we can
	// therefore we need to start at offset zero and continue until we are done or get an exception
	// Note: this behavior seems to be very much OS specific, i.e. different on 2000, XP and Vista
	__try
	{
		if (lpVersionInfo->dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOEXW))
		{
			PRTL_OSVERSIONINFOEXW viex = (PRTL_OSVERSIONINFOEXW)lpVersionInfo;
			viex->dwMajorVersion = 5;
			viex->dwMinorVersion = 1;
			viex->dwBuildNumber = 0xA28;
			viex->dwPlatformId = 2;
			wcscpy_s(viex->szCSDVersion, L"Service Pack 3");
			viex->wServicePackMajor = 3;
			viex->wServicePackMinor = 0;
			viex->wProductType = 1;
			viex->wSuiteMask = 0x100;
		}
		else 
		{
			// mimic the original API: we try to copy OSVERIOSNINFO to the supplied buffer
			lpVersionInfo->dwMajorVersion = 5;
			lpVersionInfo->dwMinorVersion = 1;
			lpVersionInfo->dwBuildNumber = 0xA28;
			lpVersionInfo->dwPlatformId = 2;
			wcscpy_s(lpVersionInfo->szCSDVersion, L"Service Pack 3");
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return STATUS_SUCCESS;
}

// set all version infos to WinXP SP3
DWORD WINAPI GetVersionHook()
{
	return 0x0a280105;
}

NTSTATUS NTAPI NtTerminateProcessHook(HANDLE ProcessHandle, NTSTATUS ExitStatus)
{
	if (ProcessHandle && !isHandleValid(ProcessHandle)) return STATUS_INVALID_HANDLE;
	else if (!ProcessHandle)
	{
		// ProcessHandle is zero if process is shut down
		return origNtTerminateProcess(ProcessHandle, ExitStatus);
	}
	else if (handleToProcessID(ProcessHandle) == IDAProcessID)
	{
		dbgPrint("WARNING: application tried to terminate the IDA debugger!");
		return STATUS_SUCCESS;
	}
	else
	{
		return origNtTerminateProcess(ProcessHandle, ExitStatus);
	}
}

NTSTATUS NTAPI NtTerminateThreadHook(HANDLE ThreadHandle, NTSTATUS /*ExitStatus*/)
{
	if (!isHandleValid(ThreadHandle)) return STATUS_INVALID_HANDLE;
	return STATUS_SUCCESS;
}

// end hook functions

void patchPEB()
{
	__asm
	{
		mov eax, fs:[18h]
		mov eax, [eax+30h]
		mov byte ptr [eax+2], 0
	}
}

void patchNtGlobalFlag()
{
	unsigned char* ntGlobalFlag = NULL;
	__asm
	{
		mov eax, fs:[30h]
		lea eax, [eax+68h]
		mov ntGlobalFlag, eax
	}
	// remove only 0x70 bits, leave other flags intact
	*ntGlobalFlag &= ~0x70;
}

void patchHeapFlags()
{
	unsigned int* heapFlag = NULL;
	unsigned int* forceFlag = NULL;
	if (vistaOrLater_)
	{
		__asm
		{
			mov eax, fs:[30h]
			mov eax, [eax+18h]
			lea ecx, [eax+40h]
			mov heapFlag, ecx
			lea ecx, [eax+44h]
			mov forceFlag, ecx
		}
	}
	else
	{
		__asm
		{
			mov eax, fs:[30h]
			mov eax, [eax+18h]
			lea ecx, [eax+0Ch]
			mov heapFlag, ecx
			lea ecx, [eax+10h]
			mov forceFlag, ecx
		}
	}

	*heapFlag &= 2;
	*forceFlag = 0;
}

void applyConfigFromFile(const std::string& configFile)
{
	try
	{
		uberstealth::HideDebuggerProfile profile = uberstealth::HideDebuggerProfile::readProfile(configFile);

		bool forceAbsJumps = profile.getInlinePatchingMethodValue() == uberstealth::ForceAbsolute;
		nCodeHook.forceAbsoluteJumps(forceAbsJumps);

		if (profile.getHeapFlagsEnabled()) patchHeapFlags();
		if (profile.getPEBIsBeingDebuggedEnabled()) patchPEB();
		if (profile.getNtGlobalFlagEnabled()) patchNtGlobalFlag();
		ntQuerySysInfo_ = profile.getNtQuerySystemInformationEnabled();
		fakeParentProcess_ = profile.getFakeParentProcessEnabled();
		hideIDAProcess_ = profile.getHideDebuggerProcessEnabled();
		if (ntQuerySysInfo_ || fakeParentProcess_ || hideIDAProcess_) 
			origNtQuerySystemInformation = nCodeHook.createHookByName(NTDLL, "NtQuerySystemInformation", NtQuerySystemInformationHook);
		if (profile.getNtQueryInformationProcessEnabled() || fakeParentProcess_)
		{
			origNtQueryInformationProcess = nCodeHook.createHookByName(NTDLL, "NtQueryInformationProcess", NtQueryInformationProcessHook);
			// important: we need to store the *original* function pointer here!
			// otherwise we will run into an infinite recursion if handleToProcessID is called
			sysAPIs[NtQueryInfoProcess] = origNtQueryInformationProcess;
		}

		// only hook if OS >= XP
		if (profile.getNtQueryObjectEnabled() && xpOrLater_)
			origNtQueryObject = nCodeHook.createHookByName(NTDLL, "NtQueryObject", NtQueryObjectHook);
		if (profile.getNtCloseEnabled())
			origKiRaiseUED = nCodeHook.createHookByName(NTDLL, "KiRaiseUserExceptionDispatcher", KiRaiseUserExceptionDispatcherHook);
		if (profile.getOutputDbgStringEnabled())
		{
			origOutputDebugStringA = nCodeHook.createHookByName(K32DLL, "OutputDebugStringA", OutputDebugStringAHook);
			origOutputDebugStringW = nCodeHook.createHookByName(K32DLL, "OutputDebugStringW", OutputDebugStringWHook);
		}
		if (profile.getNtSetInformationThreadEnabled())
			origNtSetInformationThread = nCodeHook.createHookByName(NTDLL, "NtSetInformationThread", NtSetInformationThreadHook);
		if (profile.getSuspendThreadEnabled())
			nCodeHook.createHookByName(K32DLL, "SuspendThread", SuspendThreadHook);
		if (profile.getGetTickCountEnabled())
		{
			nCodeHook.createHookByName(K32DLL, "GetTickCount", GetTickCountHook);
			tickDelta_ = profile.getGetTickCountDeltaValue();
		}
		if (profile.getBlockInputEnabled())
			nCodeHook.createHookByName(U32DLL, "BlockInput", BlockInputHook);
		if (profile.getOpenProcessEnabled())
			origOpenProcess = nCodeHook.createHook(OpenProcess, OpenProcessHook);
		if (profile.getSwitchDesktopEnabled()) nCodeHook.createHook(SwitchDesktop, SwitchDesktopHook);
		if (profile.getProtectDebugRegistersEnabled())
		{
			origKiUserExceptDisp = nCodeHook.createHookByName(NTDLL, "KiUserExceptionDispatcher", KiUserExceptionDispatcherHook);
			origSetThreadContext = nCodeHook.createHookByName(NTDLL, "NtSetContextThread", NtSetContextThreadHook);
			origGetThreadContext = nCodeHook.createHookByName(NTDLL, "NtGetContextThread", NtGetContextThreadHook);
		}
		if (profile.getNtYieldExecutionEnabled())
			origNtYieldExecution = nCodeHook.createHookByName(NTDLL, "NtYieldExecution", NtYieldExecutionHook);
		if (profile.getHideDebuggerWindowsEnabled())
		{
			origFindWindowA = nCodeHook.createHookByName(U32DLL, "FindWindowA", FindWindowAHook);
			origFindWindowExA = nCodeHook.createHookByName(U32DLL, "FindWindowExA", FindWindowExAHook);
			origFindWindowW = nCodeHook.createHookByName(U32DLL, "FindWindowW", FindWindowWHook);
			origFindWindowExW = nCodeHook.createHookByName(U32DLL, "FindWindowExW", FindWindowExWHook);
			origEnumWindows = nCodeHook.createHookByName(U32DLL, "EnumWindows", EnumWindowsHook);
		}
		if (profile.getNtTerminateEnabled())
		{
			nCodeHook.createHookByName(NTDLL, "NtTerminateThread", NtTerminateThreadHook);
			origNtTerminateProcess = nCodeHook.createHookByName(NTDLL, "NtTerminateProcess", NtTerminateProcessHook);
		}
		if (profile.getGetVersionEnabled())
		{
			nCodeHook.createHookByName(K32DLL, "GetVersion", GetVersionHook);
			nCodeHook.createHookByName(NTDLL, "RtlGetVersion", RtlGetVersionHook);
		}
	}
	catch (const std::runtime_error& e)
	{
		dbgPrint("Exception while trying to create stealth hooks: %s", e.what());
	}
	catch (...)
	{
		dbgPrint("Unknown exception while creating stealth hooks");
	}
}

void acquireDebugRegistersLock()
{
	EnterCriticalSection(&section_);
}

void releaseDebugRegistersLock()
{
	LeaveCriticalSection(&section_);
}

/**
* Retrieves the debug register state for the given thread id. If now state is found, a new one is created.
* This function is thread-safe.
**/
boost::shared_ptr<ThreadDebugRegisterState> getDebugRegisterState(DWORD threadId)
{
	acquireDebugRegistersLock();

	std::map<DWORD, boost::shared_ptr<ThreadDebugRegisterState>>::iterator it = threads.find(threadId);
	boost::shared_ptr<ThreadDebugRegisterState> state;
	if (it == threads.end())
	{
		state = boost::shared_ptr<ThreadDebugRegisterState>(new ThreadDebugRegisterState(threadId));
		threads.insert(std::make_pair(threadId, state));
	}
	else
	{
		state = it->second;
	}

	releaseDebugRegistersLock();

	return state;
}