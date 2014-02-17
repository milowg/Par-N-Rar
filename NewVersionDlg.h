#pragma once


// NewVersionDlg dialog

class NewVersionDlg : public CDialog
{
	DECLARE_DYNAMIC(NewVersionDlg)

public:
	NewVersionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NewVersionDlg();

// Dialog Data
	enum { IDD = IDD_NEWVERSION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void Resize();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CRichEditCtrl ctlDetails;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
