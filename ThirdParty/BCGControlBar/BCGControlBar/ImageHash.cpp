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

// ImageHash.cpp: implementation of the CImageHash class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageHash.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CImageHash g_ImageHash;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageHash::CImageHash()
{
}
//****************************************************************************************
CImageHash::~CImageHash()
{
}
//****************************************************************************************
void CImageHash::Set (UINT uiCmd, int iImage, BOOL bUserImage)
{
	if (uiCmd == 0 || uiCmd == (UINT) -1)
	{
		return;
	}

	if (bUserImage)
	{
		if (Get (uiCmd, FALSE) < 0)
		{
			m_UserImages.SetAt (uiCmd, iImage);
		}
	}
	else
	{
		if (Get (uiCmd, TRUE) < 0)
		{
			m_StdImages.SetAt (uiCmd, iImage);
		}
	}
}
//****************************************************************************************
int CImageHash::Get (UINT uiCmd, BOOL bUserImage) const
{
	int iImage = -1;

	if (bUserImage)
	{
		if (!m_UserImages.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	else
	{
		if (!m_StdImages.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	
	return iImage;
}
//****************************************************************************************
int CImageHash::GetImageOfCommand (UINT uiCmd, BOOL bUserImage)
{
	return g_ImageHash.Get (uiCmd, bUserImage);
}
//***************************************************************************************
void CImageHash::Clear (UINT uiCmd)
{
	m_UserImages.RemoveKey (uiCmd);
}
//****************************************************************************************
void CImageHash::ClearAll ()
{
	m_UserImages.RemoveAll ();
	m_StdImages.RemoveAll ();
}
