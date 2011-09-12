#ifdef OLLYSTEALTH

#include <Windows.h>
#include "OllyEngine.h"
#pragma warning(disable : 4200)
#include <plugin.h>
#pragma warning(default : 4200)

// TODO: NEEDS OFFICIAL IMPORT LIB!!!
extern "C" __declspec(dllimport) int Run(t_status status,int pass);

bool uberstealth::OllyEngine::setBreakpoint(uintptr_t /*address*/) const
{
	//Setint3breakpoint
	return false;
}

bool uberstealth::OllyEngine::removeBreakpoint(uintptr_t /*address*/) const
{
	//Removeint3breakpoint
	return false;
}

void uberstealth::OllyEngine::setExceptionOption(unsigned int /*exceptionCode*/, bool /*ignore*/) const
{

}

bool uberstealth::OllyEngine::continueProcess() const
{
	//Run(t_status status,int pass);
	Run(STAT_RUNNING, 0);
	return false;
}

void uberstealth::OllyEngine::logString(const char* /*str*/, ...) const
{

}

#endif