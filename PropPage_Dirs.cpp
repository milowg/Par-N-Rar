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

// PropPage_Dirs.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "PropPage_Dirs.h"
#include "XBrowseForFolder.h"
#include "DialogUtils.h"

// PropPage_Dirs dialog

IMPLEMENT_DYNCREATE(PropPage_Dirs, CPropertyPage) 

PropPage_Dirs::PropPage_Dirs()
	: CPropertyPage(PropPage_Dirs::IDD)
{
	m_bInitialized = false;
}

PropPage_Dirs::~PropPage_Dirs()
{
}

void PropPage_Dirs::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PropPage_Dirs, CPropertyPage)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BROWSE_DIR, OnBnClickedBrowseDir)
	ON_BN_CLICKED(IDC_BROWSE_DIR2, OnBnClickedBrowseDir2)
END_MESSAGE_MAP()

BOOL PropPage_Dirs::OnWizardFinish()
{
	return TRUE;
}

BOOL PropPage_Dirs::OnApply()
{
	CString cstr = "";

	GetDlgItem(IDC_MONITORDIR)->GetWindowText(cstr);
	theApp.m_Options.SetExtractDir(cstr.GetBuffer(0));
	GetDlgItem(IDC_NonRarDir)->GetWindowText(cstr);
	theApp.m_Options.SetNonRarDir(cstr.GetBuffer(0));
	return TRUE;
}

void PropPage_Dirs::OnBnClickedBrowseDir()
{
	BrowseDirClick(IDC_EXTRACTDIR);
}

void PropPage_Dirs::OnBnClickedBrowseDir2()
{
	BrowseDirClick(IDC_NonRarDir);
}

void PropPage_Dirs::BrowseDirClick(int nId)
{
	CString cstrCurFolder = "";
	GetDlgItem(nId)->GetWindowText(cstrCurFolder);
	cstrCurFolder.Replace("%s", "");
	cstrCurFolder.Replace("%d", "");
	cstrCurFolder.Replace("%m", "");
	auto_ptr<char> apCurFolder( new char[cstrCurFolder.GetLength() + 1] );
	char *szCurFolder = apCurFolder.get();
	szCurFolder[cstrCurFolder.GetLength()] = 0;

	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = _T('\0');

	if (cstrCurFolder != "")
		strcpy(szCurFolder, cstrCurFolder.GetBuffer(0));

	BOOL bRet = XBrowseForFolder(m_hWnd, szCurFolder, szFolder, sizeof(szFolder)/sizeof(TCHAR)-2);

	if (bRet)
		GetDlgItem(nId)->SetWindowText(szFolder);
}

void PropPage_Dirs::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	if (m_bInitialized)
		return;	

	GetDlgItem(IDC_EXTRACTDIR)->SetWindowText(theApp.m_Options.GetExtractDir().c_str());
	GetDlgItem(IDC_NonRarDir)->SetWindowText(theApp.m_Options.GetNonRarDir().c_str());

	m_bInitialized = true;
}
