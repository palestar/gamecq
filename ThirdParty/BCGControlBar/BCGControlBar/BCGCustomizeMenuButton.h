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
// BCGCustomizeMenuButton.h: interface for the CBCGCustomizeMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGCUSTOMIZEMENUBUTTON_H__381A6CA8_669E_448F_BF30_61D079ED9951__INCLUDED_)
#define AFX_BCGCUSTOMIZEMENUBUTTON_H__381A6CA8_669E_448F_BF30_61D079ED9951__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarMenuButton.h"

#define BCGCUSTOMIZE_INTERNAL_ID ((UINT)-20)

class BCGCONTROLBARDLLEXPORT  CBCGCustomizeMenuButton : public CBCGToolbarMenuButton  
{
	friend class CBCGPopupMenuBar;

	DECLARE_DYNCREATE(CBCGCustomizeMenuButton)

public:
	CBCGCustomizeMenuButton(UINT uiID,HMENU hMenu,int iImage,LPCTSTR lpszText=NULL,BOOL bUserButton=FALSE);
	CBCGCustomizeMenuButton();
	virtual ~CBCGCustomizeMenuButton();

	static BOOL SetParentToolbar(CBCGToolBar* pToolBar)
	{
		m_pWndToolBar = pToolBar;

		return TRUE;
	}

	static CBCGToolBar* GetParentToolbar()
	{
		return m_pWndToolBar;
	}

	void SetItemIndex(UINT uiIndex, BOOL bExist = TRUE, BOOL bAddSp = FALSE);


//Overridables
protected:
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClickMenuItem();
	virtual void DrawCheckBox(CDC* pDC, const CRect& rect, BOOL bHighlight);

public:

	void SetSeparator()
	{
		bSeparator = TRUE;
		SetStyle(TBBS_DISABLED);
	}

	void RestoreRecentlyUsedState()
	{
		CBCGMenuBar::SetRecentlyUsedMenus(m_bRecentlyUsedOld);	
	}

	//Save Resently Used State
	static BOOL m_bRecentlyUsedOld;

	void EnableCustomization(BOOL bEnable = TRUE)
	{
		m_bIsEnabled = bEnable;
	}

	void SetBrothersButton()
	{
		m_bBrothersBtn = TRUE;
	}

	static CMap<UINT, UINT, int, int>	m_mapPresentIDs;

	static BOOL IsCommandExist(UINT uiCmdId);

protected:
	static CBCGToolBar* m_pWndToolBar;  
	UINT m_uiIndex;
	BOOL m_bShow;
	BOOL bSeparator;
	BOOL m_bExist;
	BOOL m_bAddSpr;
	BOOL m_bIsEnabled;
	BOOL m_bBrothersBtn;

protected:
		CString SearchCommandText(CMenu* pMenu, UINT in_uiCmd);
		void UpdateCustomizeButton();
};

#endif // !defined(AFX_BCGCUSTOMIZEMENUBUTTON_H__381A6CA8_669E_448F_BF30_61D079ED9951__INCLUDED_)
