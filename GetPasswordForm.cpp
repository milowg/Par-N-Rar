//  This file is part of Par-N-Rar
//  http://www.milow.net/site/projects/parnrar.html
//
//  Copyright (c) Gil Milow
//
//  Par-N-Rar is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  Par-N-Rar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
//  This code may not be used to develop a RAR (WinRAR) compatible archiver.
//

#include "stdafx.h"
#include "GetPasswordForm.h"
#include "ParNRar.h"
#include <string>
using namespace std;

extern CParNRarApp theApp;

CGetPasswordDlg::CGetPasswordDlg(CWnd *pParentWnd) : CDialog(CGetPasswordDlg::IDD, pParentWnd)
{
}

CGetPasswordDlg::CGetPasswordDlg() : CDialog(CGetPasswordDlg::IDD)
{	
}

void CGetPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetPasswordDlg)
	DDX_Control(pDX, IDC_PASSWORD, m_ceditPassword);
	//}}AFX_DATA_MAP

	m_ceditPassword.GetWindowText(m_cstrPassword);
}

BEGIN_MESSAGE_MAP(CGetPasswordDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CGetPasswordDlg::SetLimit(int iLimit)
{
	m_iLimit = iLimit;
}

void CGetPasswordDlg::SetPrompt(CString cstrPrompt)
{
	m_cstrPrompt = cstrPrompt;
}

void CGetPasswordDlg::CheckSecondsLeft()
{
	CString cstr;
	m_iSecondsLeft--;
	cstr.Format("OK (%d)", m_iSecondsLeft);

	GetDlgItem(IDOK)->SetWindowText(cstr);
	if (m_iSecondsLeft == 0)
	{
		KillTimer(2);
		OnCancel();
	}
}

BOOL CGetPasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();	

	m_ceditPassword.SetLimitText(m_iLimit);
	GetDlgItem(IDC_QUESTION)->SetWindowText(m_cstrPrompt.GetBuffer(0));

	SetTimer(2, 1000, NULL); 
	m_iSecondsLeft = 30;
	CheckSecondsLeft();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGetPasswordDlg::OnTimer(UINT nIDEvent)
{
	CheckSecondsLeft();
}