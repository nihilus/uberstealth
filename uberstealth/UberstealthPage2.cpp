// Represents the second page of the options dialog.

#include "UberstealthPage2.h"

void uberstealth::UberstealthPage2::loadProfile(const uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		blockInput_ = profile.getBlockInputEnabled();
		suspThread_ = profile.getSuspendThreadEnabled();
		ntTerminate_ = profile.getNtTerminateEnabled();
		parentProcess_ = profile.getFakeParentProcessEnabled();
		hideIDAProcess_ = profile.getHideDebuggerProcessEnabled();
		hideIDAWnd_ = profile.getHideDebuggerWindowsEnabled();
		dbgPrintExcp_ = profile.getDbgPrintExceptionEnabled();
		openProcess_ = profile.getOpenProcessEnabled();
		switch_ = profile.getSwitchDesktopEnabled();
		antiAttach_ = profile.getKillAntiAttachEnabled();
		ntYield_ = profile.getNtYieldExecutionEnabled();
		outputDbgStr_ = profile.getOutputDbgStringEnabled();
		ntSetInfoThread_ = profile.getNtSetInformationThreadEnabled();
		DoDataExchange(FALSE);
	}
}

bool uberstealth::UberstealthPage2::saveProfile(uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		if (DoDataExchange(TRUE))
		{
			profile.setBlockInputEnabled(blockInput_);
			profile.setSuspendThreadEnabled(suspThread_);
			profile.setNtTerminateEnabled(ntTerminate_);
			profile.setFakeParentProcessEnabled(parentProcess_);
			profile.setHideDebuggerProcessEnabled(hideIDAProcess_);
			profile.setHideDebuggerWindowsEnabled(hideIDAWnd_);
			profile.setDbgPrintExceptionEnabled(dbgPrintExcp_);
			profile.setOpenProcessEnabled(openProcess_);
			profile.setSwitchDesktopEnabled(switch_);
			profile.setKillAntiAttachEnabled(antiAttach_);
			profile.setNtYieldExecutionEnabled(ntYield_);
			profile.setOutputDbgStringEnabled(outputDbgStr_);
			profile.setNtSetInformationThreadEnabled(ntSetInfoThread_);
			return true;
		}
		else return false;
	}
	return true;
}

void uberstealth::UberstealthPage2::enableControls(bool enabled)
{
	BOOL state = enabled ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_BLOCKINPUT), state);
	::EnableWindow(GetDlgItem(IDC_SUSPENDTHREAD), state);
	::EnableWindow(GetDlgItem(IDC_TERMINATE), state);
	::EnableWindow(GetDlgItem(IDC_PARENTPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_HIDEIDAPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_HIDEIDAWND), state);
	::EnableWindow(GetDlgItem(IDC_DBGPRNTEXCEPT), state);
	::EnableWindow(GetDlgItem(IDC_OPENPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_SWITCH), state);
	::EnableWindow(GetDlgItem(IDC_ANTIATTACH), state);
	::EnableWindow(GetDlgItem(IDC_NTYIELD), state);
	::EnableWindow(GetDlgItem(IDC_OUTDBGSTR), state);
	::EnableWindow(GetDlgItem(IDC_NTSIT), state);
}

bool uberstealth::UberstealthPage2::isProfileDirty()
{
	if (m_hWnd)
	{
		return (IsDlgButtonChecked(IDC_BLOCKINPUT) == BST_CHECKED ? true : false) != blockInput_ ||
			(IsDlgButtonChecked(IDC_SUSPENDTHREAD) == BST_CHECKED ? true : false) != suspThread_ ||
			(IsDlgButtonChecked(IDC_TERMINATE) == BST_CHECKED ? true : false) != ntTerminate_ ||
			(IsDlgButtonChecked(IDC_PARENTPROCESS) == BST_CHECKED ? true : false) != parentProcess_ ||
			(IsDlgButtonChecked(IDC_HIDEIDAPROCESS) == BST_CHECKED ? true : false) != hideIDAProcess_ ||
			(IsDlgButtonChecked(IDC_HIDEIDAWND) == BST_CHECKED ? true : false) != hideIDAWnd_ ||
			(IsDlgButtonChecked(IDC_DBGPRNTEXCEPT) == BST_CHECKED ? true : false) != dbgPrintExcp_ ||
			(IsDlgButtonChecked(IDC_OPENPROCESS) == BST_CHECKED ? true : false) != openProcess_ ||
			(IsDlgButtonChecked(IDC_SWITCH) == BST_CHECKED ? true : false) != switch_ ||
			(IsDlgButtonChecked(IDC_ANTIATTACH) == BST_CHECKED ? true : false) != antiAttach_ ||
			(IsDlgButtonChecked(IDC_NTYIELD) == BST_CHECKED ? true : false) != ntYield_ ||
			(IsDlgButtonChecked(IDC_OUTDBGSTR) == BST_CHECKED ? true : false) != outputDbgStr_ ||
			(IsDlgButtonChecked(IDC_NTSIT) == BST_CHECKED ? true : false) != ntSetInfoThread_ ;	
	}
	return false;
}