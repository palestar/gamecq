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

// BCGToolbarRegularMenuButton.cpp: implementation of the CBCGToolbarRegularMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGToolbarRegularMenuButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL (CBCGToolbarRegularMenuButton, CBCGToolbarMenuButton, VERSIONABLE_SCHEMA | 1)

BOOL CBCGToolbarRegularMenuButton::OnClick(CWnd* pWnd, BOOL bDelay)
{
	// Standardverarbeitung
	CBCGToolbarMenuButton::OnClick(pWnd, bDelay);

	if (CBCGToolBar::IsCustomizeMode())
		return FALSE;

	CPoint ptPos;

	if (m_bHorz)
	{
		ptPos.x = m_rect.left; 
		ptPos.y = m_rect.bottom;
	}

	else
	{
		ptPos.x = m_rect.right; 
		ptPos.y = m_rect.top;
	}

	CWnd* pParent = m_pWndParent == NULL ? 
						AfxGetMainWnd () :
						BCGGetTopLevelFrame (m_pWndParent);
	pParent->ClientToScreen(&ptPos);

	pParent->SendMessage(BCGM_SHOWREGULARMENU, m_nID, MAKELONG(ptPos.x, ptPos.y));
	return TRUE;
}
