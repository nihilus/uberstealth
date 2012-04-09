// Represents the debug registers thread context and provides methods to handle different events
// corresponding to invocations of (internal) windows API functions.

#pragma once

#include <Windows.h>
#include <iostream>

class ThreadDebugRegisterState {
public:
	ThreadDebugRegisterState(DWORD threadId) :
		threadId(threadId) {};

	bool handleSetContext(LPCONTEXT context);
	void handleGetContext(const LPCONTEXT context) const;
	void handlePreSEH(LPCONTEXT context);
	void handlePostSEH(LPCONTEXT context);

private:
	struct DebugRegisters {
		DebugRegisters() :
			dr0(0),
			dr1(0),
			dr2(0),
			dr3(0),
			dr6(0),
			dr7(0) {}

		DebugRegisters(DWORD dr0, DWORD dr1, DWORD dr2, DWORD dr3, DWORD dr6, DWORD dr7) :
			dr0(dr0),
			dr1(dr1),
			dr2(dr2),
			dr3(dr3),
			dr6(dr6),
			dr7(dr7) {}

		DWORD dr0;
		DWORD dr1;
		DWORD dr2;
		DWORD dr3;
		DWORD dr6;
		DWORD dr7;
	};

	void copyToContext(const DebugRegisters& debugRegisters, LPCONTEXT context) const;
	DebugRegisters copyFromContext(LPCONTEXT context) const;

	DWORD threadId;
	DebugRegisters debugRegisters;
	DebugRegisters preSehDebugRegisters;
};
