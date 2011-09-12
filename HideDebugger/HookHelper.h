#pragma once

#include <iostream>
#include <NCodeHook/NCodeHookInstantiation.h>
#include "ntdll.h"

void dbgPrint(const char* message, ...);
DWORD handleToProcessID(HANDLE hProcess);
DWORD handleToThreadID(HANDLE hThread);
std::wstring getProcessName();
void restoreNTHeaders();
bool isHandleValid(HANDLE handle);
void initSystemAPIs();

enum SystemApi { ZwRaiseExcept, RtlDispatchExcept, RtlRaiseExcept, 
NtQueryInfoProcess, NtQueryInfoThread, NtContinue, MaxSystemApi };

// original function pointers

extern NtQueryInformationProcessFPtr origNtQueryInformationProcess;
extern NtSetInformationThreadFPtr origNtSetInformationThread;
extern NtQueryObjectFPtr origNtQueryObject;
extern NtQuerySystemInformationFPtr origNtQuerySystemInformation;
extern OutputDebugStringAFPtr origOutputDebugStringA;
extern OutputDebugStringWFPtr origOutputDebugStringW;
extern OpenProcessFPtr origOpenProcess;
extern KiUserExceptionDispatcherFPtr origKiUserExceptDisp;
extern NtSetContextThreadFPtr origSetThreadContext;
extern NtGetContextThreadFPtr origGetThreadContext;
extern NtYieldExecutionFPtr origNtYieldExecution;
extern FindWindowAFPtr origFindWindowA;
extern FindWindowWFPtr origFindWindowW;
extern FindWindowExAFPtr origFindWindowExA;
extern FindWindowExWFPtr origFindWindowExW;
extern EnumWindowsFPtr origEnumWindows;
extern NtTherminateProcessFPtr origNtTerminateProcess;
extern KiRaiseUEDHookFPtr origKiRaiseUED;

extern const char* NTDLL;
extern const char* K32DLL;
extern const char* U32DLL;

extern NCodeHookIA32 nCodeHook;

extern void* sysAPIs[];

#ifdef _DEBUG
#define DBG_PRINT(x) dbgPrint(x)
#else
#define DBG_PRINT(x)
#endif