#ifdef IDASTEALTH

#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include "IDACommon.h"
#include "IDAEngine.h"
#include <vector>

namespace
{
	const int TimerIntervall = 66;
	std::vector<std::string> messages_;
	boost::mutex mutex_;

	void CALLBACK timerCallbackProc(HWND hwnd, UINT, UINT idTimer, DWORD)
	{
		KillTimer(hwnd, idTimer);
		mutex_.lock();
		BOOST_FOREACH(const std::string& str, messages_)
		{
			msg(str.c_str());
		}
		messages_.clear();
		mutex_.unlock();
	}
}

uberstealth::IDAEngine::IDAEngine()
{
	hIDAWnd_ = (HWND)callui(ui_get_hwnd).vptr;
	idaMainThread_ = GetWindowThreadProcessId(hIDAWnd_, NULL);
}

bool uberstealth::IDAEngine::setBreakpoint(uintptr_t address) const
{
	if (exist_bpt(address)) return true;
	else return add_bpt(address);
}

bool uberstealth::IDAEngine::removeBreakpoint(uintptr_t address) const
{
	if (exist_bpt(address)) return true;
	return del_bpt(address);
}

void uberstealth::IDAEngine::setExceptionOption(unsigned int /*exceptionCode*/, bool /*ignore*/) const
{
	// TODO: implement!
}

bool uberstealth::IDAEngine::continueProcess() const
{
	return continue_process();
}

void uberstealth::IDAEngine::logString(const char* str, ...) const
{
	char buffer[500];
	va_list arglist;	
	va_start(arglist, str);
	vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, str, arglist);
	va_end(arglist);

	if (GetCurrentThreadId() == idaMainThread_)
	{
		msg(buffer);
	}
	else
	{
		mutex_.lock();
		messages_.push_back(std::string(buffer));
		mutex_.unlock();
		UINT_PTR id = SetTimer(0, 0, TimerIntervall, NULL);
		SetTimer(hIDAWnd_, id, TimerIntervall, (TIMERPROC)timerCallbackProc);
	}
}

#endif

