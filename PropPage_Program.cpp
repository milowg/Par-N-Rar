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

// PropPage_Program.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "PropPage_Program.h"
#include "DialogUtils.h"

// PropPage_Program dialog

IMPLEMENT_DYNCREATE(PropPage_Program, CPropertyPage)

PropPage_Program::PropPage_Program() : CPropertyPage(PropPage_Program::IDD)
{
	m_bInitialized = false;
}

PropPage_Program::~PropPage_Program()
{
}

void PropPage_Program::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBOPRIORITY, cboPriority);
	DDX_Control(pDX, IDC_CBODONESCAN, cboDoneScan);
	DDX_Control(pDX, IDC_CBOCHECKFORNEWVERSION, cboCheckForNewVersion);	
	DDX_Control(pDX, IDC_CHK_MINSYSTRAY, chkMinSysTray);
	DDX_Control(pDX, IDC_CHK_CLOSETOTRAY, chkCloseToTray);
}


BEGIN_MESSAGE_MAP(PropPage_Program, CPropertyPage)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_CBODONESCAN, OnDoneScanChange)
	ON_BN_CLICKED(IDC_CHK_MINSYSTRAY, OnBnClickedChkMinSysTray)
END_MESSAGE_MAP()


BOOL PropPage_Program::OnWizardFinish()
{
	//Checks
	CString cstr = "";
	GetDlgItem(IDC_RestartDelay)->GetWindowText(cstr);
	long l = atol(cstr.GetBuffer(0));
	if (l < 1 || l > 999999)
	{
		MessageBox("Restart Delay must be between 1 and 999999", "Error", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	GetDlgItem(IDC_CBOPRIORITY)->GetWindowText(cstr);
	if (cstr == "Realtime")
	{
		if (MessageBox("Are you sure? This priority setting may cause your system to respond slowly!", "Are you sure?", MB_ICONWARNING | MB_YESNOCANCEL) != IDYES)
			return FALSE;
	}

	return TRUE;
}

BOOL PropPage_Program::OnApply()
{
	CString cstr = "";

	GetDlgItem(IDC_RestartDelay)->GetWindowText(cstr);
	theApp.m_Options.SetRestartDelay(cstr.GetBuffer(0));

	GetDlgItem(IDC_CBOPRIORITY)->GetWindowText(cstr);
	theApp.m_Options.SetPriority(cstr.GetBuffer(0));
	theApp.SetPriority();

	GetDlgItem(IDC_CBODONESCAN)->GetWindowText(cstr);
	theApp.m_Options.SetDoneScan(cstr.GetBuffer(0));

	GetDlgItem(IDC_CBOCHECKFORNEWVERSION)->GetWindowText(cstr);
	theApp.m_Options.SetCheckForNewVersion(cstr.GetBuffer(0));

	theApp.m_Options.SetMinSysTray(IsDlgButtonChecked(IDC_CHK_MINSYSTRAY));
	theApp.m_Options.SetCloseToTray(IsDlgButtonChecked(IDC_CHK_CLOSETOTRAY));
	theApp.m_Options.SetKeepWorkItemInFocus(IsDlgButtonChecked(IDC_CHK_WRKITEMINFOCUS));
	return TRUE;
}

void PropPage_Program::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	if (m_bInitialized)
		return;
	
	cboPriority.AddString("Realtime");
	cboPriority.AddString("High");
	cboPriority.AddString("Above Normal");
	cboPriority.AddString("Normal");
	cboPriority.AddString("Below Normal");
	cboPriority.AddString("Idle");
	cboPriority.SelectString(0, theApp.m_Options.GetPriority().c_str());
	GetDlgItem(IDC_RestartDelay)->SetWindowText(theApp.m_Options.GetRestartDelay().c_str());
	
	cboDoneScan.AddString("Restart");
	cboDoneScan.AddString("Do Nothing");
	cboDoneScan.AddString("Beep");
	cboDoneScan.AddString("Shutdown System");
	cboDoneScan.AddString("Close Par-N-Rar");	
	cboDoneScan.SelectString(0, theApp.m_Options.GetDoneScan().c_str());
	OnDoneScanChange();

	CDialogUtils::SetCheckbox(this, IDC_CHK_MINSYSTRAY, theApp.m_Options.GetMinSysTray());
	MinSysTrayOptionChanged();
	CDialogUtils::SetCheckbox(this, IDC_CHK_CLOSETOTRAY, theApp.m_Options.GetCloseToTray());

	cboCheckForNewVersion.AddString("Weekly");
	cboCheckForNewVersion.AddString("Monthly");
	cboCheckForNewVersion.AddString("Yearly");
	cboCheckForNewVersion.AddString("Never");
	cboCheckForNewVersion.SelectString(0, theApp.m_Options.GetCheckForNewVersion().c_str());

	CDialogUtils::SetCheckbox(this, IDC_CHK_WRKITEMINFOCUS, theApp.m_Options.GetKeepWorkItemInFocus());

	m_bInitialized = true;
}

void PropPage_Program::OnDoneScanChange()
{
	CString cstr;
	GetDlgItem(IDC_CBODONESCAN)->GetWindowText(cstr);
	GetDlgItem(IDC_LBL_RESTARTDELAY)->EnableWindow(cstr == "Restart");
	GetDlgItem(IDC_LBL_SECONDS)->EnableWindow(cstr == "Restart");
	GetDlgItem(IDC_RestartDelay)->EnableWindow(cstr == "Restart");
}

void PropPage_Program::OnBnClickedChkMinSysTray()
{
	MinSysTrayOptionChanged();
}

void PropPage_Program::MinSysTrayOptionChanged()
{
	chkCloseToTray.EnableWindow(IsDlgButtonChecked(IDC_CHK_MINSYSTRAY));
}