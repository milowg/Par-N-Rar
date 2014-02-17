#pragma once


// AskDlg dialog

class AskDlg : public CDialog
{
	DECLARE_DYNAMIC(AskDlg)

public:
	AskDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~AskDlg();

// Dialog Data
	enum { IDD = IDD_DLG_ASK };

	void SetQuestion(CString cstrQuestion);
	void SetButtons(CString cstrButton1, CString cstrButton2);
	void SetCaption(CString cstrCaption);
	bool NeverAskAgain();
	int ButtonClicked();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);	
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	CString m_cstrCaption;
	CString m_cstrQuestion;
	CString m_cstrButton1;
	CString m_cstrButton2;
	bool m_bNeverAskAgain;
	int m_iButtonClicked;
};
