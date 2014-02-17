#include "StdAfx.h"
#include "MyComboBox.h"

IMPLEMENT_DYNAMIC(CMyComboBox, CComboBox)

BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	//{{AFX_MSG_MAP(CMyComboBox)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMyComboBox::CMyComboBox(void)
{
}

CMyComboBox::~CMyComboBox(void)
{
}

void CMyComboBox::PreSubclassWindow()
{
	DragAcceptFiles(TRUE);
   __super::PreSubclassWindow();
}

void CMyComboBox::OnDropFiles(HDROP hdrop)
{
	CString Filename;

	if (hdrop) {

		int iFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
		if (iFiles > 0)
		{
			char* pFilename = Filename.GetBuffer(_MAX_PATH);
			DragQueryFile(hdrop, 0, pFilename, _MAX_PATH);

			SetWindowText(Filename);
		}
	}

	DragFinish(hdrop);
	hdrop = 0;
}