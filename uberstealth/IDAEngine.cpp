#ifdef IDASTEALTH

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include "IDACommon.h"
#include "IDAEngine.h"
#include <vector>

namespace {
	const int TimerIntervall = 66;
	std::vector<std::string> messages_;
	boost::mutex mutex_;

	void CALLBACK timerCallbackProc(HWND hwnd, UINT, UINT idTimer, DWORD) {
		KillTimer(hwnd, idTimer);
		mutex_.lock();
		BOOST_FOREACH(const std::string& str, messages_) {
			msg(str.c_str());
		}
		messages_.clear();
		mutex_.unlock();
	}
}

uberstealth::IDAEngine::IDAEngine() {
	hIDAWnd_ = (HWND)callui(ui_get_hwnd).vptr;
	idaMainThread_ = GetWindowThreadProcessId(hIDAWnd_, NULL);
}

uberstealth::IDAEngine::~IDAEngine() {
	restoreExceptions();
}

bool uberstealth::IDAEngine::setBreakpoint(uintptr_t address) const {
	if (exist_bpt(address)) return true;
	else return add_bpt(address);
}

bool uberstealth::IDAEngine::removeBreakpoint(uintptr_t address) const {
	if (exist_bpt(address)) return true;
	return del_bpt(address);
}

void uberstealth::IDAEngine::showExceptionDialog(bool showDialog) const {
	uint oldSettings = set_debugger_options(0) & ~(EXCDLG_ALWAYS | EXCDLG_UNKNOWN);
	uint newSettings = showDialog ? oldSettings | EXCDLG_ALWAYS : oldSettings | EXCDLG_NEVER;
	set_debugger_options(newSettings);
}

// Delete exceptions which were added by us during the debugging session.
void uberstealth::IDAEngine::restoreExceptions() {
	excvec_t* exceptions = retrieve_exceptions();
	exceptions->erase(std::remove_if(exceptions->begin(), exceptions->end(), ExceptionFilter(&addedExceptions_)), exceptions->end());
	store_exceptions();
}

const exception_info_t* uberstealth::IDAEngine::findException(unsigned int exceptionCode) const {
	BOOST_FOREACH(const exception_info_t& exInfo, *retrieve_exceptions()) {
		if (exceptionCode == exInfo.code) {
			return &exInfo;
		}
	}
	return NULL;
}

void uberstealth::IDAEngine::setExceptionOption(unsigned int exceptionCode, bool ignore) {	
	// Due to the design of the IDA API we need to (globally) tell IDA whether to display the exception dialog or not.
	// If we encounter a known exception, we show the dialog depending on the setting of the exception.
	// If it is an unknown exception, we hide the dialog and pass the exception to the application.
	const exception_info_t* existingException = findException(exceptionCode);
	if (!existingException && ignore) {
		exception_info_t newException(exceptionCode, 0, "added by uberstealth", "");
		retrieve_exceptions()->push_back(newException);
		store_exceptions();
		showExceptionDialog(false);
		addedExceptions_.insert(newException);
	} else if (existingException) {
		showExceptionDialog(existingException->break_on());
	}
}

bool uberstealth::IDAEngine::continueProcess() const {
	return continue_process();
}

void uberstealth::IDAEngine::logString(const char* str, ...) const {
	char buffer[500];
	va_list arglist;	
	va_start(arglist, str);
	vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, str, arglist);
	va_end(arglist);

	if (GetCurrentThreadId() == idaMainThread_) {
		msg(buffer);
	}
	else {
		mutex_.lock();
		messages_.push_back(std::string(buffer));
		mutex_.unlock();
		UINT_PTR id = SetTimer(0, 0, TimerIntervall, NULL);
		SetTimer(hIDAWnd_, id, TimerIntervall, (TIMERPROC)timerCallbackProc);
	}
}

#endif