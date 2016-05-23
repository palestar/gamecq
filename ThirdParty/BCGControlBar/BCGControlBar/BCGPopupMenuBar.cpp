//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

// BCGPopupMenuBar.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>

#pragma warning (disable : 4201)
	#include "mmsystem.h"
#pragma warning (default : 4201)

#include "BCGPopupMenuBar.h"
#include "BCGToolbarButton.h"
#include "BCGToolbarMenuButton.h"
#include "BCGPopupMenu.h"
#include "BCGCommandManager.h"
#include "BCGTearOffManager.h"
#include "globals.h"
#include "BCGToolbarMenuButton.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGMenuBar.h"
#include "BCGToolbarComboBoxButton.h"
#include "BCGUserToolsManager.h"
#include "BCGRegistry.h"
#include "BCGKeyboardManager.h"
#include "bcgsound.h"
#include "BCGFrameImpl.h"
#include "MenuHash.h"
#include "BCGVisualManager.h"
#include "BCGDrawManager.h"
#include "BCGContextMenuManager.h"
#include "BCGShowAllButton.h"
#include "CustomizeButton.h"
#include "BCGCustomizeMenuButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iVertMargin = 1;
static const int iHorzMargin = 1;
static const int iSeparatorHeight = 8;
static const int iMinTabSpace = 10;
static const int iEmptyMenuWidth = 50;
static const int iEmptyMenuHeight = 20;

static const int uiPopupTimerEvent = 1;
static const int uiRemovePopupTimerEvent = 2;


UINT CBCGPopupMenuBar::m_uiPopupTimerDelay = (UINT) -1;
int	CBCGPopupMenuBar::m_nLastCommandIndex = -1;

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenuBar

IMPLEMENT_SERIAL(CBCGPopupMenuBar, CBCGToolBar, 1)

CBCGPopupMenuBar::CBCGPopupMenuBar() :
	m_uiDefaultMenuCmdId (0),
	m_pDelayedPopupMenuButton (NULL),
	m_pDelayedClosePopupMenuButton (NULL),
	m_bFirstClick (TRUE),
	m_bFirstMove (TRUE),
	m_iOffset (0),
	m_xSeparatorOffsetLeft (0),
	m_xSeparatorOffsetRight (0),
	m_iMaxWidth (-1),
	m_bAreAllCommandsShown (TRUE),
	m_bInCommand (FALSE),
	m_bTrackMode (FALSE)
{
	m_bMenuMode = TRUE;
	m_bIsClickOutsideItem = TRUE;
	m_pRelatedToolbar = NULL;
	m_bDisableSideBarInXPMode = FALSE;
}

CBCGPopupMenuBar::~CBCGPopupMenuBar()
{
}

BEGIN_MESSAGE_MAP(CBCGPopupMenuBar, CBCGToolBar)
	//{{AFX_MSG_MAP(CBCGPopupMenuBar)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, OnToolbarImageAndText)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_TEXT, OnToolbarText)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenuBar message handlers

