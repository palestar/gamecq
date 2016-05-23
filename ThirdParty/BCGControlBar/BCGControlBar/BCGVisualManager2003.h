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
// BCGVisualManager2003.h: interface for the CBCGVisualManager2003 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGER2003_H__2245BF35_31DE_4AF1_B190_9BD9F0922C0D__INCLUDED_)
#define AFX_BCGVISUALMANAGER2003_H__2245BF35_31DE_4AF1_B190_9BD9F0922C0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGVisualManagerXP.h"

class CBCGCaptionBar;

class BCGCONTROLBARDLLEXPORT CBCGVisualManager2003 : public CBCGVisualManagerXP
{
	DECLARE_DYNCREATE(CBCGVisualManager2003)

public:
	CBCGVisualManager2003();
	virtual ~CBCGVisualManager2003();

	static void SetUseGlobalTheme (BOOL bUseGlobalTheme = TRUE);
	static BOOL IsUseGlobalTheme ()
	{
		return m_bUseGlobalTheme;
	}

	virtual BOOL IsWinXPThemeSupported () const	{	return m_hThemeWindow != NULL;	}

	static void SetStatusBarOfficeXPLook (BOOL bStatusBarOfficeXPLook = TRUE);
	static BOOL IsStatusBarOfficeXPLook ()
	{
		return m_bStatusBarOfficeXPLook;
	}

	static void SetDefaultWinXPColors (BOOL bDefaultWinXPColors = TRUE);
	static BOOL IsDefaultWinXPColorsEnabled ()
	{
		return m_bDefaultWinXPColors;
	}

	virtual COLORREF GetBaseThemeColor ();
	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);

	virtual int GetToolBarCustomizeButtonMargin () const
	{
		return 1;
	}

	virtual COLORREF GetToolbarDisabledColor () const
	{
		return m_clrToolbarDisabled;
	}

	virtual BOOL IsToolbarRoundShape (CBCGToolBar* pToolBar);

	virtual void OnHighlightQuickCustomizeMenuButton (CDC* pDC, CBCGToolbarMenuButton* pButton, CRect rect);
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual void DrawCustomizeButton (CDC* pDC, CRect rect, BOOL bIsHorz,
						  CBCGVisualManager::BCGBUTTON_STATE state,
						  BOOL bIsCustomize, BOOL bIsMoreButtons);

	virtual void OnUpdateSystemColors ();
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGToolbarButton* pButton);

	virtual BOOL IsOffsetPressedButton () const
	{
		return FALSE;
	}

	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawCaptionBarBorder (CDC* pDC, CBCGCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder);
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz, BOOL bMaximized, BOOL bDisabled);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual BOOL IsHighlightOneNoteTabs () const	{	return TRUE;	}

	// Tasks pane:
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, BOOL bCanCollapse = FALSE);

	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, 
											BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);

	// Outlook bar:
	virtual void OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, CBCGButton* pButton);
	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual void OnDrawControlBorder (CWnd* pWndCtrl);

	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
				CRect rectSizeBox);
	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);

	virtual void OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);

	virtual COLORREF GetPropListGroupColor (CBCGPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPropList* pPropList);

	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* pPopupWnd);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);

	virtual COLORREF GetHighlightMenuItemColor () const { return m_clrHighlightMenuItem; }

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

protected:
	WinXpTheme	m_WinXPTheme;

	COLORREF GetThemeColor (HTHEME hTheme, int nIndex) const;

	COLORREF	m_clrBarGradientDark;
	COLORREF	m_clrBarGradientLight;

	COLORREF	m_clrToolBarGradientDark;
	COLORREF	m_clrToolBarGradientLight;

	COLORREF	m_clrToolBarGradientVertLight;
	COLORREF	m_clrToolBarGradientVertDark;

	COLORREF	m_clrToolbarDisabled;

	COLORREF	m_clrCustomizeButtonGradientDark;
	COLORREF	m_clrCustomizeButtonGradientLight;

	COLORREF	m_clrToolBarBottomLine;
	CPen		m_penBottomLine;

	COLORREF	m_colorToolBarCornerTop;
	COLORREF	m_colorToolBarCornerBottom;

	COLORREF	m_clrHighlightMenuItem;

	COLORREF	m_clrHighlightGradientLight;
	COLORREF	m_clrHighlightGradientDark;

	COLORREF	m_clrHighlightDnGradientLight;
	COLORREF	m_clrHighlightDnGradientDark;

	COLORREF	m_clrHighlightCheckedGradientLight;
	COLORREF	m_clrHighlightCheckedGradientDark;

	CPen		m_penSeparatorLight;

	COLORREF	m_clrGripper;

	COLORREF	m_clrCaptionBarGradientLight;
	COLORREF	m_clrCaptionBarGradientDark;

	CBrush		m_brTearOffCaption;
	CBrush		m_brFace;

	COLORREF	m_clrTaskPaneGradientDark;
	COLORREF	m_clrTaskPaneGradientLight;
	COLORREF	m_clrTaskPaneGroupCaptionDark;
	COLORREF	m_clrTaskPaneGroupCaptionLight;
	COLORREF	m_clrTaskPaneGroupCaptionSpecDark;
	COLORREF	m_clrTaskPaneGroupCaptionSpecLight;
	COLORREF	m_clrTaskPaneGroupAreaLight;
	COLORREF	m_clrTaskPaneGroupAreaDark;
	COLORREF	m_clrTaskPaneGroupAreaSpecLight;
	COLORREF	m_clrTaskPaneGroupAreaSpecDark;
	COLORREF	m_clrTaskPaneGroupBorder;
	COLORREF	m_clrTaskPaneText;
	COLORREF	m_clrTaskPaneTextHot;

	CPen		m_penTaskPaneGroupBorder;

	BOOL		m_bIsStandardWinXPTheme;

	static BOOL	m_bUseGlobalTheme;
	static BOOL m_bStatusBarOfficeXPLook;
	static BOOL	m_bDefaultWinXPColors;

	virtual void ModifyGlobalColors ();
	virtual COLORREF GetWindowColor () const;
};

#endif // !defined(AFX_BCGVISUALMANAGER2003_H__2245BF35_31DE_4AF1_B190_9BD9F0922C0D__INCLUDED_)
