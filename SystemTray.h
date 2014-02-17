/////////////////////////////////////////////////////////////////////////////
// SystemTray.h : header file
//
// Written by Chris Maunder (cmaunder@mail.com)
// Copyright (c) 1998.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 

#ifndef _INCLUDED_SYSTEMTRAY_H_
#define _INCLUDED_SYSTEMTRAY_H_

#ifdef NOTIFYICONDATA_V1_SIZE   // If NOTIFYICONDATA_V1_SIZE, then we can use fun stuff
#define SYSTEMTRAY_USEW2K
#else
#define NIIF_NONE 0
#endif

// #include <afxwin.h>
#include <afxtempl.h>
#include <afxdisp.h>    // COleDateTime

/////////////////////////////////////////////////////////////////////////////
// CSystemTray window

class CSystemTray : public CWnd
{
// Construction/destruction
public:
    CSystemTray();
    CSystemTray(CWnd* pWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID, 
                BOOL bhidden = FALSE,
                LPCTSTR szBalloonTip = NULL, LPCTSTR szBalloonTitle = NULL, 
                DWORD dwBalloonIcon = NIIF_NONE, UINT uBalloonTimeout = 10);
    virtual ~CSystemTray();

    DECLARE_DYNAMIC(CSystemTray)

// Operations
public:
    BOOL Visible() { return !m_bHidden; }

    // Create the tray icon
    BOOL Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID,
           BOOL bHidden = FALSE,
           LPCTSTR szBalloonTip = NULL, LPCTSTR szBalloonTitle = NULL, 
           DWORD dwBalloonIcon = NIIF_NONE, UINT uBalloonTimeout = 10);
    BOOL DestroyWindow();

    // Change or retrieve the Tooltip text
    BOOL    SetTooltipText(LPCTSTR pszTooltipText);
    BOOL    SetTooltipText(UINT nID);
    CString GetTooltipText() const;

    // Change or retrieve the icon displayed
    BOOL  SetIcon(HICON hIcon);
    BOOL  SetIcon(LPCTSTR lpszIconName);
    BOOL  SetIcon(UINT nIDResource);
    BOOL  SetStandardIcon(LPCTSTR lpIconName);
    BOOL  SetStandardIcon(UINT nIDResource);
    HICON GetIcon() const;

    void  SetFocus();
    BOOL  HideIcon();
    BOOL  ShowIcon();
    BOOL  AddIcon();
    BOOL  RemoveIcon();
    BOOL  MoveToRight();

    BOOL ShowBalloon(LPCTSTR szText, LPCTSTR szTitle = NULL,
                     DWORD dwIcon = NIIF_NONE, UINT uTimeout = 10);

    // For icon animation
    BOOL  SetIconList(UINT uFirstIconID, UINT uLastIconID); 
    BOOL  SetIconList(HICON* pHIconList, UINT nNumIcons); 
    BOOL  Animate(UINT nDelayMilliSeconds, int nNumSeconds = -1);
    BOOL  StepAnimation();
    BOOL  StopAnimation();

    // Change menu default item
    void GetMenuDefaultItem(UINT& uItem, BOOL& bByPos);
    BOOL SetMenuDefaultItem(UINT uItem, BOOL bByPos);

    // Hatr added
    // Change or retrive the window to send menu commands to
    BOOL  SetTargetWnd(CWnd* pTargetWnd);
    CWnd* GetTargetWnd() const;

    BOOL SetMenuText(UINT uiCmd, LPCTSTR szText);
    BOOL SetMenuText(UINT uiCmd, UINT uiID);
    BOOL CheckMenuItem(UINT uiCmd, BOOL bCheck);
    BOOL EnableMenuItem(UINT uiCmd, BOOL bEnable);
    BOOL DeleteMenu(UINT uiCmd);
    BOOL AddMenuTitle(LPCTSTR szMenuText);

    // Change or retrieve the window to send notification messages to
    BOOL  SetNotificationWnd(CWnd* pNotifyWnd);
    CWnd* GetNotificationWnd() const;

    // Change or retrieve  notification messages sent to the window
    BOOL  SetCallbackMessage(UINT uCallbackMessage);
    UINT  GetCallbackMessage() const;

// Static functions
public:
    void MinimiseToTray(CWnd* pWnd);
    void MaximiseFromTray(CWnd* pWnd);
	CMenu        m_Menu;
	BOOL IsMinimized();

public:
    // Default handler for tray notification message
    virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSystemTray)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
    void Initialise();
    void InstallIconPending();

	BOOL m_bMinimized;
    BOOL            m_bHidden;          // Has the icon been hidden?
    BOOL            m_bRemoved;         // Has the icon been removed?
    BOOL            m_bShowIconPending; // Show the icon once tha taskbar has been created
    BOOL            m_bWin2K;           // Use new W2K features?
    NOTIFYICONDATA  m_tnd;
	bool m_bAnimating;

    CArray<HICON, HICON> m_IconList;
    UINT         m_uIDTimer;
    int          m_nCurrentIcon;
    COleDateTime m_StartTime;
    int          m_nAnimationPeriod;
    HICON        m_hSavedIcon;
    UINT         m_DefaultMenuItemID;
    BOOL         m_DefaultMenuItemByPos;
    CWnd         *m_pTargetWnd;     // Hatr added    

// Static data
public:
    static const UINT  sm_uiCallbackMessage; // Hatr added
protected:
    static const UINT m_nTimerID;
    static UINT  m_nMaxTooltipLength;
    static const UINT m_nTaskbarCreatedMsg;
    static CWnd  m_wndInvisible;

    static BOOL GetW2K();
#ifndef _WIN32_WCE
    static CRect GetTrayWndRect();
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CSystemTray)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
#ifndef _WIN32_WCE
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
#endif
    LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
}; // class CSystemTray


#endif

/////////////////////////////////////////////////////////////////////////////
