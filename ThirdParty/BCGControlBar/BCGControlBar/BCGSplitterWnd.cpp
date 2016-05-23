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
//
// BCGSplitterWnd.cpp : implementation file
//

#include "stdafx.h"
#include "BCGVisualManager.h"
#include "BCGSplitterWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGSplitterWnd

CBCGSplitterWnd::CBCGSplitterWnd()
{
}

CBCGSplitterWnd::~CBCGSplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CBCGSplitterWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGSplitterWnd message handlers

void CBCGSplitterWnd::OnDrawSplitter (CDC* pDC, ESplitType nType, 
									   const CRect& rectArg)
{
	// if pDC == NULL, then just invalidate
	if (pDC == NULL)
	{
		RedrawWindow(rectArg, NULL, RDW_INVALIDATE|RDW_NOCHILDREN);
		return;
	}

	CRect rect = rectArg;

	switch (nType)
	{
	case splitBorder:
		CBCGVisualManager::GetInstance ()->OnDrawSplitterBorder (pDC, this, rect);
		return;

	case splitBox:
		CBCGVisualManager::GetInstance ()->OnDrawSplitterBox (pDC, this, rect);
		break;

	case splitIntersection:
	case splitBar:
		break;

	default:
		ASSERT(FALSE);  // unknown splitter type
	}

	// fill the middle
	CBCGVisualManager::GetInstance ()->OnFillSplitterBackground (pDC, this, rect);
}

