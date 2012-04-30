#include <boost/foreach.hpp>
#include "UberstealthOptionsDialog.h"
#include "UberstealthPageMisc.h"
#include <HideDebugger/HideDebuggerProfile.h>
#include "SaveChangesDialog.h"
#include <common/StringHelper.h>
#include "WTLInputBox.h"

LRESULT uberstealth::UberstealthPageMisc::OnAddProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WTLInputBox inputBox;
	inputBox.DoModal(m_hWnd);
	std::string input = inputBox.getInput();
	if (input.length()) {
		try	{
			// If profile was modified, let user decide if it should be saved before creating a new one.
			if (configProvider_->isProfileModified()) {
				SaveChangesDialog saveChangesDlg;
				saveChangesDlg.DoModal(m_hWnd);
				switch (saveChangesDlg.getAnswer())	{
				case SaveChangesDialog::Save:
					configProvider_->triggerFlushProfileEvent();
					HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(),  getCurrentProfileName());
					HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(), input);
					break;

				case SaveChangesDialog::DontSave:
					configProvider_->triggerFlushProfileEvent();
					HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(), input);
					break;

				case SaveChangesDialog::Cancel:
					return 0;
				}
				setCurrentProfileName(input);
				int item = cboProfiles_.AddString(uberstealth::StringToUnicode(input));
				cboProfiles_.SetCurSel(item);
			} else {
				HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(), input);
				setCurrentProfileName(input);
				int item = cboProfiles_.AddString(uberstealth::StringToUnicode(input));
				cboProfiles_.SetCurSel(item);
			}
		} catch (const std::runtime_error& e) {
			MessageBox(StringToUnicode("Error while writing new profile file: " + std::string(e.what())), L"Error", MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT uberstealth::UberstealthPageMisc::OnDelProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR) {
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		if (deleteProfileByName(std::string(CT2A(itemText, CP_UTF8)))) {
			cboProfiles_.DeleteString(itemID);
			itemID = itemID > 0 ? --itemID : 0;
			cboProfiles_.SetCurSel(itemID);
			switchProfile();
		} else {
			MessageBox(L"Unable to delete selected file.", L"Error", MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT uberstealth::UberstealthPageMisc::OnProfilesSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (configProvider_->isProfileModified()) {
		SaveChangesDialog saveChangesDlg;
		saveChangesDlg.DoModal(m_hWnd);
		switch (saveChangesDlg.getAnswer())	{
		case SaveChangesDialog::Save:
			configProvider_->triggerFlushProfileEvent();
			HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(),  getCurrentProfileName());
			break;

		case SaveChangesDialog::DontSave:
			break;

		case SaveChangesDialog::Cancel:
			return 0;
		}
	}
	switchProfile();
	return 0;
}

void uberstealth::UberstealthPageMisc::enableControls(bool enabled) {
	BOOL state = enabled ? TRUE : FALSE;
#ifdef OLLYSTEALTH
	::EnableWindow(GetDlgItem(IDC_TCP_PORT), FALSE);
	::EnableWindow(GetDlgItem(IDC_PASS_EXCEPTIONS), FALSE);
	::EnableWindow(GetDlgItem(IDC_HALT_IN_SEH), FALSE);
	::EnableWindow(GetDlgItem(IDC_HALT_AFTER_SEH), FALSE);
	::EnableWindow(GetDlgItem(IDC_LOG_SEH), FALSE);
#else
	::EnableWindow(GetDlgItem(IDC_TCP_PORT), state);
	::EnableWindow(GetDlgItem(IDC_PASS_EXCEPTIONS), state);
	::EnableWindow(GetDlgItem(IDC_HALT_IN_SEH), state);
	::EnableWindow(GetDlgItem(IDC_HALT_AFTER_SEH), state);
	::EnableWindow(GetDlgItem(IDC_LOG_SEH), state);
#endif
	::EnableWindow(GetDlgItem(IDC_AUTO_SELECTION), state);
	::EnableWindow(GetDlgItem(IDC_FORCE_ABS), state);
}

void uberstealth::UberstealthPageMisc::loadProfile(const uberstealth::HideDebuggerProfile& profile) {
	if (m_hWnd) {
		guiData_.patchingMethod_ = profile.getInlinePatchingMethodValue();
		guiData_.passExceptions_ = profile.getPassUnknownExceptionsEnabled();
		guiData_.tcpPort_ = profile.getRemoteTCPPortValue();
		guiData_.haltInSEH_ = profile.getHaltInSEHHandlerEnabled();
		guiData_.haltAfterSEH_ = profile.getHaltAfterSEHHandlerEnabled();
		guiData_.logSEH_ = profile.getLogSEHEnabled();
		DoDataExchange(FALSE);
	}
}

void uberstealth::UberstealthPageMisc::flushProfile(uberstealth::HideDebuggerProfile& profile) {
	if (m_hWnd) {
		if (DoDataExchange(TRUE)) {
			profile.setPassUnknownExceptionsEnabled(guiData_.passExceptions_);
			profile.setInlinePatchingMethodValue((InlinePatching)guiData_.patchingMethod_);
			profile.setRemoteTCPPortValue(guiData_.tcpPort_);
			profile.setHaltInSEHHandlerEnabled(guiData_.haltInSEH_);
			profile.setHaltAfterSEHHandlerEnabled(guiData_.haltAfterSEH_);
			profile.setLogSEHEnabled(guiData_.logSEH_);
		}
	}
}

BOOL uberstealth::UberstealthPageMisc::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	DoDataExchange(FALSE);
	initComboBox();

#ifdef OLLYSTEALTH
	::EnableWindow(GetDlgItem(IDC_TCP_PORT), FALSE);
	::EnableWindow(GetDlgItem(IDC_PASS_EXCEPTIONS), FALSE);
	::EnableWindow(GetDlgItem(IDC_HALT_IN_SEH), FALSE);
	::EnableWindow(GetDlgItem(IDC_HALT_AFTER_SEH), FALSE);
	::EnableWindow(GetDlgItem(IDC_LOG_SEH), FALSE);
#endif

	bHandled = FALSE;
	return TRUE;
}

void uberstealth::UberstealthPageMisc::switchProfile() {
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR) {		
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		std::string profile = std::string(CT2A(itemText, CP_UTF8));
		configProvider_->triggerSwitchProfile(profile);
		setCurrentProfileName(profile);
	}
}

void uberstealth::UberstealthPageMisc::initComboBox() {
	BOOST_FOREACH(const std::string& profile, getProfileNames()) {
		int itemID = cboProfiles_.AddString(StringToUnicode(profile));
		if (profile == getCurrentProfileName()) {
			cboProfiles_.SetCurSel(itemID);
		}
	}
}

bool uberstealth::UberstealthPageMisc::isProfileDirty() {
	PageMiscGuiData oldguiData = guiData_;
	if (m_hWnd && DoDataExchange(TRUE)) {
		return oldguiData != guiData_;
	} else {
		return false;
	}
}