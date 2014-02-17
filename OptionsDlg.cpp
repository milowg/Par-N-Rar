//  This file is part of Par-N-Rar
//  http://www.milow.net/site/projects/parnrar.html
//
//  Copyright (c) 2005-2007 Gil Milow
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
#include "ParNRar.h"
#include "OptionsDlg.h"
#include ".\optionsdlg.h"
#include "XBrowseForFolder.h"

// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CDialog)
COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_USERECYCLEBIN, chkUseRecycleBin);
	DDX_Control(pDX, IDC_CHK_USERECYCLEBIN_DIR, chkUseRecycleBinDir);
	DDX_Control(pDX, IDC_CBOPRIORITY, cboPriority);
	DDX_Control(pDX, IDC_CBODONESCAN, cboDoneScan);
	DDX_Control(pDX, IDC_CBOCHECKFORNEWVERSION, cboCheckForNewVersion);	
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE_DIR, OnBnClickedBrowseDir)
	ON_BN_CLICKED(IDC_BROWSE_DIR2, OnBnClickedBrowseDir2)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHK_DELETEWHENDONE, OnBnClickedChkDeletewhendone)
	ON_CBN_SELCHANGE(IDC_CBODONESCAN, OnDoneScanChange)
	ON_BN_CLICKED(IDC_CHK_DELETEDIR, OnBnClickedChkDeletedir)
END_MESSAGE_MAP()


void COptionsDlg::OnBnClickedBrowseDir()
{
	BrowseDirClick(IDC_EXTRACTDIR);
}

void COptionsDlg::OnBnClickedBrowseDir2()
{
	BrowseDirClick(IDC_NonRarDir);
}

void COptionsDlg::BrowseDirClick(int nId)
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


void COptionsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	cboPriority.AddString("Realtime");
	cboPriority.AddString("High");
	cboPriority.AddString("Above Normal");
	cboPriority.AddString("Normal");
	cboPriority.AddString("Below Normal");
	cboPriority.AddString("Idle");
	cboPriority.SelectString(0, theApp.m_Options.GetPriority().c_str());
	GetDlgItem(IDC_EXTRACTDIR)->SetWindowText(theApp.m_Options.GetExtractDir().c_str());
	GetDlgItem(IDC_NonRarDir)->SetWindowText(theApp.m_Options.GetNonRarDir().c_str());
	GetDlgItem(IDC_RestartDelay)->SetWindowText(theApp.m_Options.GetRestartDelay().c_str());
	
	cboDoneScan.AddString("Restart");
	cboDoneScan.AddString("Do Nothing");
	cboDoneScan.AddString("Beep");
	cboDoneScan.AddString("Shutdown System");
	cboDoneScan.AddString("Close Par-N-Rar");	
	cboDoneScan.SelectString(0, theApp.m_Options.GetDoneScan().c_str());
	OnDoneScanChange();

	SetCheckbox(IDC_CHK_DELETEWHENDONE, theApp.m_Options.GetDeleteWhenDone());
	SetCheckbox(IDC_CHK_MINSYSTRAY, theApp.m_Options.GetMinSysTray());
	DeleteOptionChanged();	
	SetCheckbox(IDC_CHK_RECURSE, theApp.m_Options.GetRecurseMonitorDir());
	SetCheckbox(IDC_CHK_MOVENONRAR, theApp.m_Options.GetMoveNonRar());
	SetCheckbox(IDC_CHK_USERECYCLEBIN, theApp.m_Options.GetUseRecycleBin());
	SetCheckbox(IDC_CHK_DELETEDIR, theApp.m_Options.GetDeleteEmptyDirs());
	SetCheckbox(IDC_CHK_USERECYCLEBIN_DIR, theApp.m_Options.GetEmptyDirsUseRecycleBin());
	DeleteDirOptionChanged();

	cboCheckForNewVersion.AddString("Weekly");
	cboCheckForNewVersion.AddString("Monthly");
	cboCheckForNewVersion.AddString("Yearly");
	cboCheckForNewVersion.AddString("Never");
	cboCheckForNewVersion.SelectString(0, theApp.m_Options.GetCheckForNewVersion().c_str());
}

