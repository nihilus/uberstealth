// The implementation of IDAStealth.

#ifdef IDASTEALTH

// General note: ALWAYS include Windows/ATL/WTL headers BEFORE including the IDA headers!
// Otherwise, weird compiler errors will appear!

#include "WTLWrapper.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <common/StringHelper.h>
#include <iostream>
#include "IDACommon.h"
#include "IDAEngine.h"
#include "LocalStealthSession.h"
#include "resource.h"
#include "version.h"

int idaapi callback(void* user_data, int notification_code, va_list va);

namespace
{
	boost::shared_ptr<uberstealth::StealthSession<uberstealth::IDAEngine>> session_;
	uberstealth::ProfileHelper profileHelper_;
}

/*********************************************************************
* Function: init
*
* init is a plugin_t function. It is executed when the plugin is
* initially loaded by IDA.
* Three return codes are possible:
*    PLUGIN_SKIP - Plugin is unloaded and not made available
*    PLUGIN_KEEP - Plugin is kept in memory
*    PLUGIN_OK   - Plugin will be loaded upon 1st use
*
* Check are added here to ensure the plug-in is compatible with
* the current disassembly.
*********************************************************************/
int __stdcall init()
{
	if (inf.filetype != f_PE || !inf.is_32bit()) return PLUGIN_SKIP;
	
	std::string dashes;
	dashes.resize(80, '-');
	msg("%s\n%s\n%s\n", dashes.c_str(), (const char*)uberstealth::UnicodeToString(UBERSTEALTH_INFO_STRING), dashes.c_str());
	
	if (!hook_to_notification_point(HT_DBG, callback, NULL))
	{
		msg("uberstealth: Unable to hook to notification point\n");
		return PLUGIN_SKIP;
	}
	
	return PLUGIN_KEEP;
}

/*********************************************************************
* Function: term
*
* term is a plugin_t function. It is executed when the plugin is
* unloading. Typically cleanup code is executed here.
*********************************************************************/
void __stdcall term()
{
	try
	{
		profileHelper_.writeLastProfile();
	}
	catch (const std::runtime_error& e)
	{
		msg("Error while saving last profile: %s\n", e.what());
	}
	unhook_from_notification_point(HT_DBG, callback, NULL);
}

/*********************************************************************
* Function: run
*
* run is a plugin_t function. It is executed when the plugin is run.
*
* The argument 'arg' can be passed by adding an entry in
* plugins.cfg or passed manually via IDC:
*
*   success RunPlugin(string name, long arg);
*********************************************************************/
void __stdcall run(int arg)
{

	//  Uncomment the following code to allow plugin unloading.
	//  This allows the editing/building of the plugin without
	//  restarting IDA.
	//
	//  1. to unload the plugin execute the following IDC statement:
	//        RunPlugin("IDAStealth", 666);
	//  2. Make changes to source code and rebuild within Visual Studio
	//  3. Copy plugin to IDA plugin dir
	//     (may be automatic if option was selected within wizard)
	//  4. Run plugin via the menu, hotkey, or IDC statement
	//
	if (arg == 666)
	{
		PLUGIN.flags |= PLUGIN_UNL;
		msg("Unloading uberstealth plugin...\n");
	}
	else
	{
		uberstealth::WTLWrapper& wtlWrapper = uberstealth::WTLWrapper::getInstance();
		wtlWrapper.showGUI((HWND)callui(ui_get_hwnd).vptr, &profileHelper_);
	}
}

int idaapi callback(void*, int notification_code, va_list va)
{
	try
	{
		switch (notification_code)
		{
		case dbg_process_attach:
			{
				// TODO: instantiate RemoteStealthSession if appropriate
				const debug_event_t* dbgEvent = va_arg(va, const debug_event_t*);
				session_ = boost::make_shared<uberstealth::LocalStealthSession<uberstealth::IDAEngine>>(&profileHelper_);
				session_->handleDbgAttach(dbgEvent->pid);
			}
			break;

		case dbg_process_start:
			{
				const debug_event_t* dbgEvent = va_arg(va, const debug_event_t*);
				session_ = boost::make_shared<uberstealth::LocalStealthSession<uberstealth::IDAEngine>>(&profileHelper_);
				session_->handleProcessStart(dbgEvent->pid, dbgEvent->modinfo.base);
			}
			break;

		case dbg_process_exit:
			va_arg(va, const debug_event_t*);
			session_->handleProcessExit();
			break;

		case dbg_bpt:
			{
				thid_t tid = va_arg(va, thid_t);
				ea_t breakpoint_ea = va_arg(va, ea_t);
				va_arg(va, int*);
				session_->handleBreakPoint(tid, breakpoint_ea);
			}
			break;

		case dbg_exception:
			{
				const debug_event_t* dbgEvent = va_arg(va, const debug_event_t*);
				va_arg(va, int*);
				session_->handleException(dbgEvent->exc.code);
			}
			break;
		}
	}
	catch (const std::exception& e)
	{
		msg("uberstealth: Error while processing debug event: %s\n", e.what());
	}
	catch (...)
	{
		msg("uberstealth: Unknown error (this should never happen!)\n");
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

char comment[] 	= "Short one line description about the plugin";
char help[] 	= "My plugin:\n"
"\n"
"Multi-line\n"
"description\n";

/* Plugin name listed in (Edit | Plugins) */
char wanted_name[] 	= "uberstealth";

/* plug-in hotkey */
char wanted_hotkey[] 	= "";

/* defines the plugins interface to IDA */
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	0,              // plugin flags
	init,           // initialize
	term,           // terminate. this pointer may be NULL.
	run,            // invoke plugin
	comment,        // comment about the plugin
	help,           // multiline help about the plugin
	wanted_name,    // the preferred short name of the plugin
	wanted_hotkey   // the preferred hotkey to run the plugin
};

#endif