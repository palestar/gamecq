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

// BCGShowAllButton.cpp: implementation of the CBCGShowAllButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGShowAllButton.h"
#include "BCGMenuBar.h"
#include "BCGPopupMenuBar.h"
#include "BCGPopupMenu.h"
#include "MenuImages.h"
#include "globals.h"
#include "KeyHelper.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGVisualManager.h"
#include "BCGDrawManager.h"

const int nMinMenuWidth = 50;

IMPLEMENT_DYNCREATE(CBCGShowAllButton, CBCGToolbarMenuButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGShowAllButton::CBCGShowAllButton()
{
}
//***************************************************************************************
CBCGShowAllButton::~CBCGShowAllButton()
{
}
//***************************************************************************************
void CBCGShowAllButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* /*pImages*/,
								BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight,
								BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CRect rectBar = rect;
	rectBar.DeflateRect (1, 1);

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight);

	//-----------------------
	// Draw "show all" image:
	//-----------------------
	CBCGVisualManager::BCGBUTTON_STATE state = CBCGVisualManager::ButtonsIsRegular;

	if (bHighlight)
	{
		state = CBCGVisualManager::ButtonsIsHighlighted;
	}
	else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		//-----------------------
		// Pressed in or checked:
		//-----------------------
		state = CBCGVisualManager::ButtonsIsPressed;
	}

	CBCGVisualManager::GetInstance ()->OnDrawShowAllMenuItems (pDC,
		rectBar, state);

	//--------------------
	// Draw button border:
	//--------------------
	if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		//-----------------------
		// Pressed in or checked:
		//-----------------------
		CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectBar, CBCGVisualManager::ButtonsIsPressed);
	}
	else if (bHighlight)
	{
		CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectBar, CBCGVisualManager::ButtonsIsHighlighted);
	}
}
//***********************************************************************************
SIZE CBCGShowAllButton::OnCalculateSize (
								CDC* pDC,
								const CSize& sizeDefault,
								BOOL /*bHorz*/)
{
	return CSize (nMinMenuWidth, 
		CBCGVisualManager::GetInstance ()->GetShowAllMenuItemsHeight (pDC, sizeDefault));
}
//************************************************************************************
BOOL CBCGShowAllButton::OnClick (CWnd* /*pWnd*/, BOOL bDelay)
{
	CBCGPopupMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pWndParent);
	if (pParentMenuBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (bDelay)
	{
		if (CBCGMenuBar::IsShowAllCommandsDelay ())
		{
			pParentMenuBar->StartPopupMenuTimer (this, 2);
		}

		return TRUE;
	}

	CBCGPopupMenu* pParentMenu = 
		DYNAMIC_DOWNCAST (CBCGPopupMenu, pParentMenuBar->GetParent ());
	if (pParentMenu == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	pParentMenu->ShowAllCommands ();
	return TRUE;
}
//************************************************************************************
BOOL CBCGShowAllButton::OpenPopupMenu (CWnd* pWnd)
{
	return OnClick (pWnd, FALSE);
}
//************************************************************************************
BOOL CBCGShowAllButton::OnToolHitTest (const CWnd* /*pWnd*/, TOOLINFO* pTI)
{
	ASSERT (pTI != NULL);

	CString strText;
	CString strKey;

	ACCEL accel;
	accel.fVirt = FVIRTKEY | FCONTROL;
	accel.key = VK_DOWN;

	CBCGKeyHelper helper (&accel);
	helper.Format (strKey);

	CBCGLocalResource locaRes;
	strText.Format (IDS_BCGBARRES_EXPAND_FMT, strKey);

	pTI->lpszText = (LPTSTR) ::calloc ((strText.GetLength () + 1), sizeof (TCHAR));
	_tcscpy (pTI->lpszText, strText);

	pTI->uId = 0;
	return TRUE;
}
