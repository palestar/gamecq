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

// BCGToolbarMenuButton.cpp: implementation of the CBCGToolbarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bcgbarres.h"
#include "BCGToolbarMenuButton.h"
#include "BCGMenuBar.h"
#include "BCGPopupMenuBar.h"
#include "BCGCommandManager.h"
#include "globals.h"
#include "BCGKeyboardManager.h"

#include "BCGFrameWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"

#include "MenuImages.h"
#include "BCGUserToolsManager.h"
#include "BCGTearOffManager.h"
#include "BCGUserTool.h"
#include "BCGRegistry.h"
#include "BCGWorkspace.h"
#include "BCGVisualManager.h"
#include "BCGTabWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGToolbarMenuButton, CBCGToolbarButton, VERSIONABLE_SCHEMA | 1)

extern CBCGWorkspace* g_pWorkspace;

BOOL CBCGToolbarMenuButton::m_bAlwaysCallOwnerDraw = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarMenuButton::CBCGToolbarMenuButton()
{
	Initialize ();
}
//*****************************************************************************************
CBCGToolbarMenuButton::CBCGToolbarMenuButton (UINT uiID, HMENU hMenu, 
								int iImage, LPCTSTR lpszText, BOOL bUserButton)
{
	Initialize (uiID, hMenu, iImage, lpszText, bUserButton);
}
//*****************************************************************************************
void CBCGToolbarMenuButton::Initialize ()
{
	m_bDrawDownArrow = FALSE;
	m_bMenuMode = FALSE;
	m_pPopupMenu = NULL;
	m_pWndParent = NULL;
	m_bDefault = FALSE;
	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;
	m_bMenuOnly	= FALSE;	//JRG
	m_bToBeClosed = FALSE;
	m_uiTearOffBarID = 0;
	m_bIsRadio = FALSE;
	m_pWndMessage = NULL;
	m_bQuickCustomMode = FALSE;
	m_bShowAtRightSide = FALSE;
	m_rectArrow.SetRectEmpty ();
}
//*****************************************************************************************
void CBCGToolbarMenuButton::Initialize (UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText,
								BOOL bUserButton)
{
	Initialize ();

	m_nID = uiID;
	m_bUserButton = bUserButton;

	SetImage (iImage);
	m_strText = (lpszText == NULL) ? _T("") : lpszText;

	CreateFromMenu (hMenu);
}
//*****************************************************************************************
CBCGToolbarMenuButton::CBCGToolbarMenuButton (const CBCGToolbarMenuButton& src)
{
	m_nID = src.m_nID;
	m_nStyle = src.m_nStyle;
	m_bUserButton = src.m_bUserButton;

	SetImage (src.GetImage ());
	m_strText = src.m_strText;
	m_bDragFromCollection = FALSE;
	m_bText = src.m_bText;
	m_bImage = src.m_bImage;
	m_bDrawDownArrow = src.m_bDrawDownArrow;
	m_bMenuMode = src.m_bMenuMode;
	m_bDefault = src.m_bDefault;
	m_bMenuOnly	= src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;
	m_pWndMessage = src.m_pWndMessage;

	SetTearOff (src.m_uiTearOffBarID);

	HMENU hmenu = src.CreateMenu ();
	ASSERT (hmenu != NULL);

	CreateFromMenu (hmenu);
	::DestroyMenu (hmenu);

	m_rect.SetRectEmpty ();

	m_pPopupMenu = NULL;
	m_pWndParent = NULL;

	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;

	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;
}
//*****************************************************************************************
CBCGToolbarMenuButton::~CBCGToolbarMenuButton()
{
	if (m_pPopupMenu != NULL)
	{
		m_pPopupMenu->m_pParentBtn = NULL;
	}

	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	if (m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetInUse (m_uiTearOffBarID, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////
// Overrides:

void CBCGToolbarMenuButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);

	const CBCGToolbarMenuButton& src = (const CBCGToolbarMenuButton&) s;

	m_bDefault = src.m_bDefault;
	m_bMenuOnly	= src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;
	m_pWndMessage = src.m_pWndMessage;
	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;

	SetTearOff (src.m_uiTearOffBarID);

	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	for (POSITION pos = src.m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarMenuButton* pItem = (CBCGToolbarMenuButton*) src.m_listCommands.GetNext (pos);
		ASSERT (pItem != NULL);
		ASSERT_KINDOF (CBCGToolbarMenuButton, pItem);

		CRuntimeClass* pSrcClass = pItem->GetRuntimeClass ();
		ASSERT (pSrcClass != NULL);

		CBCGToolbarMenuButton* pNewItem = (CBCGToolbarMenuButton*) pSrcClass->CreateObject ();
		ASSERT (pNewItem != NULL);
		ASSERT_KINDOF (CBCGToolbarMenuButton, pNewItem);

		pNewItem->CopyFrom (*pItem);
		m_listCommands.AddTail (pNewItem);
	}
}
//*****************************************************************************************
void CBCGToolbarMenuButton::Serialize (CArchive& ar)
{
	CBCGToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		while (!m_listCommands.IsEmpty ())
		{
			delete m_listCommands.RemoveHead ();
		}

		UINT uiTearOffBarID;
		ar >> uiTearOffBarID;

		SetTearOff (uiTearOffBarID);
	}
	else
	{
		ar << m_uiTearOffBarID;
	}

	m_listCommands.Serialize (ar);
}
//*****************************************************************************************
void CBCGToolbarMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	m_rectArrow.SetRectEmpty ();

	if (m_bMenuMode)
	{
		DrawMenuItem (pDC, rect, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons);
		return;
	}

	BOOL bIsFlatLook = CBCGVisualManager::GetInstance ()->IsMenuFlatLook ();

	const int nSeparatorSize = 2;

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight || IsDroppedDown ());

	CSize sizeImage = CMenuImages::Size ();
	if (CBCGToolBar::IsLargeIcons ())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ? 
		CBCGVisualManager::GetInstance ()->GetButtonExtraBorder () : CSize (0, 0);

	if (sizeExtra != CSize (0, 0))
	{
		rectInternal.DeflateRect (sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);
	}

	CRect rectParent = rect;
	m_rectArrow = rectInternal;

	const int nMargin = CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
	const int xMargin = bHorz ? nMargin : 0;
	const int yMargin = bHorz ? 0 : nMargin;

	rectParent.DeflateRect (xMargin, yMargin);

	if (m_bDrawDownArrow)
	{
		if (bHorz)
		{
			rectParent.right -= sizeImage.cx + nSeparatorSize - 2 + sizeExtra.cx;
			m_rectArrow.left = rectParent.right + 1;

			if (sizeExtra != CSize (0, 0))
			{
				m_rectArrow.OffsetRect (
					-sizeExtra.cx / 2 + 1,
					-sizeExtra.cy / 2 + 1);
			}
		}
		else
		{
			rectParent.bottom -= sizeImage.cy + nSeparatorSize - 1;
			m_rectArrow.top = rectParent.bottom;
		}
	}

	UINT uiStyle = m_nStyle;

	if (bIsFlatLook)
	{
		m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);
	}
	else
	{
		if (m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly) 
		{
			m_nStyle &= ~TBBS_PRESSED;
		}
		else if (m_pPopupMenu != NULL)
		{
			m_nStyle |= TBBS_PRESSED;
		}
	}

	BOOL bDisableFill = m_bDisableFill;
	m_bDisableFill = TRUE;

	CBCGToolbarButton::OnDraw (pDC, rectParent, pImages, bHorz, 
			bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

	m_bDisableFill = bDisableFill;

	if (m_bDrawDownArrow)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) && !bIsFlatLook)
		{
			m_rectArrow.OffsetRect (1, 1);
		}

		if ((bHighlight || (m_nStyle & TBBS_PRESSED) ||
			m_pPopupMenu != NULL) &&
			m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
		{
			//----------------
			// Draw separator:
			//----------------
			CRect rectSeparator = m_rectArrow;

			if (bHorz)
			{
				rectSeparator.right = rectSeparator.left + nSeparatorSize;
			}
			else
			{
				rectSeparator.bottom = rectSeparator.top + nSeparatorSize;
			}

			CBCGVisualManager::BCGBUTTON_STATE state = CBCGVisualManager::ButtonsIsRegular;

			if (bHighlight || (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)))
			{
				//-----------------------
				// Pressed in or checked:
				//-----------------------
				state = CBCGVisualManager::ButtonsIsPressed;
			}

			if (!m_bClickedOnMenu)
			{
				CBCGVisualManager::GetInstance ()->OnDrawButtonSeparator (pDC,
												this, rectSeparator, state, bHorz);
			}
		}

		BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
			(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

		int iImage;
		if (bHorz && !m_bMenuOnly)
		{
			iImage = (bDisabled) ? CMenuImages::IdArowDownDsbl : CMenuImages::IdArowDown;
		}
		else
		{
			iImage = (bDisabled) ? CMenuImages::IdArowLeftDsbl : CMenuImages::IdArowLeft;
		}

		CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, m_rectArrow,
							sizeImage);
	}

	m_nStyle = uiStyle;

	if (!bCustomizeMode)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ||
			m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			if (!bIsFlatLook &&
				m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly) //JRG
			{
				rectParent.right++;

				CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rectParent, CBCGVisualManager::ButtonsIsHighlighted);

				CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, m_rectArrow, CBCGVisualManager::ButtonsIsPressed);
			}
			else
			{
				CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGVisualManager::ButtonsIsPressed);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rect, CBCGVisualManager::ButtonsIsHighlighted);
		}
	}
}
//*****************************************************************************************
SIZE CBCGToolbarMenuButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;

	if (!IsVisible())
	{
		return CSize (0,0);
	}

	int nArrowSize = 0;
	const int nSeparatorSize = 2;

	if (m_bDrawDownArrow || m_bMenuMode)
	{
		if (m_bMenuMode)
		{
			nArrowSize = (bHorz) ? 
				globalData.GetTextWidth () : globalData.GetTextHeight ();
		}
		else
		{
			nArrowSize = (bHorz) ? 
				CMenuImages::Size ().cx : CMenuImages::Size ().cy;

			if (CBCGToolBar::IsLargeIcons ())
			{
				nArrowSize *= 2;
			}
		}

		nArrowSize += nSeparatorSize - TEXT_MARGIN - 1;
	}

	//--------------------
	// Change accelerator:
	//--------------------
	if (g_pKeyboardManager != NULL &&
		m_bMenuMode &&
		(m_nID < 0xF000 || m_nID >= 0xF1F0))	// Not system.
	{
		//-----------------------------------
		// Remove standard aceleration label:
		//-----------------------------------
		int iTabOffset = m_strText.Find (_T('\t'));
		if (iTabOffset >= 0)
		{
			m_strText = m_strText.Left (iTabOffset);
		}

		//---------------------------------
		// Add an actual accelartion label:
		//---------------------------------
		CString strAccel;
		CFrameWnd* pParent = m_pWndParent == NULL ?
			DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ()) :
			BCGGetTopLevelFrame (m_pWndParent);

		if (pParent != NULL &&
			(CBCGKeyboardManager::FindDefaultAccelerator (
				m_nID, strAccel, pParent, TRUE) ||
			CBCGKeyboardManager::FindDefaultAccelerator (
				m_nID, strAccel, pParent->GetActiveFrame (), FALSE)))
		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}

	CSize size = CBCGToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);

	if (bHorz)
	{	
		size.cx += nArrowSize;
	}
	else
	{
		size.cy += nArrowSize;
	}

	if (m_bMenuMode)
	{
		size.cx += sizeDefault.cx + 2 * TEXT_MARGIN;
	}

	CBCGPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		size.cy = pParentMenu->GetRowHeight ();
	}

	if (!m_bMenuMode)
	{
		const int nMargin = CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();

		if (bHorz)
		{
			size.cx += nMargin * 2;
		}
		else
		{
			size.cy += nMargin * 2;
		}
	}

	return size;
}
//*****************************************************************************************
BOOL CBCGToolbarMenuButton::OnClick (CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID (pWnd);
	
	m_bClickedOnMenu = FALSE;

	if (m_bDrawDownArrow && !bDelay && !m_bMenuMode)
	{
		if (m_nID == 0 || m_nID == (UINT) -1)
		{
			m_bClickedOnMenu = TRUE;
		}
		else
		{
			CPoint ptMouse;
			::GetCursorPos (&ptMouse);
			pWnd->ScreenToClient (&ptMouse);

			m_bClickedOnMenu = m_rectArrow.PtInRect (ptMouse);

			if (!m_bClickedOnMenu)
			{
				return FALSE;
			}
		}
	}

	if (!m_bClickedOnMenu && m_nID > 0 && m_nID != (UINT) -1 && !m_bDrawDownArrow &&
		!m_bMenuOnly)
	{
		return FALSE;
	}

	CBCGMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGMenuBar, m_pWndParent);

	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------		
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow ();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (NULL);
		}
	}
	else
	{
		CBCGPopupMenuBar* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);

		if (bDelay && pParentMenu != NULL && !CBCGToolBar::IsCustomizeMode ())
		{
			pParentMenu->StartPopupMenuTimer (this);
		}
		else
		{
			if (pMenuBar != NULL)
			{
				CBCGToolbarMenuButton* pCurrPopupMenuButton = 
					pMenuBar->GetDroppedDownMenu();
				if (pCurrPopupMenuButton != NULL)
				{
					pCurrPopupMenuButton->OnCancelMode ();
				}
			}
			
			if (!OpenPopupMenu (pWnd))
			{
				return FALSE;
			}
		}

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (this);
		}
	}

	if (m_pWndParent != NULL)
	{
		CRect rect = m_rect;

		const int nShadowSize = 
			CBCGVisualManager::GetInstance ()->GetMenuShadowDepth ();

		rect.InflateRect (nShadowSize, nShadowSize);
		m_pWndParent->RedrawWindow (rect, NULL, RDW_FRAME | RDW_INVALIDATE);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGToolbarMenuButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	if (pWndParent != NULL)
	{
		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGMenuBar)))
		{
            m_bDrawDownArrow = (m_nID != 0 && !m_listCommands.IsEmpty ()) ||
                ((CBCGMenuBar *)pWndParent)->GetForceDownArrows();
			m_bText = TRUE;
			m_bImage = FALSE;
		}
		else
		{
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty ());
		}

		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
		{
			m_bMenuMode = TRUE;
			m_bText = TRUE;
			m_bImage = FALSE;
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty ());
		}
		else
		{
			m_bMenuMode = FALSE;
		}
	}

	m_pWndParent = pWndParent;
}
//****************************************************************************************
void CBCGToolbarMenuButton::CreateFromMenu (HMENU hMenu)
{
	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		return;
	}

	UINT uiDefaultCmd = ::GetMenuDefaultItem (hMenu, FALSE, GMDI_USEDISABLED);

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		CBCGToolbarMenuButton* pItem = STATIC_DOWNCAST(CBCGToolbarMenuButton, GetRuntimeClass()->CreateObject());
		ASSERT_VALID (pItem);

		pItem->m_nID = pMenu->GetMenuItemID (i);
		pMenu->GetMenuString (i, pItem->m_strText, MF_BYPOSITION);

		if (pItem->m_nID == -1)	// Sub-menu...
		{
			if (g_pTearOffMenuManager != NULL)
			{
				pItem->SetTearOff (g_pTearOffMenuManager->Parse (pItem->m_strText));
			}

			CMenu* pSubMenu = pMenu->GetSubMenu (i);
			pItem->CreateFromMenu (pSubMenu->GetSafeHmenu ());
		}
		else if (pItem->m_nID == uiDefaultCmd)
		{
			pItem->m_bDefault = TRUE;
		}

		UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);

		if (uiState & MF_MENUBREAK)
		{
			pItem->m_nStyle |= TBBS_BREAK;
		}

		if ((uiState & MF_DISABLED) || (uiState & MF_GRAYED))
		{
			pItem->m_nStyle |= TBBS_DISABLED;
		}

		m_listCommands.AddTail (pItem);
	}
}
//****************************************************************************************
HMENU CBCGToolbarMenuButton::CreateMenu () const
{
	if (m_listCommands.IsEmpty () && m_nID != (UINT) -1 && m_nID != 0 && !m_bMenuOnly)
	{
		return NULL;
	}

	CMenu menu;
	if (!menu.CreatePopupMenu ())
	{
		TRACE(_T("CBCGToolbarMenuButton::CreateMenu (): Can't create popup menu!\n"));
		return NULL;
	}

	BOOL bRes = TRUE;
	DWORD dwLastError = 0;

	UINT uiDefaultCmd = (UINT) -1;

	int i = 0;
	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGToolbarMenuButton* pItem = (CBCGToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT (pItem != NULL);
		ASSERT_KINDOF (CBCGToolbarMenuButton, pItem);

		UINT uiStyle = MF_STRING;

		if (pItem->m_nStyle & TBBS_BREAK)
		{
			uiStyle |= MF_MENUBREAK;
		}

		if (pItem->m_nStyle & TBBS_DISABLED)   
		{
		   uiStyle |= MF_DISABLED;
		}


		if (pItem->IsTearOffMenu ())
		{
			uiStyle |= MF_MENUBARBREAK;
		}

		switch (pItem->m_nID)
		{
		case 0:	// Separator
			bRes = menu.AppendMenu (MF_SEPARATOR);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
			break;

		case -1:			// Sub-menu
			{
				HMENU hSubMenu = pItem->CreateMenu ();
				ASSERT (hSubMenu != NULL);

				CString strText = pItem->m_strText;
				if (pItem->m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
				{
					g_pTearOffMenuManager->Build (pItem->m_uiTearOffBarID, strText);
				}

				bRes = menu.AppendMenu (uiStyle | MF_POPUP, (UINT_PTR) hSubMenu, strText);
				if (!bRes)
				{
					dwLastError = GetLastError ();
				}

				//--------------------------------------------------------
				// This is incompatibility between Windows 95 and 
				// NT API! (IMHO). CMenu::AppendMenu with MF_POPUP flag 
				// COPIES sub-menu resource under the Win NT and 
				// MOVES sub-menu under Win 95/98 and 2000!
				//--------------------------------------------------------
				if (globalData.bIsWindowsNT4)
				{
					::DestroyMenu (hSubMenu);
				}
			}
			break;

		default:
			if (pItem->m_bDefault)
			{
				uiDefaultCmd = pItem->m_nID;
			}

			bRes = menu.AppendMenu (uiStyle, pItem->m_nID, pItem->m_strText);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
		}

		if (!bRes)
		{
			TRACE(_T("CBCGToolbarMenuButton::CreateMenu (): Can't add menu item: %d\n Last error = %x\n"), pItem->m_nID, dwLastError);
			return NULL;
		}
	}

	HMENU hMenu = menu.Detach ();
	if (uiDefaultCmd != (UINT)-1)
	{
		::SetMenuDefaultItem (hMenu, uiDefaultCmd, FALSE);
	}

	return hMenu;
}
//*****************************************************************************************
void CBCGToolbarMenuButton::DrawMenuItem (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages, 
					BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons,
					BOOL bContentOnly)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CBCGToolBarImages* pLockedImages = NULL;
	CBCGDrawState ds;

	CBCGPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);

	CSize sizeMenuImage = CBCGToolBar::GetMenuImageSize ();

	if (pParentMenu != NULL)
	{
		if (pParentMenu->m_pRelatedToolbar != NULL && 
			pParentMenu->m_pRelatedToolbar->IsLocked ())
		{
			pLockedImages = (CBCGToolBarImages*) pParentMenu->m_pRelatedToolbar->GetLockedMenuImages ();

			if (pLockedImages != NULL)
			{
				CSize sizeDest (0, 0);

				if (sizeMenuImage != pParentMenu->GetCurrentMenuImageSize ())
				{
					sizeDest = sizeMenuImage;
				}

				pLockedImages->PrepareDrawImage (ds, sizeDest);

				pImages = pLockedImages;
			}
		}
	}

	BOOL bDisableImage = CMD_MGR.IsMenuItemWithoutImage (m_nID);
	if (m_nID == ID_BCGBARRES_TASKPANE_BACK ||
		m_nID == ID_BCGBARRES_TASKPANE_FORWARD)
	{
		bDisableImage = TRUE;
	}

	CBCGUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (m_nID);
	}

	HICON hDocIcon = CBCGTabWnd::GetDocumentIcon (m_nID);

	CSize sizeImage = CMenuImages::Size ();

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

	if (m_pPopupMenu != NULL && !m_bToBeClosed)
	{
		bHighlight = TRUE;
	}

	COLORREF clrText = CBCGVisualManager::GetInstance ()->GetMenuItemTextColor (
		this, bHighlight, bDisabled);

	BOOL bDrawImageFrame = !CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem ();

	if (bHighlight && !bContentOnly &&
		CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
	{
		CBCGVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rect, clrText);
		bDrawImageFrame = FALSE;
	}

	if ((m_nStyle & TBBS_CHECKED) &&
		!CBCGVisualManager::GetInstance ()->IsOwnerDrawMenuCheck ())
	{
		bDrawImageFrame = TRUE;
	}

	CFont* pOldFont = NULL;

	if (m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly && 
		pParentMenu != NULL && pParentMenu->GetDefaultMenuId () == m_nID)
	{
		pOldFont = (CFont*) pDC->SelectObject (&globalData.fontBold);
	}

	CRect rectImage;
	rectImage = rect;
	rectImage.left += CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
	rectImage.right = rectImage.left + sizeMenuImage.cx + 
		CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();

	CRect rectFrameBtn = rectImage;

	if (CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
	{
		rectFrameBtn = rect;

		rectFrameBtn.left += 2;
		rectFrameBtn.top++;
		rectFrameBtn.bottom -= 2;
		rectFrameBtn.right = rectImage.right;
	}
	else
	{
		rectFrameBtn.InflateRect (1, -1);
	}

	BOOL bIsRarelyUsed = (CBCGMenuBar::IsRecentlyUsedMenus () && 
		CBCGToolBar::IsCommandRarelyUsed (m_nID));
	
	if (bIsRarelyUsed)
	{
		bIsRarelyUsed = FALSE;

		CBCGPopupMenuBar* pParentMenuBar =
			DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);

		if (pParentMenuBar != NULL)
		{
			CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, 
				pParentMenuBar->GetParent ());
			if (pParentMenu != NULL && pParentMenu->HideRarelyUsedCommands ())
			{
				bIsRarelyUsed = TRUE;
			}
		}
	}

	BOOL bLightImage = FALSE;
	BOOL bFadeImage = !bHighlight && CBCGVisualManager::GetInstance ()->IsFadeInactiveImage ();

	if (bIsRarelyUsed)
	{
		bLightImage = TRUE;
		if (bHighlight && (m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}

		if (GetImage () < 0 && !(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}
	}
	else if (m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE))
	{
		bLightImage = !bHighlight;
	}

	//----------------
	// Draw the image:
	//----------------
	if (!IsDrawImage () && hDocIcon == NULL)	// Try to find a matched image
	{
		BOOL bImageSave = m_bImage;
		BOOL bUserButton = m_bUserButton;
		BOOL bSuccess = TRUE;

		m_bImage = TRUE;	// Always try to draw image!
		m_bUserButton = TRUE;

		if (GetImage () < 0)
		{
			m_bUserButton = FALSE;

			if (GetImage () < 0)
			{
				bSuccess = FALSE;
			}
		}

		if (!bSuccess)
		{
			m_bImage = bImageSave;
			m_bUserButton = bUserButton;
		}
	}

	BOOL bImageIsReady = FALSE;

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect (&rectImage);

	if (bDrawImageFrame && !bContentOnly)
	{
		FillInterior (pDC, rectFrameBtn, bHighlight);
	}

	if (!bDisableImage && (IsDrawImage () && pImages != NULL) || hDocIcon != NULL)
	{
		BOOL bDrawImageShadow = 
			bHighlight && !bCustomizeMode &&
			CBCGVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
			!globalData.IsHighContastMode () &&
			((m_nStyle & TBBS_CHECKED) == 0) &&
			((m_nStyle & TBBS_DISABLED) == 0);

		pDC->SelectObject (&rgnClip);

		CPoint ptImageOffset (
			(rectImage.Width () - sizeMenuImage.cx) / 2, 
			(rectImage.Height () - sizeMenuImage.cy) / 2);

		if ((m_nStyle & TBBS_PRESSED) || 
			!(m_nStyle & TBBS_DISABLED) ||
			!bGrayDisabledButtons ||
			bCustomizeMode)
		{
			CRect rectIcon (CPoint (rectImage.left + ptImageOffset.x, 
							rectImage.top + ptImageOffset.y),
							sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon (pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC, rectIcon);
			}
			else
			{
				CPoint pt = rectImage.TopLeft ();
				pt += ptImageOffset;

				if (bDrawImageShadow)
				{
					pt.Offset (1, 1);

					pImages->Draw (pDC, 
						pt.x,
						pt.y, 
						GetImage (), FALSE, FALSE, FALSE, TRUE);

					pt.Offset (-2, -2);
				}

				pImages->Draw (pDC, 
					pt.x, 
					pt.y, 
					GetImage (),
					FALSE, bDisabled && bGrayDisabledButtons,
					FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}

		if (!bImageIsReady)
		{
			CRect rectIcon (CPoint (rectImage.left + ptImageOffset.x, 
							rectImage.top + ptImageOffset.y),
							sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon (pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC, rectIcon);
			}
			else
			{
				if (bDrawImageShadow)
				{
					rectImage.OffsetRect (1, 1);

					pImages->Draw (pDC, 
						rectImage.left + ptImageOffset.x,
						rectImage.top + ptImageOffset.y,
						GetImage (), FALSE, FALSE, FALSE, TRUE);

					rectImage.OffsetRect (-2, -2);
				}

				pImages->Draw (pDC, 
					rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, 
					GetImage (), FALSE, bDisabled && bGrayDisabledButtons,
					FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}
	}
	
	
	if (m_bAlwaysCallOwnerDraw || !bImageIsReady)
	{
		CFrameWnd* pParentFrame = m_pWndParent == NULL ?
			DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ()) :
			BCGGetTopLevelFrame (m_pWndParent);

		//------------------------------------
		// Get chance to user draw menu image:
		//------------------------------------
		CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pParentFrame);
		if (pMainFrame != NULL)
		{
			bImageIsReady = pMainFrame->OnDrawMenuImage (pDC, this, rectImage);
		}
		else	// Maybe, SDI frame...
		{
			CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pParentFrame);
			if (pFrame != NULL)
			{
				bImageIsReady = pFrame->OnDrawMenuImage (pDC, this, rectImage);
			}
			else	// Maybe, OLE frame...
			{
				CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pParentFrame);
				if (pOleFrame != NULL)
				{
					bImageIsReady = pOleFrame->OnDrawMenuImage (pDC, this, rectImage);
				}
				else
				{
					CBCGOleDocIPFrameWnd* pOleDocFrame = 
						DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pParentFrame);
					if (pOleDocFrame != NULL)
					{
						bImageIsReady = pOleDocFrame->OnDrawMenuImage (pDC, this, rectImage);
					}
				}

			}
		}
	}

	pDC->SelectClipRgn (NULL);

	if (m_nStyle & TBBS_CHECKED)
	{
		if (bDrawImageFrame)
		{
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectFrameBtn, CBCGVisualManager::ButtonsIsPressed);
		}

		if (!bImageIsReady)
		{
			CBCGVisualManager::GetInstance ()->OnDrawMenuCheck (pDC, this, 
				rectFrameBtn, bHighlight, m_bIsRadio);
		}
	}
	else if (!bContentOnly && bImageIsReady && bHighlight && bDrawImageFrame)
	{
		CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectFrameBtn, CBCGVisualManager::ButtonsIsHighlighted);
	}

	rectImage.InflateRect (1, 0);
	int iSystemImageId = -1;

	//-------------------------------
	// Try to draw system menu icons:
	//-------------------------------
	if (!bImageIsReady)
	{
		switch (m_nID)
		{
		case SC_MINIMIZE:
			iSystemImageId = bDisabled ? CMenuImages::IdMinimizeDsbl : CMenuImages::IdMinimize;
			break;

		case SC_RESTORE:
			iSystemImageId = bDisabled ? CMenuImages::IdRestoreDsbl : CMenuImages::IdRestore;
			break;

		case SC_CLOSE:
			iSystemImageId = bDisabled ? CMenuImages::IdCloseDsbl : CMenuImages::IdClose;
			break;

		case SC_MAXIMIZE:
			iSystemImageId = bDisabled ? CMenuImages::IdMaximizeDsbl : CMenuImages::IdMaximize;
			break;
		}

		if (iSystemImageId != -1)
		{
			CRect rectSysImage = rectImage;
			rectSysImage.DeflateRect (CBCGVisualManager::GetInstance ()->GetMenuImageMargin (), CBCGVisualManager::GetInstance ()->GetMenuImageMargin ());

			if (!bContentOnly)
			{
				FillInterior (pDC, rectFrameBtn, bHighlight);
			}

			CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iSystemImageId, rectSysImage);

			if (bHighlight && !bContentOnly)
			{
				CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rectFrameBtn, CBCGVisualManager::ButtonsIsHighlighted);
			}
		}
	}

	//-------------------------------
	// Fill text area if highlighted:
	//-------------------------------
	CRect rectText = rect;
	rectText.left = rectFrameBtn.right + CBCGVisualManager::GetInstance ()->GetMenuImageMargin () + 2;

	if (bHighlight)
	{
		if (!CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
		{
			CRect rectFill = rectFrameBtn;

			if ((m_nStyle & (TBBS_CHECKED) || bImageIsReady) ||
				iSystemImageId != -1)
			{
				rectFill.left = rectText.left - 1;
			}

			rectFill.right = rect.right - 1;

			if (!bContentOnly)
			{
				CBCGVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rectFill, clrText);
			}
			else
			{
				clrText = CBCGVisualManager::GetInstance ()->GetHighlightedMenuItemTextColor (this);
			}
		}
		else if (bContentOnly)
		{
			clrText = CBCGVisualManager::GetInstance ()->GetHighlightedMenuItemTextColor (this);
		}
	}
	else
	{
		clrText	= bDisabled ?
						globalData.clrGrayedText : 
						globalData.clrBtnText;
	}

	//-------------------------
	// Find acceleration label:
	//-------------------------
	CString strText = m_strText;
	CString strAccel;

	int iTabOffset = m_strText.Find (_T('\t'));
	if (iTabOffset >= 0)
	{
		strText = strText.Left (iTabOffset);
		strAccel = m_strText.Mid (iTabOffset + 1);
	}

	//-----------
	// Draw text:
	//-----------
	COLORREF clrTextOld = pDC->GetTextColor ();

	rectText.left += TEXT_MARGIN;

	if (!m_bWholeText)
	{
		CString strEllipses (_T("..."));
		while (strText.GetLength () > 0 &&
			pDC->GetTextExtent (strText + strEllipses).cx > rectText.Width ())
		{
			strText = strText.Left (strText.GetLength () - 1);
		}

		strText += strEllipses;
	}

	if (bDisabled && !bHighlight && 
		CBCGVisualManager::GetInstance ()->IsEmbossDisabledImage ())
	{
		pDC->SetTextColor (globalData.clrBtnHilite);

		CRect rectShft = rectText;
		rectShft.OffsetRect (1, 1);
		pDC->DrawText (strText, &rectShft, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->SetTextColor (clrText);
	pDC->DrawText (strText, &rectText, DT_SINGLELINE | DT_VCENTER);

	//------------------------
	// Draw accelerator label:
	//------------------------
	if (!strAccel.IsEmpty ())
	{
		CRect rectAccel = rectText;
		rectAccel.right -= TEXT_MARGIN + sizeImage.cx;

		if (bDisabled && !bHighlight && CBCGVisualManager::GetInstance ()->IsEmbossDisabledImage ())
		{
			pDC->SetTextColor (globalData.clrBtnHilite);

			CRect rectAccelShft = rectAccel;
			rectAccelShft.OffsetRect (1, 1);
			pDC->DrawText (strAccel, &rectAccelShft, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
		}

		pDC->SetTextColor (clrText);
		pDC->DrawText (strAccel, &rectAccel, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	}

	//--------------------------------------------
	// Draw triangle image for the cascade menues:
	//--------------------------------------------
	if (m_nID == (UINT) -1 || m_bDrawDownArrow || m_bMenuOnly)
	{
		CFont* pRegFont = pDC->SelectObject (&globalData.fontMarlett);
		ASSERT (pRegFont != NULL);

		CRect rectTriangle = rect;

		CString strTriangle = (m_pWndParent->GetExStyle() & WS_EX_LAYOUTRTL) ?
			_T("3") : _T("4");	// Marlett's right arrow

		if (m_bQuickCustomMode)
		{
			strTriangle = _T("6");  	// Marlett's down arrow
		}

		pDC->DrawText (strTriangle, &rectTriangle, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

		pDC->SelectObject (pRegFont);
	}

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	pDC->SetTextColor (clrTextOld);

	if (pLockedImages != NULL)
	{
		pLockedImages->EndDrawImage (ds);
	}
}
//****************************************************************************************
void CBCGToolbarMenuButton::OnCancelMode ()
{
	if (m_pPopupMenu != NULL && ::IsWindow (m_pPopupMenu->m_hWnd))
	{
		if (m_pPopupMenu->InCommand ())
		{
			return;
		}

		for (int i = 0; i < m_pPopupMenu->GetMenuItemCount (); i++)
		{
			CBCGToolbarMenuButton* pSubItem = m_pPopupMenu->GetMenuItem (i);
			if (pSubItem != NULL)
			{
				pSubItem->OnCancelMode ();
			}
		}

		m_pPopupMenu->SaveState ();
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->CloseMenu ();
	}

	m_pPopupMenu = NULL;

	if (m_pWndParent != NULL && ::IsWindow (m_pWndParent->m_hWnd))
	{
		CRect rect = m_rect;
		
		const int nShadowSize = 
			CBCGVisualManager::GetInstance ()->GetMenuShadowDepth ();

		rect.InflateRect (nShadowSize, nShadowSize);

		m_pWndParent->InvalidateRect (rect);
		m_pWndParent->UpdateWindow ();
	}

	m_bToBeClosed = FALSE;
}
//****************************************************************************************
BOOL CBCGToolbarMenuButton::OpenPopupMenu (CWnd* pWnd)
{
	if (m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pWnd == NULL)
	{
		pWnd = m_pWndParent;
	}

	ASSERT (pWnd != NULL);

	HMENU hMenu = CreateMenu ();
	if (hMenu == NULL && !IsEmptyMenuAllowed ())
	{
		return FALSE;
	}

	m_pPopupMenu = CreatePopupMenu ();
	if (m_pPopupMenu == NULL)
	{
		::DestroyMenu (hMenu);
		return FALSE;
	}

	if (m_pPopupMenu->GetMenuItemCount () > 0 && hMenu != NULL)
	{
		::DestroyMenu (hMenu);
		hMenu = NULL;
	}

	//---------------------------------------------------------------
	// Define a new menu position. Place the menu in the right side
	// of the current menu in the poup menu case or under the current 
	// item by default:
	//---------------------------------------------------------------
	CPoint point;
	CBCGPopupMenu::DROP_DIRECTION dropDir = CBCGPopupMenu::DROP_DIRECTION_NONE;

	CBCGPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);

	CBCGMenuBar* pParentMenuBar =
		DYNAMIC_DOWNCAST (CBCGMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		point = CPoint (0, m_rect.top - 2);
		pWnd->ClientToScreen (&point);

		CRect rectParent;
		pParentMenu->GetWindowRect (rectParent);

		int nMenuGap = CBCGVisualManager::GetInstance ()->GetPopupMenuGap ();

		if (pParentMenu->GetExStyle() & WS_EX_LAYOUTRTL)
		{
			point.x = rectParent.left - nMenuGap;
			dropDir = CBCGPopupMenu::DROP_DIRECTION_LEFT;
		}
		else
		{
			point.x = rectParent.right + nMenuGap;
			dropDir = CBCGPopupMenu::DROP_DIRECTION_RIGHT;
		}
	}
	else if (pParentMenuBar != NULL && 
		(pParentMenuBar->m_dwStyle & CBRS_ORIENT_HORZ) == 0)
	{
		//------------------------------------------------
		// Parent menu bar is docked vertical, place menu 
		// in the left or right side of the parent frame:
		//------------------------------------------------
		point = CPoint (m_rect.right, m_rect.top);
		pWnd->ClientToScreen (&point);

		dropDir = CBCGPopupMenu::DROP_DIRECTION_RIGHT;
	}
	else
	{
		if (m_bShowAtRightSide)
		{
			point = CPoint (m_rect.right - 1, m_rect.top);
		}
		else
		{
			if (m_pPopupMenu->IsRightAlign())
			{
				point = CPoint (m_rect.right - 1, m_rect.bottom - 1);
			}
			else
			{
				point = CPoint (m_rect.left, m_rect.bottom - 1);
			}
		}

		dropDir = CBCGPopupMenu::DROP_DIRECTION_BOTTOM;
		pWnd->ClientToScreen (&point);
	}

	m_pPopupMenu->m_pParentBtn = this;
	m_pPopupMenu->m_DropDirection = dropDir;

	if (!m_pPopupMenu->Create (pWnd, point.x, point.y, hMenu))
	{
		m_pPopupMenu = NULL;
		return FALSE;
	}

	if (m_pWndMessage != NULL)
	{
		ASSERT_VALID (m_pWndMessage);
		m_pPopupMenu->SetMessageWnd (m_pWndMessage);
	}
	else
	{
		// yurig: if parent menu has a message window, the child should have the same
		CBCGPopupMenu* pCallerMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, pWnd->GetParent ());
		if (pCallerMenu != NULL && pCallerMenu->GetMessageWnd() != NULL) 
		{
			m_pPopupMenu->SetMessageWnd (pCallerMenu->GetMessageWnd ());
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarMenuButton diagnostics

#ifdef _DEBUG
void CBCGToolbarMenuButton::AssertValid() const
{
	CObject::AssertValid();
}
//******************************************************************************************
void CBCGToolbarMenuButton::Dump(CDumpContext& dc) const
{
	CObject::Dump (dc);

	CString strId;
	strId.Format (_T("%x"), m_nID);

	dc << "[" << m_strText << " >>>>> ]";
	dc.SetDepth (dc.GetDepth () + 1);

	dc << "{\n";
	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->Dump (dc);
		dc << "\n";
	}

	dc << "}\n";
	dc.SetDepth (dc.GetDepth () - 1);
	dc << "\n";
}

#endif

//******************************************************************************************
int CBCGToolbarMenuButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	CBCGToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected);

	if (m_nID == 0 || !m_listCommands.IsEmpty ())	// Popup menu
	{
		CRect rectTriangle = rect;
		rectTriangle.left = rectTriangle.right - CMenuImages::Size ().cx;

		int iImage = (bSelected) ? CMenuImages::IdArowLeftWhite : CMenuImages::IdArowLeft;
		CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rectTriangle);

		CRect rectLine = rect;
		rectLine.right = rectTriangle.left - 1;
		rectLine.left = rectLine.right - 2;
		rectLine.DeflateRect (0, 2);

		pDC->Draw3dRect (&rectLine, globalData.clrBtnShadow, globalData.clrBtnHilite);
	}

	return rect.Width ();
}
//*******************************************************************************************
BOOL CBCGToolbarMenuButton::OnBeforeDrag () const
{
	if (m_pPopupMenu != NULL)	// Is dropped down
	{
		m_pPopupMenu->CollapseSubmenus ();
		m_pPopupMenu->SendMessage (WM_CLOSE);
	}

	return CBCGToolbarButton::OnBeforeDrag ();
}
//*******************************************************************************************
void CBCGToolbarMenuButton::GetTextHorzOffsets (int& xOffsetLeft, int& xOffsetRight)
{
	xOffsetLeft = CBCGToolBar::GetMenuImageSize ().cx / 2 + TEXT_MARGIN;
	xOffsetRight = CMenuImages::Size ().cx;
}
//*******************************************************************************************
void CBCGToolbarMenuButton::SaveBarState ()
{
	if (m_pWndParent == NULL)
	{
		return;
	}

	CBCGPopupMenu* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPopupMenu, m_pWndParent->GetParent ());
	if (pParentMenu == NULL)
	{
		return;
	}

	ASSERT_VALID (pParentMenu);

	CBCGPopupMenu* pTopLevelMenu = pParentMenu;
	while ((pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, pParentMenu->GetParent ()))
		!= NULL)
	{
		pTopLevelMenu = pParentMenu;
	}

	ASSERT_VALID (pTopLevelMenu);
	pTopLevelMenu->SaveState ();
}
//*************************************************************************************************
void CBCGToolbarMenuButton::GetImageRect (CRect& rectImage)
{
	ASSERT_VALID (this);

	rectImage = m_rect;
	rectImage.left += CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();

	rectImage.right = rectImage.left + 
					CBCGToolBar::GetMenuImageSize ().cx + CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
}
//*************************************************************************************************
void CBCGToolbarMenuButton::SetTearOff (UINT uiBarID)
{
	if (m_uiTearOffBarID == uiBarID)
	{
		return;
	}

	if (g_pTearOffMenuManager != NULL)
	{
		if (m_uiTearOffBarID != 0)
		{
			g_pTearOffMenuManager->SetInUse (m_uiTearOffBarID, FALSE);
		}

		if (uiBarID != 0)
		{
			g_pTearOffMenuManager->SetInUse (uiBarID);
		}
	}

	m_uiTearOffBarID = uiBarID;
}
//*************************************************************************************************
void CBCGToolbarMenuButton::SetRadio ()
{
	m_bIsRadio = TRUE;

	if (m_pWndParent != NULL)
	{
		CRect rectImage;
		GetImageRect (rectImage);

		m_pWndParent->InvalidateRect (rectImage);
		m_pWndParent->UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGToolbarMenuButton::ResetImageToDefault ()
{
	ASSERT_VALID (this);

	CBCGToolbarButton::ResetImageToDefault ();

	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarMenuButton* pItem = (CBCGToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->ResetImageToDefault ();
	}
}
//********************************************************************************
BOOL CBCGToolbarMenuButton::CompareWith (const CBCGToolbarButton& other) const
{
	if (m_nID != other.m_nID)
	{
		return FALSE;
	}

	const CBCGToolbarMenuButton& otherMenuBtn = (const CBCGToolbarMenuButton&) other;

	if (m_listCommands.GetCount () != otherMenuBtn.m_listCommands.GetCount ())
	{
		return FALSE;
	}

	POSITION pos1 = otherMenuBtn.m_listCommands.GetHeadPosition ();

	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		ASSERT (pos1 != NULL);

		CBCGToolbarMenuButton* pItem = (CBCGToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pItem);

		CBCGToolbarMenuButton* pItem1 = (CBCGToolbarMenuButton*) otherMenuBtn.m_listCommands.GetNext (pos1);
		ASSERT_VALID (pItem1);

		if (!pItem->CompareWith (*pItem1))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//********************************************************************************
void CBCGToolbarMenuButton::DrawDocumentIcon (CDC* pDC, const CRect& rectImage, HICON hIcon)
{
	ASSERT_VALID (pDC);

	int cx = globalData.m_sizeSmallIcon.cx;
	int cy = globalData.m_sizeSmallIcon.cy;

	if (cx > rectImage.Width () ||
		cy > rectImage.Height ())
	{
		// Small icon is too large, stretch it
		cx = rectImage.Width ();
		cy = rectImage.Height ();
	}

	int x = max (0, (rectImage.Width () - cx) / 2);
	int y = max (0, (rectImage.Height () - cy) / 2);

	::DrawIconEx (pDC->GetSafeHdc (),
		rectImage.left + x, rectImage.top + y, hIcon,
		cx, cy, 0, NULL, DI_NORMAL);
}
