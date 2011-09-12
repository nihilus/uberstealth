#include "HookHelper.h"
#include "IPCConfigExchangeReader.h"
#include <string>

NtQueryInformationProcessFPtr origNtQueryInformationProcess = NULL;
NtSetInformationThreadFPtr origNtSetInformationThread = NULL;
NtQueryObjectFPtr origNtQueryObject = NULL;
NtQuerySystemInformationFPtr origNtQuerySystemInformation = NULL;
OutputDebugStringAFPtr origOutputDebugStringA = NULL;
OutputDebugStringWFPtr origOutputDebugStringW = NULL;
OpenProcessFPtr origOpenProcess = NULL;
KiUserExceptionDispatcherFPtr origKiUserExceptDisp = NULL;
NtSetContextThreadFPtr origSetThreadContext = NULL;
NtGetContextThreadFPtr origGetThreadContext = NULL;
NtYieldExecutionFPtr origNtYieldExecution = NULL;
FindWindowAFPtr origFindWindowA = NULL;
FindWindowWFPtr origFindWindowW = NULL;
FindWindowExAFPtr origFindWindowExA = NULL;
FindWindowExWFPtr origFindWindowExW = NULL;
EnumWindowsFPtr origEnumWindows = NULL;
NtTherminateProcessFPtr origNtTerminateProcess = NULL;
KiRaiseUEDHookFPtr origKiRaiseUED = NULL;

NCodeHookIA32 nCodeHook;
const char* NTDLL = "ntdll.dll";
const wchar_t* NTDLL_W = L"ntdll.dll";
const char* K32DLL = "kernel32.dll";
const char* U32DLL = "user32.dll";

// cached system APIs
void* sysAPIs[MaxSystemApi] = {0};

void dbgPrint(const char* message, ...)
{
	char buffer[100];
	va_list arglist;	
	int offset = _snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%s", "HideDebugger.dll: ");
	va_start(arglist, message);
	vsnprintf_s(buffer + offset, sizeof(buffer) - offset, _TRUNCATE, message, arglist);
	va_end(arglist);

	OutputDebugStringA(buffer);
}

// return name of current process
std::wstring getProcessName()
{
	wchar_t pathBuffer[MAX_PATH];
	DWORD size = GetModuleFileName(NULL, pathBuffer, MAX_PATH - 1);
	if (!size) return std::wstring(L"");
	std::wstring exeName = std::wstring(pathBuffer);
	if (exeName[exeName.length()-1] == '\\') exeName.erase(exeName.length()-1);
	size_t index = exeName.find_last_of(L"\\") + 1;
	exeName.erase(exeName.begin(), exeName.begin() + index);
	return exeName;
}

// try to check if a given handle is valid
bool isHandleValid(HANDLE handle)
{
	// Note: we might produce false negatives here due to access restrictions of the debuggee (PROCESS_DUP_HANDLE)
	HANDLE duplicatedHandle;
	if (DuplicateHandle(GetCurrentProcess(), handle, GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		CloseHandle(duplicatedHandle);
		return true;
	}
	return false;
}

void restoreNTHeaders()
{
	try
	{
		ipc::IPCConfigExchangeReader ipcReader;
		// we don't need to restore the PE headers when we attach to a process
		if (ipcReader.isPERestoreRequired())
		{
			ipc::IPCPEHeaderData peData = ipcReader.getIPCPEHeaderData();
			// read DOS header, advance to NT headers and restore them
			DWORD oldProtection;
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)peData.imageBase;
			VirtualProtect((LPVOID)pDosHeader, sizeof(IMAGE_DOS_HEADER), PAGE_EXECUTE_READWRITE, &oldProtection);
			PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + peData.imageBase);
			VirtualProtect((LPVOID)pDosHeader, sizeof(IMAGE_DOS_HEADER), oldProtection, &oldProtection);

			VirtualProtect(pNtHeaders, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE, &oldProtection);
			*pNtHeaders = peData.ntHeaders;
			VirtualProtect(pNtHeaders, sizeof(IMAGE_NT_HEADERS), oldProtection, &oldProtection);
		}
	}
	catch (const std::exception& e)
	{
		dbgPrint("Error while restoring NT headers: %s", e.what());
	}
	catch (...)
	{
		dbgPrint("SEH exception while trying to restore PE header");
	}
}

// convert given process handle to process ID
DWORD handleToProcessID(HANDLE hProcess)
{	
	PROCESS_BASIC_INFORMATION pbi;
	memset(&pbi, 0, sizeof(pbi));
	NtQueryInformationProcessFPtr ntqip = (NtQueryInformationProcessFPtr)sysAPIs[NtQueryInfoProcess];
	if (!NT_SUCCESS(ntqip(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL)))
	{
		dbgPrint("Failed to translate process handle to corresponding process ID");
		return 0;
	}
	return pbi.UniqueProcessId;
}

DWORD handleToThreadID(HANDLE hThread)
{
	typedef NTSTATUS (NTAPI *NtQIT)(HANDLE, THREAD_INFORMATION_CLASS, PVOID, ULONG, PULONG);
	NtQIT pNtQueryInfThread = (NtQIT)sysAPIs[NtQueryInfoThread];

	THREAD_BASIC_INFORMATION tbi = {0};
	NTSTATUS status = pNtQueryInfThread(hThread, ThreadBasicInformation, &tbi, sizeof(THREAD_BASIC_INFORMATION), NULL);
	if (!NT_SUCCESS(status))
	{
		dbgPrint("Failed to translate thread handle to corresponding thread ID");
		return 0;
	}

	return (DWORD)tbi.ClientId.UniqueThread;
}

void initSystemAPIs()
{
	HMODULE hNtDll = LoadLibrary(NTDLL_W);

	// we need to disassemble the beginning of KiUserExceptionDispatcher to get RtlDispatchException
	_DecodedInst instructions[20];
	unsigned int instructionCount = 0;
	unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
	distorm_decode(0, codePtr, 20, Decode32Bits, instructions, 20, &instructionCount);
	for (int i=0; i<(int)instructionCount; ++i)
	{
		if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0)
		{
			uintptr_t callOffset = 0;
			sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
			sysAPIs[RtlDispatchExcept] = (void*)((uintptr_t)codePtr + callOffset);
			break;
		}
	}

	sysAPIs[NtContinue] = GetProcAddress(hNtDll, "ZwContinue");
	sysAPIs[RtlRaiseExcept] = GetProcAddress(hNtDll, "RtlRaiseException");
	sysAPIs[ZwRaiseExcept] = GetProcAddress(hNtDll, "ZwRaiseException");
	sysAPIs[NtQueryInfoProcess] = GetProcAddress(hNtDll, "NtQueryInformationProcess");
	sysAPIs[NtQueryInfoThread] = GetProcAddress(hNtDll, "NtQueryInformationThread");

	FreeLibrary(hNtDll);
}