#ifdef OLLYSTEALTH

#include <Windows.h>
#include "OllyEngine.h"
#pragma warning(disable : 4200)
#include <plugin.h>
#pragma warning(default : 4200)

// TODO(jan.newger@newgre.net): NEEDS OFFICIAL IMPORT LIB!!!
extern "C" __declspec(dllimport) int Run(t_status status,int pass);
extern "C" __declspec(dllimport) int Setint3breakpoint(ulong addr, ulong type, int fnindex, int limit, int count, wchar_t *condition, wchar_t *expression, wchar_t *exprtype);

bool uberstealth::OllyEngine::setBreakpoint(uintptr_t address) const {
	//Setint3breakpoint(address, BP_MANUAL, 0, 0, 0, NULL, NULL, NULL);
	return false;
}

bool uberstealth::OllyEngine::removeBreakpoint(uintptr_t /*address*/) const
{
	//Removeint3breakpoint
	return false;
}

void uberstealth::OllyEngine::setExceptionOption(unsigned int /*exceptionCode*/, bool /*ignore*/) const {
	// TODO
}

bool uberstealth::OllyEngine::continueProcess() const {
	//Run(t_status status,int pass);
	//return Run(STAT_RUNNING, 0) == 1;
	return false;
}

void uberstealth::OllyEngine::logString(const char* /*str*/, ...) const {
	// TODO
}

#endif