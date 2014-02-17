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
#include "afxcmn.h"

class DetailsDlg : public CDialog
{
public:
	DetailsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DetailsDlg();

// Dialog Data
	enum { IDD = IDD_DETAILSDLG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void Resize();
	CString m_cstrText;
	CString m_cstrCaption;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CRichEditCtrl ctlDetails;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void SetText(CString cstrText);
	void SetCaption(CString cstrCaption);
public:
	afx_msg void OnDestroy();
};
