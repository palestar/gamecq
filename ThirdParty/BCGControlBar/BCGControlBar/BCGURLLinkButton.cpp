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

// BCGURLLinkButton.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGURLLinkButton.h"
#include "globals.h"
#include "bcglocalres.h"
#include "bcgbarres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGURLLinkButton, CBCGButton)

/////////////////////////////////////////////////////////////////////////////
// CBCGURLLinkButton

CBCGURLLinkButton::CBCGURLLinkButton()
{
	m_nFlatStyle = BUTTONSTYLE_NOBORDERS;
	m_sizePushOffset = CSize (0, 0);
	m_bTransparent = TRUE;

	m_bMultilineText = FALSE;
	m_bAlwaysUnderlineText = TRUE;
	m_bDefaultClickProcess = FALSE;

	SetMouseCursorHand ();
}

CBCGURLLinkButton::~CBCGURLLinkButton()
{
}

BEGIN_MESSAGE_MAP(CBCGURLLinkButton, CBCGButton)
	//{{AFX_MSG_MAP(CBCGURLLinkButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGURLLinkButton message handlers

void CBCGURLLinkButton::OnDraw (CDC* pDC, const CRect& rect, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);

	// Set font:
	CFont* pOldFont = NULL;
		
	if (m_bAlwaysUnderlineText || m_bHover)
	{
		pOldFont = pDC->SelectObject (&globalData.fontDefaultGUIUnderline);
	}
	else
	{
		pOldFont = CBCGButton::SelectFont (pDC);
	}

	ASSERT (pOldFont != NULL);

	// Set text parameters:
	pDC->SetTextColor (m_bHover ? globalData.clrHotLinkText : globalData.clrHotText);
	pDC->SetBkMode (TRANSPARENT);

	// Obtain label:
	CString strLabel;
	GetWindowText (strLabel);

	CRect rectText = rect;
	pDC->DrawText (strLabel, rectText, 
		m_bMultilineText ? DT_WORDBREAK : DT_SINGLELINE);

	pDC->SelectObject (pOldFont);
}
//******************************************************************************************
BOOL CBCGURLLinkButton::OnClicked() 
{
	ASSERT_VALID (this);

	if (!IsWindowEnabled ())
	{
		return TRUE;
	}

	if (m_bDefaultClickProcess)
	{
		m_bHover = FALSE;
		Invalidate ();
		UpdateWindow ();

		return FALSE;
	}

	CWaitCursor wait;

	CString strURL = m_strURL;
	if (strURL.IsEmpty ())
	{
		GetWindowText (strURL);
	}

	if (::ShellExecute (NULL, NULL, m_strPrefix + strURL, NULL, NULL, NULL) < (HINSTANCE) 32)
	{
		TRACE(_T("Can't open URL: %s\n"), strURL);
	}

	m_bHover = FALSE;
	Invalidate ();
	UpdateWindow ();

	return TRUE;
}
//*******************************************************************************************
void CBCGURLLinkButton::SetURL (LPCTSTR lpszURL)
{
	if (lpszURL == NULL)
	{
		m_strURL.Empty ();
	}
	else
	{
		m_strURL = lpszURL;
	}
}
//*******************************************************************************************
void CBCGURLLinkButton::SetURLPrefix (LPCTSTR lpszPrefix)
{
	ASSERT (lpszPrefix != NULL);
	m_strPrefix = lpszPrefix;
}
//*******************************************************************************************
CSize CBCGURLLinkButton::SizeToContent (BOOL bVCenter, BOOL bHCenter)
{
	if (m_sizeImage != CSize (0, 0))
	{
		return CBCGButton::SizeToContent ();
	}

	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	CClientDC dc (this);

	// Set font:
	CFont* pOldFont = dc.SelectObject (&globalData.fontDefaultGUIUnderline);
	ASSERT (pOldFont != NULL);

	// Obtain label:
	CString strLabel;
	GetWindowText (strLabel);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectText = rectClient;
	dc.DrawText (strLabel, rectText, DT_SINGLELINE | DT_CALCRECT);
	rectText.InflateRect (3, 3);

	if (bVCenter || bHCenter)
	{
		ASSERT (GetParent ()->GetSafeHwnd () != NULL);
		MapWindowPoints (GetParent (), rectClient);

		int dx = bHCenter ? (rectClient.Width () - rectText.Width ()) / 2 : 0;
		int dy = bVCenter ? (rectClient.Height () - rectText.Height ()) / 2 : 0;

		SetWindowPos (NULL, rectClient.left + dx, rectClient.top + dy, 
			rectText.Width (), rectText.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos (NULL, -1, -1, rectText.Width (), rectText.Height (),
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	dc.SelectObject (pOldFont);
	return rectText.Size ();
}
//*****************************************************************************
void CBCGURLLinkButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	CRect rectFocus = rectClient;
	pDC->DrawFocusRect (rectFocus);
}
//****************************************************************************************
BOOL CBCGURLLinkButton::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_SPACE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
		break;

	case WM_KEYUP:
		if (pMsg->wParam == VK_SPACE)
		{
			return TRUE;
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			OnClicked ();
			return TRUE;
		}
		break;
	}

	return CBCGButton::PreTranslateMessage (pMsg);
}
