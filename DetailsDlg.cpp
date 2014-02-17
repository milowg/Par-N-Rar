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
#include "ParNRar.h"
#include "DetailsDlg.h"
#include ".\detailsdlg.h"

extern CParNRarApp theApp;

DetailsDlg::DetailsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DetailsDlg::IDD)
{
}

DetailsDlg::~DetailsDlg()
{
}

void DetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DETAILS, ctlDetails);
}

BOOL DetailsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(DetailsDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// DetailsDlg message handlers

void DetailsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return;
	Resize();
}

void DetailsDlg::Resize()
{
	CRect rectForm, rectButton;
    
	GetClientRect(&rectForm);
	GetDlgItem(IDOK)->GetClientRect(rectButton);
	GetDlgItem(IDOK)->MoveWindow(rectForm.Width() - rectButton.Width() - 2, rectForm.Height() - rectButton.Height() - 2, rectButton.Width(), rectButton.Height(), TRUE);	
	ctlDetails.MoveWindow(0, 0, rectForm.Width(), rectForm.Height() - rectButton.Height() - 5, TRUE);
}

void DetailsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	//Load window settings	
	CRect rectForm;
	DWORD dw;
	GetWindowRect(&rectForm);
	dw = theApp.m_Options.GetDwordOption("DetailsWidth", -1);
	if (dw != -1)
	{
		rectForm.right = rectForm.left + dw;
		dw = theApp.m_Options.GetDwordOption("DetailsHeight", -1);
		if (dw != -1)
		{
			rectForm.bottom = rectForm.top + dw;
		}
		MoveWindow(&rectForm);
	}

	int iTotalTextLength = ctlDetails.GetWindowTextLength();
	ctlDetails.SetSel(iTotalTextLength, iTotalTextLength);
	ctlDetails.ReplaceSel(m_cstrText);
	SetWindowText(m_cstrCaption.GetBuffer(0));

	Resize();
}

void DetailsDlg::SetCaption(CString cstrCaption)
{
	m_cstrCaption = cstrCaption;
}

void DetailsDlg::SetText(CString cstrText)
{
	m_cstrText = cstrText;

	/*
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	ctlLog.GetScrollInfo(SB_VERT, &si, SIF_ALL);

	int iTotalTextLength = ctlLog.GetWindowTextLength();
	ctlLog.SetSel(iTotalTextLength, iTotalTextLength);
	ctlLog.ReplaceSel(sLog.c_str());
	
	//Scroll to bottom
	ctlLog.SendMessage(WM_VSCROLL, MAKELPARAM(SB_THUMBTRACK, si.nMax), NULL);
	ctlLog.SendMessage(WM_VSCROLL, MAKELPARAM(SB_THUMBPOSITION, si.nMax), NULL);
	ctlLog.SendMessage(WM_VSCROLL, MAKELPARAM(SB_ENDSCROLL, 0), NULL);
	*/
}
void DetailsDlg::OnDestroy()
{
	//Save window settings
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	if (wp.showCmd != SW_MAXIMIZE)
	{
		CRect rectForm;
		GetWindowRect(&rectForm);
		theApp.m_Options.SetDwordOption("DetailsWidth", rectForm.Width());
		theApp.m_Options.SetDwordOption("DetailsHeight", rectForm.Height());
	}

	CDialog::OnClose();
}
