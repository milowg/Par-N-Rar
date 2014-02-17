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
#include ".\parnrardlg.h"
#include "ScanDir.h"
#include "XBrowseForFolder.h"
#include "About.h"
#include <memory>
#include <algorithm>
#include "VerifyPar.h"
#include "DetailsDlg.h"
#include "io.h"
#include "Hyperlink.h"
#include "AskDlg.h"
#include "NewVersionDlg.h"
#include "FileUtils.h"
#include "SettingsDlg.h"
#include "PropPage_Dirs.h"
#include "PropPage_File.h"
#include "PropPage_Program.h"

HWND g_hwndMain;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static UINT BASED_CODE indicators[] =
{
    ID_STATUSBAR_IND
};

extern CParNRarApp theApp;

CParNRarDlg::CParNRarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParNRarDlg::IDD, pParent)
{
	m_bInitialized = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bTimer = false;
	m_bSingleItemVerify = false;
	m_bWorking = false;
}

void CParNRarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DONATE, m_lnkDonate);
	DDX_Control(pDX, IDC_STATUSLIST, m_lstStatus);
	DDX_Control(pDX, IDC_MONITORDIR, m_MonitorDir);
}

BEGIN_MESSAGE_MAP(CParNRarDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_GO, OnBnClickedGo)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_DONEERROR, OnDoneError)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARPROGRESS, OnParProgress)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARFOUND, OnFoundPar)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARDONE, OnParDone)	
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARESTATUS, OnPareStatus)	
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARSTATUS, OnParStatus)		
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARADDDETAILS, OnParAddDetails)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PARNOPARS, OnParNoPars)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_REMOVE, OnRemove)
	ON_REGISTERED_MESSAGE(WM_PARNRAR_PAUSE, OnPauseMsg)

	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BROWSE_DIR, OnBnClickedBrowseDir)
	ON_BN_CLICKED(ID_ABOUT, OnBnClickedAbout)
	ON_BN_CLICKED(IDOK3, OnBnClickedOk2)
	ON_BN_CLICKED(ID_OPTIONS, OnBnClickedOptions)
	ON_NOTIFY(NM_DBLCLK, IDC_STATUSLIST, OnNMDblclkStatuslist)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CONTEXTSTATUS_DETAILS, OnContextstatusDetails)
	ON_COMMAND(ID_CONTEXTSTATUS_RESCAN, OnContextstatusRescan)
	ON_COMMAND(ID_CONTEXTSTATUS_REMOVE, OnContextstatusRemove)
	ON_COMMAND(ID_CONTEXTSTATUS_DELETEFILES, OnContextstatusDeleteFiles)
	ON_COMMAND(ID_CONTEXTSTATUS_REMOVEDONE, OnContextstatusRemoveDone)
	ON_COMMAND(ID_CONTEXT_OPENCONTAININGFOLDER, OnContextOpenContainingFolder)
	ON_COMMAND(ID_CONTEXT_SKIPPROCESSING, OnContextSkipProcessing)
	ON_NOTIFY(LVN_KEYDOWN, IDC_STATUSLIST, OnLvnKeydownStatuslist)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_HELP, &CParNRarDlg::OnBnClickedHelp)
	ON_WM_SIZE()
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_COMMAND(ID_SYSTRAY_OPEN, &CParNRarDlg::OnSystrayOpen)
	ON_COMMAND(ID_SYSTRAY_EXIT, &CParNRarDlg::OnSystrayExit)
	ON_COMMAND(ID_SYSTRAY_OPTIONS, &CParNRarDlg::OnSystrayOptions)
	ON_COMMAND(ID_SYSTRAY_ABOUT, &CParNRarDlg::OnSystrayAbout)
	ON_COMMAND(ID_SYSTRAY_GO, &CParNRarDlg::OnSystrayGo)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CLEARHISTORY, &CParNRarDlg::OnBnClickedClearhistory)
	ON_NOTIFY(NM_CLICK, IDC_DONATE, OnClickDonate) 
END_MESSAGE_MAP()


void CParNRarDlg::OnClickDonate(NMHDR* pNMHDR, LRESULT* pResult)
{
	CHyperLink::GotoURL("", SW_SHOW);
}

