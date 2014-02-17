/////////////////////////////////////////////////////////////////////////////
// SystemTray.cpp : implementation file
//
// This is a conglomeration of ideas from the MSJ "Webster" application,
// sniffing round the online docs, and from other implementations such
// as PJ Naughter's "CTrayNotifyIcon" (http://indigo.ie/~pjn/ntray.html)
// especially the "CSystemTray::OnTrayNotification" member function.
// Joerg Koenig suggested the icon animation stuff
//
// This class is a light wrapper around the windows system tray stuff. It
// adds an icon to the system tray with the specified ToolTip text and 
// callback notification value, which is sent back to the Parent window.
//
// The tray icon can be instantiated using either the constructor or by
// declaring the object and creating (and displaying) it later on in the
// program. eg.
//
//        CSystemTray m_SystemTray;    // Member variable of some class
//        
//        ... 
//        // in some member function maybe...
//        m_SystemTray.Create(pParentWnd, WM_MY_NOTIFY, "Click here", 
//                          hIcon, nSystemTrayID);
//
// Written by Chris Maunder (cmaunder@mail.com)
// Copyright (c) 1998.
//
// Updated: 25 Jul 1998 - Added icon animation, and derived class
//                        from CWnd in order to handle messages. (CJM)
//                        (icon animation suggested by Joerg Koenig.
//                        Added API to set default menu item. Code provided
//                        by Enrico Lelina.
//
// Updated: 6 June 1999 - SetIcon can now load non-standard sized icons (Chip Calvert)
//                        Added "bHidden" parameter when creating icon
//                        (Thanks to Michael Gombar for these suggestions)
//                        Restricted tooltip text to 64 characters.
//
// Updated: 9 Nov 1999  - Now works in WindowsCE.
//                        Fix for use in NT services (Thomas Mooney, TeleProc, Inc)
//                        Added W2K stuff by Michael Dunn
//
// Updated: 1 Jan 2000  - Added tray minimisation stuff.
// 
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
// The author accepts no liability for any damage caused through use.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//
/////////////////////////////////////////////////////////////////////////////
    
#include "stdafx.h"
#include "TitledPopupMenu.h"
#include "SystemTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _WIN32_WCE  // Use C++ exception handling instead of structured.
#undef TRY
#undef CATCH
#undef END_CATCH
#define TRY try
#define CATCH(ex_class, ex_object) catch(ex_class* ex_object)
#define END_CATCH
#endif  // _WIN32_WCE

#ifndef _countof
#define _countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)

const UINT CSystemTray::m_nTimerID    = 4567;
UINT CSystemTray::m_nMaxTooltipLength  = 64;     // This may change...
const UINT CSystemTray::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));
CWnd  CSystemTray::m_wndInvisible;
const UINT CSystemTray::sm_uiCallbackMessage = ::RegisterWindowMessage(_T("CSystemTray::TrayNotify"));

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

CSystemTray::CSystemTray()
{
    Initialise();
} // CSystemTray::CSystemTray()

CSystemTray::CSystemTray(CWnd* pParent,             // The window that will recieve tray notifications
                         UINT uCallbackMessage,     // the callback message to send to parent
                         LPCTSTR szToolTip,         // tray icon tooltip
                         HICON icon,                // Handle to icon
                         UINT uID,                  // Identifier of tray icon
                         BOOL bHidden /*=FALSE*/,   // Hidden on creation?                  
                         LPCTSTR szBalloonTip /*=NULL*/,    // Ballon tip (w2k only)
                         LPCTSTR szBalloonTitle /*=NULL*/,  // Balloon tip title (w2k)
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,// Ballon tip icon (w2k)
                         UINT uBalloonTimeout /*=10*/)      // Balloon timeout (w2k)
{
    Initialise();
    Create(pParent, uCallbackMessage, szToolTip, icon, uID, bHidden,
           szBalloonTip, szBalloonTitle, dwBalloonIcon, uBalloonTimeout);
} // CSystemTray::CSystemTray()

CSystemTray::~CSystemTray()
{
} // CSystemTray::~CSystemTray()

