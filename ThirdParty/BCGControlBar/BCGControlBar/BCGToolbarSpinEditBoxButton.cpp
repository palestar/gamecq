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
// BCGToolbarSpinEditBoxButton.cpp: implementation of the CBCGToolbarSpinEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGToolbarSpinEditBoxButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGToolbarSpinEditBoxButton, CBCGToolbarEditBoxButton, 1)

CBCGToolbarSpinEditBoxButton::CBCGToolbarSpinEditBoxButton()
{
	Init ();
}
//***************************************************************************************
CBCGToolbarSpinEditBoxButton::CBCGToolbarSpinEditBoxButton (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGToolbarEditBoxButton (uiId, iImage, dwStyle, iWidth)
{
	Init ();
}
//***************************************************************************************
void CBCGToolbarSpinEditBoxButton::Init ()
{
	m_nMin = INT_MIN;
	m_nMax = INT_MAX;
}
//****************************************************************************************
CBCGToolbarSpinEditBoxButton::~CBCGToolbarSpinEditBoxButton()
{
	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.DestroyWindow ();
	}
}
//***************************************************************************************
CEdit* CBCGToolbarSpinEditBoxButton::CreateEdit(CWnd* pWndParent, const CRect& rect)
{
   CEdit *pEdit = CBCGToolbarEditBoxButton::CreateEdit(pWndParent,rect);
   if (pEdit == NULL)
   {
	   return NULL;
   }

	if (!m_wndSpin.Create(
		WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT,
		  rect, pWndParent, m_nID))
		  return NULL;

   m_wndSpin.SetBuddy (pEdit);
   m_wndSpin.SetRange32 (m_nMin, m_nMax);

   return pEdit;
}
//**************************************************************************************
void CBCGToolbarSpinEditBoxButton::OnMove ()
{
	CBCGToolbarEditBoxButton::OnMove ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_wndSpin.SetBuddy (m_pWndEdit);
	}	
}
//**************************************************************************************
void CBCGToolbarSpinEditBoxButton::GetEditBorder (CRect& rectBorder)
{
	ASSERT (m_pWndEdit->GetSafeHwnd () != NULL);

	m_pWndEdit->GetWindowRect (rectBorder);
	m_pWndEdit->GetParent ()->ScreenToClient (rectBorder);

	CRect rectSpin;
	m_wndSpin.GetWindowRect (rectSpin);
	m_wndSpin.GetParent ()->ScreenToClient (rectSpin);

	rectBorder.right = rectSpin.right;

	rectBorder.InflateRect (1, 1);
}
//**************************************************************************************
void CBCGToolbarSpinEditBoxButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarEditBoxButton::CopyFrom (s);

	const CBCGToolbarSpinEditBoxButton& src = (const CBCGToolbarSpinEditBoxButton&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
}
//**************************************************************************************
void CBCGToolbarSpinEditBoxButton::Serialize (CArchive& ar)
{
	CBCGToolbarEditBoxButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_nMin;
		ar >> m_nMax;
	}
	else
	{
		ar << m_nMin;
		ar << m_nMax;
	}
}
//***************************************************************************************
void CBCGToolbarSpinEditBoxButton::SetRange (int nMin, int nMax)
{
	ASSERT_VALID (this);

	m_nMin = nMin;
	m_nMax = nMax;

	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.SetRange32 (nMin, nMax);
	}
}
//***************************************************************************************
void CBCGToolbarSpinEditBoxButton::GetRange (int& nMin, int& nMax)
{
	ASSERT_VALID (this);

	nMin = m_nMin;
	nMax = m_nMax;
}
//****************************************************************************************
void CBCGToolbarSpinEditBoxButton::OnShowEditbox (BOOL bShow)
{
	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.ShowWindow (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
}
