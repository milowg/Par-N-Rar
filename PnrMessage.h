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

#if !defined(PNRMESSAGE_H)
#define PNRMESSAGE_H

#include <algorithm>
#include "VerifyPar.h"

enum eStatus {NONE, VERIFIED, DONE};

struct sFoundPar
{
	string m_sParfileName;
	string m_sParfilePath;
	string m_sMonitorDir;
	string m_sPathOffMonitorDir;
	set<string> m_setContainedFiles;
	vector<string> m_vOtherFiles;
	eStatus Status;
	bool bWorking;
	bool bFilesDeleted;
	CString cstrDetails;	
	VerifyPar *verifyPar;
	bool bVerified;
	bool bSkip;
public:
	void sFoundPar::DeleteVerifyPar()
	{
		if (verifyPar != NULL)
			delete verifyPar;
		verifyPar = NULL;
	}
	sFoundPar::sFoundPar()
	{
		verifyPar = NULL;
	}
	sFoundPar::~sFoundPar()
	{
		DeleteVerifyPar();
	}
};
struct sParDone
{
	string m_sParfileName;
	string m_sDoneMessage;
	bool bFilesDeleted;
	eStatus Status;
};
struct sParRemove
{
	string m_sParfileName;
};
struct sParStatus
{
	string m_sParfileName;
	string m_sStatus;
};
struct sParProgress
{
	string m_sParfileName;
	string m_sPercentage;
};
struct sParAddDetails
{
	string m_sParfileName;
	string m_sNewDetails;
};
struct sDoneError
{
	string m_sErrMessage;
};

//Message-related items
const UINT WM_PARNRAR_DONEERROR = ::RegisterWindowMessage("PARNRAR_DONEERROR");
const UINT WM_PARNRAR_PARFOUND = ::RegisterWindowMessage("PARNRAR_FOUNDPAR");
const UINT WM_PARNRAR_PARDONE = ::RegisterWindowMessage("PARNRAR_PARDONE");
const UINT WM_PARNRAR_PARSTATUS = ::RegisterWindowMessage("PARNRAR_PARSTATUS");
const UINT WM_PARNRAR_PARPROGRESS = ::RegisterWindowMessage("PARNRAR_PARPROGRESS");
const UINT WM_PARNRAR_PARADDDETAILS = ::RegisterWindowMessage("PARNRAR_PARADDDETAILS");
const UINT WM_PARNRAR_PARNOPARS = ::RegisterWindowMessage("PARNRAR_PARNOPARS");
const UINT WM_PARNRAR_REMOVE = ::RegisterWindowMessage("PARNRAR_REMOVE");
const UINT WM_PARNRAR_PAUSE = ::RegisterWindowMessage("PARNRAR_PAUSE");
const UINT WM_PARNRAR_PARESTATUS = ::RegisterWindowMessage("PARNRAR_PARESTATUS");
const UINT WM_FOUNDPAR_DONE = WM_USER + 1;

//CPnrMessage Class
#include "ParNRar.h"
#include "StringUtils.h"

extern HWND g_hwndMain;
extern CParNRarApp theApp;

class CPnrMessage 
{
public:
	static void SendFoundPar(string sParfileName, string sParfilePath, string sMonitorDir, set<string> setContainedFiles, vector<string> vOtherFiles)
	{		
		theApp.CheckPaused();
		Sleep(10);
		sFoundPar *pS = new sFoundPar;
		pS->m_sParfileName = sParfileName;
		pS->m_sParfilePath = sParfilePath;
		pS->m_sMonitorDir = sMonitorDir;
		pS->m_sPathOffMonitorDir = sParfilePath;
		pS->m_setContainedFiles = setContainedFiles;
		pS->m_vOtherFiles = vOtherFiles;
		pS->bSkip = false;
		sort(pS->m_vOtherFiles.begin(), pS->m_vOtherFiles.end());

		//Get the Path Off Monitor Dir from the Monitor Dir and Found Path
		StringUtils::replace_all(pS->m_sPathOffMonitorDir, sMonitorDir, "");

		//Make sure there are no extraneous backslashes
		if (pS->m_sMonitorDir.find_last_of("\\") == pS->m_sMonitorDir.length() - 1)
			pS->m_sMonitorDir = pS->m_sMonitorDir.substr(0, pS->m_sMonitorDir.size()-1);
		if (pS->m_sPathOffMonitorDir.find_last_of("\\") == pS->m_sPathOffMonitorDir.length() - 1)
			pS->m_sPathOffMonitorDir = pS->m_sPathOffMonitorDir.substr(0, pS->m_sPathOffMonitorDir.size()-1);

		PostMessage(g_hwndMain, WM_PARNRAR_PARFOUND, GetCurrentThreadId(), (LPARAM)pS);
		MSG msg;
		//Wait for response
		while ( GetMessage( &msg, 0, 0, 0 ) )
		{
			switch (msg.message)
			{        
			case WM_FOUNDPAR_DONE:                
				return;
			default:    
				DispatchMessage( &msg );
			}
		}

	}