void CSystemTray::Initialise()
{
    memset(&m_tnd, 0, sizeof(m_tnd));
    m_tnd.cbSize = sizeof(m_tnd);

    m_bHidden  = TRUE;
    m_bRemoved = TRUE;
	m_bAnimating = false;

    m_DefaultMenuItemID    = 0;
    m_DefaultMenuItemByPos = TRUE;

    m_bShowIconPending = FALSE;

    m_uIDTimer   = 0;
    m_hSavedIcon = NULL;

#ifdef SYSTEMTRAY_USEW2K
    OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);
    m_bWin2K = ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 );
#else
    m_bWin2K = FALSE;
#endif
} // CSystemTray::Initialise()

// update by Michael Dunn, November 1999
//
//  New version of Create() that handles new features in Win 2K.
//
// Changes:
//  szTip: Same as old, but can be 128 characters instead of 64.
//  szBalloonTip: Text for a balloon tooltip that is shown when the icon
//                is first added to the tray.  Pass "" if you don't want
//                a balloon.
//  szBalloonTitle: Title text for the balloon tooltip.  This text is shown
//                  in bold above the szBalloonTip text.  Pass "" if you
//                  don't want a title.
//  dwBalloonIcon: Specifies which icon will appear in the balloon.  Legal
//                 values are:
//                     NIIF_NONE: No icon
//                     NIIF_INFO: Information
//                     NIIF_WARNING: Exclamation
//                     NIIF_ERROR: Critical error (red circle with X)
//  uBalloonTimeout: Number of seconds for the balloon to remain visible.
//                   Must be between 10 and 30 inclusive.

//	5/3/06 gmilow - Modified 

BOOL CSystemTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, 
                         HICON icon, UINT uID, BOOL bHidden /*=FALSE*/,
                         LPCTSTR szBalloonTip /*=NULL*/, 
                         LPCTSTR szBalloonTitle /*=NULL*/,  
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,
                         UINT uBalloonTimeout /*=10*/)
{
    m_nMaxTooltipLength = _countof(m_tnd.szTip);
    
    // Make sure we avoid conflict with other messages
    if (uCallbackMessage == 0) uCallbackMessage = CSystemTray::sm_uiCallbackMessage;
    ASSERT(uCallbackMessage >= WM_APP);

    // Tray only supports tooltip text up to m_nMaxTooltipLength) characters
    ASSERT(AfxIsValidString(szToolTip));
    ASSERT(_tcslen(szToolTip) <= m_nMaxTooltipLength);

    // Create an invisible window
    CWnd::CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0,0,0,0, NULL, 0);

    // load up the NOTIFYICONDATA structure
    m_tnd.hWnd   = pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
    m_tnd.uID    = uID;
    m_tnd.hIcon  = icon;
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON;
    m_tnd.uCallbackMessage = uCallbackMessage;

    // Add tip if exist
    if (szToolTip)
        _tcsncpy(m_tnd.szTip, szToolTip, m_nMaxTooltipLength-1);
    if (m_tnd.szTip[0])
        m_tnd.uFlags |= NIF_TIP;


#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K && szBalloonTip)
    {
        // The balloon tooltip text can be up to 255 chars long.
        ASSERT(AfxIsValidString(szBalloonTip));
        ASSERT(lstrlen(szBalloonTip) < 256);

        // The balloon title text can be up to 63 chars long.
        if (szBalloonTitle)
        {
            ASSERT(AfxIsValidString(szBalloonTitle));
            ASSERT(lstrlen(szBalloonTitle) < 64);
        }

        // dwBalloonIcon must be valid.
        if (NIIF_NONE != dwBalloonIcon    && NIIF_INFO  != dwBalloonIcon &&
            NIIF_WARNING != dwBalloonIcon && NIIF_ERROR != dwBalloonIcon ) 
        {
            ASSERT(FALSE);
        }

        // The timeout must be between 10 and 30 seconds.
        if (uBalloonTimeout < 10 || uBalloonTimeout > 30)
        {
            ASSERT(FALSE);
        }

        m_tnd.uFlags |= NIF_INFO;

        _tcsncpy(m_tnd.szInfo, szBalloonTip, 255);
        if (szBalloonTitle)
            _tcsncpy(m_tnd.szInfoTitle, szBalloonTitle, 63);
        else
            m_tnd.szInfoTitle[0] = _T('\0');

        m_tnd.uTimeout    = uBalloonTimeout * 1000; // convert time to ms
        m_tnd.dwInfoFlags = dwBalloonIcon;
    }
