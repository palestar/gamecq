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

// BCGToolbarDateTimeCtrl.cpp: implementation of the CBCGToolbarDateTimeCtrl class.
//

#include "stdafx.h"
#include "BCGToolbar.h"
#include "globals.h"
#include "BCGToolbarDateTimeCtrl.h"
#include "BCGToolbarMenuButton.h"
#include "MenuImages.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGToolbarDateTimeCtrl, CBCGToolbarButton, 1)

static const int iDefaultSize = 100;
static const int iHorzMargin = 3;
static const int iVertMargin = 3;

BEGIN_MESSAGE_MAP(CBCGDateTimeCtrlWin, CDateTimeCtrl)
	//{{AFX_MSG_MAP(CBCGDateTimeCtrlWin)
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE, OnDateTimeChange)
	ON_NOTIFY_REFLECT(DTN_DROPDOWN, OnDateTimeDropDown)
	ON_NOTIFY_REFLECT(DTN_CLOSEUP, OnDateTimeCloseUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#pragma warning (disable : 4310)

void CBCGDateTimeCtrlWin::OnDateTimeChange (NMHDR * pNotifyStruct, LRESULT* pResult)
{
	// Send a WM_COMMAND message with the DTN_DATETIMECHANGE notification (truncated to lower 16 bits)
	// so that we will process it first then it will get forwarded by CBCGToolbar to the current
	// frame window, otherwise everyone has to subclass this class and reflect the WM_NOTIFY messages.
	// Yeah, a hack, but a pretty safe one.
	if (!m_bMonthCtrlDisplayed)
	{
		LPNMDATETIMECHANGE pNotify = (LPNMDATETIMECHANGE) pNotifyStruct;

		GetOwner()->PostMessage (WM_COMMAND,
			MAKEWPARAM (pNotify->nmhdr.idFrom, LOWORD (DTN_DATETIMECHANGE)), (LPARAM)GetSafeHwnd());
	}

	*pResult = 0;
}
//**************************************************************************************
void CBCGDateTimeCtrlWin::OnDateTimeDropDown (NMHDR* /* pNotifyStruct */, LRESULT* pResult)
{
	m_bMonthCtrlDisplayed = true;

	*pResult = 0;
}
//**************************************************************************************
void CBCGDateTimeCtrlWin::OnDateTimeCloseUp (NMHDR* pNotifyStruct, LRESULT* pResult)
{
	m_bMonthCtrlDisplayed = false;

	LPNMDATETIMECHANGE pNotify = (LPNMDATETIMECHANGE) pNotifyStruct;

	GetOwner()->PostMessage (WM_COMMAND,
		MAKEWPARAM (pNotify->nmhdr.idFrom, LOWORD (DTN_DATETIMECHANGE)), (LPARAM)GetSafeHwnd());

	*pResult = 0;
}

#pragma warning (default : 4310)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarDateTimeCtrl::CBCGToolbarDateTimeCtrl()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE;
	m_iWidth = iDefaultSize;

	Initialize ();
}
//**************************************************************************************
CBCGToolbarDateTimeCtrl::CBCGToolbarDateTimeCtrl (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGToolbarButton (uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE;
	m_iWidth = (iWidth == 0) ? iDefaultSize : iWidth;

	Initialize ();
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::Initialize ()
{
	m_pWndDateTime = NULL;
	m_bHorz = TRUE;
	m_dwTimeStatus = GDT_VALID;
	m_time = CTime::GetCurrentTime ();
}
//**************************************************************************************
CBCGToolbarDateTimeCtrl::~CBCGToolbarDateTimeCtrl ()
{
	if (m_pWndDateTime != NULL)
	{
		m_pWndDateTime->DestroyWindow ();
		delete m_pWndDateTime;
	}
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);

	DuplicateData ();

	const CBCGToolbarDateTimeCtrl& src = (const CBCGToolbarDateTimeCtrl&) s;
	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::Serialize (CArchive& ar)
{
	CBCGToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_dwTimeStatus;
		ar >> m_time;

		if (m_pWndDateTime)
			m_pWndDateTime->SetTime (m_dwTimeStatus == GDT_VALID? &m_time : NULL);

		DuplicateData ();
	}
	else
	{
		m_dwTimeStatus = m_pWndDateTime->GetTime(m_time);

		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_dwTimeStatus;
		ar << m_time;
	}
}
//**************************************************************************************
SIZE CBCGToolbarDateTimeCtrl::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if(!IsVisible())
	{
		return CSize(0,0);
	}

	m_bHorz = bHorz;
	m_sizeText = CSize (0, 0);

	if (bHorz)
	{
		if (m_pWndDateTime->GetSafeHwnd () != NULL && !m_bIsHidden)
		{
			m_pWndDateTime->ShowWindow (SW_SHOWNOACTIVATE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText (0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText (m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size ();
		}

		return CSize (m_iWidth, sizeDefault.cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndDateTime->GetSafeHwnd () != NULL &&
			(m_pWndDateTime->GetStyle () & WS_VISIBLE))
		{
			m_pWndDateTime->ShowWindow (SW_HIDE);
		}

		return CBCGToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);
	}
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::OnMove ()
{
	if (m_pWndDateTime->GetSafeHwnd () == NULL ||
		(m_pWndDateTime->GetStyle () & WS_VISIBLE) == 0)
	{
		return;
	}

	CRect rectDateTime;
	m_pWndDateTime->GetWindowRect (rectDateTime);

	m_pWndDateTime->SetWindowPos (NULL, 
		m_rect.left + iHorzMargin, 
		m_rect.top + (m_rect.Height () - m_sizeText.cy - rectDateTime.Height ()) / 2,
		m_rect.Width () - 2 * iHorzMargin, 
		globalData.GetTextHeight() + 2 * iVertMargin,
		SWP_NOZORDER | SWP_NOACTIVATE);

	AdjustRect ();
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::OnSize (int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	if (m_pWndDateTime->GetSafeHwnd () != NULL &&
		(m_pWndDateTime->GetStyle () & WS_VISIBLE))
	{
		m_pWndDateTime->SetWindowPos (NULL, 
			m_rect.left + iHorzMargin, m_rect.top,
			m_rect.Width () - 2 * iHorzMargin, 
			globalData.GetTextHeight() + 2 * iVertMargin,
			SWP_NOZORDER | SWP_NOACTIVATE);

		AdjustRect ();
	}
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	if (m_pWndDateTime->GetSafeHwnd () != NULL)
	{
		CWnd* pWndParentCurr = m_pWndDateTime->GetParent ();
		ASSERT (pWndParentCurr != NULL);

		if (pWndParent != NULL &&
			pWndParentCurr->GetSafeHwnd () == pWndParent->GetSafeHwnd ())
		{
			return;
		}

		m_pWndDateTime->DestroyWindow ();
		delete m_pWndDateTime;
		m_pWndDateTime = NULL;
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rect = m_rect;
	rect.InflateRect (-2, 0);
	rect.bottom = rect.top + globalData.GetTextHeight() + 2 * iVertMargin;

	if ((m_pWndDateTime = CreateDateTimeCtrl (pWndParent, rect)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	AdjustRect ();

	m_pWndDateTime->SetFont (&globalData.fontRegular);

	m_pWndDateTime->SetTime (m_dwTimeStatus == GDT_VALID? &m_time : NULL);
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::AdjustRect ()
{
	if (m_pWndDateTime->GetSafeHwnd () == NULL ||
		(m_pWndDateTime->GetStyle () & WS_VISIBLE) == 0 ||
		m_rect.IsRectEmpty ())
	{
		return;
	}

	m_pWndDateTime->GetWindowRect (&m_rect);
	m_pWndDateTime->ScreenToClient (&m_rect);
	m_pWndDateTime->MapWindowPoints (m_pWndDateTime->GetParent (), &m_rect);
	m_rect.InflateRect (iHorzMargin, iVertMargin);
}
//**************************************************************************************

#pragma warning (disable : 4310)

BOOL CBCGToolbarDateTimeCtrl::NotifyCommand (int iNotifyCode)
{
	if (m_pWndDateTime->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	switch (iNotifyCode)
	{
	case LOWORD(DTN_DATETIMECHANGE):
	case DTN_DATETIMECHANGE:
		{
			m_dwTimeStatus = m_pWndDateTime->GetTime (m_time);

			//------------------------------------------------------
			// Try set selection in ALL DateTimeCtrl's with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
				{
					CBCGToolbarDateTimeCtrl* pDateTime = 
						DYNAMIC_DOWNCAST (CBCGToolbarDateTimeCtrl, listButtons.GetNext (posCombo));
					ASSERT (pDateTime != NULL);

					if (pDateTime != this)
					{
						pDateTime->m_pWndDateTime->SetTime (m_dwTimeStatus == GDT_VALID? &m_time : NULL);
					}
				}
			}
		}

		return TRUE;
	}

	return TRUE;
}

#pragma warning (default : 4310)

//**************************************************************************************
void CBCGToolbarDateTimeCtrl::OnAddToCustomizePage ()
{
	CObList listButtons;	// Existing buttons with the same command ID

	if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) == 0)
	{
		return;
	}

	CBCGToolbarDateTimeCtrl* pOther = 
		(CBCGToolbarDateTimeCtrl*) listButtons.GetHead ();
	ASSERT_VALID (pOther);
	ASSERT_KINDOF (CBCGToolbarDateTimeCtrl, pOther);

	CopyFrom (*pOther);
}
//**************************************************************************************
HBRUSH CBCGToolbarDateTimeCtrl::OnCtlColor (CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor (::GetSysColor (COLOR_WINDOWTEXT));
	pDC->SetBkColor (::GetSysColor (COLOR_WINDOW));

	return ::GetSysColorBrush (COLOR_WINDOW);
}
//**************************************************************************************
void CBCGToolbarDateTimeCtrl::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight,
						BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndDateTime->GetSafeHwnd () == NULL ||
		(m_pWndDateTime->GetStyle () & WS_VISIBLE) == 0)
	{
		CBCGToolbarButton::OnDraw (pDC, rect, pImages,
							bHorz, bCustomizeMode,
							bHighlight, bDrawBorder, bGrayDisabledButtons);
	}
	else if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		//-----------------------------------
		// Draw button's text - Guy Hachlili:
		//-----------------------------------
		BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
			(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));
		
		pDC->SetTextColor (bDisabled ?
			globalData.clrGrayedText : 
				(bHighlight) ?	CBCGToolBar::GetHotTextColor () :
								globalData.clrBarText);
		CRect rectText;
		rectText.left = (rect.left + rect.right - m_sizeText.cx) / 2;
		rectText.right = (rect.left + rect.right + m_sizeText.cx) / 2;
		rectText.top = rect.bottom + rect.top;
		rectText.bottom = rectText.top + m_sizeText.cy;
		pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}
//**************************************************************************************
BOOL CBCGToolbarDateTimeCtrl::OnClick (CWnd* /*pWnd*/, BOOL /*bDelay*/)
{	
	return m_pWndDateTime->GetSafeHwnd () != NULL &&
			(m_pWndDateTime->GetStyle () & WS_VISIBLE);
}
//******************************************************************************************
int CBCGToolbarDateTimeCtrl::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CBCGToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected);

	//------------------------------
	// Simulate DateTimeCtrl appearance:
	//------------------------------
	CRect rectDateTime = rect;
	int iDateTimeWidth = rect.Width () - iWidth;

	if (iDateTimeWidth < 20)
	{
		iDateTimeWidth = 20;
	}

	rectDateTime.left = rectDateTime.right - iDateTimeWidth;
	rectDateTime.DeflateRect (2, 3);

	pDC->FillSolidRect (rectDateTime, ::GetSysColor (COLOR_WINDOW));
	pDC->Draw3dRect (&rectDateTime,
		globalData.clrBarDkShadow,
		globalData.clrBarHilite);

	rectDateTime.DeflateRect (1, 1);

	pDC->Draw3dRect (&rectDateTime,
		globalData.clrBarShadow,
		globalData.clrBarLight);

	CRect rectBtn = rectDateTime;
	rectBtn.left = rectBtn.right - rectBtn.Height ();
	rectBtn.DeflateRect (1, 1);

	pDC->FillSolidRect (rectBtn, globalData.clrBarFace);
	pDC->Draw3dRect (&rectBtn,
		globalData.clrBarHilite,
		globalData.clrBarDkShadow);

	CMenuImages::Draw (pDC, CMenuImages::IdArowDown, rectBtn);
	return rect.Width ();
}
//********************************************************************************************
CBCGDateTimeCtrlWin* CBCGToolbarDateTimeCtrl::CreateDateTimeCtrl (CWnd* pWndParent, const CRect& rect)
{
	CBCGDateTimeCtrlWin* pWndDateTime = new CBCGDateTimeCtrlWin;
	if (!pWndDateTime->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndDateTime;
		return NULL;
	}

	return pWndDateTime;
}
//****************************************************************************************
void CBCGToolbarDateTimeCtrl::OnShow (BOOL bShow)
{
	if (m_pWndDateTime->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndDateTime->ShowWindow (SW_SHOWNOACTIVATE);
			OnMove ();
		}
		else
		{
			m_pWndDateTime->ShowWindow (SW_HIDE);
		}
	}
}
//*************************************************************************************
BOOL CBCGToolbarDateTimeCtrl::ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const
{
	CString strMessage;
	int iOffset;

	if (strMessage.LoadString (m_nID) &&
		(iOffset = strMessage.Find (_T('\n'))) != -1)
	{
		menuButton.m_strText = strMessage.Mid (iOffset + 1);
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTime (LPSYSTEMTIME pTimeNew /* = NULL */)
{
	BOOL bResult = m_pWndDateTime->SetTime (pTimeNew);
	NotifyCommand (DTN_DATETIMECHANGE);
	return bResult;
}
//*************************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTime (const COleDateTime& timeNew)
{
	BOOL bResult = m_pWndDateTime->SetTime (timeNew);
	NotifyCommand (DTN_DATETIMECHANGE);
	return bResult;
}
//*************************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTime (const CTime* pTimeNew)
{
	BOOL bResult = m_pWndDateTime->SetTime (pTimeNew);
	NotifyCommand (DTN_DATETIMECHANGE);
	return bResult;
}
//*********************************************************************************
CBCGToolbarDateTimeCtrl* CBCGToolbarDateTimeCtrl::GetByCmd (UINT uiCmd)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = NULL;

	CObList listButtons;
	if (CBCGToolBar::GetCommandButtons(uiCmd, listButtons) > 0)
	{
		for (POSITION posDateTime= listButtons.GetHeadPosition (); pSrcDateTime == NULL && posDateTime != NULL;)
		{
			CBCGToolbarDateTimeCtrl* pDateTime= DYNAMIC_DOWNCAST(CBCGToolbarDateTimeCtrl, listButtons.GetNext(posDateTime));
			ASSERT (pDateTime != NULL);

			pSrcDateTime = pDateTime;
		}
	}

	return pSrcDateTime;
}
//*********************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTimeAll (UINT uiCmd, LPSYSTEMTIME pTimeNew /* = NULL */)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime (pTimeNew);
	}

	return pSrcDateTime != NULL;
}
//*********************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTimeAll (UINT uiCmd, const COleDateTime& timeNew)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime (timeNew);
	}

	return pSrcDateTime != NULL;
}
//*********************************************************************************
BOOL CBCGToolbarDateTimeCtrl::SetTimeAll (UINT uiCmd, const CTime* pTimeNew)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime (pTimeNew);
	}

	return pSrcDateTime != NULL;
}
//*********************************************************************************
DWORD CBCGToolbarDateTimeCtrl::GetTimeAll (UINT uiCmd, LPSYSTEMTIME pTimeDest)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime (pTimeDest);
	}
	else
		return GDT_NONE;
}
//*********************************************************************************
BOOL CBCGToolbarDateTimeCtrl::GetTimeAll (UINT uiCmd, COleDateTime& timeDest)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime (timeDest);
	}
	else
		return FALSE;
}
//*********************************************************************************
DWORD CBCGToolbarDateTimeCtrl::GetTimeAll (UINT uiCmd, CTime& timeDest)
{
	CBCGToolbarDateTimeCtrl* pSrcDateTime = GetByCmd (uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime (timeDest);
	}
	else
		return GDT_NONE;
}
//*********************************************************************************
void CBCGToolbarDateTimeCtrl::SetStyle (UINT nStyle)
{
	CBCGToolbarButton::SetStyle (nStyle);

	if (m_pWndDateTime != NULL && m_pWndDateTime->GetSafeHwnd () != NULL)
	{
		BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () && !IsEditable ()) ||
			(!CBCGToolBar::IsCustomizeMode () && (m_nStyle & TBBS_DISABLED));

		m_pWndDateTime->EnableWindow (!bDisabled);
	}
}
//*****************************************************************************************
void CBCGToolbarDateTimeCtrl::OnGlobalFontsChanged()
{
	CBCGToolbarButton::OnGlobalFontsChanged ();

	if (m_pWndDateTime->GetSafeHwnd () != NULL)
	{
		m_pWndDateTime->SetFont (&globalData.fontRegular);
	}
}
