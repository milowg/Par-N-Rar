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
#include "TextProgressCtrl.h"
#include "afxtempl.h"

// CListCtrlEx
#define IDC_PROGRESS_LIST WM_USER

class CListCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlEx)

	CArray<CProgressCtrl*, CProgressCtrl*> m_ProgressList;
	int m_ProgressColumn;
	
public:
	CListCtrlEx();
	virtual ~CListCtrlEx();
	void InitProgressColumn(int ColNum);
	int InsertItem(int nItem, LPCSTR lpszItem, int nImage);
	int InsertItem(const LVITEM* pItem);
	int InsertItem(int nItem, LPCTSTR lpszItem);
	BOOL DeleteItem(int nItem);
	
protected:
	void CreateProgressCtrl();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


