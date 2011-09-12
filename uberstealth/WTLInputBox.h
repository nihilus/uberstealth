//  A simple dialog to query the user for text input.

#pragma once

#include "WTLCommon.h"
#include <iostream>
#include <string>

namespace uberstealth {

class WTLInputBox : 
	public CDialogImpl<WTLInputBox>,
	public CWinDataExchange<WTLInputBox>
{
public:
	enum { IDD = IDD_INPUTBOX };

	std::string getInput() const
	{
		if (txt_.GetLength())
		{
			return std::string(CT2A(txt_, CP_UTF8));
		}
		else return "";
	}

private:
	LRESULT OnOkClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoDataExchange(TRUE);
		EndDialog(0);
		return 0;
	}

	LRESULT OnCancelClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		txt_ = "";
		EndDialog(0);
		return 0;
	}

	BOOL OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		DoDataExchange(FALSE);
		return TRUE;
	}

	WTL::CString txt_;
	
	BEGIN_DDX_MAP(WTLInputBox)
		DDX_TEXT(IDC_EDITBOX, txt_)
	END_DDX_MAP()

	BEGIN_MSG_MAP(WTLInputBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOkClick)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancelClick)
	END_MSG_MAP()
};

}