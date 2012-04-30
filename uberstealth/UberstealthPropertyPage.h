// All property sheets dealing with profile configuiration have to derive from this class.

#pragma once

#include "WTLCommon.h"
#include "ConfigProvider.h"
#include <iostream>
#include "ProfileEventConsumer.h"

namespace uberstealth {

template <class baseT, int idd>
class UberstealthPropertyPage : 
	public CPropertyPageImpl<baseT>,
	public CWinDataExchange<baseT>,
	public ProfileEventConsumer {
public:
	enum { IDD = idd };

	UberstealthPropertyPage(ConfigProvider* configProvider) :
		configProvider_(configProvider),
		enableState_(false) {
		configProvider_->addListener(this);
	}

	void changeGlobalEnableState(bool enableState) {
		enableState_ = enableState;
		if (m_hWnd) {
			enableControls(enableState);
		}
	}

protected:
	BOOL OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lParam*/)
	{
		loadProfile(configProvider_->getCurrentProfile());
		enableControls(enableState_);
		return TRUE;
	}

	virtual void enableControls(bool enabled) =0;

	BEGIN_MSG_MAP(UberstealthPropertyPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<baseT>)
	END_MSG_MAP()

	ConfigProvider* configProvider_;

private:
	bool enableState_;
};

}