#endif

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K && m_bHidden)
    {
        m_tnd.uFlags |= NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;
    }
#endif

    BOOL bResult = TRUE;
    if (!bHidden || m_bWin2K)
    {
        bResult = Shell_NotifyIcon(NIM_ADD, &m_tnd);
        m_bShowIconPending = m_bHidden = m_bRemoved = !bResult;
    }
    m_bHidden = bHidden;

#ifdef SYSTEMTRAY_USEW2K    
    if (m_bWin2K && szBalloonTip)
    {
        // Zero out the balloon text string so that later operations won't redisplay
        // the balloon.
        m_tnd.szInfo[0] = _T('\0');
    }
#endif

    m_Menu.LoadMenu(m_tnd.uID);

    return bResult;
} // CSystemTray::Create()

BOOL CSystemTray::DestroyWindow()
{
    //TRACE("CSystemTray::DestroyWindow()\n");
    RemoveIcon();
    m_IconList.RemoveAll();
    m_Menu.DestroyMenu();
    return CWnd::DestroyWindow();
} // CSystemTray::DestroyWindow()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

//////////////////////////////////////////////////////////////////////////
//
// Function:    SetFocus()
//
// Description:
//  Sets the focus to the tray icon.  Microsoft's Win 2K UI guidelines
//  say you should do this after the user dismisses the icon's context
//  menu.
//
// Input:
//  Nothing.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November, 1999
//////////////////////////////////////////////////////////////////////////

void CSystemTray::SetFocus()
{
#ifdef SYSTEMTRAY_USEW2K
    Shell_NotifyIcon ( NIM_SETFOCUS, &m_tnd );
#endif
} // CSystemTray::SetFocus()

BOOL CSystemTray::MoveToRight()
{
    RemoveIcon();
    return AddIcon();
} // CSystemTray::MoveToRight()

BOOL CSystemTray::AddIcon()
{
    if (!m_bRemoved)
        RemoveIcon();

    if (m_tnd.hIcon)
        m_tnd.uFlags |= NIF_ICON;
    if (m_tnd.szTip)
        m_tnd.uFlags |= NIF_TIP;
    if (m_tnd.uCallbackMessage)
        m_tnd.uFlags |= NIF_MESSAGE;

    if (!Shell_NotifyIcon(NIM_ADD, &m_tnd))
        m_bShowIconPending = TRUE;
    else
        m_bRemoved = m_bHidden = FALSE;

    return (m_bRemoved == FALSE);
} // CSystemTray::AddIcon()

BOOL CSystemTray::RemoveIcon()
{
    m_bShowIconPending = FALSE;

    if (m_bRemoved)
        return TRUE;

    UINT uOldFlags = m_tnd.uFlags;
    m_tnd.uFlags = 0;
    if (Shell_NotifyIcon(NIM_DELETE, &m_tnd))
        m_bRemoved = m_bHidden = TRUE;

    m_tnd.uFlags = uOldFlags;

    return (m_bRemoved == TRUE);
} // CSystemTray::RemoveIcon()

BOOL CSystemTray::HideIcon()
{
    if (m_bRemoved || m_bHidden)
        return TRUE;

    RemoveIcon();

    return (m_bHidden == TRUE);
} // CSystemTray::HideIcon()

BOOL CSystemTray::ShowIcon()
{	
    if (m_bRemoved)
        return AddIcon();

    if (!m_bHidden)
        return TRUE;

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K)
    {
        UINT uOldFlags = m_tnd.uFlags;
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = 0;
        m_tnd.dwStateMask = NIS_HIDDEN;

		if (m_tnd.szTip)
			m_tnd.uFlags |= NIF_TIP;

        Shell_NotifyIcon ( NIM_MODIFY, &m_tnd );
        m_tnd.uFlags = uOldFlags;
    }
    else
