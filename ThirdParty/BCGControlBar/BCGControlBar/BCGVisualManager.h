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

// BCGVisualManager.h: interface for the CBCGVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_)
#define AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

class CBCGToolBar;
class CBCGPopupMenu;
class CBCGToolbarButton;
class CBCGToolbarMenuButton;
class CBCGSCBButton;
class CBCGTabWnd;
class CBCGStatusBar;
class CBCGToolbarComboBoxButton;
class CBCGToolbarEditBoxButton;
class CBCGButton;
class CBCGCaptionBar;
class CBCGHeaderCtrl;
class CBCGTask;
class CBCGTasksGroup;
class CBCGSpinButtonCtrl;
class CBCGAppBarWnd;
class CBCGSplitterWnd;
class CBCGPopupWndButton;
class CBCGPopupWindow;
class CBCGPropList;
class CBCGSizingControlBar;

#define	SPIN_PRESSEDUP			0x0001
#define	SPIN_PRESSEDDOWN		0x0002
#define	SPIN_HIGHLIGHTEDUP		0x0004
#define	SPIN_HIGHLIGHTEDDOWN	0x0008
#define	SPIN_DISABLED			0x0010

#ifndef _UXTHEME_H_

// From uxtheme.h:
typedef HANDLE HTHEME;          // handle to a section of theme data for class

#endif // THEMEAPI

typedef HTHEME (__stdcall * OPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall * CLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT (__stdcall * DRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
						int iPartId, int iStateId, const RECT *pRect, 
						OPTIONAL const RECT *pClipRect);
typedef HRESULT (__stdcall * GETTHEMECOLOR)(HTHEME hTheme, int iPartId, 
						int iStateId, int iPropId, OUT COLORREF *pColor);
typedef COLORREF (__stdcall * GETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);

typedef HRESULT (__stdcall * GETCURRENTTHEMENAME)(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
    OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);

typedef HTHEME (__stdcall * GETWINDOWTHEME)(HWND hWnd);
typedef BOOL (__stdcall * ISAPPTHEMED)(void);

class BCGCONTROLBARDLLEXPORT CBCGWinXPThemeManager : public CObject
{
	friend class CBCGButton;

public:
	CBCGWinXPThemeManager ();
	virtual ~CBCGWinXPThemeManager ();

	enum WinXpTheme
	{
		WinXpTheme_None = -1,
		WinXpTheme_NonStandard,
		WinXpTheme_Blue,
		WinXpTheme_Olive,
		WinXpTheme_Silver
	};

	virtual BOOL DrawPushButton (CDC* pDC, CRect rect, CBCGButton* pButton, UINT uiState);
	virtual BOOL DrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual BOOL DrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);
	virtual BOOL DrawComboBorder	(CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);

	virtual BOOL DrawCheckBox (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 BOOL bChecked,
										 BOOL bEnabled);

	virtual WinXpTheme GetStandardWinXPTheme ();

	virtual void FillRebarPane (CDC* pDC, 
								CControlBar* pBar, 
								CRect rect);

protected:
	void UpdateSystemColors ();
	void CleanUpThemes ();

	HTHEME				m_hThemeToolBar;
	HTHEME				m_hThemeRebar;
	HTHEME				m_hThemeButton;
	HTHEME				m_hThemeStatusBar;
	HTHEME				m_hThemeWindow;
	HTHEME				m_hThemeComboBox;
	HTHEME				m_hThemeProgress;
	HTHEME				m_hThemeHeader;
	HTHEME				m_hThemeScrollBar;
	HTHEME				m_hThemeExplorerBar;
	HTHEME				m_hThemeTree;
	HTHEME				m_hThemeStartPanel;
	HTHEME				m_hThemeTaskBand;
	HTHEME				m_hThemeTaskBar;
	HTHEME				m_hThemeSpin;
	HTHEME				m_hThemeTab;

	HINSTANCE			m_hinstUXDLL;

	OPENTHEMEDATA		m_pfOpenThemeData;
	CLOSETHEMEDATA		m_pfCloseThemeData;
	DRAWTHEMEBACKGROUND	m_pfDrawThemeBackground;
	GETTHEMECOLOR		m_pfGetThemeColor;
	GETTHEMESYSCOLOR	m_pfGetThemeSysColor;
	GETCURRENTTHEMENAME	m_pfGetCurrentThemeName;
	GETWINDOWTHEME		m_pfGetWindowTheme;
	ISAPPTHEMED			m_pfIsAppThemed;
};

class BCGCONTROLBARDLLEXPORT CBCGVisualManager : public CBCGWinXPThemeManager
{
	friend class CBCGSkinManager;

	DECLARE_DYNCREATE (CBCGVisualManager)

public:
	CBCGVisualManager(BOOL bIsTemporary = FALSE);
	virtual ~CBCGVisualManager();

	static void SetDefaultManager (CRuntimeClass* pRTI);

