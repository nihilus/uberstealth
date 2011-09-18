#include "UberstealthPage1.h"
#include "UberstealthOptionsDialog.h"

LRESULT uberstealth::UberstealthPage1::OnDbgEnableClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool newState = IsDlgButtonChecked(IDC_DBGSTART) || IsDlgButtonChecked(IDC_DBGATTACH);
	if (newState != oldState_)
	{
		configProvider_->triggerChangeGlobalEnableState(newState);
	}
	oldState_ = newState;
	return 0;
}

void uberstealth::UberstealthPage1::OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
{
	if (nCtrlID == IDC_TICK_DELTA)
	{
		MessageBox(L"Please enter a valid tick count delta!", L"uberstealth", MB_ICONWARNING);
		::SetFocus(GetDlgItem(IDC_TICK_DELTA));
	}
}

void uberstealth::UberstealthPage1::enableControls(bool enabled)
{
	BOOL state = enabled ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_NTQO), state);
	::EnableWindow(GetDlgItem(IDC_RTLNTGF), state);
	::EnableWindow(GetDlgItem(IDC_NTQSI), state);
	::EnableWindow(GetDlgItem(IDC_NTQIP), state);
	::EnableWindow(GetDlgItem(IDC_GETTICKCOUNT), state);
	::EnableWindow(GetDlgItem(IDC_PROTECTDRS), state);
	::EnableWindow(GetDlgItem(IDC_GETVERSION), state);
	::EnableWindow(GetDlgItem(IDC_NTCLOSE), state);
	::EnableWindow(GetDlgItem(IDC_DBGPRESENT), state);
	::EnableWindow(GetDlgItem(IDC_NTGF), state);
	::EnableWindow(GetDlgItem(IDC_NTHF), state);
	::EnableWindow(GetDlgItem(IDC_TICK_DELTA), state);
}

void uberstealth::UberstealthPage1::loadProfile(const uberstealth::HideDebuggerProfile& profile)
{
	ntQueryObj_ = profile.getNtQueryObjectEnabled();
	rtlGetFlags_ = profile.getRtlGetNtGlobalFlagsEnabled();
	ntQuerySysInfo_ = profile.getNtQuerySystemInformationEnabled();
	ntQueryInfoProc_ = profile.getNtQueryInformationProcessEnabled();
	getTickCount_ = profile.getGetTickCountEnabled();
	protectDRs_ = profile.getProtectDebugRegistersEnabled();
	getVersion_ = profile.getGetVersionEnabled();
	ntClose_ = profile.getNtCloseEnabled();
	dbgPresent_ = profile.getPEBIsBeingDebuggedEnabled();
	ntGF_ = profile.getNtGlobalFlagEnabled();
	ntHF_ = profile.getHeapFlagsEnabled();
	tickDelta_ = profile.getGetTickCountDeltaValue();
	dbgAttach_ = profile.getEnableDbgAttachEnabled();
	dbgStart_ = profile.getEnableDbgStartEnabled();
	oldState_ = dbgStart_ || dbgAttach_;
	DoDataExchange(FALSE);
	configProvider_->triggerChangeGlobalEnableState(dbgStart_ || dbgAttach_);
}

void uberstealth::UberstealthPage1::flushProfile(uberstealth::HideDebuggerProfile& profile)
{
	if (DoDataExchange(TRUE))
	{
		profile.setNtQueryObjectEnabled(ntQueryObj_);
		profile.setRtlGetNtGlobalFlagsEnabled(rtlGetFlags_);
		profile.setNtQuerySystemInformationEnabled(ntQuerySysInfo_);
		profile.setNtQueryInformationProcessEnabled(ntQueryInfoProc_);
		profile.setGetTickCountEnabled(getTickCount_);
		profile.setProtectDebugRegistersEnabled(protectDRs_);
		profile.setGetVersionEnabled(getVersion_);
		profile.setNtCloseEnabled(ntClose_);
		profile.setPEBIsBeingDebuggedEnabled(dbgPresent_);
		profile.setNtGlobalFlagEnabled(ntGF_);
		profile.setHeapFlagsEnabled(ntHF_);
		profile.setGetTickCountDeltaValue(tickDelta_);
		profile.setEnableDbgAttachEnabled(dbgAttach_);
		profile.setEnableDbgStartEnabled(dbgStart_);
		ddxError_ = false;
	}
	else
	{
		ddxError_ = true;
	}
}

int uberstealth::UberstealthPage1::getTickCountDelta()
{
	// TODO: return acutal value from control.
	return tickDelta_;
}

bool uberstealth::UberstealthPage1::isProfileDirty()
{
	if (m_hWnd)
	{
		return (IsDlgButtonChecked(IDC_NTQO) == BST_CHECKED ? true : false) != ntQueryObj_ ||
			   (IsDlgButtonChecked(IDC_RTLNTGF) == BST_CHECKED ? true : false) != rtlGetFlags_ ||
			   (IsDlgButtonChecked(IDC_NTQSI) == BST_CHECKED ? true : false) != ntQuerySysInfo_ ||
			   (IsDlgButtonChecked(IDC_PROTECTDRS) == BST_CHECKED ? true : false) != protectDRs_ ||
			   (IsDlgButtonChecked(IDC_GETVERSION) == BST_CHECKED ? true : false) != getVersion_ ||
			   (IsDlgButtonChecked(IDC_NTCLOSE) == BST_CHECKED ? true : false) != ntClose_ ||
			   (IsDlgButtonChecked(IDC_DBGPRESENT) == BST_CHECKED ? true : false) != dbgPresent_ ||
			   (IsDlgButtonChecked(IDC_NTGF) == BST_CHECKED ? true : false) != ntGF_ ||
			   (IsDlgButtonChecked(IDC_NTHF) == BST_CHECKED ? true : false) != ntHF_ ||
			   (IsDlgButtonChecked(IDC_GETTICKCOUNT) == BST_CHECKED ? true : false) != getTickCount_ ||
			   getTickCountDelta() != tickDelta_ ||
			   (IsDlgButtonChecked(IDC_DBGATTACH) == BST_CHECKED ? true : false) != dbgAttach_ ||
			   (IsDlgButtonChecked(IDC_DBGSTART) == BST_CHECKED ? true : false) != dbgStart_;
	}
	return false;
}
