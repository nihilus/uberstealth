#include "DebugEvent.h"
#include <Windows.h>

extern void issueDebugEvent(DebugEvent event, uintptr_t retAddress)
{
	event;
	retAddress;
	RaiseException(DebugEventExceptionCode, 0, 0, NULL);
}