BOOL CParNRarDlg::OnInitDialog()
{	
	CDialog::OnInitDialog();

	m_lnkDonate.SetURL(_T("https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=milowg@yahoo.com&item_name=Par-N-Rar&no_shipping=2&no_note=1&currency_code=USD&tax=0&bn=PP-DonationsBF"));
	
	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon		
	
	//Initialize System Tray
	m_TrayIcon.Create(this, WM_ICON_NOTIFY, "Par-N-Rar", m_hIcon, IDR_SYSTEMTRAY, TRUE);
	m_TrayIcon.SetIconList(IDI_ANIM1, IDI_ANIM4);
	m_TrayIcon.SetTargetWnd(this);
	m_TrayIcon.SetMenuDefaultItem(32783, FALSE);	

	//Initialize ListView
	RECT rc;
	m_lstStatus.GetClientRect(&rc);
	m_lstStatus.SetExtendedStyle(LVS_EX_FULLROWSELECT); 

	m_lstStatus.SetBkColor(RGB(255,255,255));
	m_lstStatus.SetTextBkColor(RGB(255,255,255));
	m_lstStatus.InsertColumn(0, "PAR file", LVCFMT_LEFT, (int)(((rc.right - rc.left) * .50) - 2) );
	m_lstStatus.InsertColumn(1, "Status", LVCFMT_LEFT, (int)(((rc.right - rc.left) * .30) - 2) );
	m_lstStatus.InsertColumn(2, "Progress", LVCFMT_LEFT, (int)(((rc.right - rc.left) * .20) - 5) );
	m_lstStatus.InitProgressColumn(2);

	//StatusBar
	m_StatusBar.Create(this); //We create the status bar
	m_StatusBar.SetIndicators(indicators, 1);

	CRect rect;
	GetClientRect(&rect);
	m_StatusBar.SetPaneInfo(0, ID_STATUSBAR_IND, SBPS_NORMAL, rect.Width()-100);      
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_STATUSBAR_IND);
	SetStatus("Press Go to Process", false);	

	//Load "Last Monitor Dirs"
	vector<string> vLastMonitorDirs = theApp.m_Options.GetLastMonitorDirs();
	vector<string>::iterator vi = vLastMonitorDirs.begin();
	while (vi != vLastMonitorDirs.end())
	{
		m_MonitorDir.AddString((*vi).c_str());
		++vi;
	}	
	
	//Check for incomplete extracted file
	string sTemp = theApp.m_Options.GetStringOption("ExtractFile", "");
	if (sTemp.length() != 0)
	{
		theApp.Trace("Found incomplete file from previous extraction: %s", sTemp.c_str());
		theApp.Trace("Deleting file..");
		DeleteFile(sTemp.c_str());
		theApp.m_Options.SetStringOption("ExtractFile", "");
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CParNRarDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CParNRarDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CParNRarDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CParNRarDlg::OnBnClickedGo()
{		
	CString cstrText;
	m_bVerifyOnly = false;

	GetDlgItem(ID_GO)->GetWindowText(cstrText);
	if (cstrText == "&Go")
	{		
		if (!AskVerifyOnly())
			return;

		GetDlgItem(ID_GO)->SetWindowText("&Pause");
		SetStatus("Processing", true);
		StartCheck();		
	}
	else if (cstrText == "&Pause")
	{
		OnPauseMsg(NULL, NULL);
	}
	else if (cstrText == "&Continue")
	{
		if (m_bTimer)
		{
			SetTimer(1, 1000, NULL); 
		}
		theApp.m_bPaused = false;
		GetDlgItem(ID_GO)->SetWindowText("&Pause");
		SetStatus("Processing", true);
	}
	else if (cstrText == "&Stop")
	{
		theApp.m_bPaused = false;
		GetDlgItem(ID_GO)->SetWindowText("&Go");
		KillTimer(1);
		SetStatus("Press Go to Process", false);
	}
	ResetSystrayMenu();
}

void CParNRarDlg::StartCheck()
{
	//Save Monitor Directories
	CString cstrMonitorDir;
	m_MonitorDir.GetWindowText(cstrMonitorDir);
	if (m_MonitorDir.FindString(-1, cstrMonitorDir) < 0)
		m_MonitorDir.AddString(cstrMonitorDir);
	SaveLastMonitorDirs();

	//Make sure if we are in single verify mode that we change
	m_bSingleItemVerify = false;

	ClearSkipped();

	//Scan directories
	CString strMonitorDir = "";
	GetDlgItem(IDC_MONITORDIR)->GetWindowText(strMonitorDir);
	theApp.m_Options.SetMonitorDir(strMonitorDir.GetBuffer(0));
	ScanDir::Start(strMonitorDir.GetBuffer(0));
}

LRESULT CParNRarDlg::OnPareStatus(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sParDone> apS( (sParDone *)lParam );
	sParDone *pS = apS.get();	

	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for eStatus Message: " << pS->m_sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}
	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
	psFound->Status = pS->Status;
}

LRESULT CParNRarDlg::OnParDone(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sParDone> apS( (sParDone *)lParam );
	sParDone *pS = apS.get();	

	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for Done Message: " << pS->m_sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}

	AddDetail(iIndex, pS->m_sParfileName, pS->m_sDoneMessage, true);
	m_lstStatus.SetItem(iIndex, 2, LVIF_TEXT, "100", 0, 0, 0, 0, 0);

	m_lstStatus.SetItem(iIndex, 1, LVIF_TEXT, pS->m_sDoneMessage.c_str(), 0, 0, 0, 0, 0);
	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
	psFound->Status = pS->Status;
	psFound->bWorking = false;
	psFound->bFilesDeleted = pS->bFilesDeleted;	

	if (psFound->bFilesDeleted)
	{
		//Delete VerifyPar to remove any file references
		psFound->DeleteVerifyPar();

		VerifyPar vp(psFound->m_sParfileName, psFound->m_sMonitorDir);
		vp.MainDeleteFiles(psFound->m_vOtherFiles);
	}

	m_bWorking = false;

	if (m_bSingleItemVerify)
	{
		m_bSingleItemVerify = false;
		OnParNoPars(NULL, NULL);
	}
	else
		VerifyNextItem();
	return 0;
}

void CParNRarDlg::VerifyNextItem()
{
	VerifyNextItem(-1, false);
}

void CParNRarDlg::VerifyNextItem(int idxToVerify, bool bSingle)
{
	//Synchronize this function
	CRITICAL_SECTION criticalSection;
	InitializeCriticalSection(&criticalSection);
	EnterCriticalSection(&criticalSection);
	
	if (m_bWorking) goto CLEANUP;

	//Find the next non-done item (that is not skipped)
	int iIndex = -1;
	if (idxToVerify != -1)
	{
		iIndex = idxToVerify;
	}
	else
	{
		for (int i=0; i<m_lstStatus.GetItemCount(); i++)
		{
			sFoundPar *pS = (sFoundPar *)m_lstStatus.GetItemData(i);

			if (pS->bSkip)
			{
				m_lstStatus.SetItem(i, 1, LVIF_TEXT, "Skipped", 0, 0, 0, 0, 0);
			}
			else if (pS->Status == NONE || (pS->Status == VERIFIED && !m_bVerifyOnly) && iIndex == -1)
			{
				//Grab the index of a done item, but only if we don't already have one
				iIndex = i;
				break;
			}
		}
	}

	if (iIndex != -1)
	{			
		sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
		//Make sure file still exists
		if (_access((psFound->m_sParfilePath + psFound->m_sParfileName).c_str(), 0) == -1)
		{
			psFound->Status = DONE;
			psFound->bWorking = false;
			m_lstStatus.SetItem(iIndex, 1, LVIF_TEXT, "Missing", 0, 0, 0, 0, 0);
			VerifyNextItem();
		}
		else
		{
			SetIndicatorsPause();
			m_lstStatus.SetItem(iIndex, 1, LVIF_TEXT, "Verifying", 0, 0, 0, 0, 0);
			psFound->bWorking = true;
			if (bSingle)
				m_bSingleItemVerify = true;

			m_bWorking = true;
			psFound->bSkip = false;
			VerifyPar::StartVerify(m_bVerifyOnly, &(psFound->verifyPar), psFound->m_sParfileName, psFound->m_sParfilePath, psFound->m_sPathOffMonitorDir, psFound->m_sMonitorDir);				
		}
	}
	else
	{
		//No more PARs to check
		OnParNoPars(NULL, NULL);
	}
CLEANUP:
	LeaveCriticalSection(&criticalSection);
	DeleteCriticalSection(&criticalSection);
}
LRESULT CParNRarDlg::OnParStatus(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sParStatus> apS( (sParStatus *)lParam );
	sParStatus *pS = apS.get();

	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for Status Message: " << pS->m_sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}

	m_lstStatus.SetItem(iIndex, 1, LVIF_TEXT, pS->m_sStatus.c_str(), 0, 0, 0, 0, 0);
	return 0;
}

LRESULT CParNRarDlg::OnParProgress(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sParProgress> apS( (sParProgress *)lParam );
	sParProgress *pS = apS.get();

	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for Progress Message: " << pS->m_sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}

	m_lstStatus.SetItem(iIndex, 2, LVIF_TEXT, pS->m_sPercentage.c_str(), 0, 0, 0, 0, 0);
	if (theApp.m_Options.GetKeepWorkItemInFocus())
	{
		m_lstStatus.EnsureVisible(iIndex, FALSE);
	}
	return 0;
}

LRESULT CParNRarDlg::OnParAddDetails(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sParAddDetails> apS( (sParAddDetails *)lParam );
	sParAddDetails *pS = apS.get();

	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for AddDetails Message: " << pS->m_sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}

	AddDetail(iIndex, pS->m_sParfileName, pS->m_sNewDetails, false);
	return 0;
}

void CParNRarDlg::AddDetail(int iIndex, string sParfileName, string sNewDetails, bool bTrace)
{
	char szTime[32];    
	SYSTEMTIME st;
	GetLocalTime(&st);

	sprintf(szTime, "%d/%d/%d %02d:%02d:%02d.%0.03d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
	if (psFound->cstrDetails != "")
		psFound->cstrDetails += "\n";
	psFound->cstrDetails += szTime;
	psFound->cstrDetails += " ";
	psFound->cstrDetails += sNewDetails.c_str();

	if (bTrace)
		theApp.Trace("[%s] %s", sParfileName.c_str(), sNewDetails.c_str());
}

LRESULT CParNRarDlg::OnParNoPars(WPARAM wParam, LPARAM lParam) 
{	
	if (theApp.m_Options.GetDoneScan() == "Restart")
	{
		if (theApp.m_Options.GetRestartDelay() != "")
		{
			//Start timer
			m_lElapsed = 0;
			m_bTimer = true;
			SetTimer(1, 1000, NULL); 
			return 0;
		}
	}
	else if (theApp.m_Options.GetDoneScan() == "Beep")
	{
		Beep(440, 1000);
	}
	else if (theApp.m_Options.GetDoneScan() == "Shutdown System")
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;
		OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES |  TOKEN_QUERY, &hToken);
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);
		if (GetLastError() != ERROR_SUCCESS)   
			MessageBox("AdjustTokenPrivileges enable failed.");

		ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCEIFHUNG, 0);
	}
	else if (theApp.m_Options.GetDoneScan() == "Close Par-N-Rar")
	{
		CDialog::OnCancel();
	}

	GetDlgItem(ID_GO)->SetWindowText("&Go");
	SetStatus("Press Go to Process", false);
	ResetSystrayMenu();

	return 0;
}

