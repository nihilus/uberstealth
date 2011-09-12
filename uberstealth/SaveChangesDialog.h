// Ask the user if he wants to save the modified profile.

#pragma once

#include "WTLCommon.h"
#include <iostream>

namespace uberstealth {

class SaveChangesDialog : 
	public CDialogImpl<SaveChangesDialog>
{
public:
	enum { IDD = IDD_SAVE_CHANGES };
	enum SaveChanges { Save, DontSave, Cancel };

	SaveChanges getAnswer() const { return answer_; }

private:
	LRESULT OnSaveClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		answer_ = Save;
		EndDialog(0);
		return 0;
	}

	LRESULT OnDontSaveClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		answer_ = DontSave;
		EndDialog(0);
		return 0;
	}

	LRESULT OnCancelClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		answer_ = Cancel;
		EndDialog(0);
		return 0;
	}

	BOOL OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		iconControl_.Attach(GetDlgItem(IDC_QUESTION_ICON));
		iconControl_.SetIcon(LoadIcon(NULL, IDI_QUESTION));
		return TRUE;
	}

	BEGIN_MSG_MAP(SaveChangesDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_SAVE, BN_CLICKED, OnSaveClick)
		COMMAND_HANDLER(IDC_DONT_SAVE, BN_CLICKED, OnDontSaveClick)
		COMMAND_HANDLER(IDC_CANCEL, BN_CLICKED, OnCancelClick)
	END_MSG_MAP()

	CStatic iconControl_;
	SaveChanges answer_;
};

}