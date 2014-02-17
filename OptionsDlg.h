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

#pragma once
#include "afxwin.h"


// COptionsDlg dialog

class COptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionsDlg();

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void SetCheckbox(int nIdButton, bool bValue);
	void DeleteOptionChanged();
	void DeleteDirOptionChanged();
	void BrowseDirClick(int nId);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBrowseDir();
	afx_msg void OnBnClickedBrowseDir2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedOk();
	afx_msg void OnDoneScanChange();
	CButton chkUseRecycleBin;
	CButton chkUseRecycleBinDir;
	afx_msg void OnBnClickedChkDeletewhendone();
protected:
	CComboBox cboPriority;
	CComboBox cboDoneScan;
	CComboBox cboCheckForNewVersion;
public:
	afx_msg void OnBnClickedChkDeletedir();
};