LRESULT CParNRarDlg::OnPauseMsg(WPARAM wParam, LPARAM lParam) 
{
	theApp.m_bPaused = true;
	GetDlgItem(ID_GO)->SetWindowText("&Continue");
	KillTimer(1);
	SetStatus("Paused", false);
	return 0;
}

LRESULT CParNRarDlg::OnRemove(WPARAM wParam, LPARAM lParam) 
{

	auto_ptr<sParRemove> apS( (sParRemove *)lParam );
	sParRemove *pS = apS.get();
	return RemoveItemFromList(pS->m_sParfileName);
}

LRESULT CParNRarDlg::RemoveItemFromList(string sParfileName)
{
	int iIndex = FindItemInList(sParfileName);
	if (iIndex == -1)
	{
		ostringstream strm;
		strm << "Can't find list item for Remove Message: " << sParfileName;
		theApp.Trace("%s", strm.str().c_str());
		return 0;
	}

	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
	delete psFound;
	m_lstStatus.DeleteItem(iIndex);
	return 0;
}

LRESULT CParNRarDlg::OnFoundPar(WPARAM wParam, LPARAM lParam) 
{
	sFoundPar *pS = (sFoundPar *)lParam;

	//If this is a rar, find out the "first" file in the set and this is the one we add to the list
	string sParFilename;
	bool bFileIsNotfirstRar = false;
	bool bFileIsRar = false;
	string sOrigPar;
	if (FileUtils::DetermineSetType(pS->m_sParfileName) == "RAR")
	{
		bFileIsRar = true;
		sParFilename = FileUtils::GetRarFirstFilename(pS->m_sParfileName);
		sOrigPar = pS->m_sParfilePath + pS->m_sParfileName;
		if (sParFilename != pS->m_sParfileName)
		{
			//Put this file on the contained files list in case it is not there (for deletion later)
			transform(sOrigPar.begin(), sOrigPar.end(), sOrigPar.begin(), tolower);
			if (StringUtils::find(pS->m_setContainedFiles, sOrigPar, false) == false)
			{				
				pS->m_setContainedFiles.insert(sOrigPar);
			}

			//The "first" file in the set is not this file. 
			bFileIsNotfirstRar = true;
		}
		pS->m_sParfileName = sParFilename;
	}

	//Look for the name in the list to see if it already exists
	int iIndex = FindItemInList(pS->m_sParfileName);
	if (iIndex != -1)
	{
		//Make sure we want to retry this
		char szStatus[1025];
		m_lstStatus.GetItemText(iIndex, 1, szStatus, 1024);
		if ( !strcmp(szStatus, "Done") || !strcmp(szStatus, "Already Verified") ) 
		{
			VerifyNextItem();
			goto CLEANUP;
		}

		//Use the current one
		pS = (sFoundPar *)m_lstStatus.GetItemData(iIndex);
		if (bFileIsRar)
		{
			//Put this file on the contained files list in case it is not there (for deletion later)
			transform(sOrigPar.begin(), sOrigPar.end(), sOrigPar.begin(), tolower);
			if (StringUtils::find(pS->m_setContainedFiles, sOrigPar, false) == false)
			{
				pS->m_setContainedFiles.insert(sOrigPar);
			}
		}

		if (!bFileIsRar || (bFileIsRar && !bFileIsNotfirstRar))
			pS->cstrDetails += "\n------------------------------------------\n";
	}
	else
	{
		//See if any other sets in the list have the same files contained in them. If so, use the PAR over the SFV/MD5
		if (CheckForExistingItem(pS))
		{
			VerifyNextItem();
			goto CLEANUP;
		}

		//Create a new one 
		iIndex = m_lstStatus.GetItemCount();
		m_lstStatus.InsertItem(iIndex, pS->m_sParfileName.c_str());
	}
	pS->Status = NONE;
	pS->bWorking = false;
	pS->bFilesDeleted = false;
	m_lstStatus.SetItemData(iIndex, (LPARAM)pS);

	VerifyNextItem();
CLEANUP:
	PostThreadMessage(wParam, WM_FOUNDPAR_DONE, 0, 0);
	return 0;
}

