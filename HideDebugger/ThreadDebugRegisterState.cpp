#include "ThreadDebugRegisterState.h"

/**
* Updates the state of the thread by emulating an immediate modification
* of the thread context by means of the SetThreadContext API.
* Note: this method assumes that the supplied context is writable.
*
* Returns true if the original API should be called with the modified context, false otherwise.
**/
bool ThreadDebugRegisterState::handleSetContext(LPCONTEXT context)
{
	if (context->ContextFlags & CONTEXT_DEBUG_REGISTERS)
	{
		// Clear debug registers from the context flags while leaving all other flags intact
		context->ContextFlags &= ~0x00000010;
		
		debugRegisters_ = copyFromContext(context);

		// If the context flags include any other parts of the context
		// the original API has to be called
		return (context->ContextFlags & ~CONTEXT_i386) != 0;
	}

	return true;
}

/**
* Updates the supplied context with the internal state of the thread.
* Note: this method assumes that the supplied context is writable.
**/
void ThreadDebugRegisterState::handleGetContext(const LPCONTEXT context) const
{
	// Simply copy the emulated debug registers state to the context: if the emulation
	// was never assigned to, this operation sets all debug registers to zero
	if (context->ContextFlags & CONTEXT_DEBUG_REGISTERS)
	{
		copyToContext(debugRegisters_, context);
	}
}

/**
* Updates the internal state with the original context from the OS before an SEH handler is invoked.
**/
void ThreadDebugRegisterState::handlePreSEH(LPCONTEXT context)
{
	preSehDebugRegisters_ = copyFromContext(context);
	copyToContext(debugRegisters_, context);
}

void ThreadDebugRegisterState::handlePostSEH(LPCONTEXT context)
{
	// copy (possibly) modified context to emulation, but copy the actual debug registers to the context
	debugRegisters_ = copyFromContext(context);
	copyToContext(preSehDebugRegisters_, context);
}

void ThreadDebugRegisterState::copyToContext(const DebugRegisters& debugRegisters, LPCONTEXT context) const
{
	context->Dr0 = debugRegisters.dr0;
	context->Dr1 = debugRegisters.dr1;
	context->Dr2 = debugRegisters.dr2;
	context->Dr3 = debugRegisters.dr3;
	context->Dr6 = debugRegisters.dr6;
	context->Dr7 = debugRegisters.dr7;
}

ThreadDebugRegisterState::DebugRegisters ThreadDebugRegisterState::copyFromContext(LPCONTEXT context) const
{
	return DebugRegisters(context->Dr0, context->Dr1, context->Dr2, context->Dr3, context->Dr6, context->Dr7);
}