void COptionsDlg::DeleteOptionChanged()
{
	chkUseRecycleBin.EnableWindow(IsDlgButtonChecked(IDC_CHK_DELETEWHENDONE));
}

void COptionsDlg::OnBnClickedOk()
{	
	//Checks
	CString cstr = "";
	GetDlgItem(IDC_RestartDelay)->GetWindowText(cstr);
	long l = atol(cstr.GetBuffer(0));
	if (l < 1 || l > 999999)
	{
		MessageBox("Restart Delay must be between 1 and 999999", "Error", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//Save options
	GetDlgItem(IDC_MONITORDIR)->GetWindowText(cstr);
	theApp.m_Options.SetExtractDir(cstr.GetBuffer(0));
	GetDlgItem(IDC_NonRarDir)->GetWindowText(cstr);
	theApp.m_Options.SetNonRarDir(cstr.GetBuffer(0));
	GetDlgItem(IDC_RestartDelay)->GetWindowText(cstr);
	theApp.m_Options.SetRestartDelay(cstr.GetBuffer(0));

	GetDlgItem(IDC_CBOPRIORITY)->GetWindowText(cstr);
	if (cstr == "Realtime")
	{
		if (MessageBox("Are you sure? This priority setting may cause your system to respond slowly!", "Are you sure?", MB_ICONWARNING | MB_YESNOCANCEL) != IDYES)
		return;
	}
	theApp.m_Options.SetPriority(cstr.GetBuffer(0));
	theApp.SetPriority();

	GetDlgItem(IDC_CBODONESCAN)->GetWindowText(cstr);
	theApp.m_Options.SetDoneScan(cstr.GetBuffer(0));

	GetDlgItem(IDC_CBOCHECKFORNEWVERSION)->GetWindowText(cstr);
	theApp.m_Options.SetCheckForNewVersion(cstr.GetBuffer(0));

	theApp.m_Options.SetDeleteWhenDone(IsDlgButtonChecked(IDC_CHK_DELETEWHENDONE));
	theApp.m_Options.SetMinSysTray(IsDlgButtonChecked(IDC_CHK_MINSYSTRAY));
	theApp.m_Options.SetRecurseMonitorDir(IsDlgButtonChecked(IDC_CHK_RECURSE));
	theApp.m_Options.SetMoveNonRar(IsDlgButtonChecked(IDC_CHK_MOVENONRAR));
	theApp.m_Options.SetUseRecycleBin(IsDlgButtonChecked(IDC_CHK_USERECYCLEBIN));
	theApp.m_Options.SetDeleteEmptyDirs(IsDlgButtonChecked(IDC_CHK_DELETEDIR));
	theApp.m_Options.SetEmptyDirsUseRecycleBin(IsDlgButtonChecked(IDC_CHK_USERECYCLEBIN_DIR));
	OnOK();
}

void COptionsDlg::SetCheckbox(int nIdButton, bool bValue)
{
	if (bValue)
		CheckDlgButton(nIdButton, BST_CHECKED);
	else
		CheckDlgButton(nIdButton, BST_UNCHECKED);
}

void COptionsDlg::OnBnClickedChkDeletewhendone()
{
	DeleteOptionChanged();
}

void COptionsDlg::OnDoneScanChange()
{
	CString cstr;
	GetDlgItem(IDC_CBODONESCAN)->GetWindowText(cstr);
	GetDlgItem(IDC_LBL_RESTARTDELAY)->EnableWindow(cstr == "Restart");
	GetDlgItem(IDC_LBL_SECONDS)->EnableWindow(cstr == "Restart");
	GetDlgItem(IDC_RestartDelay)->EnableWindow(cstr == "Restart");

}

void COptionsDlg::DeleteDirOptionChanged()
{
	chkUseRecycleBinDir.EnableWindow(IsDlgButtonChecked(IDC_CHK_DELETEDIR));
}

void COptionsDlg::OnBnClickedChkDeletedir()
{
	DeleteDirOptionChanged();
}