bool CParNRarDlg::CheckForExistingItem(sFoundPar *psFound)
{
	for (int i=0; i<m_lstStatus.GetItemCount(); i++)
	{
		sFoundPar *pS = (sFoundPar *)m_lstStatus.GetItemData(i);

		//Check to see if found PAR itself is in the other set
		if (StringUtils::find(pS->m_setContainedFiles, psFound->m_sParfilePath + psFound->m_sParfileName, true))
		{
			if (pS->bFilesDeleted)
			{
				//Delete VerifyPar to remove any file references
				psFound->DeleteVerifyPar();

				//Delete this file as well as the other files in the PAR set				
				VerifyPar::StartDelete(&(psFound->verifyPar), psFound->m_sParfileName, psFound->m_sParfilePath, psFound->m_sPathOffMonitorDir, psFound->m_sMonitorDir, false);
			}
			else
			{				
				//Make sure other files and this file are deleted if this set is deleted 
				//TODO - move this and the statement in the if statement below to a common area
				if (find(pS->m_vOtherFiles.begin(), pS->m_vOtherFiles.end(), psFound->m_sParfilePath + psFound->m_sParfileName) == pS->m_vOtherFiles.end())
				{
					pS->m_vOtherFiles.push_back(psFound->m_sParfilePath + psFound->m_sParfileName);
					copy(psFound->m_vOtherFiles.begin(), psFound->m_vOtherFiles.end(), back_inserter(pS->m_vOtherFiles));
				}
			}
			return true;
		}
		else
		{
			//If these sets have any files in common
			set<string> setIntersection; 
			set_intersection(pS->m_setContainedFiles.begin(), pS->m_setContainedFiles.end(), psFound->m_setContainedFiles.begin(), psFound->m_setContainedFiles.end(), inserter(setIntersection, setIntersection.begin()));

			if ( (setIntersection.size() > 0) || StringUtils::find(pS->m_setContainedFiles, psFound->m_sParfilePath + psFound->m_sParfileName, true) || StringUtils::find(psFound->m_setContainedFiles, pS->m_sParfilePath + pS->m_sParfileName, true) )
			{

				if (pS->bFilesDeleted)
				{
					//Delete VerifyPar to remove any file references
					psFound->DeleteVerifyPar();

					//Delete this file as well as the other files in the PAR set
					VerifyPar::StartDelete(&(psFound->verifyPar), psFound->m_sParfileName, psFound->m_sParfilePath, psFound->m_sPathOffMonitorDir, psFound->m_sMonitorDir, false);
				}
				else
				{
					//Make sure other files and this file are deleted if this set is deleted 
					pS->m_vOtherFiles.push_back(psFound->m_sParfilePath + psFound->m_sParfileName);
					copy(psFound->m_vOtherFiles.begin(), psFound->m_vOtherFiles.end(), back_inserter(pS->m_vOtherFiles));
					//TODO ---- have a "Delete files" call from the main thread to this thread which spawns another thread to delete. if so, do not need to scan only while not working

					//Check for the case where the set type is not PAR/PAR2 and the new file found >is<
					string sCurType = FileUtils::DetermineSetType(pS->m_sParfileName);
					string sFoundType = FileUtils::DetermineSetType(psFound->m_sParfileName);
					if (sFoundType == "PAR" && sCurType != "PAR") 
						if (!pS->bWorking)							
							ReplaceItem(pS, psFound);
					if (sFoundType == "SFV" && sCurType != "SFV" && sCurType != "PAR") 
						if (!pS->bWorking)							
							ReplaceItem(pS, psFound);
				}
				return true;
			}
		}
	}
	return false;
}

void CParNRarDlg::ReplaceItem(sFoundPar *psOld, sFoundPar *psNew)
{
	theApp.Trace("Found better set for [%s].. using [%s] instead", psOld->m_sParfileName.c_str(), psNew->m_sParfileName.c_str());

	RemoveItemFromList(psOld->m_sParfileName);
	OnFoundPar(NULL , (LPARAM)psNew);
}

LRESULT CParNRarDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam) 
{		
	return m_TrayIcon.OnTrayNotification(wParam, lParam);
}

LRESULT CParNRarDlg::OnDoneError(WPARAM wParam, LPARAM lParam) 
{
	auto_ptr<sDoneError> apS( (sDoneError *)lParam );
	sDoneError *pS = apS.get();

	MessageBox(pS->m_sErrMessage.c_str(), "Error");

	GetDlgItem(ID_GO)->EnableWindow(true);
	GetDlgItem(ID_GO)->SetWindowText("&Go");
	SetStatus("Press Go to Process", false);
	ResetSystrayMenu();
	return 0;
}

void CParNRarDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{

	if (bShow == 0)
		return;
	CDialog::OnShowWindow(bShow, nStatus);

	if (!m_bInitialized)
		Initialize();
}

void CParNRarDlg::Initialize()
{
	//Load window settings	
	CRect rectForm;	
	GetWindowRect(&rectForm);

	DWORD dw, dwWidth = 406, dwHeight = 193, dwLeft = -1, dwTop = -1;
	dw = theApp.m_Options.GetDwordOption("MainWidth", -1);
	if (dw != -1)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		dwWidth = dw;
		dwHeight = theApp.m_Options.GetDwordOption("MainHeight", -1);
		dwLeft = theApp.m_Options.GetDwordOption("MainLeft", -1);
		dwTop = theApp.m_Options.GetDwordOption("MainTop", -1);
		
		if ( (dwLeft != -1) && (dwLeft + dwWidth < screenWidth) )
			rectForm.left = dwLeft;
		if ( (dwTop != -1) && (dwTop + dwHeight < screenHeight) )
			rectForm.top = dwTop;

		rectForm.right = rectForm.left + dwWidth;
		rectForm.bottom = rectForm.top + dwHeight;

		MoveWindow(&rectForm);
	}

	dw = theApp.m_Options.GetDwordOption("MainMax", -1);
	if (dw == 1)
	{
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof(WINDOWPLACEMENT));
		wp.length = sizeof(WINDOWPLACEMENT);
		wp.showCmd = SW_SHOWMAXIMIZED;
		wp.rcNormalPosition = rectForm;
		SetWindowPlacement(&wp);
	}

	char sz[64];
	CHeaderCtrl* pHeader = (CHeaderCtrl*) m_lstStatus.GetDlgItem(0);
	for (long l=0; l<pHeader->GetItemCount(); l++)
	{		
		CString cstr = "MainColWidth";
		cstr += _ltoa(l, sz, 10);
		dw = theApp.m_Options.GetDwordOption(cstr.GetBuffer(0), -1);
		if (dw != -1)
		{
			m_lstStatus.SetColumnWidth(l, dw);
		}
	}	


	//Load settings
	GetDlgItem(IDC_MONITORDIR)->SetWindowText(theApp.m_Options.GetMonitorDir().c_str());

	CString cstrCaption = "Par-N-Rar v" + theApp.GetMyVersion();
	SetWindowText(cstrCaption.GetBuffer(0));
	g_hwndMain = this->m_hWnd;

	GetDlgItem(ID_GO)->SetWindowText("&Go");
	SetStatus("Press Go to Process", false);
	ResetSystrayMenu();
	Resize();

	if (theApp.m_Options.GetGoOnStart())
	{
		OnBnClickedGo();
	}
	else
	{
		AskCheckForNewVersion();
	}

	if (theApp.m_Options.GetMinimizeOnStart())
		ShowWindow(SW_MINIMIZE);

	m_bInitialized = true;
}

