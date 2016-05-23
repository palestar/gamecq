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

// CmdUsageCount.cpp: implementation of the CCmdUsageCount class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "CmdUsageCount.h"
#include "BCGToolBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT CCmdUsageCount::m_nStartCount = 0;
UINT CCmdUsageCount::m_nMinUsagePercentage = 5;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdUsageCount::CCmdUsageCount() :
	m_nTotalUsage (0)
{
}
//*******************************************************************************
CCmdUsageCount::~CCmdUsageCount()
{
}
//*******************************************************************************
void CCmdUsageCount::Serialize (CArchive& ar)
{
	if (ar.IsLoading ())
	{
		ar >> m_nTotalUsage;
	}
	else
	{
		ar << m_nTotalUsage;
	}

	m_CmdUsage.Serialize (ar);
}
//*******************************************************************************
void CCmdUsageCount::AddCmd (UINT uiCmd)
{
	if (CBCGToolBar::IsCustomizeMode ())
	{
		return;
	}

	if ((uiCmd == 0 || uiCmd == (UINT) -1)	||	// Ignore submenus and separators,
		CBCGToolBar::IsBasicCommand (uiCmd)	||	// basic commands and
		IsStandardCommand (uiCmd))				// standard commands
	{
		return;
	}

	UINT uiCount = 0;
	if (!m_CmdUsage.Lookup (uiCmd, uiCount))
	{
		uiCount = 0;
	}

	m_CmdUsage.SetAt (uiCmd, ++uiCount);
	m_nTotalUsage ++;
}
//*******************************************************************************
void CCmdUsageCount::Reset ()
{
	m_CmdUsage.RemoveAll ();
	m_nTotalUsage = 0;
}
//*******************************************************************************
UINT CCmdUsageCount::GetCount (UINT uiCmd) const
{
	UINT uiCount = 0;
	m_CmdUsage.Lookup (uiCmd, uiCount);

	return uiCount;
}
//*******************************************************************************
BOOL CCmdUsageCount::IsFreqeuntlyUsedCmd (UINT uiCmd) const
{
	//-----------------------------------------------------
	// I say, that the specific command is frequently used,
	// if the command usage percentage  is more than 20%
	//-----------------------------------------------------
	if (m_nTotalUsage == 0)
	{
		return FALSE;
	}

	UINT uiCount = GetCount (uiCmd);
	UINT uiPercentage = uiCount * 100 / m_nTotalUsage;

	return uiPercentage > m_nMinUsagePercentage;
}
//*******************************************************************************
BOOL CCmdUsageCount::HasEnouthInformation () const
{
	return m_nTotalUsage >= m_nStartCount;
}
//*******************************************************************************
BOOL CCmdUsageCount::SetOptions (UINT nStartCount, UINT nMinUsagePercentage)
{
	if (nMinUsagePercentage >= 100)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_nStartCount = nStartCount;
	m_nMinUsagePercentage = nMinUsagePercentage;

	return TRUE;
}
