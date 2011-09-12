#pragma once

#include <iostream>

enum DebugEvent { PreSehHandlerInvocationEvent, PreNtContinueEvent };

const unsigned int DebugEventExceptionCode = 0x42424242;

extern void issueDebugEvent(DebugEvent event, uintptr_t retAddress);