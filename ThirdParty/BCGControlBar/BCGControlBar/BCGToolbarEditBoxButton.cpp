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

// BCGToolbarEditBoxButton.cpp: implementation of the CBCGToolbarEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGToolbar.h"
#include "globals.h"
#include "BCGToolbarEditBoxButton.h"
#include "BCGVisualManager.h"
#include "trackmouse.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarEditCtrl

class CBCGToolbarEditCtrl : public CEdit
{
// Construction
public:
	CBCGToolbarEditCtrl(CBCGToolbarEditBoxButton& edit);

// Attributes
protected:
	CBCGToolbarEditBoxButton&	m_buttonEdit;
	BOOL						m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGToolbarEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGToolbarEditCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGToolbarEditCtrl)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_SERIAL(CBCGToolbarEditBoxButton, CBCGToolbarButton, 1)

static const int iDefaultSize = 150;
static const int iHorzMargin = 3;
static const int iVertMargin = 1;

BOOL CBCGToolbarEditBoxButton::m_bFlat = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarEditBoxButton::CBCGToolbarEditBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	m_iWidth = iDefaultSize;

	Initialize ();
}
//**************************************************************************************
CBCGToolbarEditBoxButton::CBCGToolbarEditBoxButton (UINT uiId,
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
void CBCGToolbarEditBoxButton::Initialize ()
{
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_bChangingText = FALSE;
	m_bIsHotEdit = FALSE;
}
//**************************************************************************************
CBCGToolbarEditBoxButton::~CBCGToolbarEditBoxButton()
{
	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
	}
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);

	const CBCGToolbarEditBoxButton& src = (const CBCGToolbarEditBoxButton&) s;

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_strContents = src.m_strContents;
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::Serialize (CArchive& ar)
{
	CBCGToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_strContents;
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->GetWindowText(m_strContents);
		}
		else
		{
			m_strContents.Empty();
		}

		ar << m_strContents;
	}
}
//**************************************************************************************
SIZE CBCGToolbarEditBoxButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (!IsVisible())
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		OnShowEditbox (FALSE);
		return CSize (0,0);
	}

	m_bHorz = bHorz;

	if (bHorz)
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL && !m_bIsHidden)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
			OnShowEditbox (TRUE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText (0, 0, 
				m_iWidth, sizeDefault.cy);
			pDC->DrawText (	m_strText, rectText, 
							DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size ();
		}
		else
			m_sizeText = CSize(0,0);

		return CSize (m_iWidth, sizeDefault.cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
			OnShowEditbox (FALSE);
		}

		m_sizeText = CSize(0,0);

		return CBCGToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);
	}
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::OnMove ()
{
	if (m_pWndEdit->GetSafeHwnd () == NULL ||
		(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0)
	{
		return;
	}

	int cy = globalData.GetTextHeight();
	int yOffset = max (0, (m_rect.Height () - m_sizeText.cy - cy) / 2);

	m_pWndEdit->SetWindowPos (NULL, 
		m_rect.left + iHorzMargin, 
		m_rect.top + yOffset,
		m_rect.Width () - 2 * iHorzMargin, 
		cy,
		SWP_NOZORDER | SWP_NOACTIVATE);

	m_pWndEdit->SetSel(-1, 0);
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::OnSize (int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	OnMove ();
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		CWnd* pWndParentCurr = m_pWndEdit->GetParent ();
		ASSERT (pWndParentCurr != NULL);

		if (pWndParent != NULL &&
			pWndParentCurr->GetSafeHwnd () == pWndParent->GetSafeHwnd ())
		{
			return;
		}
		
		m_pWndEdit->GetWindowText(m_strContents);

		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rect = m_rect;
	rect.DeflateRect (iHorzMargin, iVertMargin);
	rect.bottom = rect.top + globalData.GetTextHeight ();

	if ((m_pWndEdit = CreateEdit (pWndParent, rect)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pWndEdit);

	OnMove ();
	m_pWndEdit->SetFont (&globalData.fontRegular);

	CString sText;
	m_pWndEdit->GetWindowText(sText);
	if (sText.IsEmpty())
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
	else
	{
		m_strContents = sText;
	}
}
//**************************************************************************************
BOOL CBCGToolbarEditBoxButton::NotifyCommand (int iNotifyCode)
{
	if (m_pWndEdit->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	switch (iNotifyCode)
	{
		case EN_UPDATE:
		{
			m_pWndEdit->GetWindowText(m_strContents);

			//------------------------------------------------------
			// Try set selection in ALL editboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
				{
					CBCGToolbarEditBoxButton* pEdit = 
						DYNAMIC_DOWNCAST (CBCGToolbarEditBoxButton, listButtons.GetNext (posCombo));

					if ((pEdit != NULL) && (pEdit != this))
					{
						pEdit->SetContents(m_strContents);
					}
				}
			}
		}

		return !m_bChangingText;
	}

	return FALSE;
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::OnAddToCustomizePage ()
{
	CObList listButtons;	// Existing buttons with the same command ID

	if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) == 0)
	{
		return;
	}

	CBCGToolbarEditBoxButton* pOther = 
		(CBCGToolbarEditBoxButton*) listButtons.GetHead ();
	ASSERT_VALID (pOther);
	ASSERT_KINDOF (CBCGToolbarEditBoxButton, pOther);

	CopyFrom (*pOther);
}
//**************************************************************************************
HBRUSH CBCGToolbarEditBoxButton::OnCtlColor (CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor (::GetSysColor (COLOR_WINDOWTEXT));
	pDC->SetBkColor (::GetSysColor (COLOR_WINDOW));

	return ::GetSysColorBrush (COLOR_WINDOW);
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight, BOOL bDrawBorder, 
						BOOL bGrayDisabledButtons)
{
	if (m_pWndEdit->GetSafeHwnd () == NULL ||
		(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0)
	{
		CBCGToolbarButton::OnDraw (pDC, rect, pImages,
							bHorz, bCustomizeMode,
							bHighlight, bDrawBorder, 
							bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

	CRect rectBorder;
	GetEditBorder (rectBorder);

	CBCGVisualManager::GetInstance ()->OnDrawEditBorder (
		pDC, rectBorder, bDisabled, !m_bFlat || m_bIsHotEdit, this);

	if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		//--------------------
		// Draw button's text:
		//--------------------
		BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
			(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

		pDC->SetTextColor (bDisabled ?
							globalData.clrGrayedText : 
								(bHighlight) ? CBCGToolBar::GetHotTextColor () :
										globalData.clrBarText);
		CRect rectText = rect;
		rectText.top = (rectBorder.bottom + rect.bottom - m_sizeText.cy) / 2;
		pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::GetEditBorder (CRect& rectBorder)
{
	ASSERT (m_pWndEdit->GetSafeHwnd () != NULL);

	m_pWndEdit->GetWindowRect (rectBorder);
	m_pWndEdit->GetParent ()->ScreenToClient (rectBorder);
	rectBorder.InflateRect (1, 1);
}
//***************************************************************************************
BOOL CBCGToolbarEditBoxButton::OnClick (CWnd* /*pWnd*/, BOOL /*bDelay*/)
{	
	return m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetStyle () & WS_VISIBLE);
}
//**************************************************************************************
int CBCGToolbarEditBoxButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CBCGToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected) + 10;

	//------------------------------
	// Simulate editbox appearance:
	//------------------------------
	CRect rectEdit = rect;
	int nEditWidth = max (8, rect.Width () - iWidth);

	rectEdit.left = rectEdit.right - nEditWidth;
	rectEdit.DeflateRect (2, 2);

	pDC->FillRect (rectEdit, &globalData.brWindow);
	pDC->Draw3dRect (rectEdit, globalData.clrBarShadow, globalData.clrBarShadow);

	return rect.Width ();
}
//********************************************************************************************
CEdit* CBCGToolbarEditBoxButton::CreateEdit (CWnd* pWndParent, const CRect& rect)
{
	ASSERT_VALID (this);

	CEdit* pWndEdit = new CBCGToolbarEditCtrl (*this);
	if (!pWndEdit->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}
	return pWndEdit;
}
//****************************************************************************************
void CBCGToolbarEditBoxButton::OnShow (BOOL bShow)
{
	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
			OnMove ();
		}
		else
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		OnShowEditbox (bShow);
	}
}
//*********************************************************************************
void CBCGToolbarEditBoxButton::SetContents (const CString& sContents)
{
	if (m_strContents == sContents)
		return;

	m_strContents = sContents;
	if (m_pWndEdit != NULL)
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
}
//*********************************************************************************
const CRect CBCGToolbarEditBoxButton::GetInvalidateRect () const
{
	if ((m_bTextBelow && m_bHorz) && !m_strText.IsEmpty())
	{
		CRect rect;
		rect.left = (m_rect.left + m_rect.right - m_sizeText.cx) / 2;
		rect.right = (m_rect.left + m_rect.right + m_sizeText.cx) / 2;
		rect.top = m_rect.top;
		rect.bottom = m_rect.bottom + m_rect.top + m_sizeText.cy;
		return rect;
	}
	else
		return m_rect;
}
//*********************************************************************************
CBCGToolbarEditBoxButton* CBCGToolbarEditBoxButton::GetByCmd (UINT uiCmd)
{
	CBCGToolbarEditBoxButton* pSrcEdit = NULL;

	CObList listButtons;
	if (CBCGToolBar::GetCommandButtons (uiCmd, listButtons) > 0)
	{
		for (POSITION posEdit= listButtons.GetHeadPosition (); pSrcEdit == NULL && posEdit != NULL;)
		{
			CBCGToolbarEditBoxButton* pEdit= DYNAMIC_DOWNCAST (CBCGToolbarEditBoxButton, listButtons.GetNext (posEdit));
			ASSERT (pEdit != NULL);

			pSrcEdit = pEdit;
		}
	}

	return pSrcEdit;
}
//*********************************************************************************
BOOL CBCGToolbarEditBoxButton::SetContentsAll (UINT uiCmd, const CString& strContents)
{
	CBCGToolbarEditBoxButton* pSrcEdit = GetByCmd (uiCmd);

	if (pSrcEdit)
	{
		pSrcEdit->SetContents (strContents);
	}

	return pSrcEdit != NULL;
}
//*********************************************************************************
CString CBCGToolbarEditBoxButton::GetContentsAll (UINT uiCmd)
{
	CBCGToolbarEditBoxButton* pSrcEdit = GetByCmd (uiCmd);
	CString str;

	if (pSrcEdit)
	{
		pSrcEdit->m_pWndEdit->GetWindowText (str);
	}

	return str;
}
//*********************************************************************************
void CBCGToolbarEditBoxButton::SetStyle (UINT nStyle)
{
	CBCGToolbarButton::SetStyle (nStyle);

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd () != NULL)
	{
		BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () && !IsEditable ()) ||
			(!CBCGToolBar::IsCustomizeMode () && (m_nStyle & TBBS_DISABLED));

		m_pWndEdit->EnableWindow (!bDisabled);
	}
}
//**************************************************************************************
void CBCGToolbarEditBoxButton::SetHotEdit (BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndEdit->GetParent () != NULL)
		{
			CRect rect = m_rect;

			m_pWndEdit->GetParent ()->InvalidateRect (m_rect);
			m_pWndEdit->GetParent ()->UpdateWindow ();
		}
	}
}
//*****************************************************************************************
void CBCGToolbarEditBoxButton::OnGlobalFontsChanged()
{
	CBCGToolbarButton::OnGlobalFontsChanged ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetFont (&globalData.fontRegular);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarEditCtrl

CBCGToolbarEditCtrl::CBCGToolbarEditCtrl(CBCGToolbarEditBoxButton& edit) :
	m_buttonEdit (edit)
{
	m_bTracked = FALSE;
}

CBCGToolbarEditCtrl::~CBCGToolbarEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGToolbarEditCtrl, CEdit)
	//{{AFX_MSG_MAP(CBCGToolbarEditCtrl)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarEditCtrl message handlers

BOOL CBCGToolbarEditCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_TAB:
			if (GetParent () != NULL)
			{
				ASSERT_VALID (GetParent ());
				GetParent ()->GetNextDlgTabItem (this)->SetFocus ();
				return TRUE;
			}
			break;

		case VK_ESCAPE:
			if (GetTopLevelFrame () != NULL)
			{
				GetTopLevelFrame ()->SetFocus ();
				return TRUE;
			}
			
			break;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}
//*************************************************************************************
void CBCGToolbarEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	m_buttonEdit.SetHotEdit (TRUE);
}
//*************************************************************************************
void CBCGToolbarEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	m_buttonEdit.SetHotEdit (FALSE);
}
//*************************************************************************************
void CBCGToolbarEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CEdit::OnMouseMove(nFlags, point);
	m_buttonEdit.SetHotEdit (TRUE);

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGTrackMouse (&trackmouseevent);	
	}
}
//*****************************************************************************************
LRESULT CBCGToolbarEditCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus () != this)
	{
		m_buttonEdit.SetHotEdit (FALSE);
	}

	return 0;
}
