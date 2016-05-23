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

// KeyHelper.cpp: implementation of the CBCGKeyHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KeyHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGKeyHelper::CBCGKeyHelper(LPACCEL lpAccel) :
	m_lpAccel (lpAccel)
{
}
//*******************************************************************
CBCGKeyHelper::CBCGKeyHelper() :
	m_lpAccel (NULL)
{
}
//*******************************************************************
CBCGKeyHelper::~CBCGKeyHelper()
{
}
//*******************************************************************
void CBCGKeyHelper::Format (CString& str) const
{
	str.Empty ();

	if (m_lpAccel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (m_lpAccel->fVirt & FCONTROL)
	{
		AddVirtKeyStr (str, VK_CONTROL);
	}

	if (m_lpAccel->fVirt & FSHIFT)
	{
		AddVirtKeyStr (str, VK_SHIFT);
	}

	if (m_lpAccel->fVirt & FALT)
	{
		AddVirtKeyStr (str, VK_MENU);
	}

	if (m_lpAccel->fVirt & FVIRTKEY)
	{
		AddVirtKeyStr (str, m_lpAccel->key, TRUE);
	}
	else if (m_lpAccel->key != 27)	// Don't print esc
	{
		str += (char) m_lpAccel->key;
	}
}
//******************************************************************
void CBCGKeyHelper::AddVirtKeyStr (CString& str, UINT uiVirtKey, BOOL bLast) const
{
	CString strKey;

	if (uiVirtKey == VK_PAUSE)
	{
		strKey = _T("Pause");
	}
	else
	{
		#define BUFFER_LEN 50
		TCHAR szBuffer [BUFFER_LEN + 1];

		ZeroMemory (szBuffer, sizeof (szBuffer));
		
		UINT nScanCode = ::MapVirtualKeyEx (uiVirtKey, 0, 
					::GetKeyboardLayout (0)) <<16 | 0x1;

		if (uiVirtKey >= VK_PRIOR && uiVirtKey <= VK_HELP)
		{
			nScanCode |= 0x01000000;
		}
		
		::GetKeyNameText (nScanCode, szBuffer, BUFFER_LEN);

		strKey = szBuffer;
	}
	
	strKey.MakeLower();
	
	//--------------------------------------
	// The first letter should be uppercase:
	//--------------------------------------
	for (int nCount = 0; nCount < strKey.GetLength(); nCount++)
	{
		TCHAR c = strKey[nCount];
		if (IsCharLower (c))
		{
			c = (TCHAR) toupper (c); // Convert single character JY 4-Dec-99
			strKey.SetAt (nCount, c);
			break;
		}
	}

	str += strKey;
	
	if (!bLast)
	{
		str += '+';
	}
}
