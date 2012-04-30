#include "UberstealthDriversPage.h"

void uberstealth::UberstealthDriversPage::enableControls(bool enabled)
{
	BOOL state = enabled ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_RDTSC), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_ZERO), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_INCREASING), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_DELTA), state);
	::EnableWindow(GetDlgItem(IDC_UNLOAD_DRIVER), state);
	::EnableWindow(GetDlgItem(IDC_USE_RDTSC_NAME), state);
	::EnableWindow(GetDlgItem(IDC_USE_STEALTH_NAME), state);
	::EnableWindow(GetDlgItem(IDC_STEALTH_NAME), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_NAME), state);
	::EnableWindow(GetDlgItem(IDC_NTSIT_DRV), state);
	::EnableWindow(GetDlgItem(IDC_UNLOAD_STEALTH_DRV), state);
	::EnableWindow(GetDlgItem(IDC_NTQIP_DRV), state);

	if (!IsDlgButtonChecked(IDC_RDTSC))
	{
		updateRDBs(GetDlgItem(IDC_RDTSC));
	}
}

void uberstealth::UberstealthDriversPage::updateRDBs(HWND hWndCheckBox)
{
	CButton chkRDTSC = hWndCheckBox;
	CButton rdbZero = GetDlgItem(IDC_RDTSC_ZERO);
	CButton rdbIncr = GetDlgItem(IDC_RDTSC_INCREASING);
	CEdit rdtscDelta = GetDlgItem(IDC_RDTSC_DELTA);
	CEdit rdtscName = GetDlgItem(IDC_RDTSC_NAME);
	BOOL checked = (chkRDTSC.GetCheck()) ? TRUE : FALSE;
	rdbZero.EnableWindow(checked);
	rdbIncr.EnableWindow(checked);
	rdtscDelta.EnableWindow(checked);
	rdtscName.EnableWindow(checked);
}

LRESULT uberstealth::UberstealthDriversPage::OnRDTSCClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	updateRDBs(hWndCtl);
	return 0;
}

void uberstealth::UberstealthDriversPage::loadProfile(const uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		useRDTSC_ = profile.getRDTSCDriverLoad();
		rdbIndex_ = (profile.getRDTSCDriverMode() == constant) ? 0 : 1;
		unloadRDTSCDrv_ = profile.getRDTSCDriverUnload();
		useRDTSCName_ = profile.getRDTSCDriverUseCustomName();
		useStealthName_ = profile.getStealthDriverUseCustomName();
		rdtscName_ = profile.getRDTSCDriverCustomName().c_str();
		stealthName_ = profile.getStealthDriverCustomName().c_str();
		rdtscDelta_ = profile.getRDTSCDriverDelta();
		ntSITDrv_ = profile.getStealthDriverNtSetInformationThread();
		unloadStealthDrv_ = profile.getStealthDriverUnload();
		ntQIPDrv_ = profile.getStealthDriverNtQueryInformationProcess();
		DoDataExchange(FALSE);
	}
}

void uberstealth::UberstealthDriversPage::flushProfile(uberstealth::HideDebuggerProfile& profile)
{
	if (m_hWnd)
	{
		if (DoDataExchange(TRUE))
		{
			profile.setRDTSCDriverLoad(useRDTSC_);
			if (rdbIndex_) profile.setRDTSCDriverMode(increasing);
			else profile.setRDTSCDriverMode(constant);
			profile.setRDTSCDriverUnload(unloadRDTSCDrv_);
			profile.setRDTSCDriverDelta(rdtscDelta_);
			profile.setStealthDriverNtSetInformationThread(ntSITDrv_);
			profile.setStealthDriverUnload(unloadStealthDrv_);
			profile.setStealthDriverNtQueryInformationProcess(ntQIPDrv_);
			bool useStealthDriver = ntSITDrv_ || ntQIPDrv_;
			profile.setStealthDriverLoad(useStealthDriver);
			profile.setRDTSCDriverUseCustomName(useRDTSCName_);
			profile.setStealthDriverUseCustomName(useStealthName_);
			profile.setRDTSCDriverCustomName(std::string(CT2A(rdtscName_, CP_UTF8)));
			profile.setStealthDriverCustomName(std::string(CT2A(stealthName_, CP_UTF8)));
		}
	}
}

bool uberstealth::UberstealthDriversPage::isProfileDirty()
{
	if (m_hWnd)
	{
		return (IsDlgButtonChecked(IDC_RDTSC) == BST_CHECKED ? true : false) != useRDTSC_ ||
			   getRDTSCMode() != rdbIndex_ ||
			   getRDTSCDelta() != rdtscDelta_ ||
			   (IsDlgButtonChecked(IDC_UNLOAD_DRIVER) == BST_CHECKED ? true : false) != unloadRDTSCDrv_ ||
			   (IsDlgButtonChecked(IDC_USE_RDTSC_NAME) == BST_CHECKED ? true : false) != useRDTSCName_ ||
			   (IsDlgButtonChecked(IDC_USE_STEALTH_NAME) == BST_CHECKED ? true : false) != useStealthName_ ||
			   getRDTSCName().Compare(rdtscName_) ||
			   getStealthName().Compare(stealthName_) ||
			   (IsDlgButtonChecked(IDC_NTSIT_DRV) == BST_CHECKED ? true : false) != ntSITDrv_ ||
			   (IsDlgButtonChecked(IDC_UNLOAD_STEALTH_DRV) == BST_CHECKED ? true : false) != unloadStealthDrv_ ||
			   (IsDlgButtonChecked(IDC_NTQIP_DRV) == BST_CHECKED ? true : false) != ntQIPDrv_;	
	}
	return false;
}

int uberstealth::UberstealthDriversPage::getRDTSCMode() const
{
	// TODO(jan.newger@newgre.net): get actual index.
	return rdbIndex_;
}

int uberstealth::UberstealthDriversPage::getRDTSCDelta() const
{
	// TODO get actual rdtsc delta from control.
	return rdtscDelta_;
}

WTL::CString uberstealth::UberstealthDriversPage::getRDTSCName() const
{
	// TODO
	return rdtscName_;
}

WTL::CString uberstealth::UberstealthDriversPage::getStealthName() const
{
	// TODO
	return stealthName_;
}