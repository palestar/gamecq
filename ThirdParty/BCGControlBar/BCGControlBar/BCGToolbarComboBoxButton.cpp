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

// BCGToolbarComboBoxButton.cpp: implementation of the CBCGToolbarComboBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGToolbar.h"
#include "globals.h"
#include "BCGToolbarComboBoxButton.h"
#include "BCGToolbarMenuButton.h"
#include "MenuImages.h"
#include "BCGWorkspace.h"
#include "trackmouse.h"
#include "BCGVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGToolbarComboBoxButton, CBCGToolbarButton, 1)

static const int iDefaultComboHeight = 150;
static const int iDefaultSize = 150;
static const int iHorzMargin = 1;

extern CBCGWorkspace* g_pWorkspace;

BOOL CBCGToolbarComboBoxButton::m_bFlat = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarComboBoxButton::CBCGToolbarComboBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL;
	m_iWidth = iDefaultSize;

	Initialize ();
}
//**************************************************************************************
CBCGToolbarComboBoxButton::CBCGToolbarComboBoxButton (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGToolbarButton (uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE | WS_VSCROLL;
	m_iWidth = (iWidth == 0) ? iDefaultSize : iWidth;

	Initialize ();
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::Initialize ()
{
	m_iSelIndex = -1;
	m_pWndCombo = NULL;
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_rectCombo.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();
	m_nDropDownHeight = iDefaultComboHeight;
	m_bIsHotEdit = FALSE;
	m_bSerializeContent = TRUE;
}
//**************************************************************************************
CBCGToolbarComboBoxButton::~CBCGToolbarComboBoxButton()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
	}

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarButton::CopyFrom (s);
	POSITION pos;

	m_lstItems.RemoveAll ();

	const CBCGToolbarComboBoxButton& src = (const CBCGToolbarComboBoxButton&) s;
	for (pos = src.m_lstItems.GetHeadPosition (); pos != NULL;)
	{
		m_lstItems.AddTail (src.m_lstItems.GetNext (pos));
	}

	ClearData ();

	m_lstItemData.RemoveAll ();
	for (pos = src.m_lstItemData.GetHeadPosition (); pos != NULL;)
	{
		m_lstItemData.AddTail (src.m_lstItemData.GetNext (pos));
	}

	DuplicateData ();
	ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_iSelIndex = src.m_iSelIndex;
	m_nDropDownHeight = src.m_nDropDownHeight;
	m_bSerializeContent = src.m_bSerializeContent;
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::Serialize (CArchive& ar)
{
	CBCGToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_iSelIndex;
		ar >> m_strEdit;

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x40720)
		{
			ar >> m_nDropDownHeight;
		}

		SerializeContent (ar);
		SelectItem (m_iSelIndex);
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_iSelIndex;
		ar << m_strEdit;
		ar << m_nDropDownHeight;

		if (m_pWndCombo != NULL)
		{
			m_lstItems.RemoveAll ();
			ClearData ();
			m_lstItemData.RemoveAll ();

			for (int i = 0; i < m_pWndCombo->GetCount (); i ++)
			{
				CString str;
				m_pWndCombo->GetLBText (i, str);

				m_lstItems.AddTail (str);
				m_lstItemData.AddTail (m_pWndCombo->GetItemData (i));
			}
		}

		SerializeContent (ar);
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::SerializeContent (CArchive& ar)
{
	ASSERT_VALID (this);

	if (!m_bSerializeContent)
	{
		return;
	}

	m_lstItems.Serialize (ar);

	if (ar.IsLoading ())
	{
		ClearData ();
		m_lstItemData.RemoveAll ();

		for (int i = 0; i < m_lstItems.GetCount (); i ++)
		{
			long lData;
			ar >> lData;
			m_lstItemData.AddTail ((DWORD) lData);
		}

		DuplicateData ();
	}
	else
	{
		for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL;)
		{
			DWORD_PTR dwData = m_lstItemData.GetNext (pos);
			ar << (long) dwData;
		}
	}

	ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());
}
//***************************************************************************************
SIZE CBCGToolbarComboBoxButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;
	m_sizeText = CSize (0, 0);

	if (!IsVisible())
	{
	
		if (m_bFlat)
		{
			if (m_pWndEdit->GetSafeHwnd () != NULL &&
				(m_pWndEdit->GetStyle () & WS_VISIBLE))
			{
				m_pWndEdit->ShowWindow (SW_HIDE);
			}
		}
				
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}

		return CSize(0,0);
	}

	if (m_bFlat &&
		m_pWndCombo->GetSafeHwnd () != NULL &&
		(m_pWndCombo->GetStyle () & WS_VISIBLE))
	{
		m_pWndCombo->ShowWindow (SW_HIDE);
	}

	if (bHorz)
	{
		if (!m_bFlat && m_pWndCombo->GetSafeHwnd () != NULL && !m_bIsHidden)
		{
			m_pWndCombo->ShowWindow (SW_SHOWNOACTIVATE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText (0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText (m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size ();
		}

		int cy = sizeDefault.cy;

		if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd () != NULL)
		{
			CRect rectCombo;
			m_pWndCombo->GetWindowRect (&rectCombo);

			cy = rectCombo.Height ();
		}

		if (!m_bIsHidden && m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE) == 0)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
		}

		return CSize (m_iWidth, cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetStyle () & WS_VISIBLE))
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		return CBCGToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::OnMove ()
{
	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		AdjustRect ();
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::OnSize (int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		AdjustRect ();
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		CWnd* pWndParentCurr = m_pWndCombo->GetParent ();
		ASSERT (pWndParentCurr != NULL);

		if (pWndParent != NULL &&
			pWndParentCurr->GetSafeHwnd () == pWndParent->GetSafeHwnd ())
		{
			return;
		}

		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
		m_pWndCombo = NULL;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->DestroyWindow ();
			delete m_pWndEdit;
			m_pWndEdit = NULL;
		}
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () || (m_nStyle & TBBS_DISABLED));

	CRect rect = m_rect;
	rect.InflateRect (-2, 0);
	rect.bottom = rect.top + m_nDropDownHeight;

	if ((m_pWndCombo = CreateCombo (pWndParent, rect)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	m_pWndCombo->EnableWindow (!bDisabled);

	if (m_bFlat && (m_pWndCombo->GetStyle () & CBS_DROPDOWNLIST) == CBS_DROPDOWN)
	{
		DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_AUTOHSCROLL;
		if (m_pWndCombo->GetStyle () & WS_TABSTOP)
		{
			dwEditStyle |= WS_TABSTOP;
		}
		
		if ((m_pWndEdit = CreateEdit (pWndParent, rect, dwEditStyle)) == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		m_pWndEdit->SetFont (&globalData.fontRegular);
		m_pWndEdit->SetOwner (m_pWndCombo->GetParent ()->GetOwner ());

		m_pWndEdit->EnableWindow (!bDisabled);
	}

	AdjustRect ();

	m_pWndCombo->SetFont (&globalData.fontRegular);

	if (m_pWndCombo->GetCount () > 0)
	{
		m_lstItems.RemoveAll ();
		
		ClearData ();
		m_lstItemData.RemoveAll ();

		for (int i = 0; i < m_pWndCombo->GetCount (); i ++)
		{
			CString str;
			m_pWndCombo->GetLBText (i, str);

			m_lstItems.AddTail (str);
			m_lstItemData.AddTail (m_pWndCombo->GetItemData (i));
		}

		m_iSelIndex = m_pWndCombo->GetCurSel ();
	}
	else
	{
		m_pWndCombo->ResetContent ();
		ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

		POSITION posData = m_lstItemData.GetHeadPosition ();
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL;)
		{
			ASSERT (posData != NULL);

			CString strItem = m_lstItems.GetNext (pos);
			int iIndex = m_pWndCombo->AddString (strItem);
			
			m_pWndCombo->SetItemData (iIndex, m_lstItemData.GetNext (posData));
		}

		if (m_iSelIndex != CB_ERR)
		{
			m_pWndCombo->SetCurSel (m_iSelIndex);
		}
	}

	if (m_iSelIndex != CB_ERR &&
		m_iSelIndex < m_pWndCombo->GetCount ())
	{
		m_pWndCombo->GetLBText (m_iSelIndex, m_strEdit);
		m_pWndCombo->SetWindowText (m_strEdit);

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}
}
//**************************************************************************************
int CBCGToolbarComboBoxButton::AddItem (LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT (lpszItem != NULL);

	if (m_strEdit.IsEmpty ())
	{
		m_strEdit = lpszItem;
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}

	if (m_lstItems.Find (lpszItem) == NULL)
	{
		m_lstItems.AddTail (lpszItem);
		m_lstItemData.AddTail (dwData);
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		int iIndex = m_pWndCombo->FindStringExact (-1, lpszItem);

		if (iIndex == CB_ERR)
		{
			iIndex = m_pWndCombo->AddString (lpszItem);
		}

		m_pWndCombo->SetCurSel (iIndex);
		m_pWndCombo->SetItemData (iIndex, dwData);
		m_pWndCombo->SetEditSel (-1, 0);
	}

	return (int) m_lstItems.GetCount () - 1;
}
//**************************************************************************************
LPCTSTR CBCGToolbarComboBoxButton::GetItem (int iIndex) const
{
	if (iIndex == -1)	// Current selection
	{
		if (m_pWndCombo->GetSafeHwnd () == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel ();
		}
	}

	POSITION pos = m_lstItems.FindIndex (iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstItems.GetAt (pos);
}
//**************************************************************************************
DWORD_PTR CBCGToolbarComboBoxButton::GetItemData (int iIndex) const
{
	if (iIndex == -1)	// Current selection
	{
		if (m_pWndCombo->GetSafeHwnd () == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel ();
		}
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		return m_pWndCombo->GetItemData (iIndex);
	}
	else
	{
		POSITION pos = m_lstItemData.FindIndex (iIndex);
		if (pos == NULL)
		{
			return 0;
		}

		return m_lstItemData.GetAt (pos);
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::RemoveAllItems ()
{
	m_lstItems.RemoveAll ();
	
	ClearData ();
	m_lstItemData.RemoveAll ();

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->ResetContent ();
	}

	m_strEdit.Empty ();
	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->SetWindowText (m_strEdit);
	}
}
//**************************************************************************************
int CBCGToolbarComboBoxButton::GetCount () const
{
	return (int) m_lstItems.GetCount ();
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::AdjustRect ()
{
	ASSERT_VALID (this);

	if (m_pWndCombo->GetSafeHwnd () == NULL ||
		m_rect.IsRectEmpty () || !m_bHorz)
	{
		m_rectCombo.SetRectEmpty ();
		m_rectButton.SetRectEmpty ();
		return;
	}

	CSize sizeExtra = m_bExtraSize ? 
		CBCGVisualManager::GetInstance ()->GetButtonExtraBorder () : CSize (0, 0);

	CRect rectInternal = m_rect;
	rectInternal.DeflateRect (sizeExtra.cx / 2, sizeExtra.cy / 2);

	m_pWndCombo->SetWindowPos (NULL,
		rectInternal.left + iHorzMargin, rectInternal.top,
		rectInternal.Width () - 2 * iHorzMargin, m_nDropDownHeight,
		SWP_NOZORDER | SWP_NOACTIVATE);
	m_pWndCombo->SetEditSel (-1, 0);

	m_pWndCombo->GetWindowRect (&m_rectCombo);
	m_pWndCombo->ScreenToClient (&m_rectCombo);
	m_pWndCombo->MapWindowPoints (m_pWndCombo->GetParent (), &m_rectCombo);

	if (m_bFlat)
	{
		m_rectButton = m_rectCombo;
		m_rectButton.left = m_rectButton.right - CMenuImages::Size ().cx * 2;

		m_rectButton.DeflateRect (2, 2);

		m_rect.left = m_rectCombo.left - iHorzMargin;
		m_rect.right = m_rectCombo.right + iHorzMargin;

		if (!m_bTextBelow || m_strText.IsEmpty ())
		{
			m_rect.top = m_rectCombo.top - sizeExtra.cy / 2;
			m_rect.bottom = m_rectCombo.bottom + sizeExtra.cy / 2;
		}

		if (m_pWndEdit != NULL)
		{
			CRect rectEdit = m_rect;

			const int iBorderOffset = 3;

			m_pWndEdit->SetWindowPos (NULL,
				m_rect.left + iHorzMargin + iBorderOffset,
				m_rect.top + iBorderOffset + sizeExtra.cy / 2,
				m_rect.Width () - 2 * iHorzMargin - m_rectButton.Width () - 
					iBorderOffset - 3,
				m_rectCombo.Height () - 2 * iBorderOffset,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	else
	{
		m_rectButton.SetRectEmpty ();
	}
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::SetHotEdit (BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndCombo->GetParent () != NULL)
		{
			m_pWndCombo->GetParent ()->InvalidateRect (m_rectCombo);
			m_pWndCombo->GetParent ()->UpdateWindow ();
		}
	}
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::NotifyCommand (int iNotifyCode)
{
	if (m_pWndCombo->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (m_bFlat && iNotifyCode == 0)
	{
		return TRUE;
	}

	if (m_bFlat && m_pWndCombo->GetParent () != NULL)
	{
		m_pWndCombo->GetParent ()->InvalidateRect (m_rectCombo);
		m_pWndCombo->GetParent ()->UpdateWindow ();
	}

	switch (iNotifyCode)
	{
	case CBN_SELENDOK:
		{
			m_iSelIndex = m_pWndCombo->GetCurSel ();
			if (m_iSelIndex < 0)
			{
				return FALSE;
			}

			m_pWndCombo->GetLBText (m_iSelIndex, m_strEdit);
			if (m_pWndEdit != NULL)
			{
				m_pWndEdit->SetWindowText (m_strEdit);
			}

			//------------------------------------------------------
			// Try set selection in ALL comboboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
				{
					CBCGToolbarComboBoxButton* pCombo = 
						DYNAMIC_DOWNCAST (CBCGToolbarComboBoxButton, listButtons.GetNext (posCombo));

					if (pCombo != NULL && pCombo != this && pCombo->IsAutoSynchSelection ())
					{
						pCombo->SelectItem (m_pWndCombo->GetCurSel (), FALSE /* Don't notify */);
					}
				}
			}
		}

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetFocus ();
		}

		return TRUE;

	case CBN_KILLFOCUS:
	case CBN_EDITUPDATE:
		return TRUE;

	case CBN_SELCHANGE:
		if (m_pWndEdit != NULL)
		{
			int iSel = m_pWndCombo->GetCurSel ();
			if (iSel >= 0)
			{
				CString strEdit;
				m_pWndCombo->GetLBText (iSel, strEdit);

				m_pWndEdit->SetWindowText (strEdit);
			}
		}

		return TRUE;

	case CBN_EDITCHANGE:
		{
			m_pWndCombo->GetWindowText (m_strEdit);

			//------------------------------------------------------
			// Try set text of ALL comboboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo !=
					NULL;)
				{
					CBCGToolbarComboBoxButton* pCombo = 
						DYNAMIC_DOWNCAST (CBCGToolbarComboBoxButton, listButtons.GetNext
						(posCombo));
					ASSERT (pCombo != NULL);
					
					if (pCombo != this)
					{
						if (pCombo->GetComboBox () != NULL)
						{
							pCombo->GetComboBox ()->SetWindowText(m_strEdit);
						}

						pCombo->m_strEdit = m_strEdit;
					}
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::OnAddToCustomizePage ()
{
	CObList listButtons;	// Existing buttons with the same command ID

	if (CBCGToolBar::GetCommandButtons (m_nID, listButtons) == 0)
	{
		return;
	}

	CBCGToolbarComboBoxButton* pOther = 
		(CBCGToolbarComboBoxButton*) listButtons.GetHead ();
	ASSERT_VALID (pOther);
	ASSERT_KINDOF (CBCGToolbarComboBoxButton, pOther);

	CopyFrom (*pOther);
}
//**************************************************************************************
HBRUSH CBCGToolbarComboBoxButton::OnCtlColor (CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor (::GetSysColor (COLOR_WINDOWTEXT));
	pDC->SetBkColor (::GetSysColor (COLOR_WINDOW));

	return ::GetSysColorBrush (COLOR_WINDOW);
}
//**************************************************************************************
void CBCGToolbarComboBoxButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight,
						BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd () == NULL || !bHorz)
	{
		CBCGToolbarButton::OnDraw (pDC, rect, pImages,
							bHorz, bCustomizeMode,
							bHighlight, bDrawBorder, bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));
		
	pDC->SetTextColor (bDisabled ?
		globalData.clrGrayedText : 
			(bHighlight) ?	CBCGToolBar::GetHotTextColor () :
							globalData.clrBarText);

	if (m_bFlat)
	{
		if (m_bIsHotEdit)
		{
			bHighlight = TRUE;
		}
		
		//--------------
		// Draw combbox:
		//--------------
		CRect rectCombo = m_rectCombo;

		//-------------
		// Draw border:
		//-------------
		CBCGVisualManager::GetInstance ()->OnDrawComboBorder (
			pDC, rectCombo, bDisabled, m_pWndCombo->GetDroppedState (),
			bHighlight, this);

		rectCombo.DeflateRect (2, 2);

		int nPrevTextColor = pDC->GetTextColor ();

		pDC->FillSolidRect (rectCombo, 
			bDisabled ? globalData.clrBarFace : ::GetSysColor (COLOR_WINDOW));

		if (bDisabled)
		{
			pDC->Draw3dRect (&rectCombo,
				globalData.clrBarHilite,
				globalData.clrBarHilite);
		}

		//-----------------------
		// Draw drop-down button:
		//-----------------------
		CRect rectButton = m_rectButton;
		if (globalData.m_bIsBlackHighContrast)
		{
			rectButton.DeflateRect (1, 1);
		}

		CBCGVisualManager::GetInstance ()->OnDrawComboDropButton (
			pDC, rectButton, bDisabled, m_pWndCombo->GetDroppedState (),
			bHighlight, this);

		pDC->SetTextColor (nPrevTextColor);

		//-----------------
		// Draw combo text:
		//-----------------
		if (!bDisabled)
		{
			CRect rectText = rectCombo;
			rectText.right = m_rectButton.left;
			rectText.DeflateRect (2, 2);

			if (m_pWndEdit == NULL)
			{
				if (m_pWndCombo->GetStyle () & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))
				{
					DRAWITEMSTRUCT dis;
					memset (&dis, 0, sizeof (DRAWITEMSTRUCT));

					dis.hDC = pDC->GetSafeHdc ();
					dis.rcItem = rectText;
					dis.CtlID = m_nID;
					dis.itemID = m_pWndCombo->GetCurSel ();
					dis.hwndItem = m_pWndCombo->GetSafeHwnd ();
					dis.CtlType = ODT_COMBOBOX;
					dis.itemState |= ODS_COMBOBOXEDIT;
					dis.itemData = m_pWndCombo->GetItemData (dis.itemID);

					if (bDisabled)
					{
						dis.itemState |= ODS_DISABLED;
					}

					m_pWndCombo->DrawItem (&dis);
				}
				else
				{
					pDC->DrawText (m_strEdit, rectText, DT_VCENTER | DT_SINGLELINE);
				}
			}
		}
	}

	if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		//-----------------------------------
		// Draw button's text - Guy Hachlili:
		//-----------------------------------
		CRect rectText = rect;
		rectText.top = (m_rectCombo.bottom + rect.bottom - m_sizeText.cy) / 2;
		
		pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::OnClick (CWnd* pWnd, BOOL /*bDelay*/)
{	
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd () == NULL || !m_bHorz)
	{
		return FALSE;
	}

	if (m_bFlat)
	{
		if (m_pWndEdit == NULL)
		{
			m_pWndCombo->SetFocus ();
		}
		else
		{
			m_pWndEdit->SetFocus ();
		}

		m_pWndCombo->ShowDropDown ();

		if (pWnd != NULL)
		{
			pWnd->InvalidateRect (m_rectCombo);
		}
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItem (int iIndex, BOOL bNotify)
{
	if (iIndex < 0 || iIndex >= m_lstItems.GetCount ())
	{
		return FALSE;
	}

	m_iSelIndex = iIndex;

	if (m_pWndCombo->GetSafeHwnd () == NULL)
	{
		return TRUE;
	}

	m_pWndCombo->GetLBText (iIndex, m_strEdit);
	if (m_pWndEdit != NULL)
	{
		CString strEdit;
		m_pWndEdit->GetWindowText (strEdit);

		if (strEdit != m_strEdit)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}
	else if (m_bFlat && m_pWndCombo->GetParent () != NULL)
	{
		m_pWndCombo->GetParent ()->InvalidateRect (m_rectCombo);
		m_pWndCombo->GetParent ()->UpdateWindow ();
	}

	if (m_pWndCombo->GetCurSel () == iIndex)
	{
		// Already selected
		return TRUE;
	}

	if (m_pWndCombo->SetCurSel (iIndex) != CB_ERR)
	{
		if (bNotify)
		{
			NotifyCommand (CBN_SELENDOK);
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItem (DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext (pos) == dwData)
		{
			return SelectItem (iIndex);
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItem (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItems.GetNext (pos) == lpszText)
		{
			return SelectItem (iIndex);
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGToolbarComboBoxButton::DeleteItem (int iIndex)
{
	if (iIndex < 0 || iIndex >= m_lstItems.GetCount ())
	{
		return FALSE;
	}

	POSITION pos = m_lstItems.FindIndex (iIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstItems.RemoveAt (pos);

	pos = m_lstItemData.FindIndex (iIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstItemData.RemoveAt (pos);

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->DeleteString (iIndex);
	}

	if (iIndex == m_iSelIndex)
	{
		int iSelIndex = m_iSelIndex;
		if (iSelIndex >= m_lstItems.GetCount ())
		{
			iSelIndex = (int) m_lstItems.GetCount () - 1;
		}

		SelectItem (iSelIndex, FALSE);
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::DeleteItem (DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext (pos) == dwData)
		{
			return DeleteItem (iIndex);
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGToolbarComboBoxButton::DeleteItem (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItems.GetNext (pos) == lpszText)
		{
			return DeleteItem (iIndex);
		}
	}

	return FALSE;
}
//******************************************************************************************
int CBCGToolbarComboBoxButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = 
		CBCGToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected) + 10;

	//------------------------------
	// Simulate combobox appearance:
	//------------------------------
	CRect rectCombo = rect;
	int nComboWidth = max (20, rect.Width () - iWidth);

	rectCombo.left = rectCombo.right - nComboWidth;

	int nMargin = 1;
	rectCombo.DeflateRect (nMargin, nMargin);

	pDC->FillRect (rectCombo, &globalData.brWindow);

	pDC->Draw3dRect (rectCombo, globalData.clrBarShadow, globalData.clrBarShadow);

	CRect rectBtn = rectCombo;
	rectBtn.left = rectBtn.right - rectBtn.Height () + 2;
	rectBtn.DeflateRect (nMargin, nMargin);

	CBCGVisualManager::GetInstance ()->OnDrawComboDropButton (
		pDC, rectBtn, FALSE, FALSE, FALSE, this);

	return rect.Width ();
}
//********************************************************************************************
CComboBox* CBCGToolbarComboBoxButton::CreateCombo (CWnd* pWndParent, const CRect& rect)
{
	CComboBox* pWndCombo = new CComboBox;
	if (!pWndCombo->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
CBCGComboEdit* CBCGToolbarComboBoxButton::CreateEdit (CWnd* pWndParent, const CRect& rect, DWORD dwEditStyle)
{
	CBCGComboEdit* pWndEdit = new CBCGComboEdit (*this);

	if (!pWndEdit->Create (dwEditStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	return pWndEdit;
}
//***************************************************************************************
void CBCGToolbarComboBoxButton::OnShow (BOOL bShow)
{
	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			OnMove ();
			m_pWndCombo->ShowWindow (m_bFlat ? SW_HIDE : SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}
	}

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}
	}
}
//*************************************************************************************
BOOL CBCGToolbarComboBoxButton::ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const
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
//*********************************************************************************
void CBCGToolbarComboBoxButton::SetDropDownHeight (int nHeight)
{
	if (m_nDropDownHeight == nHeight)
	{
		return;
	}

	m_nDropDownHeight = nHeight;
	OnMove ();
}
//*********************************************************************************
void CBCGToolbarComboBoxButton::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);
	
	if (!SelectItem (lpszText))
	{
		m_strEdit = lpszText;

		if (m_pWndCombo != NULL && !m_bFlat)
		{
			CString strText;
			m_pWndCombo->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndCombo->SetWindowText(lpszText);
				NotifyCommand (CBN_EDITCHANGE);
			}
		}

		if (m_pWndEdit != NULL)
		{
			CString strText;
			m_pWndEdit->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndEdit->SetWindowText (lpszText);
			}
		}
	}
}
//*********************************************************************************
CBCGToolbarComboBoxButton* CBCGToolbarComboBoxButton::GetByCmd (UINT uiCmd,
																BOOL bIsFocus)
{
	CBCGToolbarComboBoxButton* pSrcCombo = NULL;

	CObList listButtons;
	if (CBCGToolBar::GetCommandButtons (uiCmd, listButtons) > 0)
	{
		for (POSITION posCombo= listButtons.GetHeadPosition (); posCombo != NULL;)
		{
			CBCGToolbarComboBoxButton* pCombo = DYNAMIC_DOWNCAST (CBCGToolbarComboBoxButton, listButtons.GetNext (posCombo));
			ASSERT (pCombo != NULL);

			if (pCombo != NULL && (!bIsFocus || pCombo->HasFocus ()))
			{
				pSrcCombo = pCombo;
				break;
			}
		}
	}

	return pSrcCombo;
}
//*********************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItemAll (UINT uiCmd, int iIndex)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (iIndex);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItemAll (UINT uiCmd, DWORD_PTR dwData)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (dwData);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
BOOL CBCGToolbarComboBoxButton::SelectItemAll (UINT uiCmd, LPCTSTR lpszText)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (lpszText);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
int CBCGToolbarComboBoxButton::GetCountAll (UINT uiCmd)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetCount ();
	}

	return CB_ERR;
}
//*********************************************************************************
int CBCGToolbarComboBoxButton::GetCurSelAll (UINT uiCmd)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetCurSel ();
	}

	return CB_ERR;
}
//*********************************************************************************
LPCTSTR CBCGToolbarComboBoxButton::GetItemAll (UINT uiCmd, int iIndex)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItem (iIndex);
	}

	return NULL;
}
//*********************************************************************************
DWORD_PTR CBCGToolbarComboBoxButton::GetItemDataAll (UINT uiCmd, int iIndex)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItemData (iIndex);
	}

	return (DWORD)CB_ERR;
}
//*********************************************************************************
void* CBCGToolbarComboBoxButton::GetItemDataPtrAll (UINT uiCmd, int iIndex)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetComboBox ()->GetItemDataPtr (iIndex);
	}

	return NULL;
}
//*********************************************************************************
LPCTSTR CBCGToolbarComboBoxButton::GetTextAll (UINT uiCmd)
{
	CBCGToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetText ();
	}

	return NULL;
}
//*********************************************************************************
void CBCGToolbarComboBoxButton::SetStyle (UINT nStyle)
{
	CBCGToolbarButton::SetStyle (nStyle);

	BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () && !IsEditable ()) ||
		(!CBCGToolBar::IsCustomizeMode () && (m_nStyle & TBBS_DISABLED));

	if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->EnableWindow (!bDisabled);
	}

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->EnableWindow (!bDisabled);
	}
}
//*******************************************************************************
BOOL CBCGToolbarComboBoxButton::HasFocus () const
{
	if (m_pWndCombo == NULL)
	{
		return FALSE;
	}

	CWnd* pWndFocus = CWnd::GetFocus ();

	if (m_pWndCombo->GetDroppedState () ||
		pWndFocus == m_pWndCombo || m_pWndCombo->IsChild (pWndFocus))
	{
		return TRUE;
	}

	if (m_pWndEdit == NULL)
	{
		return FALSE;
	}

	return pWndFocus == m_pWndEdit || m_pWndEdit->IsChild (pWndFocus);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGComboEdit

CBCGComboEdit::CBCGComboEdit(CBCGToolbarComboBoxButton& combo) :
	m_combo (combo)
{
	m_bTracked = FALSE;
}

CBCGComboEdit::~CBCGComboEdit()
{
}


BEGIN_MESSAGE_MAP(CBCGComboEdit, CEdit)
	//{{AFX_MSG_MAP(CBCGComboEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGComboEdit message handlers

BOOL CBCGComboEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if ((GetKeyState(VK_MENU) >= 0) && (GetKeyState(VK_CONTROL) >= 0) &&
			m_combo.GetComboBox () != NULL)
		{
			switch (pMsg->wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:
			case VK_PRIOR:
				if (!m_combo.GetComboBox ()->GetDroppedState ())
				{
					break;
				}

			case VK_RETURN:
				SetFocus ();

				if (m_combo.GetComboBox ()->GetDroppedState ())
				{
					m_combo.GetComboBox ()->SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
				}
				else if (m_combo.GetComboBox ()->GetOwner () != NULL)
				{
					GetWindowText (m_combo.m_strEdit);

					m_combo.GetComboBox ()->GetOwner ()->PostMessage (
						WM_COMMAND, MAKEWPARAM (m_combo.m_nID, 0),
						(LPARAM) m_combo.GetComboBox ()->GetSafeHwnd ());
				}

				return TRUE;
			}
		}

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
			if (m_combo.GetComboBox () != NULL)
			{
				m_combo.GetComboBox ()->ShowDropDown (FALSE);
			}

			if (GetTopLevelFrame () != NULL)
			{
				GetTopLevelFrame ()->SetFocus ();
				return TRUE;
			}
			
			break;

		case VK_UP:
		case VK_DOWN:
			if ((GetKeyState(VK_MENU) >= 0) && (GetKeyState(VK_CONTROL) >=0) &&
				m_combo.GetComboBox () != NULL)
			{
				if (!m_combo.GetComboBox ()->GetDroppedState())
				{
					m_combo.GetComboBox ()->ShowDropDown();

					if (m_combo.GetComboBox ()->GetParent () != NULL)
					{
						m_combo.GetComboBox ()->GetParent ()->InvalidateRect (m_combo.m_rectCombo);
					}
				}
				return TRUE;
			}
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CBCGComboEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	m_combo.SetHotEdit ();
	m_combo.NotifyCommand (CBN_SETFOCUS);
}

void CBCGComboEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	m_combo.SetHotEdit (FALSE);
	m_combo.NotifyCommand (CBN_KILLFOCUS);
}

void CBCGComboEdit::OnChange() 
{
	m_combo.NotifyCommand (CBN_EDITCHANGE);
}
//*************************************************************************************
void CBCGComboEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	CEdit::OnMouseMove(nFlags, point);
	m_combo.SetHotEdit ();

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
afx_msg LRESULT CBCGComboEdit::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus () != this)
	{
		m_combo.SetHotEdit (FALSE);
	}

	return 0;
}
//*****************************************************************************************
void CBCGToolbarComboBoxButton::OnGlobalFontsChanged()
{
	CBCGToolbarButton::OnGlobalFontsChanged ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetFont (&globalData.fontRegular);
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->SetFont (&globalData.fontRegular);
	}
}
