#pragma once

#include <HideDebugger/HideDebuggerProfile.h>
#include "WTLCommon.h"
#include "UberstealthPropertyPage.h"
#include <iostream>
#include "ProfileEventConsumer.h"
#include "resource.h"

namespace uberstealth {

class UberstealthDriversPage : public UberstealthPropertyPage<UberstealthDriversPage, IDD_DIALOG3>
{
public:
	UberstealthDriversPage(ConfigProvider* configProvider) :
		UberstealthPropertyPage(configProvider) {}

	void loadProfile(const uberstealth::HideDebuggerProfile& profile);
	bool saveProfile(uberstealth::HideDebuggerProfile& profile);
	bool isProfileDirty();

protected:
	void enableControls(bool enabled);

private:
	int uberstealth::UberstealthDriversPage::getRDTSCMode() const;
	int uberstealth::UberstealthDriversPage::getRDTSCDelta() const;
	WTL::CString uberstealth::UberstealthDriversPage::getRDTSCName() const;
	WTL::CString uberstealth::UberstealthDriversPage::getStealthName() const;

	typedef UberstealthPropertyPage<UberstealthDriversPage, IDD_DIALOG3> BasePropertyPage;

	BEGIN_MSG_MAP(UberstealthDriversPage)
		COMMAND_HANDLER(IDC_RDTSC, BN_CLICKED, OnRDTSCClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<UberstealthDriversPage>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	BEGIN_DDX_MAP(UberstealthDriversPage)
		DDX_CHECK(IDC_RDTSC, useRDTSC_)
		DDX_RADIO(IDC_RDTSC_ZERO, rdbIndex_)
		DDX_CHECK(IDC_UNLOAD_DRIVER, unloadRDTSCDrv_)
		DDX_CHECK(IDC_USE_RDTSC_NAME, useRDTSCName_)
		DDX_CHECK(IDC_USE_STEALTH_NAME, useStealthName_)
		DDX_INT(IDC_RDTSC_DELTA, rdtscDelta_)
		DDX_CHECK(IDC_UNLOAD_STEALTH_DRV, unloadStealthDrv_)
		DDX_CHECK(IDC_NTSIT_DRV, ntSITDrv_)
		DDX_CHECK(IDC_NTQIP_DRV, ntQIPDrv_)
		DDX_TEXT(IDC_RDTSC_NAME, rdtscName_)
		DDX_TEXT(IDC_STEALTH_NAME, stealthName_)
	END_DDX_MAP()

	void updateRDBs(HWND hWndCheckBox);
	LRESULT OnRDTSCClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	bool useRDTSC_;
	bool unloadRDTSCDrv_;
	bool unloadStealthDrv_;
	bool ntSITDrv_;
	bool ntQIPDrv_;
	bool useRDTSCName_;
	bool useStealthName_;
	WTL::CString rdtscName_;
	WTL::CString stealthName_;
	int rdtscDelta_;
	int rdbIndex_;
};

}