void CParNRarDlg::AskCheckForNewVersion()
{	
	//Check registry to see if we should ask for a new version check
	DWORD dwTemp;
	SYSTEMTIME stNow, stLastChecked;	
	GetLocalTime(&stNow);
	GetLocalTime(&stLastChecked);
	dwTemp = theApp.m_Options.GetDwordOption("lstVerChkM", -1);
	if (dwTemp == -1)
	{					
		theApp.m_Options.SetDwordOption("lstVerChkM", stNow.wMonth);
		theApp.m_Options.SetDwordOption("lstVerChkD", stNow.wDay);
		theApp.m_Options.SetDwordOption("lstVerChkY", stNow.wYear);
	}
	else
	{
		DWORD wMonth, wDay, wYear;
		
		wMonth = theApp.m_Options.GetDwordOption("lstVerChkM", -1);
		wDay = theApp.m_Options.GetDwordOption("lstVerChkD", -1);
		wYear = theApp.m_Options.GetDwordOption("lstVerChkY", -1);

		stLastChecked.wMonth = (WORD)wMonth;
		stLastChecked.wDay = (WORD)wDay;
		stLastChecked.wYear = (WORD)wYear;

		CTime timeNow(stNow);
		if (theApp.m_Options.GetCheckForNewVersion() == "Never")
		{
			//None
			return;
		}
		if (theApp.m_Options.GetCheckForNewVersion() == "Weekly")
		{
			//Week
			CTime timeLastChecked(stLastChecked);
			if (timeLastChecked + CTimeSpan(7, 0, 0, 0) > timeNow)
				return;			
		}
		else if (theApp.m_Options.GetCheckForNewVersion() == "Monthly")
		{
			//Month
			DWORD dwNewMonth, dwNewYear;
			if (stLastChecked.wMonth = 12)
			{
				dwNewMonth = 1;
				dwNewYear = stLastChecked.wYear + 1;
			}
			else
			{
				dwNewMonth = stLastChecked.wMonth + 1;
				dwNewYear = stLastChecked.wYear;
			}
			SYSTEMTIME stTemp;
			GetLocalTime(&stTemp);
			stTemp.wMonth = (WORD)dwNewMonth;
			stTemp.wYear = (WORD)dwNewYear;
			stTemp.wDay = stLastChecked.wDay;
			CTime timeTemp(stTemp);
			if (timeTemp > timeNow)
				return;						
		}
		else if (theApp.m_Options.GetCheckForNewVersion() == "Yearly")
		{
			//Year
			SYSTEMTIME stTemp;
			GetLocalTime(&stTemp);
			stTemp.wMonth = stLastChecked.wMonth;
			stTemp.wYear = stLastChecked.wYear + 1;
			stTemp.wDay = stLastChecked.wDay;
			CTime timeTemp(stTemp);
			if (timeTemp > timeNow)
				return;	
		}
		
		//Reset registry before asking
		theApp.m_Options.SetDwordOption("lstVerChkM", stNow.wMonth);
		theApp.m_Options.SetDwordOption("lstVerChkD", stNow.wDay);
		theApp.m_Options.SetDwordOption("lstVerChkY", stNow.wYear);

		AskDlg dlgAsk;
		dlgAsk.SetCaption("Check for new version");
		dlgAsk.SetQuestion("Do you want to check for a new version of Par-N-Rar?");
		dlgAsk.SetButtons("&Yes", "&No");
		if (dlgAsk.DoModal() != IDCANCEL)
		{
			if (dlgAsk.NeverAskAgain())
				theApp.m_Options.SetCheckForNewVersion("n");
			if (dlgAsk.ButtonClicked() == 1)
			{
				NewVersionDlg dlg;
				dlg.DoModal();
			}

		}
	}
}

void CParNRarDlg::OnBnClickedBrowseDir()
{
	CString cstrCurFolder = "";
	GetDlgItem(IDC_MONITORDIR)->GetWindowText(cstrCurFolder);
	auto_ptr<char> apCurFolder( new char[cstrCurFolder.GetLength() + 1] );
	char *szCurFolder = apCurFolder.get();
	szCurFolder[cstrCurFolder.GetLength()] = 0;

	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = _T('\0');

	if (cstrCurFolder != "")
		strcpy(szCurFolder, cstrCurFolder.GetBuffer(0));

	BOOL bRet = XBrowseForFolder(m_hWnd, szCurFolder, szFolder, sizeof(szFolder)/sizeof(TCHAR)-2);

	if (bRet)
		GetDlgItem(IDC_MONITORDIR)->SetWindowText(szFolder);
}

void CParNRarDlg::OnBnClickedAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CParNRarDlg::OnBnClickedOk2()
{
	CDialog::OnCancel();
}

void CParNRarDlg::OnBnClickedOptions()
{
	CSettingsDialog dlg;

	PropPage_Dirs *d = new PropPage_Dirs();
	dlg.AddPage(RUNTIME_CLASS(PropPage_Dirs), _T("Directories"), IDD_OPT_DIRS,  _T("Directory Options"));
	dlg.AddPage(RUNTIME_CLASS(PropPage_File), _T("File Processing"), IDD_OPT_FILE,  _T("File Processing Options"));
	dlg.AddPage(RUNTIME_CLASS(PropPage_Program), _T("Program"), IDD_OPT_PROGRAM,  _T("Program Options"));

	dlg.SetTitle("Par-N-Rar Options");
	dlg.SetLogoText("Par-N-Rar");    
	int nResponse = dlg.DoModal();

	if (nResponse == IDOK)
	{
		//Check for options change
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wp);	

		//If we are minimized to tray, and the option is unchecked, then maximize
		if (!theApp.m_Options.GetMinSysTray() && m_TrayIcon.IsMinimized())
		{
			m_TrayIcon.HideIcon();
			m_TrayIcon.MaximiseFromTray(this);
		}
	}
}

int CParNRarDlg::FindItemInList(string sParfileName, int iStart)
{
	LVFINDINFO lvf;
	lvf.flags = LVFI_STRING;
	lvf.psz = sParfileName.c_str();
	return m_lstStatus.FindItem(&lvf, iStart);
}

