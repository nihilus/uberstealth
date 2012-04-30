// The Implementation for OllyStealth.

#ifdef OLLYSTEALTH

#include "WTLWrapper.h"
#include <Windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include "LocalStealthSession.h"
#include "OllyEngine.h"
#pragma warning(disable : 4200)
#include <plugin.h>
#pragma warning(default : 4200)
#include "resource.h"
#include "version.h"

// TODO(jan.newger@newgre.net): NEEDS official import library!
extern "C" __declspec(dllimport) const HWND _hwollymain;
extern "C" __declspec(dllimport) t_run _run;
extern "C" __declspec(dllimport) ulong _processid;
//extern "C" __declspec(dllimport) t_module* Findmainmodule(void);

namespace
{
	typedef uberstealth::LocalStealthSession<uberstealth::OllyEngine> OllySession;
	boost::shared_ptr<OllySession> session_;
	uberstealth::ProfileHelper profileHelper_;
}

int showDialog(t_table* /*pt*/, wchar_t* /*name*/, ulong /*index*/, int mode)
{
	if (mode == MENU_VERIFY) return MENU_NORMAL;
	else if (mode == MENU_EXECUTE)
	{
		uberstealth::WTLWrapper& wtlWrapper = uberstealth::WTLWrapper::getInstance();
		wtlWrapper.showGUI(_hwollymain, &profileHelper_);
		return MENU_NOREDRAW;
	}
	return MENU_ABSENT;
}

extc _export int cdecl ODBG2_Pluginquery(int ollyDbgVersion, wchar_t pluginName[SHORTNAME], wchar_t pluginVersion[SHORTNAME])
{
	if (ollyDbgVersion < 201) return 0;
	wcscpy_s(pluginName, SHORTNAME, L"uberstealth");
	wcscpy_s(pluginVersion, SHORTNAME, UBERSTEALTH_PRODUCT_VERSION_STRING);
	return PLUGIN_VERSION;
}

extc _export void cdecl ODBG2_Pluginmainloop(DEBUG_EVENT* debugEvent)
{
	if (!debugEvent) return;

	// TODO(jan.newger@newgre.net): wrap the rest of the procedure in try...catch! do the same for IDAStealth.cpp!
	switch (debugEvent->dwDebugEventCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
			if (_run.status == STAT_LOADING)
			{
				session_ = boost::make_shared<OllySession>(&profileHelper_);
				session_->handleProcessStart(_processid, reinterpret_cast<uintptr_t>(debugEvent->u.CreateProcessInfo.lpBaseOfImage));
			}		
			else if (_run.status == STAT_ATTACHING)
			{
				session_ = boost::make_shared<OllySession>(&profileHelper_);
				session_->handleDbgAttach(_processid);
			}
			break;

		case EXCEPTION_DEBUG_EVENT:
			if (debugEvent->dwDebugEventCode == EXCEPTION_BREAKPOINT)
			{
				// TODO(jan.newger@newgre.net): do we catch breakpoints set by ollydbg here as well?
				session_->handleBreakPoint(debugEvent->dwThreadId, reinterpret_cast<uintptr_t>(debugEvent->u.Exception.ExceptionRecord.ExceptionAddress));
			}
			break;
	}
}

static t_menu uberstealthMenu[] = {
	{ L"uberstealth", L"Open uberstealth options dialog", K_NONE, showDialog, NULL, 0 },
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

extc _export t_menu* cdecl ODBG2_Pluginmenu(wchar_t* type)
{
	if (wcscmp(type, PWM_MAIN) == 0) return uberstealthMenu;
	return NULL;
}

#endif