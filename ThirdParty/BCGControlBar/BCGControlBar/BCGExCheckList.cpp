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

// BCGExCheckList.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "afxpriv.h"
#include "BCGExCheckList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGExCheckList

CBCGExCheckList::CBCGExCheckList()
{
}
//*******************************************************************************************
CBCGExCheckList::~CBCGExCheckList()
{
}

BEGIN_MESSAGE_MAP(CBCGExCheckList, CCheckListBox)
	//{{AFX_MSG_MAP(CBCGExCheckList)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(LB_ADDSTRING, OnLBAddString)
	ON_MESSAGE(LB_INSERTSTRING, OnLBInsertString)
	ON_MESSAGE(LB_RESETCONTENT, OnLBResetContent)
	ON_MESSAGE(LB_DELETESTRING, OnLBDeleteString)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGExCheckList message handlers

void CBCGExCheckList::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled (nIndex))
	{
		MessageBeep ((UINT) -1);
		return;
	}
	
	CCheckListBox::OnLButtonDown(nFlags, point);
}
//*******************************************************************************************
void CBCGExCheckList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE)
	{
		int nIndex = GetCaretIndex();
		if (nIndex != LB_ERR && !IsCheckEnabled (nIndex))
		{
			MessageBeep ((UINT) -1);
			return;
		}
	}
	
	CCheckListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*******************************************************************************************
LRESULT CBCGExCheckList::OnLBAddString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBAddString(wParam, lParam);
	OnNewString ((int) lRes);
	return lRes;
}
//*******************************************************************************************
LRESULT CBCGExCheckList::OnLBInsertString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBInsertString(wParam, lParam);
	OnNewString ((int) lRes);
	return lRes;
}
//*******************************************************************************************
LRESULT CBCGExCheckList::OnLBDeleteString(WPARAM wParam, LPARAM /*lParam*/)
{
	LRESULT lRes = Default ();
	if (lRes != LB_ERR)
	{
		m_arCheckData.RemoveAt ((int) wParam);
	}

	return lRes;
}
//*******************************************************************************************
LRESULT CBCGExCheckList::OnLBResetContent(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_arCheckData.SetSize (0);
	return Default ();
}
//*******************************************************************************************
void CBCGExCheckList::EnableCheck (int nIndex, BOOL bEnable)
{
	ASSERT (nIndex >= 0 && nIndex < m_arCheckData.GetSize ());
	m_arCheckData.SetAt (nIndex, bEnable);
}
//*******************************************************************************************
BOOL CBCGExCheckList::IsCheckEnabled (int nIndex) const
{
	ASSERT (nIndex >= 0 && nIndex < m_arCheckData.GetSize ());
	return m_arCheckData.GetAt (nIndex);
}
//*******************************************************************************************
void CBCGExCheckList::OnNewString (int iIndex)
{
	if (iIndex >= 0)
	{
		int iSize = GetCount ();
		m_arCheckData.SetSize (iSize);

		for (int i = iSize - 1; i > iIndex; i --)
		{
			m_arCheckData.SetAt (i, m_arCheckData.GetAt (i - 1));
		}
		
		m_arCheckData.SetAt (iIndex, TRUE);	// Enabled by default
	}
}
//****************************************************************************************
void CBCGExCheckList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled (nIndex))
	{
		MessageBeep ((UINT) -1);
		return;
	}
	
	CCheckListBox::OnLButtonDblClk(nFlags, point);
	GetParent()->SendMessage(WM_COMMAND,
                            MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE),
                            (LPARAM)m_hWnd);
}

#endif // BCG_NO_CUSTOMIZATION