#endif
        AddIcon();

	m_bHidden = FALSE;
    return (m_bHidden == FALSE);
} // CSystemTray::ShowIcon()

BOOL CSystemTray::SetIcon(HICON hIcon)
{
    if (hIcon == NULL)
        return FALSE;

    m_tnd.hIcon = hIcon;

    if (!m_bHidden) {
        BOOL bRet = TRUE;
        UINT uOldFlags = m_tnd.uFlags;
        m_tnd.uFlags = NIF_ICON;
        bRet = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
        m_tnd.uFlags = uOldFlags;
        if (hIcon)
            m_tnd.uFlags |= NIF_ICON;
        else
            m_tnd.uFlags &= ~NIF_ICON;
        return bRet;
    }
    else
        return TRUE;
} // CSystemTray::SetIcon()

BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName)
{
    if (lpszIconName == NULL) return FALSE;
    HICON hIcon = (HICON) ::LoadImage(AfxGetResourceHandle(), 
                                      lpszIconName,
                                      IMAGE_ICON, 
                                      0, 0,
                                      LR_DEFAULTCOLOR);

    return SetIcon(hIcon);
} // CSystemTray::SetIcon()

BOOL CSystemTray::IsMinimized()
{
	return m_bMinimized;
}

BOOL CSystemTray::SetIcon(UINT nIDResource)
{
    return SetIcon(MAKEINTRESOURCE(nIDResource));
} // CSystemTray::SetIcon()

BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName)
{
    HICON hIcon = LoadIcon(NULL, lpIconName);

    return SetIcon(hIcon);
} // CSystemTray::SetStandardIcon()

BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
    HICON hIcon = LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));

    return SetIcon(hIcon);
} // CSystemTray::SetStandardIcon()
 
HICON CSystemTray::GetIcon() const
{
    return m_tnd.hIcon;
} // CSystemTray::GetIcon()

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) 
{
	if (uFirstIconID > uLastIconID)
        return FALSE;

	const CWinApp* pApp = AfxGetApp();
    if (!pApp)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_IconList.RemoveAll();
    TRY {
	    for (UINT i = uFirstIconID; i <= uLastIconID; i++)
		    m_IconList.Add(pApp->LoadIcon(i));
    }
    CATCH(CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
} // CSystemTray::SetIconList()

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
    m_IconList.RemoveAll();

    TRY {
	    for (UINT i = 0; i <= nNumIcons; i++)
		    m_IconList.Add(pHIconList[i]);
    }
    CATCH (CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
} // CSystemTray::SetIconList()

BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/)
{
	if (m_bAnimating)
		return TRUE;

	m_bAnimating = TRUE;
    StopAnimation();

    m_nCurrentIcon = 0;
    m_StartTime = COleDateTime::GetCurrentTime();
    m_nAnimationPeriod = nNumSeconds;
    m_hSavedIcon = GetIcon();

	// Setup a timer for the animation
	m_uIDTimer = SetTimer(m_nTimerID, nDelayMilliSeconds, NULL);

    return (m_uIDTimer != 0);
} // CSystemTray::Animate()

BOOL CSystemTray::StepAnimation()
{
	m_bAnimating = FALSE;
    if (!m_IconList.GetSize())
        return FALSE;

    m_nCurrentIcon++;
    if (m_nCurrentIcon >= m_IconList.GetSize())
        m_nCurrentIcon = 0;

    return SetIcon(m_IconList[m_nCurrentIcon]);
} // CSystemTray::StepAnimation()