	virtual BOOL IsWinXPThemeSupported () const	{	return FALSE;	}
	static void DestroyInstance (BOOL bAutoDestroyOnly = FALSE);

// Operations:
public:
	static void RedrawAll ();

// Overrides:
public:
	virtual void OnUpdateSystemColors () {}

	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuShadow (CDC* pDC, const CRect& rectClient, const CRect& rectExclude,
									int nDepth,  int iMinBrightness,  int iMaxBrightness,  
									CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight, BOOL bRTL);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz, BOOL bMaximized, BOOL bDisabled);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);

	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
				CRect rectSizeBox);

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

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	enum BCGBUTTON_STATE
	{
		ButtonsIsRegular,
		ButtonsIsPressed,
		ButtonsIsHighlighted,
	};

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
	virtual BOOL IsHighlightWholeMenuItem ()	{	return FALSE;	}
	
	virtual COLORREF GetMenuItemTextColor (CBCGToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual void OnDrawMenuCheck (CDC* pDC, CBCGToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio);

	virtual BOOL IsOwnerDrawMenuCheck ()	{	return FALSE;	}

	virtual COLORREF GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												CBCGVisualManager::BCGBUTTON_STATE state);
	virtual COLORREF GetToolbarDisabledColor () const
	{
		return (COLORREF)-1;
	}

	virtual BOOL IsToolbarRoundShape (CBCGToolBar* /*pToolBar*/)
	{
		return FALSE;
	}

	virtual COLORREF GetToolbarHighlightColor ();

	// Caption bar:
	virtual COLORREF GetCaptionBarTextColor (CBCGCaptionBar* pBar);

	// Outlook bar overrides:
	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);

	virtual void OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, CBCGButton* pButton);
	virtual void OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState);
	
	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	// Tab overrides:
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTabContent (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd,
							COLORREF clrText);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);
	virtual void GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual BOOL IsHighlightOneNoteTabs () const	{	return FALSE;	}
	virtual BOOL AlwaysHighlight3DTabs () const		{	return FALSE;	}

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual CSize GetButtonExtraBorder () const
	{
		return CSize (0, 0);
	}

	// Header control:
	virtual void OnFillHeaderCtrlBackground (CBCGHeaderCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
								CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

	// Tasks pane:
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, BOOL bCanCollapse = FALSE);

	virtual void OnEraseTasksGroupArea(CDC* pDC, CRect rect);
	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);

	// Property list:
	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);
	virtual COLORREF GetPropListGroupColor (CBCGPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPropList* pPropList);

	// Splitter:
	virtual void OnDrawSplitterBorder (CDC* pDC, CBCGSplitterWnd* pSplitterWnd, CRect rect);
	virtual void OnDrawSplitterBox (CDC* pDC, CBCGSplitterWnd* pSplitterWnd, CRect& rect);
	virtual void OnFillSplitterBackground (CDC* pDC, CBCGSplitterWnd* pSplitterWnd, CRect rect);

	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawControlBorder (CWnd* pWndCtrl);

	// Spin control:
	virtual void OnDrawSpinButtons(CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGSpinButtonCtrl* pSpinCtrl);
	
	// Appbar window:
	virtual void OnDrawAppBarBorder (CDC* pDC, CBCGAppBarWnd* pAppBarWnd,
									CRect rectBorder, CRect rectBorderSize);

	virtual void OnDrawAppBarCaption (	CDC* pDC, CBCGAppBarWnd* pAppBarWnd, 
										CRect rectCaption, CString strCaption);

	// Popup window:
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* pPopupWnd);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPopupWndButton* pButton);
	virtual BOOL IsDefaultWinXPPopupButton (CBCGPopupWndButton* /*pButton*/) const	{	return FALSE;	}

	// Windows XP drawing methods:
	virtual BOOL DrawPushButtonWinXP (CDC* pDC, CRect rect, CBCGButton* pButton, UINT uiState)	
	{	
		return DrawPushButton (pDC, rect, pButton, uiState);
	}

	virtual BOOL DrawComboDropButtonWinXP (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
	{
		return DrawComboDropButton (pDC, rect,
										bDisabled,
										bIsDropped,
										bIsHighlighted);
	}

	virtual BOOL DrawComboBorderWinXP (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
	{
		return DrawComboBorder (pDC, rect,
								bDisabled,
								bIsDropped,
								bIsHighlighted);
	}

// Attributes:
public:

	void SetMenuShadowDepth (int nDepth)	// Pixels
	{
		m_nMenuShadowDepth = nDepth;
	}

	int GetMenuShadowDepth () const
	{
		return m_nMenuShadowDepth;
	}
	
	static CBCGVisualManager* GetInstance ()
	{
		if (m_pVisManager != NULL)
		{
			ASSERT_VALID (m_pVisManager);
			return m_pVisManager;
		}

		if (m_pRTIDefault == NULL)
		{
			m_pVisManager = new CBCGVisualManager;
		}
		else
		{
			m_pVisManager = (CBCGVisualManager*) m_pRTIDefault->CreateObject ();
			ASSERT_VALID (m_pVisManager);
		}

		m_pVisManager->m_bAutoDestroy = TRUE;
		m_pVisManager->OnUpdateSystemColors ();

		return m_pVisManager;
	}

	BOOL IsLook2000 () const			{	return m_bLook2000; }
	void SetLook2000 (BOOL bLook2000 = TRUE);

	BOOL IsMenuFlatLook () const			{	return m_bMenuFlatLook; }
	void SetMenuFlatLook (BOOL bMenuFlatLook = TRUE)
	{
		m_bMenuFlatLook = bMenuFlatLook;
		RedrawAll ();
	}

	BOOL IsAutoDestroy () const
	{
		return m_bAutoDestroy;
	}

	void SetShadowHighlightedImage (BOOL bShadow = TRUE)
	{
		m_bShadowHighlightedImage = bShadow;
	}

	BOOL IsShadowHighlightedImage () const
	{
		return m_bShadowHighlightedImage;
	}

	void EnableToolbarButtonFill (BOOL bEnable = TRUE)
	{
		m_bEnableToolbarButtonFill = bEnable;
	}

	BOOL IsToolbarButtonFillEnabled () const
	{
		return m_bEnableToolbarButtonFill;
	}

	BOOL IsEmbossDisabledImage () const
	{
		return m_bEmbossDisabledImage;
	}

	void SetEmbossDisabledImage (BOOL bEmboss = TRUE)
	{
		m_bEmbossDisabledImage = bEmboss;
	}

	BOOL IsFadeInactiveImage () const
	{
		return m_bFadeInactiveImage;
	}

	void SetFadeInactiveImage (BOOL bFade = TRUE)
	{
		m_bFadeInactiveImage = bFade;
	}

	virtual int GetMenuImageMargin () const
	{
		return 2;
	}

	virtual int GetPopupMenuGap () const
	{
		return 1;
	}

	virtual BOOL IsLook2000Allowed () const
	// Allows choose "Look 2000" in the customization dialog
	{
		return TRUE;
	}

	// TasksPane:
	int GetTasksPaneVertMargin() const
	{
		return m_nVertMargin;
	}

	int GetTasksPaneHorzMargin() const
	{
		return m_nHorzMargin;
	}

	int GetTasksPaneGroupVertOffset() const
	{
		return m_nGroupVertOffset;
	}

	int GetTasksPaneGroupCaptionHeight() const
	{
		return m_nGroupCaptionHeight;
	}

	int GetTasksPaneGroupCaptionHorzOffset() const
	{
		return m_nGroupCaptionHorzOffset;
	}

	int GetTasksPaneGroupCaptionVertOffset() const
	{
		return m_nGroupCaptionVertOffset;
	}

	int GetTasksPaneTaskHorzOffset() const
	{
		return m_nTasksHorzOffset;
	}

	int GetTasksPaneIconHorzOffset() const
	{
		return m_nTasksIconHorzOffset;
	}

	int GetTasksPaneIconVertOffset() const
	{
		return m_nTasksIconVertOffset;
	}
	
	virtual int GetToolBarCustomizeButtonMargin () const
	{
		return 2;
	}

	virtual BOOL IsOffsetPressedButton () const
	{
		return TRUE;
	}

	virtual BOOL IsOfficeXPStyleMenus () const
	{
		return m_bOfficeXPStyleMenus;
	}

	virtual BOOL GetPopupMenuBorderSize () const
	{
		return m_nMenuBorderSize;
	}

	virtual BOOL IsOutlookToolbarHotBorder () const
	{
		return m_bIsOutlookToolbarHotBorder;
	}

	virtual BOOL IsDrawControlBarEdges () const
	{
		return TRUE;
	}

	static void CleanUp ();

protected:
	static CBCGVisualManager* CreateVisualManager (CRuntimeClass* pVisualManager);

// Attributes:
protected:
	static CRuntimeClass*		m_pRTIDefault;
	static CBCGVisualManager*	m_pVisManager;

	BOOL	m_bLook2000;				// Single grippers
	int		m_nMenuShadowDepth;
	BOOL	m_bMenuFlatLook;			// Menu item is always stil unpressed
	BOOL	m_bShadowHighlightedImage;
	BOOL	m_bEmbossDisabledImage;
	BOOL	m_bFadeInactiveImage;
	BOOL	m_bEnableToolbarButtonFill;

	BOOL	m_bIsTemporary;

	int m_nVertMargin;
	int m_nHorzMargin;
	int m_nGroupVertOffset;
	int m_nGroupCaptionHeight;
	int m_nGroupCaptionHorzOffset;
	int m_nGroupCaptionVertOffset;
	int m_nTasksHorzOffset;
	int m_nTasksIconHorzOffset;
	int m_nTasksIconVertOffset;
	BOOL m_bActiveCaptions;
	BOOL m_bOfficeXPStyleMenus;
	int	 m_nMenuBorderSize;
	BOOL m_bIsOutlookToolbarHotBorder;
	BOOL m_bAlwaysFillTab;
	BOOL m_b3DTabWideBorder;

private:
	BOOL	m_bAutoDestroy;
};

#endif // !defined(AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_)
