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

// KeyAssign.cpp : implementation file
//

//*********************************************************
// The code is based on the Thierry Maurel's CKeyAssign:
//*********************************************************

////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Thierry Maurel
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    tmaurel@caramail.com   (or tmaurel@hol.fr)
//
////////////////////////////////////////////////////////////////////////////////
// File    : KeyboardEdit.h
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Authors : A.Lebatard + T.Maurel
// Date    : 17.08.98
//
// Remarks : 
//

#include "stdafx.h"
#include "KeyAssign.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyAssign

CKeyAssign::CKeyAssign() :
	m_Helper (&m_Accel)
{
    m_bKeyDefined = FALSE;
	m_bFocused = FALSE;
	m_bIsInitialized = FALSE;

	ResetKey ();
}
//******************************************************************
CKeyAssign::~CKeyAssign()
{
}

BEGIN_MESSAGE_MAP(CKeyAssign, CEdit)
	//{{AFX_MSG_MAP(CKeyAssign)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyAssign message handlers

#pragma warning( disable : 4706 )

BOOL CKeyAssign::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDOWN ||
		pMsg->message == WM_MBUTTONDOWN ||
		pMsg->message == WM_RBUTTONDOWN)
	{
		m_bFocused = TRUE;
		SetFocus ();
		return TRUE;
	}

    BOOL bPressed;
    if ((bPressed = (pMsg->message == WM_KEYDOWN)) || pMsg->message == WM_KEYUP || 
		(bPressed = (pMsg->message == WM_SYSKEYDOWN)) || pMsg->message == WM_SYSKEYUP) 
	{
		if (!m_bIsInitialized)
		{
			m_bFocused = CWnd::GetFocus () == this;
			m_bIsInitialized = TRUE;
		}

        if (bPressed && m_bKeyDefined && !((1 << 30) & pMsg->lParam))
            ResetKey ();
        if (pMsg->wParam == VK_SHIFT && !m_bKeyDefined)
			SetAccelFlag (FSHIFT, bPressed);
        else if (pMsg->wParam == VK_CONTROL &&!m_bKeyDefined) {
			SetAccelFlag (FCONTROL, bPressed);
        }
        else if (pMsg->wParam == VK_MENU && !m_bKeyDefined)
			SetAccelFlag (FALT, bPressed);
        else {
            if (!m_bKeyDefined) 
			{
				if (!m_bFocused)
				{
					m_bFocused = TRUE;
					return TRUE;
				}

				m_Accel.key = (WORD) pMsg->wParam;
                if (bPressed)
				{
                    m_bKeyDefined = TRUE;
					SetAccelFlag (FVIRTKEY, TRUE);
				}
            }
        }

		BOOL bDefaultProcess = FALSE;

		if ((m_Accel.fVirt & FCONTROL) == 0 &&
			(m_Accel.fVirt & FSHIFT) == 0 &&
			(m_Accel.fVirt & FALT) == 0 &&
			(m_Accel.fVirt & FVIRTKEY))
		{
			switch (m_Accel.key)
			{
			case VK_ESCAPE:
				ResetKey ();
				return TRUE;

			case VK_TAB:
				bDefaultProcess = TRUE;
			}
		}

		if (!bDefaultProcess)
		{
			CString strKbd;
			m_Helper.Format (strKbd);

			SetWindowText (strKbd);
			return TRUE;
		}

		ResetKey ();
    }
        
    return CEdit::PreTranslateMessage(pMsg);
}

#pragma warning( default : 4706 )

//******************************************************************
void CKeyAssign::ResetKey ()
{
	memset (&m_Accel, 0, sizeof (ACCEL));
    m_bKeyDefined = FALSE;

	if (m_hWnd != NULL)
	{
		SetWindowText (_T(""));
	}
}
//******************************************************************
void CKeyAssign::SetAccelFlag (BYTE bFlag, BOOL bOn)
{
	if (bOn)
	{
		m_Accel.fVirt |= bFlag;
	}
	else
	{
		m_Accel.fVirt &= ~bFlag;
	}
}
//******************************************************************
void CKeyAssign::OnKillFocus(CWnd* pNewWnd) 
{
	m_bFocused = FALSE;

	CEdit::OnKillFocus(pNewWnd);
}
