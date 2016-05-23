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

// BCGToolbarMenuButtonsButton.cpp: implementation of the CBCGToolbarMenuButtonsButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGToolbarMenuButtonsButton.h"
#include "BCGVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGToolbarMenuButtonsButton, CBCGToolbarButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarMenuButtonsButton::CBCGToolbarMenuButtonsButton()
{
	m_uiSystemCommand = 0;
}
//****************************************************************************************
CBCGToolbarMenuButtonsButton::CBCGToolbarMenuButtonsButton(UINT uiCmdId)
{
	if (uiCmdId != SC_CLOSE &&
		uiCmdId != SC_MINIMIZE &&
		uiCmdId != SC_RESTORE)
	{
		ASSERT (FALSE);
	}

	m_uiSystemCommand = uiCmdId;
}
//****************************************************************************************
CBCGToolbarMenuButtonsButton::~CBCGToolbarMenuButtonsButton()
{
}
//****************************************************************************************
void CBCGToolbarMenuButtonsButton::OnDraw (CDC* pDC, const CRect& rect, 
					CBCGToolBarImages* /*pImages*/,
					BOOL /*bHorz*/, BOOL /*bCustomizeMode*/,
					BOOL bHighlight,
					BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	CBCGVisualManager::GetInstance ()->OnDrawMenuSystemButton (
		pDC, rect, m_uiSystemCommand, m_nStyle, bHighlight);
}
//****************************************************************************************
SIZE CBCGToolbarMenuButtonsButton::OnCalculateSize (CDC* /*pDC*/, const CSize& /*sizeDefault*/,
													BOOL /*bHorz*/)
{
	return CSize (	::GetSystemMetrics (SM_CXMENUSIZE),
					::GetSystemMetrics (SM_CYMENUSIZE));
}
//****************************************************************************************
void CBCGToolbarMenuButtonsButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);

	const CBCGToolbarMenuButtonsButton& src = (const CBCGToolbarMenuButtonsButton&) s;
	m_uiSystemCommand = src.m_uiSystemCommand;
}
