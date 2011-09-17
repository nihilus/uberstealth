#pragma once

#include "WTLCommon.h"
#include "UberstealthPropertyPage.h"
#include <iostream>
#include "resource.h"

namespace uberstealth {

class UberstealthPageMisc : public UberstealthPropertyPage<UberstealthPageMisc, IDD_DIALOG4>
{
public:

	UberstealthPageMisc(ConfigProvider* configProvider, ProfileHelper* profileHelper) :
		UberstealthPropertyPage<UberstealthPageMisc, IDD_DIALOG4>(configProvider),
		profileHelper_(profileHelper) {}

	void loadProfile(const uberstealth::HideDebuggerProfile& profile);
	bool saveProfile(uberstealth::HideDebuggerProfile& profile);
	bool isProfileDirty();

protected:
	void enableControls(bool enabled);

private:
	LRESULT OnAddProfileClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelProfileClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnProfilesSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void switchProfile();
	void initComboBox();
	int getPatchingMethod();
	int getTCPPort();

	typedef UberstealthPropertyPage<UberstealthPageMisc, IDD_DIALOG4> BasePropertyPage;
	
	BEGIN_MSG_MAP(UberstealthPageMisc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_ADD_PROFILE, BN_CLICKED, OnAddProfileClick)
		COMMAND_HANDLER(IDC_DEL_PROFILE, BN_CLICKED, OnDelProfileClick)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnProfilesSelChange)
		CHAIN_MSG_MAP(CPropertyPageImpl<UberstealthPageMisc>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	CComboBox cboProfiles_;
	int rdbPatchingMethod_;
	bool passExceptions_;
	int tcpPort_;
	bool haltInSEH_;
	bool haltAfterSEH_;
	bool logSEH_;
	ProfileHelper* profileHelper_;

	BEGIN_DDX_MAP(UberstealthPageMisc)
		DDX_CONTROL_HANDLE(IDC_PROFILES, cboProfiles_)
		DDX_RADIO(IDC_AUTO_SELECTION, rdbPatchingMethod_)
		DDX_CHECK(IDC_PASS_EXCEPTIONS, passExceptions_)
		DDX_CHECK(IDC_HALT_IN_SEH, haltInSEH_)
		DDX_CHECK(IDC_HALT_AFTER_SEH, haltAfterSEH_)
		DDX_CHECK(IDC_LOG_SEH, logSEH_)
		DDX_INT(IDC_TCP_PORT, tcpPort_)
	END_DDX_MAP()
};

}