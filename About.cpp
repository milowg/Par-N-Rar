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
#include "About.h"
#include "ParNRar.h"
#include <string>
#include "NewVersionDlg.h"

using namespace std;

extern CParNRarApp theApp;

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_LINK, m_lnkAbout);
	DDX_Control(pDX, IDC_CHECKFORNEWVERSION, m_lnkCheckForNewVersion);
	//}}AFX_DATA_MAP

}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


void OnClickCheckForNewVersion()
{
	NewVersionDlg dlg;
	dlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	string sAboutText = "Par-N-Rar v";
	sAboutText += theApp.GetMyVersion().GetBuffer(0);
	sAboutText += "\n\nCopyright (c) Gil Milow\n";
	sAboutText += "Par-N-Rar is distributed in the hope that it will be useful,\n";
	sAboutText += "but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
	sAboutText += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n";
	sAboutText += "GNU General Public License for more details.\n\n";
	GetDlgItem(IDC_ABOUT)->SetWindowText(sAboutText.c_str());

	m_lnkAbout.SetURL(_T("http://www.milow.net/site/projects/parnrar.html"));	
	m_lnkCheckForNewVersion.SetCall(&OnClickCheckForNewVersion);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