BOOL CSystemTray::StopAnimation()
{
    BOOL bResult = FALSE;

    if (m_uIDTimer)
	    bResult = KillTimer(m_uIDTimer);
    m_uIDTimer = 0;

    if (m_hSavedIcon)
        SetIcon(m_hSavedIcon);
    m_hSavedIcon = NULL;

    return bResult;
} // CSystemTray::StopAnimation()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip)
{
    ASSERT(AfxIsValidString(pszTip)); // (md)
    ASSERT(_tcslen(pszTip) < m_nMaxTooltipLength);

    _tcsncpy(m_tnd.szTip, pszTip, m_nMaxTooltipLength-1);

    if (!m_bHidden) {
        BOOL bRet = TRUE;
        UINT uOldFlags = m_tnd.uFlags;
        m_tnd.uFlags = NIF_TIP;

        bRet = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
        m_tnd.uFlags = uOldFlags;
        if (pszTip)
            m_tnd.uFlags |= NIF_TIP;
        else
            m_tnd.uFlags &= ~NIF_TIP;
        return bRet;
    }
    else
        return TRUE;
} // CSystemTray::SetTooltipText()

BOOL CSystemTray::SetTooltipText(UINT nID)
{
    CString strText;
    VERIFY(strText.LoadString(nID));

    return SetTooltipText(strText);
} // CSystemTray::SetTooltipText()

CString CSystemTray::GetTooltipText() const
{
    return m_tnd.szTip;
} // CSystemTray::GetTooltipText()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray support for Win 2K features.

//////////////////////////////////////////////////////////////////////////
//
// Function:    ShowBalloon
//
// Description:
//  Shows a balloon tooltip over the tray icon.
//
// Input:
//  szText: [in] Text for the balloon tooltip.
//  szTitle: [in] Title for the balloon.  This text is shown in bold above
//           the tooltip text (szText).  Pass "" if you don't want a title.
//  dwIcon: [in] Specifies an icon to appear in the balloon.  Legal values are:
//                 NIIF_NONE: No icon
//                 NIIF_INFO: Information
//                 NIIF_WARNING: Exclamation
//                 NIIF_ERROR: Critical error (red circle with X)
//  uTimeout: [in] Number of seconds for the balloon to remain visible.  Can
//            be between 10 and 30 inclusive.
//
// Returns:
//  TRUE if successful, FALSE if not.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November 1999
//////////////////////////////////////////////////////////////////////////

BOOL CSystemTray::ShowBalloon(LPCTSTR szText,
                              LPCTSTR szTitle  /*=NULL*/,
                              DWORD   dwIcon   /*=NIIF_NONE*/,
                              UINT    uTimeout /*=10*/ )
{
#ifndef SYSTEMTRAY_USEW2K
    return FALSE;
#else
    // Bail out if we're not on Win 2K.
    if (!m_bWin2K)
        return FALSE;

    // Verify input parameters.

    // The balloon tooltip text can be up to 255 chars long.
    ASSERT(AfxIsValidString(szText));
    ASSERT(lstrlen(szText) < 256);

    // The balloon title text can be up to 63 chars long.
    if (szTitle)
    {
        ASSERT(AfxIsValidString( szTitle));
        ASSERT(lstrlen(szTitle) < 64);
    }

    // dwBalloonIcon must be valid.
    ASSERT(NIIF_NONE == dwIcon    || NIIF_INFO == dwIcon ||
           NIIF_WARNING == dwIcon || NIIF_ERROR == dwIcon);

    // The timeout must be between 10 and 30 seconds.
    ASSERT(uTimeout >= 10 && uTimeout <= 30);

    // Remember flags
    UINT uOldFlags = m_tnd.uFlags;

    m_tnd.uFlags = NIF_INFO;
    _tcsncpy(m_tnd.szInfo, szText, 256);
    if (szTitle)
        _tcsncpy(m_tnd.szInfoTitle, szTitle, 64);
    else
        m_tnd.szInfoTitle[0] = _T('\0');
    m_tnd.dwInfoFlags = dwIcon;
    m_tnd.uTimeout = uTimeout * 1000;   // convert time to ms

    BOOL bSuccess = Shell_NotifyIcon (NIM_MODIFY, &m_tnd);

    // Zero out the balloon text string so that later operations won't redisplay
    // the balloon.
    m_tnd.szInfo[0] = _T('\0');

    // Restore flags
    m_tnd.uFlags = uOldFlags;
    
    return bSuccess;
#endif
} // CSystemTray::ShowBalloon()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

