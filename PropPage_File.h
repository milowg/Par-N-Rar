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

#pragma once


// PropPage_File dialog

class PropPage_File : public CPropertyPage
{
	DECLARE_DYNCREATE(PropPage_File)

public:
	PropPage_File();
	virtual ~PropPage_File();

// Dialog Data
	enum { IDD = IDD_OPT_FILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnApply();
	virtual BOOL OnWizardFinish();

	CButton chkUseRecycleBin;
	CButton chkUseRecycleBinDir;
	
	void DeleteDirOptionChanged();
	void DeleteOptionChanged();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedChkDeletewhendone();	
	afx_msg void OnBnClickedChkDeletedir();

private:
	bool m_bInitialized;
};
