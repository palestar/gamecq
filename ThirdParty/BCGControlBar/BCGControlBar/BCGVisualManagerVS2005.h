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
// BCGVisualManagerVS2005.h: interface for the CBCGVisualManagerVS2005 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGERVS2005_H__53FD788C_ABDA_459C_83A2_136A269EE355__INCLUDED_)
#define AFX_BCGVISUALMANAGERVS2005_H__53FD788C_ABDA_459C_83A2_136A269EE355__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGVisualManager2003.h"

class BCGCONTROLBARDLLEXPORT CBCGVisualManagerVS2005 : public CBCGVisualManager2003  
{
	DECLARE_DYNCREATE(CBCGVisualManagerVS2005)

public:
	CBCGVisualManagerVS2005();
	virtual ~CBCGVisualManagerVS2005();

	virtual void OnUpdateSystemColors ();
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton,
									BOOL bHorz, BOOL bMaximized, BOOL bDisabled);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);

	virtual void GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGToolbarButton* pButton);

	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual COLORREF GetPropListGroupColor (CBCGPropList* pPropList);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	static BOOL m_bRoundedAutohideButtons;

protected:
	COLORREF	m_colorActiveTabBorder;
	CPen		m_penActiveTabBorder;
	CBrush		m_brMenuButtonDroppedDown;
	CBrush		m_brMenuItemCheckedHighlight;
	WinXpTheme	m_CurrAppTheme;
};

#endif // !defined(AFX_BCGVISUALMANAGERVS2005_H__53FD788C_ABDA_459C_83A2_136A269EE355__INCLUDED_)