BOOL CBCGPopupMenuBar::OnSendCommand (const CBCGToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	CBCGCustomizeMenuButton* pCustomMenuButton = 
		DYNAMIC_DOWNCAST (CBCGCustomizeMenuButton, pButton);
	
	if ((pCustomMenuButton != NULL) &&
		((pButton->m_nStyle & TBBS_DISABLED) != 0 ))
	{
		pCustomMenuButton->OnClickMenuItem ();

		return TRUE;
	}

	if ((pButton->m_nStyle & TBBS_DISABLED) != 0 ||
		pButton->m_nID < 0 || pButton->m_nID == (UINT)-1)
	{
		return FALSE;
	}

	CBCGToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
	if (pMenuButton != NULL && pMenuButton->m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pMenuButton != NULL && pMenuButton->OnClickMenuItem ())
	{
		return TRUE;
	}

	if (pMenuButton->IsKindOf (RUNTIME_CLASS (CBCGShowAllButton)))
	{
		pMenuButton->OnClick (this, FALSE);		
		return TRUE;
	}

	InvokeMenuCommand (pButton->m_nID, pButton);
	return TRUE;
}
//**************************************************************************************
void CBCGPopupMenuBar::InvokeMenuCommand (UINT uiCmdId, const CBCGToolbarButton* pMenuItem)
{
	ASSERT (uiCmdId != (UINT) -1);

	CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());

	// yurig: if we have message window, should use it instead of owner window
	if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
	{
		pParentMenu->GetMessageWnd()->SendMessage 
			(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	} 
	else 
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	//--------------------
	// Deactivate menubar:
	//--------------------
	if (pParentMenu != NULL)
	{
		CBCGToolBar* pToolBar = NULL;
		for (CBCGPopupMenu* pMenu = pParentMenu; pMenu != NULL;
			pMenu = pMenu->GetParentPopupMenu ())
		{
			CBCGToolbarMenuButton* pParentButton = pMenu->GetParentButton ();
			if (pParentButton == NULL)
			{
				break;
			}
		
			pToolBar = 
				DYNAMIC_DOWNCAST (CBCGToolBar, pParentButton->GetParentWnd ());
		}

		if (pToolBar != NULL)
		{
			pToolBar->Deactivate ();
		}
	}

	if (uiCmdId != 0)
	{
		SetInCommand ();

		BCGPlaySystemSound (BCGSOUND_MENU_COMMAND);

		if (!m_bTrackMode)
		{
			BOOL bDone = FALSE;

			CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
			if (pParentMenu != NULL)
			{
				ASSERT_VALID (pParentMenu);
				
				CCustomizeButton* pCustomizeButton = DYNAMIC_DOWNCAST (
					CCustomizeButton, pParentMenu->GetParentButton ());
				if (pCustomizeButton != NULL)
				{
					bDone = pCustomizeButton->InvokeCommand (this, pMenuItem);
				}
			}

			if (!bDone)
			{
				//----------------------------------
				// Send command to the parent frame:
				//----------------------------------
				AddCommandUsage (uiCmdId);

				if (!pParentMenu->PostCommand (uiCmdId) &&
					(g_pUserToolsManager == NULL ||
					!g_pUserToolsManager->InvokeTool (uiCmdId)))
				{
					GetOwner()->PostMessage (WM_COMMAND, uiCmdId);
				}
			}
		}
		else
		{
			if (g_pContextMenuManager == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				g_pContextMenuManager->m_nLastCommandID = uiCmdId;
			}
		}
	}

	m_nLastCommandIndex = pMenuItem == NULL ? -1 : ButtonToIndex (pMenuItem);

	CFrameWnd* pParentFrame = GetParentFrame ();
	ASSERT_VALID (pParentFrame);

	SetInCommand (FALSE);
	pParentFrame->DestroyWindow ();
}
//***************************************************************
void CBCGPopupMenuBar::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL ||
		!::IsWindow (m_hWnd))
	{
		return;
	}

	ASSERT_VALID(this);

	if (m_xSeparatorOffsetLeft == 0)
	{
		//-----------------------------------------------------------
		// To enable MS Office 2000 look, we'll draw the separators
		// bellow the menu text only (in the previous versions
		// separator has been drawn on the whole menu row). Ask
		// menu button about text area offsets:
		//-----------------------------------------------------------
		CBCGToolbarMenuButton::GetTextHorzOffsets (
			m_xSeparatorOffsetLeft,
			m_xSeparatorOffsetRight);
	}

	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	CClientDC dc (this);
	CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	int y = rectClient.top + iVertMargin - m_iOffset * GetRowHeight ();

	int origy = y;
	int x = rectClient.left;
	int right = (m_arColumns.GetSize() == 0 ||
		CBCGToolBar::IsCustomizeMode ()) ?	
			rectClient.Width() :
			m_arColumns [0];
	int nColumn = 0;
	/////////

	CSize sizeMenuButton = GetMenuImageSize ();
	sizeMenuButton += CSize (2 * iHorzMargin, 2 * iVertMargin);

	sizeMenuButton.cy = max (sizeMenuButton.cy, 
							globalData.GetTextHeight ());

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if ((pButton->m_nStyle & TBBS_BREAK) && (y != origy) &&
			!CBCGToolBar::IsCustomizeMode ())
		{
			y = origy;
			nColumn ++;
			x = right + iHorzMargin;
			right = m_arColumns [nColumn];
		}
		////////////////////
		
		CRect rectButton;
		rectButton.top = y;

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			rectButton.left = x + m_xSeparatorOffsetLeft;
			rectButton.right = right + rectClient.left - m_xSeparatorOffsetRight;
			rectButton.bottom = rectButton.top + iSeparatorHeight;
		}
		else
		{
			CSize sizeButton = pButton->OnCalculateSize (&dc, 
									sizeMenuButton, TRUE);

			rectButton.left = x;
			rectButton.right = right + rectClient.left;
			rectButton.bottom = rectButton.top + sizeButton.cy;
		}

		pButton->SetRect (rectButton);
		y += rectButton.Height ();
	}

	dc.SelectObject (pOldFont);

	//--------------------------------------------------
	// Something may changed, rebuild acceleration keys:
	//--------------------------------------------------
	RebuildAccelerationKeys ();

	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (HitTest (ptCursor) >= 0)
	{
		m_bIsClickOutsideItem = FALSE;
	}
}
//***************************************************************************************
void CBCGPopupMenuBar::DrawSeparator (CDC* pDC, const CRect& rect, BOOL /*bHorz*/)
{
	CBCGVisualManager::GetInstance ()->OnDrawSeparator (pDC, this, rect, FALSE);
}
//***************************************************************************************
CSize CBCGPopupMenuBar::CalcSize (BOOL /*bVertDock*/)
{
	CSize size (0, 0);

	CClientDC dc (this);
	CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	if (m_Buttons.IsEmpty ())
	{
		size = CSize (iEmptyMenuWidth, iEmptyMenuHeight);
	}
	else
	{
		CSize column (0, 0);
		m_arColumns.RemoveAll ();
		//////////////////////////

		CSize sizeMenuButton = GetMenuImageSize ();
		sizeMenuButton += CSize (2 * iHorzMargin, 2 * iVertMargin);

		sizeMenuButton.cy = max (sizeMenuButton.cy, 
								globalData.GetTextHeight ());

		for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
		{
			CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
			ASSERT (pButton != NULL);

			BOOL bRestoreFont = FALSE;

			if (m_uiDefaultMenuCmdId != 0 &&
				pButton->m_nID == m_uiDefaultMenuCmdId)
			{
				dc.SelectObject (&globalData.fontBold);
				bRestoreFont = TRUE;
			}

			CSize sizeButton = pButton->OnCalculateSize (&dc, 
				sizeMenuButton, TRUE);

			if ((pButton->m_nStyle & TBBS_BREAK) &&
				!CBCGToolBar::IsCustomizeMode ())
			{
				if ((column.cx != 0) && (column.cy != 0))
				{
					size.cy = max (column.cy, size.cy);
					size.cx += column.cx + iHorzMargin;
					m_arColumns.Add (size.cx);
				}
				column.cx = column.cy = 0;
			}
			///////////////////////////////

			int iHeight = sizeButton.cy;

			if (pButton->m_nStyle & TBBS_SEPARATOR)
			{
				iHeight = iSeparatorHeight;
			}
			else
			{
				if (pButton->IsDrawText () &&
					pButton->m_strText.Find (_T('\t')) > 0)
				{
					sizeButton.cx += iMinTabSpace;
				}

				pButton->m_bWholeText = 
					(m_iMaxWidth <= 0 || 
					sizeButton.cx <= m_iMaxWidth - 2 * iHorzMargin);

				column.cx = max (sizeButton.cx, column.cx);
			}

			column.cy += iHeight;

			if (bRestoreFont)
			{
				dc.SelectObject (&globalData.fontRegular);
			}
		}

		size.cy = max (column.cy, size.cy);
		size.cx += column.cx;
	}

	size.cy += 2 * iVertMargin;
	size.cx += 2 * iHorzMargin;

	if (m_iMaxWidth > 0 && size.cx > m_iMaxWidth)
	{
		size.cx = m_iMaxWidth;
	}

	m_arColumns.Add (size.cx);

	dc.SelectObject (pOldFont);
	return size;
}
//***************************************************************************************
void CBCGPopupMenuBar::OnNcPaint() 
{
	//--------------------------------------
	// Disable gripper and borders painting!
	//--------------------------------------
}
//***************************************************************************************
void CBCGPopupMenuBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/) 
{
	//-----------------------------------------------
	// Don't leave space for the gripper and borders!
	//-----------------------------------------------
}
//****************************************************************************************
void CBCGPopupMenuBar::DrawDragMarker (CDC* pDC)
{
	CPen* pOldPen = (CPen*) pDC->SelectObject (&m_penDrag);

	for (int i = 0; i < 2; i ++)
	{
		pDC->MoveTo (m_rectDrag.left, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);
		pDC->LineTo (m_rectDrag.right, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);

		pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
		pDC->LineTo (m_rectDrag.left + i, m_rectDrag.bottom - i);

		pDC->MoveTo (m_rectDrag.right - i - 1, m_rectDrag.top + i);
		pDC->LineTo (m_rectDrag.right - i - 1, m_rectDrag.bottom - i);
	}

	pDC->SelectObject (pOldPen);
}
//********************************************************************************
int CBCGPopupMenuBar::FindDropIndex (const CPoint p, CRect& rectDrag) const
{
	const int iCursorSize = 6;

	GetClientRect (&rectDrag);

	if (m_Buttons.IsEmpty ())
	{
		rectDrag.bottom = rectDrag.top + iCursorSize;
		return 0;
	}

	CPoint point = p;
	if (point.y < 0)
	{
		point.y = 0;
	}

	int iDragButton = -1;
	int iIndex = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		CRect rect = pButton->Rect ();
		if (point.y < rect.top)
		{
			iDragButton = iIndex;
			rectDrag.top = rect.top;
			break;
		}
		else if (point.y <= rect.bottom)
		{
			rectDrag = rect;
			if (point.y - rect.top > rect.bottom - point.y)
			{
				iDragButton = iIndex + 1;
				rectDrag.top = rectDrag.bottom;
			}
			else
			{
				iDragButton = iIndex;
				rectDrag.top = rect.top;
			}
			break;
		}
	}

	if (iDragButton == -1)
	{
		rectDrag.top = rectDrag.bottom - iCursorSize;
		iDragButton = iIndex;
	}

	rectDrag.bottom = rectDrag.top + iCursorSize;
	rectDrag.OffsetRect (0, -iCursorSize / 2);

	return iDragButton;
}
//***************************************************************************************
CBCGToolbarButton* CBCGPopupMenuBar::CreateDroppedButton (COleDataObject* pDataObject)
{
	CBCGToolbarButton* pButton = CBCGToolbarButton::CreateFromOleData (pDataObject);
	ASSERT (pButton != NULL);

	CBCGToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

	if (pMenuButton == NULL)
	{
		pMenuButton = new CBCGToolbarMenuButton (
			pButton->m_nID, NULL, 
				pButton->IsLocked () ? -1 : pButton->GetImage (), 
				pButton->m_strText,
			pButton->m_bUserButton);
		ASSERT (pMenuButton != NULL);

		pMenuButton->m_bText = TRUE;
		pMenuButton->m_bImage = !pButton->IsLocked ();

		BOOL bRes = pButton->ExportToMenuButton (*pMenuButton);
		delete pButton;
		
		if (!bRes || pMenuButton->m_strText.IsEmpty ())
		{
			delete pMenuButton;
			return NULL;
		}
	}

	return pMenuButton;
}
//****************************************************************************************
BOOL CBCGPopupMenuBar::ImportFromMenu (HMENU hMenu, BOOL bShowAllCommands)
{
	RemoveAllButtons ();
	m_bAreAllCommandsShown = TRUE;
	m_HiddenItemsAccel.RemoveAll ();

	if (hMenu == NULL)
	{
		return FALSE;
	}

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		return FALSE;
	}

	// We need to update the menu items first (OnUpdate*** for the target message
	// window need to be invoked:
	CWnd* pMsgWindow = BCGGetTopLevelFrame (this);

	if (pMsgWindow == NULL)
	{
		pMsgWindow = AfxGetMainWnd ();
	}

	if (GetSafeHwnd () != NULL)
	{
		CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
		{
			pMsgWindow = pParentMenu->GetMessageWnd ();
		}
	}

	if (pMsgWindow != NULL)
	{
		WPARAM theMenu = WPARAM(hMenu);
		LPARAM theItem = MAKELPARAM(m_iOffset, 0);
		pMsgWindow->SendMessage(WM_INITMENUPOPUP, theMenu, theItem);
	}

	int iCount = (int) pMenu->GetMenuItemCount ();
	BOOL bPrevWasSeparator = FALSE;
	BOOL bFirstItem = TRUE;

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiTearOffId = 0;

		HMENU hSubMenu = NULL;

		CString strText;
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);

        MENUITEMINFO mii;
		ZeroMemory(&mii,sizeof(MENUITEMINFO));

        mii.cbSize = sizeof(mii);
        mii.cch = 0;
		mii.dwTypeData = 0;
        mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE;
        pMenu->GetMenuItemInfo(i, &mii, TRUE);

        UINT uiCmd = mii.wID; 
		UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);

        if (mii.fType == MFT_SEPARATOR)
        {
			if (!bPrevWasSeparator && !bFirstItem && i != iCount - 1)
			{
				InsertSeparator ();
				bFirstItem = FALSE;
				bPrevWasSeparator = TRUE;
			}
        }
        else
        {
            if (mii.hSubMenu != NULL)
            {
                uiCmd = (UINT)-1;  // force value (needed due to Windows bug)
    			hSubMenu = mii.hSubMenu;
    			ASSERT (hSubMenu != NULL);

    			if (g_pTearOffMenuManager != NULL)
    			{
    				uiTearOffId = g_pTearOffMenuManager->Parse (strText);
    			}
            }

			if (m_bTrackMode || bShowAllCommands ||
				CBCGMenuBar::IsShowAllCommands () ||
				!CBCGToolBar::IsCommandRarelyUsed (uiCmd))
			{
				CBCGToolbarMenuButton item (uiCmd, hSubMenu,
											-1, strText);
				item.m_bText = TRUE;
				item.m_bImage = FALSE;

				if (CMD_MGR.GetCmdImage (uiCmd, FALSE) == -1)
				{
					item.m_bUserButton = TRUE;
				}

				int iIndex = InsertButton (item);
				if (iIndex >= 0)
				{
					CBCGToolbarButton* pButton = GetButton (iIndex);
					ASSERT (pButton != NULL);

					pButton->m_bImage = (pButton->GetImage () >= 0);

					if (g_pUserToolsManager == NULL ||
						!g_pUserToolsManager->IsUserToolCmd (uiCmd))
					{
						if ((uiState & MF_DISABLED) || (uiState & MF_GRAYED))
						{
							pButton->m_nStyle |= TBBS_DISABLED;
						}
					}

					CBCGToolbarMenuButton* pMenuButton = 
						DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
					if (pMenuButton != NULL)
					{
						pMenuButton->SetTearOff (uiTearOffId);
					}

					if (uiState & MF_CHECKED)
					{
						pButton->m_nStyle |= TBBS_CHECKED;
					}

					if (mii.fType & MF_MENUBREAK)
					{
						pButton->m_nStyle |= TBBS_BREAK;
					}
				}

				bPrevWasSeparator = FALSE;
				bFirstItem = FALSE;
			}
			else if (CBCGToolBar::IsCommandRarelyUsed (uiCmd) &&
				CBCGToolBar::IsCommandPermitted (uiCmd))
			{
				m_bAreAllCommandsShown = FALSE;
				
				int iAmpOffset = strText.Find (_T('&'));
				if (iAmpOffset >= 0 && iAmpOffset < strText.GetLength () - 1)
				{
					TCHAR szChar[2] = {strText.GetAt (iAmpOffset + 1), '\0'};
					CharUpper (szChar);

					UINT uiHotKey = (UINT) (szChar [0]);
					m_HiddenItemsAccel.SetAt (uiHotKey, uiCmd);
				}
			}
		}
	}

	m_uiDefaultMenuCmdId = ::GetMenuDefaultItem (hMenu, FALSE, GMDI_USEDISABLED);
	return TRUE;
}
//****************************************************************************************
HMENU CBCGPopupMenuBar::ExportToMenu () const
{
	CMenu menu;
	menu.CreatePopupMenu ();

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			menu.AppendMenu (MF_SEPARATOR);
			continue;
		}

		if (!pButton->IsKindOf (RUNTIME_CLASS (CBCGToolbarMenuButton)))
		{
			continue;
		}

		CBCGToolbarMenuButton* pMenuButton = (CBCGToolbarMenuButton*) pButton;

		HMENU hPopupMenu = pMenuButton->CreateMenu ();
		if (hPopupMenu != NULL)
		{
			UINT uiStyle = (MF_STRING | MF_POPUP);
			
			if (pButton->m_nStyle & TBBS_BREAK)
			{
				uiStyle |= MF_MENUBREAK;
			}

			CString strText = pMenuButton->m_strText;
			if (pMenuButton->m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
			{
				g_pTearOffMenuManager->Build (pMenuButton->m_uiTearOffBarID, strText);
			}

			menu.AppendMenu (uiStyle, (UINT_PTR) hPopupMenu, strText);

			//--------------------------------------------------------
			// This is incompatibility between Windows 95 and 
			// NT API! (IMHO). CMenu::AppendMenu with MF_POPUP flag 
			// COPIES sub-menu resource under the Win NT and 
			// MOVES sub-menu under Win 95/98 and 2000!
			//--------------------------------------------------------
			if (globalData.bIsWindowsNT4)
			{
				::DestroyMenu (hPopupMenu);
			}
		}
		else
		{
			menu.AppendMenu (MF_STRING, pMenuButton->m_nID, pMenuButton->m_strText);
		}
	}

	HMENU hMenu = menu.Detach ();

	::SetMenuDefaultItem (hMenu, m_uiDefaultMenuCmdId, FALSE);
	return hMenu;
}
//***************************************************************************************
void CBCGPopupMenuBar::OnChangeHot (int iHot)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (GetSafeHwnd ()));

	if (iHot == -1)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (HitTest (ptCursor) == m_iHot)
		{
			m_iHighlighted = m_iHot;
			return;
		}
	}

	CBCGToolbarMenuButton* pCurrPopupMenu = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pCurrPopupMenu = pMenuButton;
			break;
		}
	}

	CBCGToolbarMenuButton* pMenuButton = NULL;
	if (iHot >= 0)
	{
		CBCGToolbarButton* pButton = GetButton (iHot);
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}
		else
		{
			ASSERT_VALID (pButton);
			pMenuButton = DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
		}
	}

	if (pMenuButton != pCurrPopupMenu)
	{
		CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());

		if (pCurrPopupMenu != NULL)
		{
			const MSG* pMsg = GetCurrentMessage ();

			if (CBCGToolBar::IsCustomizeMode () ||
				(pMsg != NULL && pMsg->message == WM_KEYDOWN))
			{
				KillTimer (uiRemovePopupTimerEvent);
				m_pDelayedClosePopupMenuButton = NULL;

				pCurrPopupMenu->OnCancelMode ();

				if (pParentMenu != NULL)
				{
					CBCGPopupMenu::ActivatePopupMenu (
						BCGGetTopLevelFrame (this), pParentMenu);
				}
			}
			else
			{
				m_pDelayedClosePopupMenuButton = pCurrPopupMenu;
				m_pDelayedClosePopupMenuButton->m_bToBeClosed = TRUE;

				SetTimer (uiRemovePopupTimerEvent, 
						max (0, m_uiPopupTimerDelay - 1), NULL);

				InvalidateRect (pCurrPopupMenu->Rect ());
				UpdateWindow ();
			}
		}

		if (pMenuButton != NULL && 
			(pMenuButton->m_nID == (UINT) -1 || pMenuButton->m_bDrawDownArrow))
		{
			pMenuButton->OnClick (this);
		}

		// Maybe, this menu will be closed by the parent menu bar timer proc.?
		CBCGPopupMenuBar* pParentBar = NULL;

		if (pParentMenu != NULL && pParentMenu->GetParentPopupMenu () != NULL &&
			(pParentBar = pParentMenu->GetParentPopupMenu ()->GetMenuBar ()) != NULL &&
			pParentBar->m_pDelayedClosePopupMenuButton == pParentMenu->GetParentButton ())
		{
			pParentBar->RestoreDelayedSubMenu ();
		}
	}
	else if (pMenuButton != NULL &&
		pMenuButton == m_pDelayedClosePopupMenuButton)
	{
		m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;
		m_pDelayedClosePopupMenuButton = NULL;

		KillTimer (uiRemovePopupTimerEvent);
	}

	m_iHot = iHot;

	if (CBCGPopupMenu::IsSendMenuSelectMsg ())
	{
		CWnd* pMsgWindow = BCGGetTopLevelFrame (this);
		if (pMsgWindow == NULL)
		{
			pMsgWindow = AfxGetMainWnd ();
		}

		CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
		{
			pMsgWindow = pParentMenu->GetMessageWnd ();
		}

		if (pMsgWindow != NULL && pParentMenu != NULL)
		{
			UINT nFlags = MF_HILITE;
			UINT nItem = 0;

			if (pMenuButton != NULL)
			{
				if ((pMenuButton->m_nStyle & TBBS_DISABLED) != 0)
				{
					nFlags |= MF_DISABLED;
				}

				if ((pMenuButton->m_nStyle & TBBS_CHECKED) != 0)
				{
					nFlags |= MF_CHECKED;
				}

				if ((nItem = pMenuButton->m_nID) == (UINT)-1)
				{
					nItem = iHot;
					nFlags |= MF_POPUP;
				}
			}

			pMsgWindow->SendMessage (WM_MENUSELECT,
				MAKEWPARAM (nItem, nFlags),
				(WPARAM) pParentMenu->GetMenu ());
		}
	}
}
//****************************************************************************************
void CBCGPopupMenuBar::OnDestroy() 
{
	KillTimer (uiPopupTimerEvent);
	KillTimer (uiRemovePopupTimerEvent);

	m_pDelayedPopupMenuButton = NULL;
	m_pDelayedClosePopupMenuButton = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			CBCGPopupMenu* pMenu = pMenuButton->m_pPopupMenu;
			if (pMenu != NULL && ::IsWindow (pMenu->m_hWnd))
			{
				pMenu->SaveState ();
				pMenu->PostMessage (WM_CLOSE);
			}
		}
	}

	CBCGToolBar::OnDestroy();
}
//****************************************************************************************
BOOL CBCGPopupMenuBar::OnKey(UINT nChar)
{
	BOOL bProcessed = FALSE;

	POSITION posSel = 
		(m_iHighlighted < 0) ? NULL : m_Buttons.FindIndex (m_iHighlighted);
	CBCGToolbarButton* pOldSelButton = 
		(posSel == NULL) ? NULL : (CBCGToolbarButton*) m_Buttons.GetAt (posSel);
	CBCGToolbarButton* pNewSelButton = pOldSelButton;
	int iNewHighlight = m_iHighlighted;

	if (nChar == VK_TAB)
	{
		if (::GetKeyState(VK_SHIFT) & 0x80)
		{
			nChar = VK_UP;
		}
		else
		{
			nChar = VK_DOWN;
		}
	}

	switch (nChar)
	{
	case VK_RETURN:
		{
			bProcessed = TRUE;

			// Try to cascase a popup menu and, if failed 
			CBCGToolbarMenuButton* pMenuButton = DYNAMIC_DOWNCAST (CBCGToolbarMenuButton,
							pOldSelButton);
			if (pMenuButton != NULL &&
				!pMenuButton->OpenPopupMenu ())
			{
				GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
				OnSendCommand (pMenuButton);
			}
		}
		break;

	case VK_HOME:
		posSel = NULL;
		// Like "Before first"...

	case VK_DOWN:
		//-----------------------------
		// Find next "selecteble" item:
		//-----------------------------
		{
			bProcessed = TRUE;
			if (m_Buttons.IsEmpty ())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetNext (pos);
			}

			if (pos == NULL)
			{
				pos = m_Buttons.GetHeadPosition ();
				iNewHighlight = 0;
			}
			else
			{
				iNewHighlight ++;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT_VALID (pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 &&
					!pButton->Rect ().IsRectEmpty ())
				{
					break;
				}

				iNewHighlight ++;
				if (pos == NULL)
				{
					pos = m_Buttons.GetHeadPosition ();
					iNewHighlight = 0;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CBCGToolbarButton*) m_Buttons.GetAt (posFound);
	
				globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetParent ()->GetSafeHwnd (), 
					OBJID_CLIENT , iNewHighlight);
			}
		}
		break;

	case VK_END:
		posSel = NULL;
		// Like "After last"....

	case VK_UP:
		//---------------------------------
		// Find previous "selecteble" item:
		//---------------------------------
		{
			bProcessed = TRUE;
			if (m_Buttons.IsEmpty ())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetPrev (pos);
			}
			if (pos == NULL)
			{
				pos = m_Buttons.GetTailPosition ();
				iNewHighlight = (int) m_Buttons.GetCount () - 1;
			}
			else
			{
				iNewHighlight --;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetPrev (pos);
				ASSERT_VALID (pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 &&
					!pButton->Rect ().IsRectEmpty ())
				{
					break;
				}

				iNewHighlight --;
				if (pos == NULL)
				{
					pos = m_Buttons.GetTailPosition ();
					iNewHighlight = (int) m_Buttons.GetCount () - 1;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CBCGToolbarButton*) m_Buttons.GetAt (posFound);

				globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetParent ()->GetSafeHwnd (), 
					OBJID_CLIENT , iNewHighlight);
			}
		}
		break;

	default:
		// Process acceleration key:
		if (!IsCustomizeMode () &&
			(::GetAsyncKeyState (VK_CONTROL) & 0x8000) == 0)
		{
			// ----------------------------
			// Ensure the key is printable:
			// ----------------------------
			WORD wChar = 0;
			BYTE lpKeyState [256];
			::GetKeyboardState (lpKeyState);

			int nRes = ::ToAsciiEx (nChar,
						MapVirtualKey (nChar, 0),
						lpKeyState,
						&wChar,
						1,
						::GetKeyboardLayout (AfxGetThread()->m_nThreadID));

			BOOL bKeyIsPrintable = nRes > 0;

			UINT nUpperChar = nChar;
			if (bKeyIsPrintable)
			{
				nUpperChar = CBCGKeyboardManager::TranslateCharToUpper (nChar);
			}

			CBCGToolbarButton* pButton;
			if (bKeyIsPrintable && m_AcellKeys.Lookup (nUpperChar, pButton))
			{
				ASSERT_VALID (pButton);

				pNewSelButton = pButton;

				//-------------------
				// Find button index:
				//-------------------
				int iIndex = 0;
				for (POSITION pos = m_Buttons.GetHeadPosition ();
					pos != NULL; iIndex ++)
				{
					CBCGToolbarButton* pListButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
					ASSERT (pListButton != NULL);

					if (pListButton == pButton)
					{
						iNewHighlight = iIndex;
						break;
					}
				}
				
				CBCGToolbarMenuButton* pMenuButton =
					DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

				if (pMenuButton != NULL)
				{
					if (pMenuButton->OpenPopupMenu ())
					{
						if (pMenuButton->m_pPopupMenu != NULL)
						{
							//--------------------------
							// Select a first menu item:
							//--------------------------
							pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
						}
					}
					else
					{
						if ((pButton->m_nStyle & TBBS_DISABLED) != 0)
						{
							InvokeMenuCommand (0, pButton);
							return TRUE;
						}
						//-----------------------------------------

						bProcessed = OnSendCommand (pMenuButton);
						if (bProcessed)
						{
							return TRUE;
						}
					}
				}
			}
			else if (CBCGMenuBar::m_bRecentlyUsedMenus &&
				!m_bAreAllCommandsShown)
			{
				///---------------------------------------------------
				// Maybe, this accelerator is belong to "hidden' item?
				//----------------------------------------------------
				UINT uiCmd = 0;
				if (m_HiddenItemsAccel.Lookup (nUpperChar, uiCmd))
				{
					InvokeMenuCommand (uiCmd, NULL);
					return TRUE;
				}
			}
		}
	}

	if (pNewSelButton != pOldSelButton)
	{
		ASSERT_VALID (pNewSelButton);
		ASSERT (iNewHighlight >= 0 && iNewHighlight < m_Buttons.GetCount ());
		ASSERT (GetButton (iNewHighlight) == pNewSelButton);

		if (IsCustomizeMode ())
		{
			m_iSelected = iNewHighlight;
		}

		m_iHighlighted = iNewHighlight;

		if (pOldSelButton != NULL)
		{
			InvalidateRect (pOldSelButton->Rect ());
		}

		InvalidateRect (pNewSelButton->Rect ());
		UpdateWindow ();

		if (pNewSelButton->m_nID != (UINT) -1)
		{
			ShowCommandMessageString (pNewSelButton->m_nID);
		}
	}

	return bProcessed;
}
//**************************************************************************************
void CBCGPopupMenuBar::OnTimer(UINT_PTR nIDEvent) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (nIDEvent == uiPopupTimerEvent)
	{
		KillTimer (uiPopupTimerEvent);

		//---------------------------------
		// Remove current tooltip (if any):
		//---------------------------------
#if _MSC_VER >= 1300
		if (AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip != NULL &&
			AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip->GetSafeHwnd () != NULL)
		{
			AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip->DestroyWindow();
			delete AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip;
			AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip = NULL;
		}
#else
		if (AfxGetThreadState()->m_pToolTip != NULL &&
			AfxGetThreadState()->m_pToolTip->GetSafeHwnd () != NULL)
		{
			AfxGetThreadState()->m_pToolTip->DestroyWindow();
			delete AfxGetThreadState()->m_pToolTip;
			AfxGetThreadState()->m_pToolTip = NULL;
		}
#endif

		if (m_pDelayedClosePopupMenuButton != NULL &&
			m_pDelayedClosePopupMenuButton->Rect ().PtInRect (ptCursor))
		{
			return;
		}

		CloseDelayedSubMenu ();

		CBCGToolbarMenuButton* pDelayedPopupMenuButton = m_pDelayedPopupMenuButton;
		m_pDelayedPopupMenuButton = NULL;

		if (pDelayedPopupMenuButton != NULL &&
			m_iHighlighted >= 0 &&
			m_iHighlighted < m_Buttons.GetCount () &&
			GetButton (m_iHighlighted) == pDelayedPopupMenuButton)
		{
			ASSERT_VALID (pDelayedPopupMenuButton);
			pDelayedPopupMenuButton->OpenPopupMenu (this);
		}
	}
	else if (nIDEvent == uiRemovePopupTimerEvent)
	{
		KillTimer (uiRemovePopupTimerEvent);

		if (m_pDelayedClosePopupMenuButton != NULL)
		{
			ASSERT_VALID (m_pDelayedClosePopupMenuButton);
			CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());

			if (m_pDelayedClosePopupMenuButton->Rect ().PtInRect (ptCursor))
			{
				return;
			}

			m_pDelayedClosePopupMenuButton->OnCancelMode ();
			m_pDelayedClosePopupMenuButton = NULL;

			if (pParentMenu != NULL)
			{
				CBCGPopupMenu::ActivatePopupMenu (BCGGetTopLevelFrame (this), pParentMenu);
			}
		}
	}
	else if (nIDEvent == uiAccNotifyEvent)
	{
		KillTimer (uiAccNotifyEvent);

		CRect rc;
		GetClientRect (&rc);
		if (!rc.PtInRect (ptCursor))
		{
			return;
		}

		if (m_iAccHotItem != -1 && m_iHighlighted == m_iAccHotItem)
		{ 
			if ((m_bMenuMode || m_iButtonCapture == -1 || 
				m_iHighlighted == m_iButtonCapture) &&
				m_iHighlighted != -1)
			{	
				globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetParent ()->GetSafeHwnd (), 
					OBJID_CLIENT , m_iHighlighted);
			}
		}
	}
}
//**************************************************************************************
void CBCGPopupMenuBar::StartPopupMenuTimer (CBCGToolbarMenuButton* pMenuButton,
											int nDelayFactor/* = 1*/)
{
	ASSERT (nDelayFactor > 0);

	if (m_pDelayedPopupMenuButton != NULL)
	{
		KillTimer (uiPopupTimerEvent);
	}

	if ((m_pDelayedPopupMenuButton = pMenuButton) != NULL)
	{
		if (m_pDelayedPopupMenuButton == m_pDelayedClosePopupMenuButton)
		{
			RestoreDelayedSubMenu ();
			m_pDelayedPopupMenuButton = NULL;
		}
		else
		{
			SetTimer (uiPopupTimerEvent, m_uiPopupTimerDelay * nDelayFactor, NULL);
		}
	}
}
//**********************************************************************************
void CBCGPopupMenuBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bFirstClick = FALSE;
	m_bIsClickOutsideItem = TRUE;

	CRect rectClient;
	GetClientRect (&rectClient);

	if (!IsCustomizeMode () && 
		!rectClient.PtInRect (point))
	{
		CBCGToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			ASSERT_VALID (pDestBar);

			CPoint ptDest = point;
			MapWindowPoints (pDestBar, &ptDest, 1);

			pDestBar->SendMessage (	WM_LBUTTONDOWN, 
									nFlags, 
									MAKELPARAM (ptDest.x, ptDest.y));
		}
	}

	CBCGToolBar::OnLButtonDown(nFlags, point);
}
//**********************************************************************************
void CBCGPopupMenuBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rectClient;
	GetClientRect (&rectClient);

	if (!m_bFirstClick &&
		!IsCustomizeMode () && 
		!rectClient.PtInRect (point))
	{
		CBCGToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			MapWindowPoints (pDestBar, &point, 1);
			pDestBar->SendMessage (	WM_LBUTTONUP, 
									nFlags, 
									MAKELPARAM (point.x, point.y));
		}

		CFrameWnd* pParentFrame = GetParentFrame ();
		ASSERT_VALID (pParentFrame);

		pParentFrame->DestroyWindow ();
		return;
	}

	if (!IsCustomizeMode () && m_iHighlighted >= 0)
	{
		m_iButtonCapture = m_iHighlighted;
	}

	m_bFirstClick = FALSE;

	if (m_bIsClickOutsideItem)
	{
		CBCGToolBar::OnLButtonUp (nFlags, point);
	}
}
//**********************************************************************************
BOOL CBCGPopupMenuBar::OnSetDefaultButtonText (CBCGToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
	if (pParentMenu != NULL)
	{
		CBCGToolBar* pToolBar = pParentMenu->GetParentToolBar ();
		if (pToolBar != NULL && pToolBar->OnSetDefaultButtonText (pButton))
		{
			return TRUE;
		}
	}

	return CBCGToolBar::OnSetDefaultButtonText (pButton);
}
//****************************************************************************************
BOOL CBCGPopupMenuBar::EnableContextMenuItems (CBCGToolbarButton* pButton, CMenu* pPopup)
{
	if (!CBCGToolBar::IsCustomizeMode ())
	{
		// Disable context menu
		return FALSE;
	}

	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	pButton->m_bText = TRUE;
	CBCGToolBar::EnableContextMenuItems (pButton, pPopup);

	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);

	if (pButton->m_bImage && pButton->GetImage () >= 0)
	{
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_ENABLED | MF_BYCOMMAND);

		if (CMD_MGR.IsMenuItemWithoutImage (pButton->m_nID))
		{
			pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_CHECKED  | MF_BYCOMMAND);
			pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_UNCHECKED  | MF_BYCOMMAND);
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPopupMenuBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bFirstMove)
	{
		m_bFirstMove = FALSE;
		return;
	}

	CRect rectClient;
	GetClientRect (&rectClient);

	if (IsCustomizeMode () ||
		rectClient.PtInRect (point))
	{
		CBCGToolBar::OnMouseMove(nFlags, point);
	}
	else
	{
		CBCGToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			MapWindowPoints (pDestBar, &point, 1);
			pDestBar->SendMessage (	WM_MOUSEMOVE, 
									nFlags, 
									MAKELPARAM (point.x, point.y));
		}
	}
}
//***************************************************************************************
CBCGToolBar* CBCGPopupMenuBar::FindDestBar (CPoint point)
{
	ScreenToClient (&point);

	CRect rectClient;

	CBCGPopupMenu* pPopupMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
	if (pPopupMenu == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pPopupMenu);

	CBCGPopupMenu* pLastPopupMenu = pPopupMenu;

	//-------------------------------
	// Go up trougth all popup menus:
	//-------------------------------
	while ((pPopupMenu = pPopupMenu->GetParentPopupMenu ()) != NULL)
	{
		CBCGPopupMenuBar* pPopupMenuBar = pPopupMenu->GetMenuBar ();
		ASSERT_VALID (pPopupMenuBar);

		pPopupMenuBar->GetClientRect (&rectClient);
		pPopupMenuBar->MapWindowPoints (this, &rectClient);

		if (rectClient.PtInRect (point))
		{
			return pPopupMenuBar;
		}

		pLastPopupMenu = pPopupMenu;
	}

	ASSERT_VALID (pLastPopupMenu);

	//--------------------
	// Try parent toolbar:
	//--------------------
	CBCGToolBar* pToolBar = pLastPopupMenu->GetParentToolBar ();
	if (pToolBar != NULL)
	{
		pToolBar->GetClientRect (&rectClient);
		pToolBar->MapWindowPoints (this, &rectClient);

		if (rectClient.PtInRect (point))
		{
			return pToolBar;
		}
	}

	return NULL;
}
//*********************************************************************************************
DROPEFFECT CBCGPopupMenuBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	//-----------------------------------------------
	// Disable MOVING menu item into one of submenus!
	//-----------------------------------------------
	if ((dwKeyState & MK_CONTROL) == 0)
	{
		CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
		if (pParentMenu != NULL)
		{
			CBCGToolBar* pParentBar = pParentMenu->GetParentToolBar ();
			CBCGToolbarMenuButton* pParentButton = pParentMenu->GetParentButton ();

			if (pParentBar != NULL && pParentButton != NULL &&
				pParentBar->IsDragButton (pParentButton))
			{
				return DROPEFFECT_NONE;
			}
		}
	}

	return CBCGToolBar::OnDragOver(pDataObject, dwKeyState, point);
}
//*****************************************************************************************
void CBCGPopupMenuBar::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (GetSafeHwnd ()));
	ASSERT_VALID (pDC);

	if (CBCGToolBar::IsCustomizeMode () ||
		!CBCGMenuBar::m_bRecentlyUsedMenus)
	{
		return;
	}

	//--------------------------------------------------------------
	// Only menubar first-level menus may hide rarely used commands:
	//--------------------------------------------------------------
	CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
	if (pParentMenu == NULL || !pParentMenu->HideRarelyUsedCommands ())
	{
		return;
	}

	BOOL bFirstRarelyUsedButton = TRUE;
	CRect rectRarelyUsed;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (pos != NULL &&
				CBCGToolBar::IsCommandRarelyUsed (
					((CBCGToolbarButton*) m_Buttons.GetAt (pos))->m_nID))
			{
				continue;
			}
		}

		BOOL bDraw = FALSE;

		if (CBCGToolBar::IsCommandRarelyUsed (pButton->m_nID))
		{
			if (bFirstRarelyUsedButton)
			{
				bFirstRarelyUsedButton = FALSE;
				rectRarelyUsed = pButton->Rect ();
			}

			if (pos == NULL)	// Last button
			{
				rectRarelyUsed.bottom = pButton->Rect ().bottom;
				bDraw = TRUE;
			}
		}
		else
		{
			if (!bFirstRarelyUsedButton)
			{
				rectRarelyUsed.bottom = pButton->Rect ().top;
				bDraw = TRUE;
			}

			bFirstRarelyUsedButton = TRUE;
		}

		if (bDraw)
		{
			CBCGVisualManager::GetInstance ()->OnHighlightRarelyUsedMenuItems (
				pDC, rectRarelyUsed);
		}
	}
}
//*************************************************************************************
INT_PTR CBCGPopupMenuBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	int nHit = ((CBCGPopupMenuBar*)this)->HitTest(point);
	if (nHit != -1)
	{
		CBCGToolbarButton* pButton = 
			DYNAMIC_DOWNCAST (CBCGToolbarButton, GetButton (nHit));

		if (pButton != NULL)
		{
			if (pTI != NULL)
			{
				pTI->uId = pButton->m_nID;
				pTI->hwnd = GetSafeHwnd ();
				pTI->rect = pButton->Rect ();
			}

			if (!pButton->OnToolHitTest (this, pTI))
			{
				nHit = pButton->m_nID;
			}
		}
	}

	return nHit;
}
//**********************************************************************************
int CBCGPopupMenuBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (m_uiPopupTimerDelay == (UINT) -1)	// Not defined yet
	{
		m_uiPopupTimerDelay = 500;
		CBCGRegistry reg (FALSE, TRUE);

		if (reg.Open (_T("Control Panel\\Desktop")))
		{
			CString strVal;

			if (reg.Read (_T("MenuShowDelay"), strVal))
			{
				m_uiPopupTimerDelay = (UINT) _ttol (strVal);

				//------------------------
				// Just limit it to 5 sec:
				//------------------------
				m_uiPopupTimerDelay = min (5000, m_uiPopupTimerDelay);
			}
		}
	}

	return 0;
}
//*****************************************************************
void CBCGPopupMenuBar::SetButtonStyle(int nIndex, UINT nStyle)
{
	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	UINT nOldStyle = pButton->m_nStyle;
	if (nOldStyle != nStyle)
	{
		// update the style and invalidate
		pButton->m_nStyle = nStyle;

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
		{
			CBCGToolbarMenuButton* pMenuButton =
				DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, GetButton (nIndex));

			BOOL bWasChecked = nOldStyle & TBBS_CHECKED;
			BOOL bChecked = nStyle & TBBS_CHECKED;

			// If checked style was changed. redraw check box (or image) area only:
			if (pMenuButton != NULL && bWasChecked != bChecked)
			{
				CRect rectImage;
				pMenuButton->GetImageRect (rectImage);

				rectImage.InflateRect (afxData.cxBorder2 * 2, afxData.cyBorder2 * 2);

				InvalidateRect (rectImage);
				UpdateWindow ();
			}
			else if ((nOldStyle ^ nStyle) != TBSTATE_PRESSED)
			{
				InvalidateButton(nIndex);
			}
		}
	}
}
// ---------------------------------------------------------------
LRESULT CBCGPopupMenuBar::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	if (m_bTrackMode)
	{
		return 0;
	}

	// handle delay hide/show
	BOOL bVis = GetStyle() & WS_VISIBLE;
	UINT swpFlags = 0;
	if ((m_nStateFlags & delayHide) && bVis)
	{
		swpFlags = SWP_HIDEWINDOW;
	}
	else if ((m_nStateFlags & delayShow) && !bVis)
	{
		swpFlags = SWP_SHOWWINDOW;
	}

	m_nStateFlags &= ~(delayShow|delayHide);
	if (swpFlags != 0)
	{
		SetWindowPos (NULL, 0, 0, 0, 0, swpFlags|
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}
	
	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if ((GetStyle() & WS_VISIBLE) &&
		(m_pDockBar == NULL || (m_pDockBar->GetStyle() & WS_VISIBLE)))
	{
		CFrameWnd* pTarget = (CFrameWnd*) GetCommandTarget ();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
		{
			pTarget = GetParentFrame();
		}

		if (pTarget != NULL)
		{
			BOOL bAutoMenuEnable = FALSE;
			if (pTarget->IsFrameWnd ())
			{
				bAutoMenuEnable = ((CFrameWnd*) pTarget)->m_bAutoMenuEnable;
			}

			OnUpdateCmdUI (pTarget, bAutoMenuEnable);
		}
	}

	return 0L;
}
// Fine aggiunta
// ---------------------------------------------------------------

