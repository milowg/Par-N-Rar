#pragma once

class CMyComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CMyComboBox)

public:
	CMyComboBox(void);
public:
	virtual ~CMyComboBox(void);

protected:
	virtual void PreSubclassWindow();

	//{{AFX_MSG(CMyComboBox)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
