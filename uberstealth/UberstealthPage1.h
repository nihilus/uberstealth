// Represents the first tab page of the options dialog.

#pragma once

#include "WTLCommon.h"
#include <HideDebugger/HideDebuggerProfile.h>
#include "UberstealthPropertyPage.h"
#include <iostream>
#include "resource.h"

namespace uberstealth {

class UberstealthPage1 : public UberstealthPropertyPage<UberstealthPage1, IDD_DIALOG1>
{
public:
	enum { IDD = IDD_DIALOG1 };

	UberstealthPage1(ConfigProvider* configProvider) :
		UberstealthPropertyPage(configProvider),
		ddxError_(false) {}

	void OnDataExchangeError(UINT nCtrlID, BOOL bSave);
	void loadProfile(const uberstealth::HideDebuggerProfile& profile);
	void flushProfile(uberstealth::HideDebuggerProfile& profile);
	bool isProfileDirty();

	int OnApply()
	{
		return ddxError_ ? PSNRET_INVALID_NOCHANGEPAGE : PSNRET_NOERROR;
	}

protected:
	void enableControls(bool enabled);

private:
	int getTickCountDelta();

	// we need a typedef - otherwise macro throws errors
	typedef UberstealthPropertyPage<UberstealthPage1, IDD_DIALOG1> BasePropertyPage;
	
	BEGIN_MSG_MAP(UberstealthPage1)
		COMMAND_HANDLER(IDC_DBGSTART, BN_CLICKED, OnDbgEnableClick)
		COMMAND_HANDLER(IDC_DBGATTACH, BN_CLICKED, OnDbgEnableClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<UberstealthPage1>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	BEGIN_DDX_MAP(UberstealthPage1)
		DDX_CHECK(IDC_NTQO, ntQueryObj_)
		DDX_CHECK(IDC_RTLNTGF, rtlGetFlags_)
		DDX_CHECK(IDC_NTQSI, ntQuerySysInfo_)
		DDX_CHECK(IDC_NTQIP, ntQueryInfoProc_)
		DDX_CHECK(IDC_GETTICKCOUNT, getTickCount_)
		DDX_CHECK(IDC_PROTECTDRS, protectDRs_)
		DDX_CHECK(IDC_GETVERSION, getVersion_)
		DDX_CHECK(IDC_NTCLOSE, ntClose_)
		DDX_CHECK(IDC_DBGPRESENT, dbgPresent_)
		DDX_CHECK(IDC_NTGF, ntGF_)
		DDX_CHECK(IDC_NTHF, ntHF_)
		DDX_INT(IDC_TICK_DELTA, tickDelta_)
		DDX_CHECK(IDC_DBGATTACH, dbgAttach_)
		DDX_CHECK(IDC_DBGSTART, dbgStart_)
	END_DDX_MAP()

	LRESULT OnDbgEnableClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	bool ntQueryObj_;
	bool rtlGetFlags_;
	bool ntQuerySysInfo_;
	bool ntQueryInfoProc_;
	bool getTickCount_;
	bool protectDRs_;
	bool getVersion_;
	bool ntClose_;
	bool dbgPresent_;
	bool ntGF_;
	bool ntHF_;
	int tickDelta_;
	bool dbgAttach_;
	bool dbgStart_;
	bool oldState_;
	bool ddxError_;
};

}