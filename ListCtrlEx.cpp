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
#include "ListCtrlEx.h"

IMPLEMENT_DYNAMIC(CListCtrlEx, CListCtrl)

CListCtrlEx::CListCtrlEx() : m_ProgressColumn(0)
{
}

CListCtrlEx::~CListCtrlEx()
{
	int Count = m_ProgressList.GetSize();
	for (int i = 0; i < Count; i++)
	{
		CProgressCtrl* pCtrl = m_ProgressList.GetAt(0);
		pCtrl->DestroyWindow();
		delete pCtrl;
		m_ProgressList.RemoveAt(0);
	}
}

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CListCtrlEx::OnPaint()
{
	int i;
	int Top = GetTopIndex();
	int Total = GetItemCount();
	int PerPage = GetCountPerPage();
	int Last = ((Top+PerPage) > Total)? Total : Top+PerPage;
	CProgressCtrl* pCtrl;
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	for (i = Top; i < Last; i++)
	{
		CRect ColRt;
		pHeader->GetItemRect(m_ProgressColumn, &ColRt);

		CRect rt;
		GetItemRect(i, &rt, LVIR_LABEL);
		rt.top += 1;
		rt.bottom -= 1;
		rt.left += ColRt.left;
		rt.right = rt.left + ColRt.Width() - 4;

		ValidateRect( rt );
		pCtrl = m_ProgressList.GetAt(i-Top);
		CString strPercent = GetItemText(i, m_ProgressColumn);
		int nPercent = atoi(strPercent);
		pCtrl->SetPos(nPercent);
		pCtrl->MoveWindow(&rt);
		pCtrl->Invalidate();
	}
	CListCtrl::OnPaint();
}

int CListCtrlEx::InsertItem(int nItem, LPCSTR lpszItem, int nImage)
{
	try
	{
		CreateProgressCtrl();
		return CListCtrl::InsertItem(nItem, lpszItem, nImage);
	}catch(...)
	{
		MessageBox("Error inserting item.", "Error");
	}
}

int CListCtrlEx::InsertItem(const LVITEM* pItem)
{
	try
	{
		CreateProgressCtrl();
		return CListCtrl::InsertItem(pItem);
	}catch(...)
	{
		MessageBox("Error inserting item.", "Error");
	}
}

int CListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem)
{
	try
	{
		CreateProgressCtrl();
		return CListCtrl::InsertItem(nItem, lpszItem);
	}catch(...)
	{
		MessageBox("Error inserting item.", "Error");
	}
}

void CListCtrlEx::CreateProgressCtrl()
{
	CProgressCtrl* pCtrl = new CProgressCtrl();
	CRect rt(1,1,1,1);
	pCtrl->Create(NULL, rt, this, IDC_PROGRESS_LIST + GetItemCount() + 1);
	m_ProgressList.Add(pCtrl);
	pCtrl->ShowWindow(SW_SHOWNORMAL);
}

void CListCtrlEx::InitProgressColumn(int ColNum)
{
	m_ProgressColumn = ColNum;
}

BOOL CListCtrlEx::DeleteItem(int nItem)
{
	delete m_ProgressList.GetAt(nItem);
	m_ProgressList.RemoveAt(nItem);

	BOOL bRet = CListCtrl::DeleteItem(nItem);

	CListCtrl::Invalidate();
	return bRet;
}