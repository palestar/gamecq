#if !defined(AFX_BCGSPLITTERWND_H__F9890E10_E398_4DED_AF2C_52469CABC617__INCLUDED_)
#define AFX_BCGSPLITTERWND_H__F9890E10_E398_4DED_AF2C_52469CABC617__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGSplitterWnd.h : header file
//

#include "bcgcontrolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGSplitterWnd window

class BCGCONTROLBARDLLEXPORT CBCGSplitterWnd : public CSplitterWnd
{
// Construction
public:
	CBCGSplitterWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGSplitterWnd)
	//}}AFX_VIRTUAL

	virtual void OnDrawSplitter (CDC* pDC, ESplitType nType, const CRect& rect);

// Implementation
public:
	virtual ~CBCGSplitterWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGSplitterWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSPLITTERWND_H__F9890E10_E398_4DED_AF2C_52469CABC617__INCLUDED_)