BOOL CSystemTray::SetNotificationWnd(CWnd* pWnd)
{
    // Make sure Notification window is valid
    if (!pWnd || !::IsWindow(pWnd->GetSafeHwnd()))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_tnd.hWnd = pWnd->GetSafeHwnd();

    if (m_bHidden)
        return TRUE;
    else {
        UINT uOldFlags = m_tnd.uFlags;
        m_tnd.uFlags = 0;
        BOOL bRet = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
        m_tnd.uFlags = uOldFlags;
        return bRet;
    }
} // CSystemTray::SetNotificationWnd()

CWnd* CSystemTray::GetNotificationWnd() const
{
    return CWnd::FromHandle(m_tnd.hWnd);
} // CSystemTray::GetNotificationWnd()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification message stuff

BOOL CSystemTray::SetCallbackMessage(UINT uCallbackMessage)
{
    // Make sure we avoid conflict with other messages
    if (uCallbackMessage == 0) uCallbackMessage = CSystemTray::sm_uiCallbackMessage;
    ASSERT(uCallbackMessage >= WM_APP);

    m_tnd.uCallbackMessage = uCallbackMessage;

    if (m_bHidden)
        return TRUE;
    else {
        UINT uOldFlags = m_tnd.uFlags;
        m_tnd.uFlags = NIF_MESSAGE;
        BOOL bRet = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
        m_tnd.uFlags = uOldFlags;
        return bRet;
    }
} // CSystemTray::SetCallbackMessage()

UINT CSystemTray::GetCallbackMessage() const
{
    return m_tnd.uCallbackMessage;
} // CSystemTray::GetCallbackMessage()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

BOOL CSystemTray::SetMenuDefaultItem(UINT uItem, BOOL bByPos)
{
#ifdef _WIN32_WCE
    return FALSE;
#else
    if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos)) 
        return TRUE;

    m_DefaultMenuItemID = uItem;
    m_DefaultMenuItemByPos = bByPos;   

    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    pSubMenu->SetDefaultItem(m_DefaultMenuItemID, m_DefaultMenuItemByPos);

    return TRUE;
#endif
} // CSystemTray::SetMenuDefaultItem()

void CSystemTray::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos)
{
    uItem = m_DefaultMenuItemID;
    bByPos = m_DefaultMenuItemByPos;
} // CSystemTray::GetMenuDefaultItem()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray message handlers

BEGIN_MESSAGE_MAP(CSystemTray, CWnd)
	//{{AFX_MSG_MAP(CSystemTray)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
#ifndef _WIN32_WCE
	ON_WM_SETTINGCHANGE()
#endif
    ON_REGISTERED_MESSAGE(CSystemTray::m_nTaskbarCreatedMsg, OnTaskbarCreated)
END_MESSAGE_MAP()

void CSystemTray::OnTimer(UINT nIDEvent) 
{
    if (nIDEvent != m_uIDTimer)
    {
        ASSERT(FALSE);
        return;
    }

    COleDateTime CurrentTime = COleDateTime::GetCurrentTime();
    COleDateTimeSpan period = CurrentTime - m_StartTime;
    if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period.GetTotalSeconds())
    {
        StopAnimation();
        return;
    }

    StepAnimation();
} // CSystemTray::OnTimer()

// This is called whenever the taskbar is created (eg after explorer crashes
// and restarts. Please note that the WM_TASKBARCREATED message is only passed
// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
LRESULT CSystemTray::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
    TRACE("OnTaskbarCreated()\n");
    InstallIconPending();
	return 0;
} // CSystemTray::OnTaskbarCreated()



#ifndef _WIN32_WCE
void CSystemTray::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);

    if (uFlags == SPI_SETWORKAREA) {
        InstallIconPending();	
    }
} // CSystemTray::OnSettingChange()
#endif

