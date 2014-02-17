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
#include <string>
#include "Hyperlink.h"
#include "TextProgressCtrl.h"
#include "ListCtrlEx.h"
#include "PnrMessage.h"
#include "SystemTray.h"
#include "MyComboBox.h"

using namespace std;

#define WM_ICON_NOTIFY  WM_APP+10	

// CParNRarDlg dialog
class CParNRarDlg : public CDialog
{
// Construction
public:
	CParNRarDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PARNRAR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support	
	virtual void OnCancel();

// Implementation
protected:
	CSystemTray m_TrayIcon;
	HICON m_hIcon;
	long m_lLogPos;				//Position of Log window scrollbar
	long m_lElapsed;			//Amount of time elapsed on the timer
	bool m_bTimer;				//Whether the timer is running
	bool m_bSingleItemVerify;	//True if we are only verifying one item
	CStatusBar m_StatusBar;
	bool m_bWorking;			//True if we are verifying a file set
	CMyComboBox m_MonitorDir;	
	bool m_bVerifyOnly;
	bool m_bInitialized;
	CHyperLink m_lnkDonate;

	void Initialize();
	void VerifyNextItem();
	void VerifyNextItem(int idxToVerify, bool bSingle);
	void SetIndicatorsPause();
	void ShowHelp();
	int FindItemInList(string sParfileName, int iStart = -1);
	void StartCheck();
	void AddDetail(int iIndex, string sParfileName, string sNewDetails, bool bTrace);
	bool CheckForExistingItem(sFoundPar *psFound);
	void Resize();
	void ResetSystrayMenu();
	void SetStatus(CString cstrStatus, bool bAnimate);
	void SaveLastMonitorDirs();
	bool AskVerifyOnly();
	void AskCheckForNewVersion();
	void OnClickDonate(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT RemoveItemFromList(string sParfileName);
	void ReplaceItem(sFoundPar *psOld, sFoundPar *psNew);
	void ClearSkipped();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void ShowDetailsDlg(int nItem);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnDoneError(WPARAM, LPARAM);
	afx_msg LRESULT OnParDone(WPARAM, LPARAM);
	afx_msg LRESULT OnFoundPar(WPARAM, LPARAM);
	afx_msg LRESULT OnParStatus(WPARAM, LPARAM);
	afx_msg LRESULT OnParProgress(WPARAM, LPARAM);
	afx_msg LRESULT OnParAddDetails(WPARAM, LPARAM);
	afx_msg LRESULT OnParNoPars(WPARAM, LPARAM);
	afx_msg LRESULT OnRemove(WPARAM, LPARAM);
	afx_msg LRESULT OnPauseMsg(WPARAM, LPARAM);
	afx_msg LRESULT OnPareStatus(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedBrowseDir();
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedOptions();
	CListCtrlEx m_lstStatus;
	afx_msg void OnNMDblclkStatuslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnContextstatusDetails();
	afx_msg void OnContextstatusRescan();
	afx_msg void OnContextstatusRemove();
	afx_msg void OnContextstatusRemoveDone();
	afx_msg void OnContextOpenContainingFolder();
	afx_msg void OnContextSkipProcessing();
	afx_msg void OnContextstatusDeleteFiles();
	afx_msg void OnLvnKeydownStatuslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedHelp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSystrayOpen();
	afx_msg void OnSystrayExit();
	afx_msg void OnSystrayOptions();
	afx_msg void OnSystrayAbout();
	afx_msg void OnSystrayGo();
	afx_msg void OnDestroy();
public:
	afx_msg void OnBnClickedClearhistory();
};

