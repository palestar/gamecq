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
//
// BCGCustomizeMenuButton.cpp: implementation of the CBCGCustomizeMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgbarres.h"
#include "BCGCustomizeMenuButton.h"
#include "CustomizeButton.h"
#include "BCGFrameWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGKeyboardManager.h"
#include "BCGLocalRes.h"
#include "afxtempl.h"
#include "BCGVisualManager.h"


IMPLEMENT_DYNCREATE(CBCGCustomizeMenuButton, CBCGToolbarMenuButton)

CMap<UINT, UINT, int, int>	 CBCGCustomizeMenuButton::m_mapPresentIDs;
CBCGToolBar* CBCGCustomizeMenuButton::m_pWndToolBar = NULL;
BOOL CBCGCustomizeMenuButton::m_bRecentlyUsedOld = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGCustomizeMenuButton::CBCGCustomizeMenuButton()
{
}
//****************************************************************************************
CBCGCustomizeMenuButton::~CBCGCustomizeMenuButton()
{
}
//****************************************************************************************
CBCGCustomizeMenuButton::CBCGCustomizeMenuButton(UINT uiID,HMENU hMenu,int iImage,LPCTSTR lpszText,BOOL bUserButton):
	CBCGToolbarMenuButton (uiID, hMenu/* HMENU */, iImage /*iImage*/, lpszText, bUserButton)
{
	m_uiIndex = (UINT)-1;
	bSeparator = FALSE;
	m_bAddSpr = FALSE;
	m_bIsEnabled = TRUE;
	m_bBrothersBtn = FALSE;
}
//****************************************************************************************
void CBCGCustomizeMenuButton::SetItemIndex(UINT uiIndex, BOOL bExist, BOOL bAddSpr)
{
	m_uiIndex = uiIndex;
	m_bExist = bExist;
	m_bAddSpr = bAddSpr;
	
	if((uiIndex != ID_BCGBARRES_TOOLBAR_RESET_PROMT)
		&& !bSeparator && bExist)
	{
		CBCGToolbarButton* pBtn = m_pWndToolBar->GetButton(uiIndex);
		m_bShow = pBtn->IsVisible();
		
	}else
	{
		m_bShow = FALSE;
	}
}
//****************************************************************************************
void CBCGCustomizeMenuButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);
	const CBCGCustomizeMenuButton& src = (const CBCGCustomizeMenuButton&) s;
	
	m_uiIndex      =   src.m_uiIndex;
	m_bShow        =   src.m_bShow;
	m_pWndToolBar  =   src.m_pWndToolBar;
	bSeparator     =   src.bSeparator;
	m_bExist       =   src.m_bExist;
	m_bAddSpr      =   src.m_bAddSpr;
	m_bIsEnabled   =   src.m_bIsEnabled;
	m_bBrothersBtn =   src.m_bBrothersBtn;	
}
//****************************************************************************************
SIZE CBCGCustomizeMenuButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (bSeparator)
	{
		return CSize (0,  4);
	}
	
	if (m_bBrothersBtn)
	{
		return CBCGToolbarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}

	//---------------------------------
	// Get actual ID to search for Accel
	//---------------------------------
	UINT nComandID = m_nID;
	if (m_uiIndex != ID_BCGBARRES_TOOLBAR_RESET_PROMT)
	{
		CBCGToolbarButton* pButton = m_pWndToolBar->GetButton (m_uiIndex);
		if (pButton != NULL)
		{
			ASSERT_VALID (pButton);
			nComandID = pButton->m_nID;
		}
	}
	
	//-----------------------------
	//  Try to Find Buttons Text
	//-----------------------------
	if (m_strText.IsEmpty ())
	{
		//-------------------------------------------
		// Try to find the command name in resources:
		//-------------------------------------------
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString (nComandID) &&
			(iOffset = strMessage.Find (_T('\n'))) != -1)
		{
			m_strText = strMessage.Mid (iOffset + 1);
		}
	}
	else
	{
		// m_strText.Remove (_T('&'));
		
		//----------------------------------------
		// Remove trailing label (ex.:"\tCtrl+S"):
		//----------------------------------------
		int iOffset = m_strText.Find (_T('\t'));
		if (iOffset != -1)
		{
			m_strText = m_strText.Left (iOffset);
		}
	}
	
	//--------------------
	// Change accelerator:
	//--------------------
	if (g_pKeyboardManager != NULL &&
		m_bMenuMode &&
		(nComandID < 0xF000 || nComandID >= 0xF1F0))	// Not system.
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
			nComandID, strAccel, pParent, TRUE) ||
			CBCGKeyboardManager::FindDefaultAccelerator (
			nComandID, strAccel, pParent->GetActiveFrame (), FALSE)))
		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}
	
	int nTolalWidth = m_strText.GetLength();

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	nTolalWidth *= tm.tmAveCharWidth;
	CSize sizeImage = CBCGToolBar::GetMenuButtonSize ();
	nTolalWidth += 2*sizeImage.cx;	
	nTolalWidth += 3*CBCGVisualManager::GetInstance ()->GetMenuImageMargin () + 50;
	
	CSize sizeStandard = CBCGToolbarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	
	int nTotalHeight = sizeStandard.cy + 2;
	
	if (!m_bMenuMode)
	{
		nTotalHeight += CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
	}
	
	return CSize (nTolalWidth,  nTotalHeight);
}
//****************************************************************************************
BOOL CBCGCustomizeMenuButton::OnClickMenuItem()
{
	if (bSeparator || !m_bIsEnabled)
	{
		return TRUE;
	}
	
	CBCGPopupMenuBar* pMenuBar = (CBCGPopupMenuBar*)m_pWndParent;
	int nIndex = pMenuBar->ButtonToIndex(this);
	if (nIndex !=-1)
	{
		if (pMenuBar->m_iHighlighted != nIndex)
		{
			pMenuBar->m_iHighlighted = nIndex;
			pMenuBar->InvalidateRect (this->Rect ());
		}
	}
	
	if (m_uiIndex == ID_BCGBARRES_TOOLBAR_RESET_PROMT) // reset pressed
	{
		//load default toolbar
		m_pWndToolBar->PostMessage (BCGM_RESETRPROMPT);
		return FALSE;	
	}
	
	if (!m_bExist)
	{	
		const CObList& lstOrignButtons = m_pWndToolBar->GetOrigResetButtons ();
		
		POSITION pos = lstOrignButtons.FindIndex (m_uiIndex);
		CBCGToolbarButton* pButton = (CBCGToolbarButton*)lstOrignButtons.GetAt (pos);
		if (pButton == NULL)
		{
			return TRUE;
		}
		
		UINT nIndex = m_pWndToolBar->InsertButton(*pButton, m_uiIndex);
		
		if (nIndex == -1)
		{
			nIndex = m_pWndToolBar->InsertButton(*pButton);	
		}
		else
		{
			CBCGPopupMenuBar* pMenuBar = (CBCGPopupMenuBar*)m_pWndParent;
			int nCount = pMenuBar->GetCount ();
			for (int i=0; i< nCount; i++)
			{
				CBCGCustomizeMenuButton* pBtn = (CBCGCustomizeMenuButton*)pMenuBar->GetButton(i);
				if ((pBtn->m_uiIndex >= nIndex) && 
					(pBtn->m_uiIndex != ID_BCGBARRES_TOOLBAR_RESET_PROMT))
				{
					if (pBtn->m_bExist)
					{
						pBtn->m_uiIndex += 1; 
					}
				}
			}
		}

		m_uiIndex = nIndex;
		
		if (m_bAddSpr) 
		{
			if (nIndex < (UINT)m_pWndToolBar->GetCount ())
			{
				CBCGToolbarButton* pBtn = m_pWndToolBar->GetButton (nIndex+1);
				if (!(pBtn->m_nStyle & TBBS_SEPARATOR))
				{
					m_pWndToolBar->InsertSeparator ();
				}	
			}
			else
			{
				m_pWndToolBar->InsertSeparator ();
			}
		}

		m_pWndToolBar->AdjustLayout ();
		UpdateCustomizeButton ();
		
		m_bExist = TRUE;
		m_bShow = TRUE;
		CBCGPopupMenuBar* pMenuBar = (CBCGPopupMenuBar*)m_pWndParent;
		pMenuBar->Invalidate ();

		return TRUE;	
	}
	
	CBCGToolbarButton* pBtn = m_pWndToolBar->GetButton (m_uiIndex);
	BOOL bVisible = pBtn->IsVisible ();
	pBtn->SetVisible (!bVisible);
	m_bShow = !bVisible;
	
	//-------------------------------------
	//  Make next Separator the same state
	//-------------------------------------
	int nNext = m_uiIndex + 1;
	if (nNext < m_pWndToolBar->GetCount ())
	{
		CBCGToolbarButton* pBtnNext = m_pWndToolBar->GetButton (nNext);
		if (pBtnNext->m_nStyle & TBBS_SEPARATOR)
		{
			pBtnNext->SetVisible (!bVisible);	
		}	
	}

	//---------------------------------
	// Hide/show last visible separator
	//---------------------------------
	if (bVisible) 
	{
		BOOL bLastVisibeBtn = TRUE;
		int nCount = m_pWndToolBar->GetCount ();
		int nNextIndex = m_uiIndex + 1;
		for (int i=nNextIndex; i< nCount; i++)
		{
			CBCGToolbarButton* pButton = (CBCGToolbarButton*)m_pWndToolBar->GetButton (i);
			CCustomizeButton* pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);
			if (pButton->IsVisible () && (pCustomizeBtn == NULL))
			{
				bLastVisibeBtn = FALSE;
				break;
			}
		}

		if (bLastVisibeBtn)
		{
			int nPrev = m_uiIndex - 1;
			if (nPrev < m_pWndToolBar->GetCount () && nPrev > 0)
			{
				CBCGToolbarButton* pBtnPrev = m_pWndToolBar->GetButton (nPrev);
				if (pBtnPrev->m_nStyle & TBBS_SEPARATOR)
				{
					pBtnPrev->SetVisible (FALSE);	
				}
			}
		}
	}
	else
	{
		int nPrev = m_uiIndex - 1;
		if (nPrev < m_pWndToolBar->GetCount () && nPrev > 0)
		{
			CBCGToolbarButton* pBtnPrev = m_pWndToolBar->GetButton (nPrev);
			if (pBtnPrev->m_nStyle & TBBS_SEPARATOR)
			{
				int nPrevPrev = m_uiIndex - 2;	
				if (nPrevPrev < m_pWndToolBar->GetCount () && nPrevPrev >= 0)
				{
					CBCGToolbarButton* pBtnPrevPrev = m_pWndToolBar->GetButton (nPrevPrev);
					if (pBtnPrevPrev->IsVisible ())
					{
						pBtnPrev->SetVisible (TRUE);
					}
				}
			}	
		}
	}

	CBCGPopupMenu* pCustomizeMenu = NULL;

	for (CBCGPopupMenu* pMenu = 
		DYNAMIC_DOWNCAST (CBCGPopupMenu, pMenuBar->GetParent ());
		pMenu != NULL; pMenu = pMenu->GetParentPopupMenu ())
	{
		pCustomizeMenu = pMenu;
	}

	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow (SW_HIDE);
	}

	m_pWndToolBar->AdjustLayout();
	UpdateCustomizeButton();
	pMenuBar->Invalidate();
	
	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow (SW_SHOWNOACTIVATE);
	}
	
	return TRUE;
}
//****************************************************************************************
void CBCGCustomizeMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
									   BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
									   BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	
	//----------------
	// Draw separator:
	//----------------
	if (bSeparator)
	{
		CRect rcSeparator(rect);
		rcSeparator.left = 2*CBCGToolBar::GetMenuImageSize ().cx + 
			CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
		
		CBCGPopupMenuBar* pMenuBar = (CBCGPopupMenuBar*)m_pWndParent;
		CBCGVisualManager::GetInstance ()->OnDrawSeparator (pDC, pMenuBar, rcSeparator, FALSE);
		
		return;
	}
	
	if (m_bBrothersBtn)
	{
		CBCGToolbarMenuButton::OnDraw (pDC, rect, NULL,
									   bHorz, bCustomizeMode, bHighlight,
									   bDrawBorder, bGrayDisabledButtons);
		return;
	}
	
	CRect rectItem = rect;
	rectItem.bottom--;
	
	if (m_bIsEnabled)
	{	  
		if (m_bShow && bHighlight)
		{		  
			SetStyle (TBBS_BUTTON|TBBS_CHECKED);
		}
		else
		{	  
			SetStyle (TBBS_BUTTON);
		}		  
	}
	else
	{
		SetStyle (TBBS_DISABLED);
		bGrayDisabledButtons = TRUE;
		bHighlight = FALSE;
	}

	BOOL bIsResetItem = m_uiIndex == ID_BCGBARRES_TOOLBAR_RESET_PROMT;

	if (bIsResetItem)
	{
		m_bImage = FALSE;
		m_iImage = -1;
	}

	//-----------------
	//	Highlight item:
	//-----------------
	if (bHighlight && m_bIsEnabled)
	{
		CRect rcHighlight = rectItem;
		rcHighlight.left += 2;
		rcHighlight.right--;
		
		if (!CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem () &&
			!bIsResetItem)
		{
			rcHighlight.left += 2 * CBCGToolBar::GetMenuImageSize ().cx + 
				5 * CBCGVisualManager::GetInstance ()->GetMenuImageMargin ();
		}
		
		COLORREF clrText;
		CBCGVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rcHighlight, clrText);
	}
	
	//---------------
	// Draw checkbox:
	//---------------
	CSize sizeMenuImage = CBCGToolBar::GetMenuImageSize ();
	
	CRect rectCheck = rectItem;
	rectCheck.left += CBCGVisualManager::GetInstance ()->GetMenuImageMargin () + 1;
	rectCheck.right = rectCheck.left + sizeMenuImage.cx + 
		CBCGVisualManager::GetInstance ()->GetMenuImageMargin () + 2;
	rectCheck.bottom--;
	
	DrawCheckBox (pDC, rectCheck, bHighlight);
	
	if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !bIsResetItem)
	{
		SetStyle (TBBS_BUTTON);
	}
	
	//------------------
	// Draw icon + text:
	//------------------
	CRect rectStdMenu = rectItem;
	rectStdMenu.left = rectCheck.right;
	
	DrawMenuItem (pDC, rectStdMenu, pImages, bCustomizeMode, 
		bHighlight, bGrayDisabledButtons, TRUE);
}
//****************************************************************************************
CString CBCGCustomizeMenuButton::SearchCommandText(CMenu* pMenu, UINT in_uiCmd)
{
	ASSERT (pMenu != NULL);
	
	int iCount = (int) pMenu->GetMenuItemCount ();
	
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID (i);
		if (uiCmd == in_uiCmd)
		{
			CString strText;
			pMenu->GetMenuString (i, strText, MF_BYPOSITION);
			return strText;
		}
		
		switch (uiCmd)
		{
		case 0:		// Separator, ignore it.
			break;
			
		case -1:	// Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu (i);
				
				CString strText = SearchCommandText (pSubMenu, in_uiCmd);
				if(strText != _T("")) return strText;
			}
			break;
			
		}//end switch
	}//end for
	
	return _T("");
}
//****************************************************************************************
void CBCGCustomizeMenuButton::DrawCheckBox (CDC* pDC, const CRect& rect, BOOL bHighlight)
{	
	if (!m_bShow)
	{
		return;
	}

	CRect rectCheck = rect;
	rectCheck.DeflateRect (0, 1, 1, 1);

	if (!CBCGVisualManager::GetInstance ()->IsOwnerDrawMenuCheck ())
	{
		UINT nStyle = m_nStyle;
		m_nStyle |= TBBS_CHECKED;
	
		FillInterior (pDC, rectCheck, bHighlight);
	
		CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectCheck, CBCGVisualManager::ButtonsIsPressed);

		m_nStyle = nStyle;
	}

	CBCGVisualManager::GetInstance ()->OnDrawMenuCheck (pDC, this, 
		rectCheck, bHighlight, FALSE);
}
//****************************************************************************************
void CBCGCustomizeMenuButton::UpdateCustomizeButton()
{
	ASSERT_VALID (m_pWndToolBar);

	if (m_pWndToolBar->GetParent ()->GetSafeHwnd () != NULL)
	{
		m_pWndToolBar->GetParent ()->RedrawWindow (
			NULL, NULL, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
	}

	m_pWndToolBar->RedrawCustomizeButton ();
}
//****************************************************************************************
BOOL CBCGCustomizeMenuButton::IsCommandExist(UINT uiCmdId)
{
	int nTmp = 0;
	return m_mapPresentIDs.Lookup(uiCmdId, nTmp);
}