void CParNRarDlg::OnNMDblclkStatuslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	//Get the double-clicked item
	CPoint pt;
	GetCursorPos(&pt);
	m_lstStatus.ScreenToClient(&pt);
	UINT Flags;
	int nItem = m_lstStatus.HitTest( pt, &Flags );
	if (nItem == -1)
		return;

	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
	if (psFound->Status == DONE || psFound->Status == VERIFIED)
		ShowDetailsDlg(nItem);
}

void CParNRarDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu mnuContextStatus;
	mnuContextStatus.LoadMenu(IDR_CONTEXTSTATUS);	

	CPoint pt;
	sFoundPar *psFound = NULL;
	GetCursorPos(&pt);
	m_lstStatus.ScreenToClient(&pt);
	UINT Flags;
	int hItem = m_lstStatus.HitTest( pt, &Flags );
	if (hItem != -1)
	{
		CMenu *mnuPopupMenu;
		int nItem = -1;
		nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);

		if (nItem != -1)
			psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);

		if (m_lstStatus.GetSelectedCount() > 1)
		{
			mnuPopupMenu = mnuContextStatus.GetSubMenu(2);	
			mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_RESCAN, MF_BYCOMMAND | MF_GRAYED);
		}
		else
		{
			sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
			if (psFound->bWorking)
			{							
				mnuPopupMenu = mnuContextStatus.GetSubMenu(3);
				if (m_lstStatus.GetItemText(nItem, 1) == "Skipping.." || ((psFound->verifyPar->m_Status != STATUS_NONE) && (psFound->verifyPar->m_Status != STATUS_VERIFYING)) )
					mnuPopupMenu->EnableMenuItem (ID_CONTEXT_SKIPPROCESSING, MF_BYCOMMAND | MF_GRAYED);
				else
					mnuPopupMenu->EnableMenuItem (ID_CONTEXT_SKIPPROCESSING, MF_BYCOMMAND);		

				mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
				return;
			}
			else
			{
				mnuPopupMenu = mnuContextStatus.GetSubMenu(0);
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_RESCAN, MF_BYCOMMAND);
			}
		}

		mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_DETAILS, MF_BYCOMMAND);		
		mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_REMOVE, MF_BYCOMMAND);
		mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_DELETEFILES, MF_BYCOMMAND);
		while (nItem != -1)
		{
			psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
			if (psFound->Status != DONE && psFound->Status != VERIFIED)
			{
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_RESCAN, MF_BYCOMMAND | MF_GRAYED);	
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_DETAILS, MF_BYCOMMAND | MF_GRAYED);	
			}
			if (psFound->bWorking)
			{								
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_DELETEFILES, MF_BYCOMMAND | MF_GRAYED);
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_REMOVE, MF_BYCOMMAND | MF_GRAYED);
			}

			if (psFound->bFilesDeleted)
			{
				//If this is a RAR set, we allow Deleting files from the set even though the first file may be missing
				if (FileUtils::DetermineSetType(psFound->m_sParfileName) != "RAR")
					mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_DELETEFILES, MF_BYCOMMAND | MF_GRAYED);
			}
			nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
		}

		mnuPopupMenu->EnableMenuItem (ID_CONTEXT_SKIPPROCESSING, MF_BYCOMMAND | MF_GRAYED);
		//If any items are being worked on..
		for (int i=0; i<m_lstStatus.GetItemCount(); i++)
		{
			sFoundPar *pS = (sFoundPar *)m_lstStatus.GetItemData(i);
			if (pS->bWorking)
			{
				mnuPopupMenu->EnableMenuItem (ID_CONTEXTSTATUS_RESCAN, MF_BYCOMMAND | MF_GRAYED);
				mnuPopupMenu->EnableMenuItem (ID_CONTEXT_SKIPPROCESSING, MF_BYCOMMAND);
				break;
			}
		}

		mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	else 
	{
		CMenu *mnuBackgroundPopupMenu = mnuContextStatus.GetSubMenu(1);
		if (m_lstStatus.GetItemCount() > 0)
			mnuBackgroundPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void CParNRarDlg::OnContextstatusDetails()
{
	int nItem = -1;
	nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	if (nItem != -1)
		ShowDetailsDlg(nItem);
}

void CParNRarDlg::OnContextstatusRescan()
{
	int nItem = -1;
	if (!AskVerifyOnly())
		return;

	ClearSkipped();

	nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	if (nItem == -1)
		return;
	VerifyNextItem(nItem, true);	
}

void CParNRarDlg::OnContextstatusDeleteFiles()
{
	//Loop through all selected items
	CString cstrMessage = "Are you sure you want to delete all files in ";
	if (m_lstStatus.GetSelectedCount() <= 1)
		cstrMessage += "this file set?";
	else
		cstrMessage += "these file sets?";

	if (MessageBox(cstrMessage, "Are you sure?", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES )
	{
		int nItem = -1;
		nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
		while (nItem != -1)
		{
			sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
			if (!psFound->bWorking)
			{
				m_lstStatus.SetItem(nItem, 1, LVIF_TEXT, "Deleting..", 0, 0, 0, 0, 0);
				psFound->bFilesDeleted = true;
				theApp.Trace("[%s] Deleting files from menu click..", psFound->m_sParfileName.c_str());

				//Delete VerifyPar to remove any file references
				psFound->DeleteVerifyPar();
			
				vector<string> vOtherFiles;
				if (psFound->bFilesDeleted)
					vOtherFiles = psFound->m_vOtherFiles;
				if (FileUtils::DetermineSetType(psFound->m_sParfileName) == "RAR")
				{
					copy(psFound->m_setContainedFiles.begin(), psFound->m_setContainedFiles.end(), back_inserter(vOtherFiles));
				}
				VerifyPar::StartDelete(&(psFound->verifyPar), psFound->m_sParfileName, psFound->m_sParfilePath, psFound->m_sPathOffMonitorDir, psFound->m_sMonitorDir, true, vOtherFiles);
			}
			nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
		}
	}
}

void CParNRarDlg::OnContextstatusRemove()
{
	//Loop through all selected items
	CString cstrMessage = "Are you sure you want to remove ";
	if (m_lstStatus.GetSelectedCount() <= 1)
		cstrMessage += "this file item?";
	else
		cstrMessage += "these items?";

	if (MessageBox(cstrMessage, "Are you sure?", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES )
	{
		int nItem = -1;
		nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
		while (nItem != -1)
		{
			sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
			if (!psFound->bWorking)
			{
				sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
				delete psFound;
				m_lstStatus.DeleteItem( nItem );
				nItem = m_lstStatus.GetNextItem(-1, LVNI_SELECTED);
			}
		}	
	}
}

void CParNRarDlg::OnContextSkipProcessing()
{
	int nItem = -1;
	nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	while (nItem != -1)
	{		
		sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);

		char szStatus[1025];
		m_lstStatus.GetItemText(nItem, 1, szStatus, 1024);
		if ( strcmp(szStatus, "Done") && strcmp(szStatus, "Already Verified") && psFound->Status != DONE && psFound->Status != VERIFIED) 
		{
			m_lstStatus.SetItem(nItem, 1, LVIF_TEXT, "Skipping..", 0, 0, 0, 0, 0);
			psFound->bSkip = true;
			if (psFound->verifyPar)
				psFound->verifyPar->SkipVerify();				
		}

		nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	}
}

void CParNRarDlg::OnContextOpenContainingFolder()
{
	int nItem = -1;
	nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	while (nItem != -1)
	{
		sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);

		ShellExecute(NULL, "open", psFound->m_sParfilePath.c_str(), 0, 0, SW_SHOW); 
		nItem = m_lstStatus.GetNextItem(nItem, LVNI_SELECTED);
	}
}

void CParNRarDlg::OnContextstatusRemoveDone()
{
	if (MessageBox("Are you sure you want to remove all done items?", "Are you sure?", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES )
	{
		for (int i=m_lstStatus.GetItemCount()-1; i>=0; i--)
		{
			char szStatus[1025];
			m_lstStatus.GetItemText(i, 1, szStatus, 1024);
			if ( !strcmp(szStatus, "Done") || !strcmp(szStatus, "Already Verified")) 
			{
				sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(i);
				delete psFound;
				m_lstStatus.DeleteItem( i );
			}
		}
	}
}
void CParNRarDlg::ShowDetailsDlg(int nItem)
{
	DetailsDlg dlgDetails;
	sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
	dlgDetails.SetCaption(psFound->m_sParfileName.c_str());
	dlgDetails.SetText(psFound->cstrDetails);
	dlgDetails.DoModal();
}
void CParNRarDlg::OnLvnKeydownStatuslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	//Ctrl-A
	if (pLVKeyDow->wVKey == 65)
	{
		if (GetAsyncKeyState(VK_CONTROL))
			m_lstStatus.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	}
	else if (pLVKeyDow->wVKey == VK_DELETE)
	{
		int nItem = m_lstStatus.GetNextItem(-1, LVNI_SELECTED);
		if (nItem != -1)
		{
			sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
			if (!psFound->bWorking)
				if (MessageBox("Are you sure you want to remove this item?", "Are you sure?", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES )
				{
					sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(nItem);
					delete psFound;
					m_lstStatus.DeleteItem( nItem );
				}
		}
	}
	*pResult = 0;
}

void CParNRarDlg::OnTimer(UINT nIDEvent)
{
	m_lElapsed++;

	//Check whether time is over
	CString cstrRestartDelay = theApp.m_Options.GetRestartDelay().c_str();	

	if (cstrRestartDelay == "")
	{
		m_bTimer = false;
		KillTimer(1);		
		GetDlgItem(ID_GO)->SetWindowText("&Go");
		SetStatus("Press Go to Process", false);
	}
	else
	{
		long lRestartDelay = atol(cstrRestartDelay.GetBuffer(0));
		if (m_lElapsed > lRestartDelay)
		{
			m_bVerifyOnly = false;
			SetIndicatorsPause();
			StartCheck();
		}
		else
		{			
			CString cstrStatus = "Restarting in ";
			char sz[256];
			cstrStatus += _ltoa(lRestartDelay - m_lElapsed, sz, 10);
			cstrStatus += " second(s)";
			GetDlgItem(ID_GO)->SetWindowText("&Stop");
			SetStatus(cstrStatus, false);
		}
	}

	ResetSystrayMenu();
	CDialog::OnTimer(nIDEvent);
}

void CParNRarDlg::SetStatus(CString cstrStatus, bool bAnimate)
{
	m_TrayIcon.SetTooltipText(cstrStatus.GetBuffer(0));
	if (!bAnimate)
		m_TrayIcon.StopAnimation();
	else
		m_TrayIcon.Animate(200, -1);
	m_StatusBar.SetPaneText(0, cstrStatus );
}

void CParNRarDlg::SetIndicatorsPause()
{
	m_bTimer = false;
	KillTimer(1);
	SetStatus("Processing", true);
	GetDlgItem(ID_GO)->SetWindowText("&Pause");
	ResetSystrayMenu();

}

void CParNRarDlg::ShowHelp()
{
	string sHelpPath = theApp.m_szOriginalPath;
	if (sHelpPath == "")
		sHelpPath += "help\\help.html";
	else
		sHelpPath += "\\help\\help.html";

	CHyperLink::GotoURL(sHelpPath.c_str(), SW_SHOW);
}

void CParNRarDlg::OnBnClickedHelp()
{
	ShowHelp();
}

void CParNRarDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType == SIZE_MINIMIZED)
	{
		if (theApp.m_Options.GetMinSysTray())
		{
			m_TrayIcon.MinimiseToTray(this);
			ResetSystrayMenu();
		}
	}
	else
	{
		Resize();
	}
}

