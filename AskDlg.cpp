// AskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "AskDlg.h"


// AskDlg dialog

IMPLEMENT_DYNAMIC(AskDlg, CDialog)

AskDlg::AskDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AskDlg::IDD, pParent)
{
	m_bNeverAskAgain = false;
}

AskDlg::~AskDlg()
{
}

void AskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AskDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(ID_BUTTON1, &AskDlg::OnBnClickedButton1)
	ON_BN_CLICKED(ID_BUTTON2, &AskDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

void AskDlg::SetButtons(CString cstrButton1, CString cstrButton2)
{
	m_cstrButton1 = cstrButton1;
	m_cstrButton2 = cstrButton2;
}

void AskDlg::SetCaption(CString cstrCaption)
{
	m_cstrCaption = cstrCaption;
}

void AskDlg::SetQuestion(CString cstrQuestion)
{
	m_cstrQuestion = cstrQuestion;
}

void AskDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	SetWindowText(m_cstrCaption.GetBuffer(0));
	GetDlgItem(IDC_QUESTION)->SetWindowText(m_cstrQuestion.GetBuffer(0));
	GetDlgItem(ID_BUTTON1)->SetWindowText(m_cstrButton1.GetBuffer(0));
	GetDlgItem(ID_BUTTON2)->SetWindowText(m_cstrButton2.GetBuffer(0));
}

void AskDlg::OnBnClickedButton1()
{
	m_bNeverAskAgain = IsDlgButtonChecked(IDC_DONOTASK);
	m_iButtonClicked = 1;
	CDialog::OnOK();
}

void AskDlg::OnBnClickedButton2()
{
	m_bNeverAskAgain = IsDlgButtonChecked(IDC_DONOTASK);
	m_iButtonClicked = 2;
	CDialog::OnOK();
}

int AskDlg::ButtonClicked()
{
	return m_iButtonClicked;
}

bool AskDlg::NeverAskAgain()
{
	return m_bNeverAskAgain;
}