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
// BCGWinXPVisualManager.h: interface for the CBCGWinXPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
#define AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
#include "BCGVisualManagerXP.h"

class CBCGButton;

class BCGCONTROLBARDLLEXPORT CBCGWinXPVisualManager : public CBCGVisualManagerXP
{
	DECLARE_DYNCREATE(CBCGWinXPVisualManager)

public:
	CBCGWinXPVisualManager(BOOL bIsTemporary = FALSE);
	virtual ~CBCGWinXPVisualManager();

	static BOOL IsWinXPThemeAvailible ();

	void SetOfficeStyleMenus (BOOL bOn = TRUE);
	BOOL IsOfficeStyleMenus () const
	{
		return m_bOfficeStyleMenus;
	}

	static BOOL	m_b3DTabsXPTheme;

	virtual BOOL IsWinXPThemeSupported () const	{	return m_hThemeWindow != NULL;	}

	virtual void OnUpdateSystemColors ();

	virtual int GetPopupMenuGap () const
	{
		return m_bOfficeStyleMenus ? 0 : 1;
	}

	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz, BOOL bMaximized, BOOL bDisabled);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
				CRect rectSizeBox);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
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
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGToolbarMenuButton* pButton);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual BOOL IsHighlightWholeMenuItem ()	{	return m_bOfficeStyleMenus;	}

	virtual COLORREF GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												CBCGVisualManager::BCGBUTTON_STATE state);

	// Outlook bar page buttons:
	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);

	// Outlook bar caption:
	virtual void OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, CBCGButton* pButton);
	virtual void OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState);

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual CSize GetButtonExtraBorder () const;

	virtual void OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual BOOL AlwaysHighlight3DTabs () const		{	return TRUE;	}

	// Tasks pane:
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, 
									BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, 
									BOOL bCanCollapse = FALSE);

	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, 
											BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);
	
	virtual void OnDrawControlBorder (CWnd* pWndCtrl);
	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGSpinButtonCtrl* pSpinCtrl);

	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);

	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual BOOL IsDefaultWinXPPopupButton (CBCGPopupWndButton* pButton) const;

protected:
	BOOL	m_bOfficeStyleMenus;
};

#endif // !defined(AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
