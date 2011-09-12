// Global wrapper to encapsulate all WTL boiler plate code and to initialize the GUI.
// This class is implemented in terms of a singleton.

#include "WTLCommon.h"
#include <boost/noncopyable.hpp>
#include <iostream>
#include <common/StringHelper.h>
#include "UberstealthOptionsDialog.h"
#include "version.h"

#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace uberstealth {

	class WTLWrapper : public boost::noncopyable
{
public:

	static WTLWrapper& getInstance()
	{
		static WTLWrapper instance_;
		return instance_;
	}

	~WTLWrapper()
	{
		module_.Term();
		::CoUninitialize();
	}

	void showGUI(HWND hWndParent)
	{
		try
		{
			UberstealthOptionsDialog dlg(UBERSTEALTH_NAME);
			dlg.DoModal(hWndParent);
		}
		catch (const std::exception& e)
		{
			std::string err = "Error in options dialog: " + std::string(e.what());
			::MessageBox(hWndParent, StringToUnicode(err), L"uberstealth", MB_ICONERROR);	
		}
	}

private:

	WTLWrapper()
	{
		HRESULT hRes = ::CoInitialize(NULL);
		ATLASSERT(SUCCEEDED(hRes));
		AtlInitCommonControls(ICC_BAR_CLASSES | ICC_LINK_CLASS);
		hRes = module_.Init(NULL, GetModuleHandle(NULL));
		ATLASSERT(SUCCEEDED(hRes));
	}

	CAppModule module_;
};

}