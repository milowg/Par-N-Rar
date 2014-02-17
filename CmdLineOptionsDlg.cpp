// CmdLineOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "CmdLineOptionsDlg.h"

extern CParNRarApp theApp;

IMPLEMENT_DYNAMIC(CmdLineOptionsDlg, CDialog)

CmdLineOptionsDlg::CmdLineOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CmdLineOptionsDlg::IDD, pParent)
{

}

CmdLineOptionsDlg::~CmdLineOptionsDlg()
{
}

void CmdLineOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CmdLineOptionsDlg, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


BOOL CmdLineOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	GetDlgItem(IDC_Text)->SetWindowText(theApp.m_sCmdLineOptions.c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