CWnd* CBCGPopupMenuBar::GetCommandTarget () const
{
	if (m_bTrackMode)
	{
		return NULL;
	}

	CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
	if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL)
	{
         return pParentMenu;
	}

	return CBCGToolBar::GetCommandTarget ();
}
//*******************************************************************************
void CBCGPopupMenuBar::CloseDelayedSubMenu ()
{
	ASSERT_VALID (this);

	if (m_pDelayedClosePopupMenuButton != NULL)
	{
		ASSERT_VALID (m_pDelayedClosePopupMenuButton);

		KillTimer (uiRemovePopupTimerEvent);

		m_pDelayedClosePopupMenuButton->OnCancelMode ();
		m_pDelayedClosePopupMenuButton = NULL;
	}
}
//*******************************************************************************
void CBCGPopupMenuBar::RestoreDelayedSubMenu ()
{
	ASSERT_VALID (this);

	if (m_pDelayedClosePopupMenuButton == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pDelayedClosePopupMenuButton);
	m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;

	int iPrevHighlighted = m_iHighlighted;

	SetHot (m_pDelayedClosePopupMenuButton);

	m_iHighlighted = m_iHot;

	m_pDelayedClosePopupMenuButton = NULL;

	if (iPrevHighlighted != m_iHighlighted)
	{
		if (iPrevHighlighted >= 0)
		{
			InvalidateButton (iPrevHighlighted);
		}

		InvalidateButton (m_iHighlighted);
		UpdateWindow ();
	}

	KillTimer (uiRemovePopupTimerEvent);
}
//*******************************************************************************
BOOL CBCGPopupMenuBar::LoadFromHash(HMENU hMenu)
{
	return g_menuHash.LoadMenuBar(hMenu, this);
}
//*******************************************************************************
void CBCGPopupMenuBar::SetInCommand (BOOL bInCommand)
{
	ASSERT_VALID (this);

	m_bInCommand = bInCommand;

	CBCGPopupMenu* pMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, GetParent ());
	if (pMenu != NULL)
	{
		while ((pMenu = pMenu->GetParentPopupMenu ()) != NULL)
		{
			CBCGPopupMenuBar* pMenuBar = pMenu->GetMenuBar ();
			if (pMenuBar != NULL)
			{
				pMenuBar->SetInCommand (bInCommand);
			}
		}
	}
}
//*******************************************************************************************
void CBCGPopupMenuBar::OnToolbarImageAndText() 
{
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CMD_MGR.EnableMenuItemImage (pButton->m_nID, TRUE);
	AdjustLayout ();
}
//*************************************************************************************
void CBCGPopupMenuBar::OnToolbarText() 
{
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CMD_MGR.EnableMenuItemImage (pButton->m_nID, FALSE);
	AdjustLayout ();
}
//****************************************************************************************
void CBCGPopupMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int iItem = HitTest (point);

	if (iItem >= 0)
	{
		CBCGToolbarMenuButton* pMenuItem = DYNAMIC_DOWNCAST (
			CBCGToolbarMenuButton, GetButton (iItem));
		if (pMenuItem != NULL && pMenuItem->m_nID == (UINT) -1)
		{
			CControlBar::OnLButtonDblClk(nFlags, point);
			return;
		}
	}

	CBCGToolBar::OnLButtonDblClk(nFlags, point);
}
