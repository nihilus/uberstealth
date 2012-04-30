#pragma once

#include "WTLCommon.h"
#include "UberstealthPropertyPage.h"
#include <iostream>
#include "resource.h"

namespace uberstealth {

class UberstealthPageMisc : public UberstealthPropertyPage<UberstealthPageMisc, IDD_DIALOG4>
{
public:
	UberstealthPageMisc(ConfigProvider* configProvider) :
		UberstealthPropertyPage<UberstealthPageMisc, IDD_DIALOG4>(configProvider) {}

	void loadProfile(const uberstealth::HideDebuggerProfile& profile);
	void flushProfile(uberstealth::HideDebuggerProfile& profile);
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

	struct PageMiscGuiData {
		int patchingMethod_;
		bool passExceptions_;
		int tcpPort_;
		bool haltInSEH_;
		bool haltAfterSEH_;
		bool logSEH_;

		bool operator==(const PageMiscGuiData& rhs) const {
			return patchingMethod_ == rhs.patchingMethod_ &&
				passExceptions_ == rhs.passExceptions_ &&
				tcpPort_ == rhs.tcpPort_ &&
				haltInSEH_ == rhs.haltInSEH_ &&
				haltAfterSEH_ == rhs.haltAfterSEH_ &&
				logSEH_ == rhs.logSEH_;
		}
		bool operator!=(const PageMiscGuiData& rhs) const {
			return !operator==(rhs);
		}
	};

	PageMiscGuiData guiData_;

	BEGIN_DDX_MAP(UberstealthPageMisc)
		DDX_CONTROL_HANDLE(IDC_PROFILES, cboProfiles_)
		DDX_RADIO(IDC_AUTO_SELECTION, guiData_.patchingMethod_)
		DDX_CHECK(IDC_PASS_EXCEPTIONS, guiData_.passExceptions_)
		DDX_CHECK(IDC_HALT_IN_SEH, guiData_.haltInSEH_)
		DDX_CHECK(IDC_HALT_AFTER_SEH, guiData_.haltAfterSEH_)
		DDX_CHECK(IDC_LOG_SEH, guiData_.logSEH_)
		DDX_INT(IDC_TCP_PORT, guiData_.tcpPort_)
	END_DDX_MAP()
};

}