LRESULT CSystemTray::OnTrayNotification(UINT wParam, LONG lParam) 
{
    //Return quickly if its not for this tray icon
    if (wParam != m_tnd.uID)
        return 0L;

    CMenu *pSubMenu;
    CWnd *pTargetWnd = GetTargetWnd();
    if (!pTargetWnd)
        return 0L;

    // Clicking with right button brings up a context menu
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    BOOL bAltPressed = ((GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0);
    if (LOWORD(lParam) == WM_LBUTTONUP && bAltPressed)
#else
    if (LOWORD(lParam) == WM_RBUTTONUP)
#endif
    {    
        pSubMenu = m_Menu.GetSubMenu(0);
        if (!pSubMenu)
            return 0;

#ifndef _WIN32_WCE
        // Make chosen menu item the default (bold font)
        pSubMenu->SetDefaultItem(m_DefaultMenuItemID, m_DefaultMenuItemByPos);
#endif

        // Display and track the popup menu
        CPoint pos;
#ifdef _WIN32_WCE
        pos = CPoint(GetMessagePos());
#else
        GetCursorPos(&pos);
#endif

        pTargetWnd->SetForegroundWindow(); 
        
#ifndef _WIN32_WCE
        pSubMenu->TrackPopupMenu(0, pos.x, pos.y, pTargetWnd);
#else
        pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pTargetWnd);
#endif

        // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
        pTargetWnd->PostMessage(WM_NULL, 0, 0);
    } 
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    if (LOWORD(lParam) == WM_LBUTTONDBLCLK && bAltPressed)
#else
    else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) 
#endif
    {
        // double click received, the default action is to execute default menu item
        pTargetWnd->SetForegroundWindow();  

        UINT uItem;
        if (m_DefaultMenuItemByPos)
        {
            pSubMenu = m_Menu.GetSubMenu(0);
            if (!pSubMenu)
                return 0;
            
            uItem = pSubMenu->GetMenuItemID(m_DefaultMenuItemID);
        }
        else
            uItem = m_DefaultMenuItemID;
        
        pTargetWnd->SendMessage(WM_COMMAND, uItem, 0);
    }

    return 1;
} // CSystemTray::OnTrayNotification()

LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if (message == m_tnd.uCallbackMessage)
        return OnTrayNotification(wParam, lParam);
	
	return CWnd::WindowProc(message, wParam, lParam);
} // CSystemTray::WindowProc()

void CSystemTray::InstallIconPending()
{
    // Is the icon display pending, and it's not been set as "hidden"?
    if (!m_bShowIconPending && m_bHidden)
        return;

    // Try and recreate the icon
    if (!Shell_NotifyIcon(NIM_ADD, &m_tnd)) {
        // If it's STILL hidden, then have another go next time...
        m_bShowIconPending = TRUE;
    }
    else {
        m_bShowIconPending = FALSE;
        m_bHidden = m_bRemoved = FALSE;
    }
} // CSystemTray::InstallIconPending()

/////////////////////////////////////////////////////////////////////////////
// For minimising/maximising from system tray

BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);

    // Did we find the Main System Tray? If so, then get its size and keep going
    if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        ::GetWindowRect(hwnd, pRect);
        return TRUE;
    }

    // Did we find the System Clock? If so, then adjust the size of the rectangle
    // we have and quit (clock will be found after the system tray)
    if (_tcscmp(szClassName, _T("TrayClockWClass")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        CRect rectClock;
        ::GetWindowRect(hwnd, rectClock);
        pRect->right = rectClock.left;
        return FALSE;
    }
 
    return TRUE;
} // FindTrayWnd()
 
#ifndef _WIN32_WCE
CRect CSystemTray::GetTrayWndRect()
{
    CRect rect(0,0,0,0);

    CWnd* pWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
    if (pWnd)
    {
        pWnd->GetWindowRect(rect);
        EnumChildWindows(pWnd->m_hWnd, FindTrayWnd, (LPARAM)&rect);
    }
    else
    {
        int nWidth = GetSystemMetrics(SM_CXSCREEN);
        int nHeight = GetSystemMetrics(SM_CYSCREEN);
        rect.SetRect(nWidth-40, nHeight-20, nWidth, nHeight);
    }

    return rect;
} // CSystemTray::GetTrayWndRect()
#endif

