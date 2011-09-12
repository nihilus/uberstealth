#pragma once

#include "WTLCommon.h"
#include <HideDebugger/HideDebuggerProfile.h>
#include "UberstealthPropertyPage.h"
#include <iostream>
#include "resource.h"

namespace uberstealth {

class UberstealthPage2 : public UberstealthPropertyPage<UberstealthPage2, IDD_DIALOG2>
{
public:
	UberstealthPage2(ConfigProvider* configProvider) :
		UberstealthPropertyPage(configProvider) {}

	void loadProfile(const uberstealth::HideDebuggerProfile& profile);
	bool saveProfile(uberstealth::HideDebuggerProfile& profile);
	bool isProfileDirty();

protected:
	void enableControls(bool enabled);

private:
	BEGIN_DDX_MAP(UberstealthPage2)
		DDX_CHECK(IDC_BLOCKINPUT, blockInput_)
		DDX_CHECK(IDC_SUSPENDTHREAD, suspThread_)
		DDX_CHECK(IDC_TERMINATE, ntTerminate_)
		DDX_CHECK(IDC_PARENTPROCESS, parentProcess_)
		DDX_CHECK(IDC_HIDEIDAPROCESS, hideIDAProcess_)
		DDX_CHECK(IDC_HIDEIDAWND, hideIDAWnd_)
		DDX_CHECK(IDC_DBGPRNTEXCEPT, dbgPrintExcp_)
		DDX_CHECK(IDC_OPENPROCESS, openProcess_)
		DDX_CHECK(IDC_SWITCH, switch_)
		DDX_CHECK(IDC_ANTIATTACH, antiAttach_)
		DDX_CHECK(IDC_NTYIELD, ntYield_)
		DDX_CHECK(IDC_OUTDBGSTR, outputDbgStr_)
		DDX_CHECK(IDC_NTSIT, ntSetInfoThread_)
	END_DDX_MAP()

	bool blockInput_;
	bool suspThread_;
	bool ntTerminate_;
	bool parentProcess_;
	bool hideIDAProcess_;
	bool hideIDAWnd_;
	bool dbgPrintExcp_;
	bool openProcess_;
	bool switch_;
	bool antiAttach_;
	bool ntYield_;
	bool outputDbgStr_;
	bool ntSetInfoThread_;
};

}