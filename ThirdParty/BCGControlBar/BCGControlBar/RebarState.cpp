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

// RebarManager.cpp: implementation of the CBCGRebarState class.
// By Nick Hodapp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGRegistry.h"
#include "RebarState.h"

#ifndef BCG_NO_REBAR

static const CString strRebarKeyFmt = _T("BCGRebar-%ld");
static const CString strRebarKey	= _T("RBI");
static const CString strRebarId		= _T("IDs");

BOOL CBCGRebarState::LoadRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a MFC rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CReBar*>(pWnd)->GetReBarCtrl();

	// retrieve our registry section:
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strRegSection))
	{
		return FALSE;
	}

	UINT ulBands = 0;

	// attempt to load this rebar:

	REBARBANDINFO* aBandInfo = NULL;
	if (!reg.Read (strRebarKey, reinterpret_cast<BYTE**>(&aBandInfo), &ulBands))
	{
		delete [] aBandInfo;
		return TRUE;
	}

	LONG* aBandIds = NULL;
	if (!reg.Read (strRebarId, reinterpret_cast<BYTE**>(&aBandIds),  &ulBands))
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	// band count should be identical
	ulBands /= sizeof(LONG);

	if (ulBands != rc.GetBandCount())
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	// reorder the bands:
	REBARBANDINFO rbi;
	for (UINT i = 0 ; i < ulBands ; i++)
	{
		// check all bands (in a release build the assert above won't fire if there's a mixup
		// and we'll happily do our best)
		for (UINT j = i; j < rc.GetBandCount (); j++)
		{
			memset(&rbi, 0, sizeof(rbi));				 
			rbi.cbSize = sizeof(rbi);
			rbi.fMask = RBBIM_CHILD;
			rc.GetBandInfo(j, &rbi);
			if (aBandIds[i] != GetWindowLong(rbi.hwndChild, GWL_ID))
				continue;

			if (i != j)
				rc.MoveBand(j, i);

			rc.SetBandInfo(i, &aBandInfo[i]);
			break;
		}
	}

	delete [] aBandInfo;
	delete [] aBandIds;
	return TRUE;
}
//**********************************************************************************
BOOL CBCGRebarState::SaveRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a MFC rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CReBar*>(pWnd)->GetReBarCtrl();

	//-------------------------------
	// retrieve our registry section:
	//-------------------------------
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strRegSection))
	{
		return FALSE;
	}

	UINT ulBands = rc.GetBandCount ();
	if (ulBands == 0)
	{
		return TRUE;
	}

	REBARBANDINFO* aBandInfo = new REBARBANDINFO[ulBands];
	LONG*          aBandIds  = new LONG[ulBands];
	if (NULL == aBandInfo || NULL == aBandIds)
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	memset(aBandInfo, 0, ulBands * sizeof(REBARBANDINFO));				 
	for (UINT i = 0 ; i < rc.GetBandCount() ; i++)
	{
		REBARBANDINFO& rbi = aBandInfo [i];
		rbi.cbSize = sizeof(rbi);
		rbi.fMask = RBBIM_CHILD | RBBIM_ID | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE | RBBIM_STYLE | RBBIM_HEADERSIZE;
		rc.GetBandInfo(i, &aBandInfo[i]);

		// apparently fixed size bands mis-report their cxMinChildSize:
		rbi.cxMinChild += rbi.fStyle & RBBS_FIXEDSIZE ? 4 : 0;

		aBandIds[i] = GetWindowLong(rbi.hwndChild, GWL_ID);
		rbi.hwndChild = 0;
		rbi.fMask ^= RBBIM_CHILD;
	}

	reg.Write (strRebarKey, reinterpret_cast<BYTE*>(aBandInfo), ulBands * sizeof(REBARBANDINFO));
	reg.Write (strRebarId, reinterpret_cast<BYTE*>(aBandIds),  ulBands * sizeof(LONG));

	delete [] aBandInfo;
	delete [] aBandIds;

	return TRUE;
}
//**********************************************************************************
void CBCGRebarState::LoadState (CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID (pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), LoadRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);
}
//**********************************************************************************
void CBCGRebarState::SaveState(CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID (pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), SaveRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);
}

#endif	// BCG_NO_REBAR
