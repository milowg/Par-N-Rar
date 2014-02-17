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

// PropPage_File.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "PropPage_File.h"
#include "DialogUtils.h"

// PropPage_File dialog

IMPLEMENT_DYNCREATE(PropPage_File, CPropertyPage)

PropPage_File::PropPage_File() : CPropertyPage(PropPage_File::IDD)
{
	m_bInitialized = false;
}

PropPage_File::~PropPage_File()
{
}

void PropPage_File::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_USERECYCLEBIN, chkUseRecycleBin);
	DDX_Control(pDX, IDC_CHK_USERECYCLEBIN_DIR, chkUseRecycleBinDir);
}


BEGIN_MESSAGE_MAP(PropPage_File, CPropertyPage)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHK_DELETEWHENDONE, OnBnClickedChkDeletewhendone)	
	ON_BN_CLICKED(IDC_CHK_DELETEDIR, OnBnClickedChkDeletedir)
END_MESSAGE_MAP()

BOOL PropPage_File::OnWizardFinish()
{
	return TRUE;
}

BOOL PropPage_File::OnApply()
{
	theApp.m_Options.SetDeleteWhenDone(IsDlgButtonChecked(IDC_CHK_DELETEWHENDONE));	
	theApp.m_Options.SetRecurseMonitorDir(IsDlgButtonChecked(IDC_CHK_RECURSE));
	theApp.m_Options.SetMoveNonRar(IsDlgButtonChecked(IDC_CHK_MOVENONRAR));
	theApp.m_Options.SetUseRecycleBin(IsDlgButtonChecked(IDC_CHK_USERECYCLEBIN));
	theApp.m_Options.SetDeleteEmptyDirs(IsDlgButtonChecked(IDC_CHK_DELETEDIR));
	theApp.m_Options.SetEmptyDirsUseRecycleBin(IsDlgButtonChecked(IDC_CHK_USERECYCLEBIN_DIR));

	return TRUE;
}

void PropPage_File::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	if (m_bInitialized)
		return;

	CDialogUtils::SetCheckbox(this, IDC_CHK_DELETEWHENDONE, theApp.m_Options.GetDeleteWhenDone());
	CDialogUtils::SetCheckbox(this, IDC_CHK_MINSYSTRAY, theApp.m_Options.GetMinSysTray());
	DeleteOptionChanged();	
	CDialogUtils::SetCheckbox(this, IDC_CHK_RECURSE, theApp.m_Options.GetRecurseMonitorDir());
	CDialogUtils::SetCheckbox(this, IDC_CHK_MOVENONRAR, theApp.m_Options.GetMoveNonRar());
	CDialogUtils::SetCheckbox(this, IDC_CHK_USERECYCLEBIN, theApp.m_Options.GetUseRecycleBin());
	CDialogUtils::SetCheckbox(this, IDC_CHK_DELETEDIR, theApp.m_Options.GetDeleteEmptyDirs());
	CDialogUtils::SetCheckbox(this, IDC_CHK_USERECYCLEBIN_DIR, theApp.m_Options.GetEmptyDirsUseRecycleBin());
	DeleteDirOptionChanged();
	m_bInitialized = true;
}

void PropPage_File::DeleteOptionChanged()
{
	chkUseRecycleBin.EnableWindow(IsDlgButtonChecked(IDC_CHK_DELETEWHENDONE));
}

void PropPage_File::DeleteDirOptionChanged()
{
	chkUseRecycleBinDir.EnableWindow(IsDlgButtonChecked(IDC_CHK_DELETEDIR));
}

void PropPage_File::OnBnClickedChkDeletewhendone()
{
	DeleteOptionChanged();
}

void PropPage_File::OnBnClickedChkDeletedir()
{
	DeleteDirOptionChanged();
}
