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

#if !defined(AFX_PREFSSTATIC_H__1B15B006_9152_11D3_A10C_00500402F30B__INCLUDED_)
#define AFX_PREFSSTATIC_H__1B15B006_9152_11D3_A10C_00500402F30B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PrefsStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrefsStatic window

class CPrefsStatic : public CStatic
{
// Construction
public:
	CPrefsStatic();

// Attributes
public:

// Operations
public:
   CString m_csFontName;

	void		SetConstantText(const char *pText)	{m_csConstantText = pText;}

   int m_fontSize, m_fontWeight;
   BOOL m_grayText;
   COLORREF m_textClr;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPrefsStatic();

protected:
   CFont m_captionFont, m_nameFont;

   CBitmap m_bm;

	CString m_csConstantText;
	

   void MakeCaptionBitmap();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrefsStatic)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText (WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFSSTATIC_H__1B15B006_9152_11D3_A10C_00500402F30B__INCLUDED_)