	static void SendPareStatus(string sParfileName, eStatus Status)
	{		
		theApp.CheckPaused();
		sParDone *pS = new sParDone;
		pS->m_sParfileName = sParfileName;
		pS->Status = Status;
		PostMessage(g_hwndMain, WM_PARNRAR_PARESTATUS, NULL, (LPARAM)pS);
	}

	static void SendParDone(string sParfileName, string sDoneMessage, bool bFilesDeleted, eStatus Status)
	{		
		theApp.CheckPaused();
		sParDone *pS = new sParDone;
		pS->m_sParfileName = sParfileName;
		pS->m_sDoneMessage = sDoneMessage;
		pS->bFilesDeleted = bFilesDeleted;		
		pS->Status = Status;
		PostMessage(g_hwndMain, WM_PARNRAR_PARDONE, NULL, (LPARAM)pS);
	}
	static void SendParStatus(string sParfileName, string sStatus)
	{		
		theApp.CheckPaused();
		sParStatus *pS = new sParStatus;
		pS->m_sParfileName = sParfileName;
		pS->m_sStatus = sStatus;
		PostMessage(g_hwndMain, WM_PARNRAR_PARSTATUS, NULL, (LPARAM)pS);
	}
	static void SendParProgress(string sParfileName, string sPercentage)
	{		
		theApp.CheckPaused();
		sParProgress *pS = new sParProgress;
		pS->m_sParfileName = sParfileName;
		pS->m_sPercentage = sPercentage;
		PostMessage(g_hwndMain, WM_PARNRAR_PARPROGRESS, NULL, (LPARAM)pS);
	}
	static void SendParAddDetails(string sParfileName, string sNewDetails, bool bCheckPause)
	{		
		theApp.Trace("[%s] %s", sParfileName.c_str(), sNewDetails.c_str());

		if (bCheckPause)
			theApp.CheckPaused();
		sParAddDetails *pS = new sParAddDetails;
		pS->m_sParfileName = sParfileName;
		pS->m_sNewDetails = sNewDetails;
		PostMessage(g_hwndMain, WM_PARNRAR_PARADDDETAILS, NULL, (LPARAM)pS);
	}
	static void SendDoneError(string sErrMessage)
	{		
		theApp.CheckPaused();
		sDoneError *pS = new sDoneError;
		pS->m_sErrMessage = sErrMessage;
		PostMessage(g_hwndMain, WM_PARNRAR_DONEERROR, NULL, (LPARAM)pS);
	}
	static void SendNoPars()
	{		
		theApp.CheckPaused();
		PostMessage(g_hwndMain, WM_PARNRAR_PARNOPARS, NULL, NULL);
	}
	static void SendRemove(string sParfileName)
	{		
		sParRemove *pS = new sParRemove;
		pS->m_sParfileName = sParfileName;
		PostMessage(g_hwndMain, WM_PARNRAR_REMOVE, NULL, (LPARAM)pS);
	}
	static void SendPause(string sPauseMsg)
	{		
		theApp.m_bPaused = true;
		PostMessage(g_hwndMain, WM_PARNRAR_PAUSE, NULL, NULL);
		AfxMessageBox( sPauseMsg.c_str(), MB_OK );
		theApp.CheckPaused();
	}
};

#endif
