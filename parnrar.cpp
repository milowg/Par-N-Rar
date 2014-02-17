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
#include "ParNRarDlg.h"
#include "unrar\dll.hpp"
#include "par2-cmdline\par2cmdline.h"
#include <io.h>
#include "CmdLineOptionsDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CParNRarApp

BEGIN_MESSAGE_MAP(CParNRarApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CParNRarApp construction

CParNRarApp::CParNRarApp()
{
	m_bDebug = false;
	m_hTraceFile = INVALID_HANDLE_VALUE;
}

// The one and only CParNRarApp object

CParNRarApp theApp;

//Utility function
LPCTSTR _find( LPCTSTR p1, LPCTSTR p2 )
{
	while ( *p1 != NULL )
	{
		LPCTSTR p = p2;

		while ( *p != NULL )
		{
			if ( *p1 == *p++ )
				return p1+1;
		}
		p1++;
	}

	return NULL;
}

void CParNRarApp::CheckCommandLine()
{
    m_sCmdLineOptions += "/debug = log to trace file\n\n";
    m_sCmdLineOptions += "/go = start verifying on load\n\n";
    m_sCmdLineOptions += "/m \"Monitor Directory\" = monitor directory to start at";
	m_sCmdLineOptions += "/min = minimize window on load";

	TCHAR szTokens[] = _T("-/");

	LPCTSTR lpszToken = _find( m_lpCmdLine, szTokens );
	while ( lpszToken != NULL )
	{
        if (strlen(lpszToken) > 0)
        {
            //NOTE: Parameters with longer names must be checked first.

			if (strncmp(lpszToken, "debug", 5) == 0)
			{		
				m_bDebug = true;
				m_hTraceFile = CreateFile(m_szLogFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
				ostringstream strm;
				strm.str(""); strm << "Starting Par-N-Rar v" << theApp.GetMyVersion().GetBuffer(0);
				Trace(strm.str());
			}

			if (strncmp(lpszToken, "min", 3) == 0)
			{
				m_Options.SetMinimizeOnStart(true);
			}

			if (strncmp(lpszToken, "go", 2) == 0)
			{
				m_Options.SetGoOnStart(true);
			}

			if (strncmp(lpszToken, "?", 1) == 0)
			{
				CmdLineOptionsDlg dlg;
				dlg.DoModal();
			}
			
			if (strncmp(lpszToken, "m", 1) == 0)
			{
                string sMonitorDir = lpszToken;
                string::size_type posStart = sMonitorDir.find_first_of("\"");
                if (posStart != string::npos)
				{
					string::size_type posEnd = sMonitorDir.find_first_of("\"", posStart + 1);
					if (posEnd != string::npos)
					{
						sMonitorDir = sMonitorDir.substr(posStart + 1, posEnd - posStart - 1);
						if (sMonitorDir != "")
							m_Options.SetMonitorDir(sMonitorDir);
					}
				}
			}

        }
		lpszToken = _find( lpszToken, szTokens );
	}
}

BOOL CParNRarApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();
	AfxInitRichEdit();

	CWinApp::InitInstance();
	SetPriority();
	GetCurrentDirectory(MAX_PATH - strlen("ParNRar.log") - 1, m_szOriginalPath);

	strcpy(m_szLogFile, m_szOriginalPath);
	if (!strcmp(m_szLogFile, ""))
		strcat(m_szLogFile, "ParNRar.log");
	else
		strcat(m_szLogFile, "\\ParNRar.log");

	CheckCommandLine();
	AfxEnableControlContainer();

	// Initialize OLE 2.0 libraries
	if (!AfxOleInit())
	{
		AfxMessageBox("AfxOleInit Error!");
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Par-N-Rar"));

	CParNRarDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	if (m_hTraceFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hTraceFile);
    m_hTraceFile = INVALID_HANDLE_VALUE;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString CParNRarApp::GetMyVersion()
{
	DWORD dwTemp, dwVerSize;
	VS_FIXEDFILEINFO *pFileInfo;
	char szFileName[1024];

	GetModuleFileName(GetModuleHandle(NULL), szFileName, sizeof(szFileName));
	dwVerSize = GetFileVersionInfoSize(szFileName, &dwTemp);
	auto_ptr<char> apVerBuf( new char[dwVerSize+10] );
	char *szVerBuf = apVerBuf.get();
	GetFileVersionInfo(szFileName, 0, dwVerSize, (void *)szVerBuf);

	char szVer[64];
	UINT BufLen;
	szVer[0] = 0;
	if (VerQueryValue (szVerBuf, "\\", (LPVOID *) &pFileInfo, (PUINT) &BufLen)) 
	{
		//Uncomment for dot released (ex: 1.20.1)
		//sprintf(szVer, "%d.%d%d.%d", HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS), LOWORD(pFileInfo->dwFileVersionLS));
		sprintf(szVer, "%d.%d%d", HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS));
	}
	return szVer;
}

void CParNRarApp::Trace(string sLog)
{
	if (!m_bDebug)
		return;

    //Allocate enough memory for the date/time data as well..
    auto_ptr<char> apTemp( new char[sLog.size() + 256] );
    char *szMsg = apTemp.get();

    SYSTEMTIME st;
    GetLocalTime(&st);

	sprintf(szMsg, "%d/%d/%d %02d:%02d:%02d.%0.03d [thread %5d] %s\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, GetCurrentThreadId(), sLog.c_str());
    
    //Output to file
    if (m_hTraceFile != INVALID_HANDLE_VALUE)
    {     
        //Use a Mutex!
        HANDLE hMutex;
		hMutex = CreateMutex( NULL, FALSE, "PARNRAR_TRACE_MUTEX" );
        if (hMutex != NULL)
        {
            //Wait a max of 60 seconds
            if ( WaitForSingleObject( hMutex, 60000 ) != WAIT_TIMEOUT )
            {
                //Make sure the file exists..
                if ( _access(m_szLogFile, 00) == -1 )
                {
                    //Try to re-connect to file
                    CloseHandle(m_hTraceFile);
                    m_hTraceFile = CreateFile(m_szLogFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
                }

                DWORD dwTemp;

                SetFilePointer( m_hTraceFile, 0, NULL, FILE_END );
                WriteFile( m_hTraceFile, szMsg, strlen(szMsg), &dwTemp, NULL);
                FlushFileBuffers( m_hTraceFile );
            }
            ReleaseMutex( hMutex );
            CloseHandle( hMutex );
        }
	}
}

void CParNRarApp::SetPriority()
{
	string sPriority = m_Options.GetPriority();
	DWORD dwPriorityClass = NORMAL_PRIORITY_CLASS;

	if (sPriority == "Idle")
		dwPriorityClass = IDLE_PRIORITY_CLASS;
	if (sPriority == "Below Normal")
		dwPriorityClass = BELOW_NORMAL_PRIORITY_CLASS;
	if (sPriority == "Normal")
		dwPriorityClass = NORMAL_PRIORITY_CLASS;
	if (sPriority == "Above Normal")
		dwPriorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
	if (sPriority == "High")
		dwPriorityClass = HIGH_PRIORITY_CLASS;
	if (sPriority == "Realtime")
		dwPriorityClass = REALTIME_PRIORITY_CLASS;

	SetPriorityClass(GetCurrentProcess(), dwPriorityClass);
}

void CParNRarApp::Trace(LPCTSTR pszFormat, ...)
{          
	memset(m_buf, 0, 2000);

	va_list arglist;
	va_start( arglist, pszFormat );

	//Make sure we only use up to 1999 bytes
	_vsnprintf( m_buf, 1998, pszFormat, arglist );
	m_buf[1999] = 0;		
	va_end( arglist );
	string sBuf = m_buf;
	Trace(sBuf);
}

void CParNRarApp::CheckPaused()
{
	while (m_bPaused)
		Sleep(100);
}

string CParNRarApp::ErrorMessage(DWORD error)
{
	string result;

	LPVOID lpMsgBuf;
	if (::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0,
		NULL))
	{
		result = (char*)lpMsgBuf;
		LocalFree(lpMsgBuf);
	}
	else
	{
		char message[40];
		_snprintf(message, sizeof(message), "Unknown error code (%d)", error);
		result = message;
	}

	StringUtils::trim(result, " \t\r\n");
	return result;
}