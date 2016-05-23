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
// BCGPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGPopupMenu.h"
#include "BCGPropertyPage.h"
#include "BCGToolbarMenuButton.h"
#include "BCGPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertyPage property page

IMPLEMENT_DYNCREATE(CBCGPropertyPage, CPropertyPage)

#pragma warning (disable : 4355)

CBCGPropertyPage::CBCGPropertyPage() :
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPropertyPage::CBCGPropertyPage(UINT nIDTemplate, UINT nIDCaption) :
	CPropertyPage (nIDTemplate, nIDCaption),
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPropertyPage::CBCGPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption) :
	CPropertyPage (lpszTemplateName, nIDCaption),
	m_Impl (*this)
{
	CommonInit ();
}

void CBCGPropertyPage::CommonInit ()
{
	m_pCategory = NULL;
	m_nIcon = -1;
	m_nSelIconNum = -1;
	m_hTreeNode = NULL;
}

#pragma warning (default : 4355)

CBCGPropertyPage::~CBCGPropertyPage()
{
}

BEGIN_MESSAGE_MAP(CBCGPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGPropertyPage)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertyPage message handlers

void CBCGPropertyPage::OnActivate(UINT nState, CWnd* pWndOther, BOOL /*bMinimized*/) 
{
	m_Impl.OnActivate (nState, pWndOther);
}
//****************************************************************************************
BOOL CBCGPropertyPage::OnNcActivate(BOOL bActive) 
{
	m_Impl.OnNcActivate (bActive);

	//-----------------------------------------------------------
	// Do not call the base class because it will call Default()
	// and we may have changed bActive.
	//-----------------------------------------------------------
	return (BOOL) DefWindowProc (WM_NCACTIVATE, bActive, 0L);
}
//**************************************************************************************
void CBCGPropertyPage::SetActiveMenu (CBCGPopupMenu* pMenu)
{
	m_Impl.SetActiveMenu (pMenu);
}
//***************************************************************************************
BOOL CBCGPropertyPage::PreTranslateMessage(MSG* pMsg) 
{
	if (m_Impl.PreTranslateMessage (pMsg))
	{
		return TRUE;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}
//****************************************************************************************
BOOL CBCGPropertyPage::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (m_Impl.OnCommand (wParam, lParam))
	{
		return TRUE;
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}
//****************************************************************************************
BOOL CBCGPropertyPage::OnSetActive() 
{
	CBCGPropertySheet* pParent = DYNAMIC_DOWNCAST(CBCGPropertySheet, GetParent ());
	if (pParent != NULL)
	{
		pParent->OnActivatePage (this);
	}
	
	return CPropertyPage::OnSetActive();
}
