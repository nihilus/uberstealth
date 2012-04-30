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
		rdbPatchingMethod_ = profile.getInlinePatchingMethodValue();
		passExceptions_ = profile.getPassUnknownExceptionsEnabled();
		tcpPort_ = profile.getRemoteTCPPortValue();
		haltInSEH_ = profile.getHaltInSEHHandlerEnabled();
		haltAfterSEH_ = profile.getHaltAfterSEHHandlerEnabled();
		logSEH_ = profile.getLogSEHEnabled();
		DoDataExchange(FALSE);
	}
}

void uberstealth::UberstealthPageMisc::flushProfile(uberstealth::HideDebuggerProfile& profile) {
	if (m_hWnd) {
		if (DoDataExchange(TRUE)) {
			profile.setPassUnknownExceptionsEnabled(passExceptions_);
			profile.setInlinePatchingMethodValue((InlinePatching)rdbPatchingMethod_);
			profile.setRemoteTCPPortValue(tcpPort_);
			profile.setHaltInSEHHandlerEnabled(haltInSEH_);
			profile.setHaltAfterSEHHandlerEnabled(haltAfterSEH_);
			profile.setLogSEHEnabled(logSEH_);
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

// Switch profile to profile selected in combobox.
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
	if (m_hWnd)	{
		return (IsDlgButtonChecked(IDC_PASS_EXCEPTIONS) == BST_CHECKED ? true : false) != passExceptions_ ||
			   (IsDlgButtonChecked(IDC_HALT_IN_SEH) == BST_CHECKED ? true : false) != haltInSEH_ ||
			   (IsDlgButtonChecked(IDC_HALT_AFTER_SEH) == BST_CHECKED ? true : false) != haltAfterSEH_ ||
			   (IsDlgButtonChecked(IDC_LOG_SEH) == BST_CHECKED ? true : false) != logSEH_ ||
			   getPatchingMethod() != rdbPatchingMethod_ ||
			   getTCPPort() != tcpPort_;
	}
	return false;
}

int uberstealth::UberstealthPageMisc::getPatchingMethod() {
	// TODO(jan.newger@newgre.net): read actual value from control.
	return rdbPatchingMethod_;
}

int uberstealth::UberstealthPageMisc::getTCPPort() {
	// TODO(jan.newger@newgre.net): read actual value from control.
	return tcpPort_;
}