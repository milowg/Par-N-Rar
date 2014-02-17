#pragma once


// CmdLineOptionsDlg dialog

class CmdLineOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CmdLineOptionsDlg)

public:
	CmdLineOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CmdLineOptionsDlg();

// Dialog Data
	enum { IDD = IDD_CMDLINEOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
};
