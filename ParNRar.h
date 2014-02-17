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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "Options.h"

// CParNRarApp:
// See ParNRar.cpp for the implementation of this class
//
class CParNRarApp : public CWinApp
{
public:
	CParNRarApp();
	CString GetMyVersion();
	bool m_bPaused;
	Options m_Options;
	char m_szOriginalPath[MAX_PATH];

	string m_sCmdLineOptions;

// Overrides
	public:
	void CheckPaused();
	virtual BOOL InitInstance();
	void Trace(string sLog);
	void Trace(LPCTSTR pszFormat, ...);
	void SetPriority();
	static string ErrorMessage(DWORD error);

// Implementation

	DECLARE_MESSAGE_MAP()

	
protected:
	char m_szLogFile[MAX_PATH];
	char m_buf[2000];
	bool m_bDebug;
	HANDLE m_hTraceFile;

	void CheckCommandLine();
};

extern CParNRarApp theApp;