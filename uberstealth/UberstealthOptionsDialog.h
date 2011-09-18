// Manages the options dialog for the plugin, contains all tab pages and sends events regarding the profiles to the tab pages.

#pragma once

#include "WTLCommon.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <HideDebugger/HideDebuggerProfile.h>
#include "UberstealthPage1.h"
#include "UberstealthPage2.h"
#include "UberstealthDriversPage.h"
#include "UberstealthAboutPage.h"
#include "UberstealthPageMisc.h"

namespace uberstealth {

class UberstealthOptionsDialog : public CPropertySheetImpl<UberstealthOptionsDialog>
{
public:
	UberstealthOptionsDialog(LPCTSTR title, ProfileHelper* profileHelper) :
		CPropertySheetImpl(title),
		profileHelper_(profileHelper)
	{
		configProvider_ = boost::make_shared<ConfigProvider>(profileHelper);
		page1_ = boost::shared_ptr<UberstealthPage1>(new UberstealthPage1(configProvider_.get()));
		page2_ = boost::shared_ptr<UberstealthPage2>(new UberstealthPage2(configProvider_.get()));
		page3_ = boost::shared_ptr<UberstealthDriversPage>(new UberstealthDriversPage(configProvider_.get()));
		page4_ = boost::shared_ptr<UberstealthPageMisc>(new UberstealthPageMisc(configProvider_.get(), profileHelper));
		pageAbout_ = boost::shared_ptr<UberstealthAboutPage>(new UberstealthAboutPage());
		AddPage(*page1_);
		AddPage(*page2_);
		AddPage(*page3_);
		AddPage(*page4_);
		AddPage(*pageAbout_);
		m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	}

private:
	BEGIN_MSG_MAP(UberstealthOptionsDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		COMMAND_ID_HANDLER(IDOK, OnOkButton)
		CHAIN_MSG_MAP(CPropertySheetImpl<UberstealthOptionsDialog>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == TRUE) CenterWindow();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnOkButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		configProvider_->triggerFlushProfileEvent();
		HideDebuggerProfile::writeProfileByName(configProvider_->getCurrentProfile(), profileHelper_->getLastProfileFilename());
		return 0;
	}

	boost::shared_ptr<UberstealthPage1> page1_;
	boost::shared_ptr<UberstealthPage2> page2_;
	boost::shared_ptr<UberstealthDriversPage> page3_;
	boost::shared_ptr<UberstealthPageMisc> page4_;
	boost::shared_ptr<UberstealthAboutPage> pageAbout_;
	boost::shared_ptr<ConfigProvider> configProvider_;
	ProfileHelper* profileHelper_;
};

}