void CParNRarDlg::Resize()
{
	CRect rectForm, rectLst;
    GetClientRect(&rectForm);
	
	//Make sure we are drawn already
	CListCtrlEx *lstStatus = (CListCtrlEx *)GetDlgItem(IDC_STATUSLIST);
	if (lstStatus == NULL)
		return;
	
	long lPaneWidth = 1;

	//Statusbar
	CRect rectStatusbar;
	long lStatusbarTop;
	m_StatusBar.GetWindowRect(&rectStatusbar);
	lStatusbarTop = rectForm.Height() - rectStatusbar.Height(); 
	m_StatusBar.MoveWindow(0, lStatusbarTop, rectForm.Width(), rectStatusbar.Height());
	if (rectForm.Width() > 100)
		lPaneWidth = rectForm.Width() - 100;
	m_StatusBar.SetPaneInfo(0, ID_STATUSBAR_IND, SBPS_NORMAL, lPaneWidth);

	//Buttons
	CRect rectButton, rectDonate;
	long lButtonTop, lDonateTop;
	GetDlgItem(ID_GO)->GetWindowRect(&rectButton);
	GetDlgItem(IDC_DONATE)->GetWindowRect(&rectDonate);

	lDonateTop = lStatusbarTop - 5 - rectDonate.Height();
	lButtonTop = lStatusbarTop - 3 - rectDonate.Height();
	GetDlgItem(ID_GO)->MoveWindow(5, lButtonTop, rectButton.Width(), rectButton.Height());
	GetDlgItem(ID_GO)->Invalidate();
	GetDlgItem(IDOK3)->MoveWindow(10 + rectButton.Width(), lButtonTop, rectButton.Width(), rectButton.Height());	
	GetDlgItem(IDOK3)->Invalidate();

	GetDlgItem(IDC_DONATE)->MoveWindow(rectForm.Width() - rectDonate.Width() - 5, lDonateTop, rectDonate.Width(), rectDonate.Height());
	GetDlgItem(IDC_DONATE)->Invalidate();

	GetDlgItem(ID_ABOUT)->MoveWindow(rectForm.Width() - rectDonate.Width() - rectButton.Width() - 20, lButtonTop, rectButton.Width(), rectButton.Height());
	GetDlgItem(ID_ABOUT)->Invalidate();
	GetDlgItem(ID_HELP)->MoveWindow(rectForm.Width() - rectDonate.Width() - (rectButton.Width()*2) - 25, lButtonTop, rectButton.Width(), rectButton.Height());
	GetDlgItem(ID_HELP)->Invalidate();
	GetDlgItem(ID_OPTIONS)->MoveWindow(rectForm.Width() - rectDonate.Width() - (rectButton.Width()*3) - 30, lButtonTop, rectButton.Width(), rectButton.Height());
	GetDlgItem(ID_OPTIONS)->Invalidate();
			
	//Listview
	CRect rectMonitor;
	long lListviewTop;
	GetDlgItem(IDC_MONITORDIR)->GetWindowRect(&rectMonitor);
	ScreenToClient(rectMonitor);
	lListviewTop = rectMonitor.top + rectMonitor.Height() + 5;
	lstStatus->GetWindowRect(&rectLst);	
	lstStatus->MoveWindow(5, lListviewTop, rectForm.Width() - 10, lDonateTop - lListviewTop - 5); 
	lstStatus->Invalidate();
}

