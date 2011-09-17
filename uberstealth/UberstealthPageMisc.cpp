#include <boost/foreach.hpp>
#include "UberstealthOptionsDialog.h"
#include "UberstealthPageMisc.h"
#include <HideDebugger/HideDebuggerProfile.h>
#include "SaveChangesDialog.h"
#include <common/StringHelper.h>
#include "WTLInputBox.h"

LRESULT uberstealth::UberstealthPageMisc::OnAddProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WTLInputBox inputBox;
	inputBox.DoModal(m_hWnd);
	std::string input = inputBox.getInput();
	if (input.length())
	{
		int item = cboProfiles_.AddString(uberstealth::StringToUnicode(input));
		cboProfiles_.SetCurSel(item);
		configProvider_->triggerSaveEvent();

		// Switch profile and save current dialogs to the new profile - leave dialog settings intact
		// TODO: re-think the workflow of managing profiles in the GUI!
		profileHelper_->setLastProfileFilename(input);
		configProvider_->triggerSaveEvent();
	}
	return 0;
}

LRESULT uberstealth::UberstealthPageMisc::OnDelProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR)
	{
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		if (uberstealth::HideDebuggerProfile::deleteProfileByName(std::string(CT2A(itemText, CP_UTF8))))
		{
			cboProfiles_.DeleteString(itemID);
			itemID = itemID > 0 ? --itemID : 0;
			cboProfiles_.SetCurSel(itemID);
			switchProfile();
		}
		else
		{
			MessageBox(L"Unable to delete selected file.", L"Error", MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT uberstealth::UberstealthPageMisc::OnProfilesSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (configProvider_->isProfileModified())
	{
		SaveChangesDialog saveChangesDlg;
		saveChangesDlg.DoModal(m_hWnd);
		switch (saveChangesDlg.getAnswer())
		{
		case SaveChangesDialog::Save:
			configProvider_->triggerSaveEvent();
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

void uberstealth::UberstealthPageMisc::enableControls(bool enabled)
{
	BOOL state = enabled ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_PASS_EXCEPTIONS), state);
	::EnableWindow(GetDlgItem(IDC_AUTO_SELECTION), state);
	::EnableWindow(GetDlgItem(IDC_FORCE_ABS), state);
	::EnableWindow(GetDlgItem(IDC_TCP_PORT), state);
	::EnableWindow(GetDlgItem(IDC_HALT_IN_SEH), state);
	::EnableWindow(GetDlgItem(IDC_HALT_AFTER_SEH), state);
	::EnableWindow(GetDlgItem(IDC_LOG_SEH), state);
}

void uberstealth::UberstealthPageMisc::loadProfile(const uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		rdbPatchingMethod_ = profile.getInlinePatchingMethodValue();
		passExceptions_ = profile.getPassUnknownExceptionsEnabled();
		tcpPort_ = profile.getRemoteTCPPortValue();
		haltInSEH_ = profile.getHaltInSEHHandlerEnabled();
		haltAfterSEH_ = profile.getHaltAfterSEHHandlerEnabled();
		logSEH_ = profile.getLogSEHEnabled();
		DoDataExchange(FALSE);
	}
}

bool uberstealth::UberstealthPageMisc::saveProfile(uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		if (DoDataExchange(TRUE))
		{
			profile.setPassUnknownExceptionsEnabled(passExceptions_);
			profile.setInlinePatchingMethodValue((InlinePatching)rdbPatchingMethod_);
			profile.setRemoteTCPPortValue(tcpPort_);
			profile.setHaltInSEHHandlerEnabled(haltInSEH_);
			profile.setHaltAfterSEHHandlerEnabled(haltAfterSEH_);
			profile.setLogSEHEnabled(logSEH_);
			return true;
		}
		else return false;
	}
	return true;
}

BOOL uberstealth::UberstealthPageMisc::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	DoDataExchange(FALSE);
	initComboBox();
	bHandled = FALSE;
	return TRUE;
}

// Switch profile to profile selected in combobox.
void uberstealth::UberstealthPageMisc::switchProfile()
{
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR)
	{		
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		std::string profile = std::string(CT2A(itemText, CP_UTF8));
		configProvider_->triggerSwitchProfile(profile);
		profileHelper_->setLastProfileFilename(profile);
	}
}

void uberstealth::UberstealthPageMisc::initComboBox()
{
	BOOST_FOREACH(const std::string& profile, uberstealth::HideDebuggerProfile::getProfiles())
	{
		int itemID = cboProfiles_.AddString(StringToUnicode(profile));
		if (profile == profileHelper_->getLastProfileFilename())
		{
			cboProfiles_.SetCurSel(itemID);
		}
	}
}

bool uberstealth::UberstealthPageMisc::isProfileDirty()
{
	if (m_hWnd)
	{
		return (IsDlgButtonChecked(IDC_PASS_EXCEPTIONS) == BST_CHECKED ? true : false) != passExceptions_ ||
			   (IsDlgButtonChecked(IDC_HALT_IN_SEH) == BST_CHECKED ? true : false) != haltInSEH_ ||
			   (IsDlgButtonChecked(IDC_HALT_AFTER_SEH) == BST_CHECKED ? true : false) != haltAfterSEH_ ||
			   (IsDlgButtonChecked(IDC_LOG_SEH) == BST_CHECKED ? true : false) != logSEH_ ||
			   getPatchingMethod() != rdbPatchingMethod_ ||
			   getTCPPort() != tcpPort_;
	}
	return false;
}

int uberstealth::UberstealthPageMisc::getPatchingMethod()
{
	// TODO: read actual value from control.
	return rdbPatchingMethod_;
}

int uberstealth::UberstealthPageMisc::getTCPPort()
{
	// TODO: read actual value from control.
	return tcpPort_;
}
