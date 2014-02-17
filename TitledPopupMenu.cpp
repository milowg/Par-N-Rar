#include "stdafx.h"
#include "TitledPopupMenu.h"

void AddMenuTitle(CMenu* popup, LPCTSTR lpszTitle)
{
    // insert a separator item at the top
    popup->InsertMenu(0, MF_BYPOSITION | MF_SEPARATOR, 0, (LPSTR)0);

    // insert an empty owner-draw item at top to serve as the title
    // note: item is not selectable (disabled) but not grayed
    popup->InsertMenu(0, MF_BYPOSITION | MF_OWNERDRAW | MF_STRING | MF_DISABLED, 0, lpszTitle);
} // AddMenuTitle(CMenu*, LPCSTR)

static HFONT CreatePopupMenuTitleFont()
{
    // start by getting the stock menu font
    HFONT font = (HFONT)::GetStockObject(ANSI_VAR_FONT);
    if (font)
    {
        // now, get the complete LOGFONT describing this font
        LOGFONT lf;
        if (::GetObject(font, sizeof(LOGFONT), &lf))
        {
            // set the weight to bold
            lf.lfWeight = FW_BOLD;

            // recreate this font with just the weight changed
            font = ::CreateFontIndirect(&lf);
        }
    }

    // return the new font - Note: Caller now owns this GDI object
    return font;
} // static HFONT CreatePopupMenuTitleFont()

void MeasurePopupMenuTitle(LPMEASUREITEMSTRUCT lpMi)
{
    LPCSTR lpszMenuTitle = (LPCSTR)lpMi->itemData;

    // create the font we will use for the title
    HFONT font = CreatePopupMenuTitleFont();
    if (font)
    {
        // get the screen dc to use for retrieving size information
        CDC dc;
        HDC hDc = ::GetDC(NULL);
        dc.Attach(hDc);

        // select the title font
        CFont* old = (CFont*)dc.SelectObject(CFont::FromHandle(font));

        // compute the size of the title
        CSize size = dc.GetTextExtent(lpszMenuTitle);
        // deselect the title font
        dc.SelectObject(old);

        // delete the title font
        ::DeleteObject(font);
        ::ReleaseDC(NULL, hDc);

        // add in the left margin for the menu item
        size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;

        // return the width and height
        lpMi->itemWidth = size.cx;
        lpMi->itemHeight = size.cy;
    }
} // MeasurePopupMenuTitle(LPMEASUREITEMSTRUCT)

//
// This is a helper function to draw the popup menu's title item
//
void DrawPopupMenuTitle(LPDRAWITEMSTRUCT lpDi)
{
    LPCSTR lpszMenuTitle = (LPCSTR)lpDi->itemData;

    // create the font we will use for the title
    HFONT font = CreatePopupMenuTitleFont();
    if (font)
    {
        // create the background menu brush
        HBRUSH bgb = CreateSolidBrush(GetSysColor(COLOR_MENU));

        // fill the rectangle with this brush
        FillRect(lpDi->hDC, &lpDi->rcItem, bgb);

        // delete the brush
        ::DeleteObject(bgb);

        // set text mode to transparent
        int mode = SetBkMode(lpDi->hDC, TRANSPARENT);

        // set text color to menu text color
        COLORREF color = SetTextColor(lpDi->hDC, GetSysColor(COLOR_MENUTEXT));

        // select this font into the dc
        HFONT old = (HFONT)SelectObject(lpDi->hDC, font);

        // add the menu margin offset
        lpDi->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK)+8;

        // draw the text left aligned and vertically centered
        DrawText(lpDi->hDC, lpszMenuTitle, -1, &lpDi->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

        // deselect and delete the font 
        SelectObject(lpDi->hDC, old);
        ::DeleteObject(font);

        // restore the text background mode
        SetBkMode(lpDi->hDC, mode);

        // restore the text color
        SetTextColor(lpDi->hDC, color);
    }
} // void DrawPopupMenuTitle()