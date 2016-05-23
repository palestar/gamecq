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

// BCGFontComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "BCGFontComboBox.h"
#include "BCGToolBar.h"
#include "BCGToolbarFontCombo.h"
#include "bcglocalres.h"
#include "bcgbarres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int nImageHeight = 16;
const int nImageWidth = 16;

/////////////////////////////////////////////////////////////////////////////
// CBCGFontComboBox

CBCGFontComboBox::CBCGFontComboBox() :
	m_bToolBarMode (FALSE)
{
}

CBCGFontComboBox::~CBCGFontComboBox()
{
}

BEGIN_MESSAGE_MAP(CBCGFontComboBox, CComboBox)
	//{{AFX_MSG_MAP(CBCGFontComboBox)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGFontComboBox message handlers

BOOL CBCGFontComboBox::PreTranslateMessage(MSG* pMsg)
{
	if (m_bToolBarMode &&
		pMsg->message == WM_KEYDOWN &&
		!CBCGToolbarFontCombo::IsFlatMode ())
	{
		CBCGToolBar* pBar = (CBCGToolBar*) GetParent ();

		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			if (BCGGetTopLevelFrame (this) != NULL)
			{
				BCGGetTopLevelFrame (this)->SetFocus ();
			}
			return TRUE;

		case VK_TAB:
			if (pBar != NULL)
			{
				pBar->GetNextDlgTabItem (this)->SetFocus();
			}
			return TRUE;

		case VK_UP:
		case VK_DOWN:
			if ((GetKeyState(VK_MENU) >= 0) && (GetKeyState(VK_CONTROL) >=0) &&
				!GetDroppedState())
			{
				ShowDropDown();
				return TRUE;
			}
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}
//*****************************************************************************************
int CBCGFontComboBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	ASSERT(lpCIS->CtlType == ODT_COMBOBOX);

	int id1 = (int)(WORD)lpCIS->itemID1;
	if (id1 == -1)
	{
		return -1;
	}

	CString str1;
	GetLBText (id1, str1);

	int id2 = (int)(WORD)lpCIS->itemID2;
	if (id2 == -1)
	{
		return 1;
	}

	CString str2;
	GetLBText (id2, str2);

	return str1.Collate (str2);
}
//******************************************************************************************
void CBCGFontComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	if (m_Images.GetSafeHandle () == NULL)
	{
		CBCGLocalResource locaRes;
		m_Images.Create (IDB_BCGBARRES_FONT, nImageWidth, 0, RGB (255, 0, 255));
	}

	ASSERT (lpDIS->CtlType == ODT_COMBOBOX);

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID (pDC);

	CRect rc = lpDIS->rcItem;
	
	if (lpDIS->itemState & ODS_FOCUS)
	{
		pDC->DrawFocusRect(rc);
	}

	int nIndexDC = pDC->SaveDC ();

	CBrush brushFill;
	if (lpDIS->itemState & ODS_SELECTED)
	{
		brushFill.CreateSolidBrush (::GetSysColor (COLOR_HIGHLIGHT));
		pDC->SetTextColor (::GetSysColor (COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		brushFill.CreateSolidBrush (pDC->GetBkColor());
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->FillRect(rc, &brushFill);

	int id = (int)lpDIS->itemID;
	if (id >= 0)
	{
		CBCGFontDesc* pDesc= (CBCGFontDesc*)lpDIS->itemData;
		if (pDesc != NULL)
		{
			if (pDesc->m_nType & (DEVICE_FONTTYPE | TRUETYPE_FONTTYPE))
			{
				CPoint ptImage (rc.left, rc.top + (rc.Height () - nImageHeight) / 2);
				m_Images.Draw (pDC, (pDesc->m_nType & DEVICE_FONTTYPE) ? 0 : 1, 
					ptImage, ILD_NORMAL);
			}

			rc.left += nImageWidth + 6;
		}
		else
		{
			rc.left += 2;
		}

		CString strText;
		GetLBText (id, strText);

		pDC->DrawText (strText, rc, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->RestoreDC (nIndexDC);
}
//****************************************************************************************
void CBCGFontComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	ASSERT(lpMIS->CtlType == ODT_COMBOBOX);

	CRect rc;
	GetWindowRect (&rc);
	lpMIS->itemWidth = rc.Width();

	int nFontHeight = max (globalData.GetTextHeight (), CBCGToolbarFontCombo::m_nFontHeight);

	lpMIS->itemHeight = max (nImageHeight, nFontHeight);
}
//****************************************************************************************
void CBCGFontComboBox::PreSubclassWindow() 
{
	CComboBox::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init ();
	}
}
//****************************************************************************************
int CBCGFontComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init ();
	return 0;
}
//***************************************************************************************
void CBCGFontComboBox::Init ()
{
	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	m_bToolBarMode = pWndParent->IsKindOf (RUNTIME_CLASS (CBCGToolBar));
	if (!m_bToolBarMode)
	{
		Setup ();
	}
}
//***************************************************************************************
void CBCGFontComboBox::CleanUp ()
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (m_hWnd));
	
	if (m_bToolBarMode)
	{
		// Font data will be destroyed by CBCGToolbarFontCombo object
		return;
	}

	for (int i = 0; i < GetCount (); i++)
	{
		delete (CBCGFontDesc*) GetItemData (i);
	}

	ResetContent ();
}
//***************************************************************************************
BOOL CBCGFontComboBox::Setup (int nFontType, BYTE nCharSet, BYTE nPitchAndFamily)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (m_hWnd));
	
	if (m_bToolBarMode)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CleanUp ();

	CBCGToolbarFontCombo combo (0, (UINT)-1, nFontType, nCharSet,
								CBS_DROPDOWN, 0, nPitchAndFamily);

	for (int i = 0; i < combo.GetCount (); i++)
	{
		CString strFont = combo.GetItem (i);

		CBCGFontDesc* pFontDescrSrc = (CBCGFontDesc*) (DWORD_PTR) combo.GetItemData (i);
		ASSERT_VALID (pFontDescrSrc);

		if (FindStringExact (-1, strFont) <= 0)
		{
			CBCGFontDesc* pFontDescr = new CBCGFontDesc (*pFontDescrSrc);
			int iIndex = AddString (strFont);
			SetItemData (iIndex, (DWORD_PTR) pFontDescr);
		}
	}

	return TRUE;
}
//***************************************************************************************
void CBCGFontComboBox::OnDestroy() 
{
	CleanUp ();
	CComboBox::OnDestroy();
}
//***************************************************************************************
BOOL CBCGFontComboBox::SelectFont (CBCGFontDesc* pDesc)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (m_hWnd));
	ASSERT_VALID (pDesc);

	for (int i = 0; i < GetCount (); i++)
	{
		CBCGFontDesc* pFontDescr = (CBCGFontDesc*) GetItemData (i);
		ASSERT_VALID (pFontDescr);

		if (*pDesc == *pFontDescr)
		{
			SetCurSel (i);
			return TRUE;
		}
	}

	return FALSE;
}
//***************************************************************************************
BOOL CBCGFontComboBox::SelectFont (LPCTSTR lpszName, BYTE nCharSet/* = DEFAULT_CHARSET*/)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (m_hWnd));
	ASSERT (lpszName != NULL);

	for (int i = 0; i < GetCount (); i++)
	{
		CBCGFontDesc* pFontDescr = (CBCGFontDesc*) GetItemData (i);
		ASSERT_VALID (pFontDescr);

		if (pFontDescr->m_strName == lpszName)
		{
			if (nCharSet == DEFAULT_CHARSET ||
				nCharSet == pFontDescr->m_nCharSet)
			{
				SetCurSel (i);
				return TRUE;
			}
		}
	}

	return FALSE;
}
//***************************************************************************************
CBCGFontDesc* CBCGFontComboBox::GetSelFont () const
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (m_hWnd));

	int iIndex = GetCurSel ();
	if (iIndex < 0)
	{
		return NULL;
	}

	return (CBCGFontDesc*) GetItemData (iIndex);
}