void CSystemTray::MinimiseToTray(CWnd* pWnd)
{
#ifndef _WIN32_WCE
    CRect rectFrom, rectTo;

	ShowIcon();
    pWnd->GetWindowRect(rectFrom);
    rectTo = GetTrayWndRect();

	pWnd->DrawAnimatedRects(IDANI_CAPTION, rectFrom, rectTo);

    // Create static invisible window
    if (!::IsWindow(m_wndInvisible.m_hWnd))
        m_wndInvisible.CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0,0,0,0, NULL, 0);

    pWnd->SetParent(&m_wndInvisible);
    pWnd->ModifyStyle(WS_VISIBLE, 0);
	m_bMinimized = TRUE;
#endif
} // CSystemTray::MinimiseToTray()

void CSystemTray::MaximiseFromTray(CWnd* pWnd)
{
#ifndef _WIN32_WCE
	HideIcon();

    CRect rectTo;
    pWnd->GetWindowRect(rectTo);

    CRect rectFrom;
    rectFrom = GetTrayWndRect();

    pWnd->SetParent(NULL);

	pWnd->DrawAnimatedRects(IDANI_CAPTION, rectFrom, rectTo);

    pWnd->ModifyStyle(0, WS_VISIBLE);
    pWnd->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME |
                                   RDW_INVALIDATE | RDW_ERASE);

    // Move focus away and back again to ensure taskbar icon is recreated
    if (::IsWindow(m_wndInvisible.m_hWnd))
        m_wndInvisible.SetActiveWindow();
    pWnd->SetActiveWindow();
    pWnd->SetForegroundWindow();
	m_bMinimized = FALSE;
#endif
} // CSystemTray::MaximiseFromTray()


// Hatr added

// Change or retrive the window to send menu commands to
BOOL CSystemTray::SetTargetWnd(CWnd* pTargetWnd)
{
    m_pTargetWnd = pTargetWnd;
    return TRUE;
} // CSystemTray::SetTargetWnd()

CWnd* CSystemTray::GetTargetWnd() const
{
    if (m_pTargetWnd)
        return m_pTargetWnd;
    else
        return AfxGetMainWnd();
} // CSystemTray::GetTargetWnd()

BOOL CSystemTray::AddMenuTitle(LPCTSTR szMenuText)
{
    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    ::AddMenuTitle(pSubMenu, szMenuText);

    return TRUE;
} // CSystemTray::AddMenuTitle()

BOOL CSystemTray::DeleteMenu(UINT uiCmd)
{
    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    pSubMenu->DeleteMenu(uiCmd, MF_BYCOMMAND);

    return TRUE;
} // CSystemTray::DeleteMenu()

BOOL CSystemTray::SetMenuText(UINT uiCmd, LPCTSTR szText)
{
    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    pSubMenu->ModifyMenu(uiCmd, MF_BYCOMMAND | MF_STRING, uiCmd, szText);

    return TRUE;
} // CSystemTray::SetMenuText()

BOOL CSystemTray::SetMenuText(UINT uiCmd, UINT nID)
{
    CString sText;
    sText.LoadString(nID);
    return SetMenuText(uiCmd, sText);
} // CSystemTray::SetMenuText()

BOOL CSystemTray::CheckMenuItem(UINT uiCmd, BOOL bCheck)
{
    CString sMenuString;
    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    pSubMenu->GetMenuString(uiCmd, sMenuString, MF_BYCOMMAND);
    if (bCheck)
        pSubMenu->ModifyMenu(uiCmd, 
                             MF_CHECKED | MF_BYCOMMAND, 
                             uiCmd,
                             sMenuString);
    else
        pSubMenu->ModifyMenu(uiCmd, 
                             MF_UNCHECKED | MF_BYCOMMAND , 
                             uiCmd,
                             sMenuString);
    return TRUE;
} // CSystemTray::CheckMenuItem()

BOOL CSystemTray::EnableMenuItem(UINT uiCmd, BOOL bEnable)
{
    CMenu *pSubMenu;
    
    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    if (bEnable)
        pSubMenu->EnableMenuItem(uiCmd,MF_ENABLED | MF_BYCOMMAND);
    else
        pSubMenu->EnableMenuItem(uiCmd,MF_GRAYED | MF_BYCOMMAND);

    return TRUE;
} // CSystemTray::EnableMenuItem()
