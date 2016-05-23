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

// BCGVisualManagerXP.h: interface for the CBCGVisualManagerXP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
#define AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
#include "BCGVisualManager.h"

class BCGCONTROLBARDLLEXPORT CBCGVisualManagerXP : public CBCGVisualManager  
{
	DECLARE_DYNCREATE(CBCGVisualManagerXP)

protected:
	CBCGVisualManagerXP(BOOL bIsTemporary = FALSE);

public:
	virtual ~CBCGVisualManagerXP();

protected:
	virtual void OnUpdateSystemColors ();

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuShadow (CDC* pDC, const CRect& rectClient, const CRect& rectExclude,
									int nDepth,  int iMinBrightness,  int iMaxBrightness,  
									CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight,
									BOOL bRTL);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnHighlightMenuItem (CDC*pDC, CBCGToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual BOOL IsHighlightWholeMenuItem ()	{	return TRUE;	}
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGToolbarMenuButton* pButton);

	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual void OnHighlightQuickCustomizeMenuButton (CDC* pDC, CBCGToolbarMenuButton* pButton, CRect rect);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);

	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual void OnDrawEditBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsHighlighted,
										CBCGToolbarEditBoxButton* pButton);

	virtual COLORREF GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												CBCGVisualManager::BCGBUTTON_STATE state);
	virtual int GetMenuImageMargin () const
	{
		return 3;
	}

	virtual int GetPopupMenuGap () const
	{
		return 0;
	}

	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	// Outlook bar caption:
	virtual void OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, CBCGButton* pButton);
	virtual void OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState);

	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);
	// Tasks pane:
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, 
										BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, 
										BOOL bCanCollapse = FALSE);

	virtual void OnEraseTasksGroupArea(CDC* pDC, CRect rect);
	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, 
											BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);
	
	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);

	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGSpinButtonCtrl* pSpinCtrl);

	virtual void OnDrawSplitterBorder (CDC* pDC, CBCGSplitterWnd* pSplitterWnd, CRect rect);
	virtual void OnDrawSplitterBox (CDC* pDC, CBCGSplitterWnd* pSplitterWnd, CRect& rect);

	BOOL IsLook2000Allowed () const
	{
		return FALSE;
	}

	virtual BOOL IsDrawControlBarEdges () const
	{
		return FALSE;
	}

	virtual COLORREF GetToolbarHighlightColor ()
	{
		return m_clrHighlight;
	}

	// Property list:
	virtual COLORREF GetPropListGroupColor (CBCGPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPropList* pPropList);

	// Popup window:
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* pPopupWnd);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);

	CBrush	m_brGripperHorz;
	CBrush	m_brGripperVert;

	COLORREF	m_clrBarBkgnd;			// Control bar background color (expect menu bar)
	CBrush		m_brBarBkgnd;

	COLORREF	m_clrMenuRarelyUsed;
	CBrush		m_brMenuRarelyUsed;

	COLORREF	m_clrMenuLight;			// Color of the light menu area
	CBrush		m_brMenuLight;

	COLORREF	m_clrHighlight;			// Highlighted toolbar/menu item color
	CBrush		m_brHighlight;

	COLORREF	m_clrHighlightDn;		// Highlighted and pressed toolbar item color
	CBrush		m_brHighlightDn;

	COLORREF	m_clrHighlightChecked;
	CBrush		m_brHighlightChecked;

	COLORREF	m_clrInactiveTabText;
	CBrush		m_brTabBack;

	CPen		m_penSeparator;

	COLORREF	m_clrPaneBorder;			// Status bar pane border

	COLORREF	m_clrMenuBorder;			// Menu border
	COLORREF	m_clrMenuItemBorder;		// Highlighted menu item border
	CPen		m_penMenuItemBorder;
	COLORREF	m_clrPressedButtonBorder;	// Used in derived classes

	COLORREF	m_clrGripper;

	BOOL		m_bConnectMenuToParent;
	COLORREF	m_clrMenuShadowBase;
	BOOL		m_bShdowDroppedDownMenuButton;

	BOOL		m_bDrawLastTabLine;

	virtual void CreateGripperBrush ();
	virtual void ExtendMenuButton (CBCGToolbarMenuButton* pMenuButton, CRect& rect);
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGToolbarButton* pButton);

	virtual COLORREF GetWindowColor () const;
};

#endif // !defined(AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