void CParNRarDlg::OnSystrayOpen()
{
	m_TrayIcon.MaximiseFromTray(this);
}

void CParNRarDlg::OnSystrayExit()
{
	DestroyWindow();
}

void CParNRarDlg::OnSystrayOptions()
{
	OnBnClickedOptions();
}

void CParNRarDlg::OnSystrayAbout()
{
	OnBnClickedAbout();
}

void CParNRarDlg::OnSystrayGo()
{
	OnBnClickedGo();
}

void CParNRarDlg::OnDestroy()
{
	//Save window settings
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	if (wp.showCmd == SW_MAXIMIZE)
		theApp.m_Options.SetDwordOption("MainMax", 1);
	else if (wp.showCmd != SW_MINIMIZE)
	{
		CRect rectForm;
		theApp.m_Options.SetDwordOption("MainMax", 0);
		GetWindowRect(&rectForm);
		theApp.m_Options.SetDwordOption("MainWidth", rectForm.Width());
		theApp.m_Options.SetDwordOption("MainHeight", rectForm.Height());
		theApp.m_Options.SetDwordOption("MainLeft", rectForm.left);
		theApp.m_Options.SetDwordOption("MainTop", rectForm.top);
	}

	char sz[64];
	CHeaderCtrl* pHeader = (CHeaderCtrl*) m_lstStatus.GetDlgItem(0);
	for (long l=0; l<pHeader->GetItemCount(); l++)
	{		
		CString cstr = "MainColWidth";
		cstr += _ltoa(l, sz, 10);
		theApp.m_Options.SetDwordOption(cstr.GetBuffer(0), m_lstStatus.GetColumnWidth(l));
	}	

	m_TrayIcon.HideIcon();

	CDialog::OnDestroy();
}

void CParNRarDlg::ResetSystrayMenu()
{
	CString cstrText;

	GetDlgItem(ID_GO)->GetWindowText(cstrText);
	m_TrayIcon.m_Menu.GetSubMenu(0)->ModifyMenu(ID_SYSTRAY_GO, MF_BYCOMMAND, ID_SYSTRAY_GO, cstrText.GetBuffer(0)); 
}

void CParNRarDlg::SaveLastMonitorDirs()
{
	vector<string> vLastMonitorDirs;

	for (int iIndex=0; iIndex<m_MonitorDir.GetCount(); iIndex++)
	{
		CString cstrMonitorDir;
		m_MonitorDir.GetLBText(iIndex, cstrMonitorDir);
		vLastMonitorDirs.push_back(cstrMonitorDir.GetBuffer(0));
	}

	theApp.m_Options.SetLastMonitorDirs(vLastMonitorDirs);	
}

//Returns false if cancel is hit
bool CParNRarDlg::AskVerifyOnly()
{
	m_bVerifyOnly = false;

	//Do not ask user if this is run from the command prompt, or they have previously told us not to
	if (!theApp.m_Options.GetGoOnStart() && theApp.m_Options.GetAskVerifyOnly())
	{
		AskDlg dlgAsk;
		dlgAsk.SetCaption(" Go");
		dlgAsk.SetQuestion("Do you want to verify files only?\nSelect No to repair and extract files also.");
		dlgAsk.SetButtons("&Yes", "&No");
		if (dlgAsk.DoModal() == IDCANCEL)
			return false;
		theApp.m_Options.SetAskVerifyOnly(!dlgAsk.NeverAskAgain());		
		if (dlgAsk.ButtonClicked() == 1)
			m_bVerifyOnly = true;
	}
	return true;
}

void CParNRarDlg::OnCancel()
{
	if (theApp.m_Options.GetMinSysTray() && theApp.m_Options.GetCloseToTray())
	{
		m_TrayIcon.MinimiseToTray(this);
		ResetSystrayMenu();
	}
	else
		CDialog::OnCancel();
}

void CParNRarDlg::OnBnClickedClearhistory()
{
	if (MessageBox("Are you sure you want to remove all items from the Monitor Directory history?", "Are you sure?", MB_DEFBUTTON2 | MB_YESNO | MB_ICONQUESTION) == IDYES )
	{
		theApp.m_Options.ClearLastMonitorDirs();

		CString cstrMonitorDir;
		m_MonitorDir.GetWindowText(cstrMonitorDir);

		m_MonitorDir.ResetContent();

		m_MonitorDir.SetWindowText(cstrMonitorDir);
	}
}

void CParNRarDlg::ClearSkipped()
{
	for (int i=m_lstStatus.GetItemCount()-1; i>=0; i--)
	{
		sFoundPar *psFound = (sFoundPar *)m_lstStatus.GetItemData(i);
		char szStatus[1025];

		m_lstStatus.GetItemText(i, 1, szStatus, 1024);
		if (!strcmp(szStatus, "Skipped"))
		{
			m_lstStatus.SetItem(i, 1, LVIF_TEXT, "", 0, 0, 0, 0, 0);
		}
		psFound->bSkip = false;
		psFound->Status == NONE;
	}

}