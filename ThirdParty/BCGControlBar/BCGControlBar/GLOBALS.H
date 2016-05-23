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

#ifndef __GLOBAL_DATA_H
#define __GLOBAL_DATA_H

#include "bcgcontrolbar.h"
#include "bcgaccessibility.h"
#include <oleacc.h>

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics
typedef BOOL (__stdcall * SETLAYEATTRIB)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
typedef LRESULT (*BCGGETACCOBJECTLRESULT)(BCGACCDATA* pAccData, IAccessible* pAccess);
typedef BOOL (__stdcall * NOTIFYWINEVENT)(DWORD event, HWND hwnd, LONG idObject, LONG idChild);
typedef HRESULT (__stdcall * DRAWTHEMEPARENTBACKGROUND)(HWND hWnd, HDC hdc,const RECT *pRec);

struct BCGCONTROLBARDLLEXPORT GLOBAL_DATA
{
	// solid brushes with convenient gray colors and system colors
	HBRUSH hbrBtnHilite, hbrBtnShadow;

	HBRUSH hbrWindow;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;
	COLORREF clrBtnDkShadow, clrBtnLight;
	COLORREF clrGrayedText;
	COLORREF clrHilite;
	COLORREF clrTextHilite;
	COLORREF clrHotText;
	COLORREF clrHotLinkText;

	COLORREF clrBarWindow;
	COLORREF clrBarFace;
	COLORREF clrBarShadow, clrBarHilite;
	COLORREF clrBarDkShadow, clrBarLight;
	COLORREF clrBarText;

	COLORREF clrWindow;
	COLORREF clrWindowText;

	COLORREF clrCaptionText;

	COLORREF clrMenuText;

	COLORREF clrActiveCaption;
	COLORREF clrInactiveCaption;

	COLORREF clrInactiveCaptionText;

	COLORREF clrActiveBorder;
	COLORREF clrInactiveBorder;

	CBrush	brBtnFace;
	CBrush	brHilite;
	CBrush	brLight;
	CBrush	brBlack;
	CBrush	brActiveCaption;
	CBrush	brInactiveCaption;
	CBrush	brWindow;

	CBrush brBarFace;

	CPen	penHilite;
	CPen	penBarFace;
	CPen	penBarShadow;

	HCURSOR	m_hcurStretch;
	HCURSOR	m_hcurStretchVert;
	HCURSOR	m_hcurHand;
	HCURSOR	m_hcurSizeAll;
	HCURSOR	m_hcurMoveTab;
	HCURSOR	m_hcurNoMoveTab;
	HCURSOR	m_hcurSelectRow;

	HICON	m_hiconTool;

	// Shell icon sizes:
	CSize	m_sizeSmallIcon;

	// Toolbar and menu fonts:
	CFont				fontRegular;
	CFont				fontTooltip;
	CFont				fontBold;
	CFont				fontDefaultGUIBold;
	CFont				fontUnderline;
	CFont				fontDefaultGUIUnderline;
	CFont				fontVert;
	CFont				fontVertCaption;
	CFont				fontSmall;
						
	CFont				fontMarlett;	// Standard Windows menu symbols
						
	BOOL				bIsWindowsNT4;
	BOOL				bIsWindows9x;
	int					m_nBitsPerPixel;

	BOOL				bIsRemoteSession;
	BOOL				bIsOSAlphaBlendingSupport;

	int					m_nDragFrameThiknessFloat;
	int					m_nDragFrameThiknessDock;
						
	int					m_nAutoHideToolBarSpacing;
	int					m_nAutoHideToolBarMargin;
						
	int					m_nCoveredMainWndClientAreaPercent;

	int					m_nMaxToolTipWidth;

	BOOL				m_bIsBlackHighContrast;
	BOOL				m_bIsWhiteHighContrast;

	BOOL				m_bUseBuiltIn32BitIcons;

	CRect				m_rectVirtual;
						
// Implementation
	GLOBAL_DATA();
	~GLOBAL_DATA();

	void UpdateSysColors();
	void UpdateFonts();
	void OnSettingChange ();

	BOOL SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz);

	int GetTextHeight (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextHeightHorz : m_nTextHeightVert;
	}

	int GetTextWidth (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextWidthHorz : m_nTextWidthVert;
	}

	BOOL IsWinXPDrawParentBackground () const
	{
		return m_pfDrawThemeBackground != NULL;
	}

	BOOL DrawParentBackground (CWnd* pWnd, CDC* pDC, LPRECT lpRect = NULL);
	void CleanUp ();

	COLORREF GetColor (int nColor);

	BOOL SetLayeredAttrib (HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	BOOL IsWindowsLayerSupportAvailable () const
	{
		return m_pfSetLayeredWindowAttributes != NULL; 
	}

	BOOL Is32BitIcons () const
	{
		return m_bUseBuiltIn32BitIcons && m_nBitsPerPixel >= 16 && !m_bIsBlackHighContrast && !m_bIsWhiteHighContrast;
	}

	BOOL IsAccessibilitySupport () const
	{
		return m_pGetAccObjectRes != NULL;
	}

	BOOL IsHighContastMode () const
	{
		return m_bIsWhiteHighContrast || m_bIsBlackHighContrast;
	}

	void EnableAccessibilitySupport (BOOL bEnable = TRUE);
	LRESULT GetAccObjectRes (BCGACCDATA* pAccData, IAccessible* pAccess);
	BOOL NotifyWinEvent (DWORD event, HWND hwnd, LONG idObject, LONG idChild);

	CString RegisterWindowClass (LPCTSTR lpszClassNamePrefix);

protected:

	void UpdateTextMetrics ();
	HBITMAP CreateDitherBitmap (HDC hDC);
	
	int	m_nTextHeightHorz;
	int	m_nTextHeightVert;

	int	m_nTextWidthHorz;
	int	m_nTextWidthVert;

	HINSTANCE m_hinstUXThemeDLL;
	HINSTANCE m_hinstUser32;
	HINSTANCE m_hinstAccDLL;

	SETLAYEATTRIB m_pfSetLayeredWindowAttributes;

	BCGGETACCOBJECTLRESULT	m_pGetAccObjectRes;
	NOTIFYWINEVENT			m_pfNotifyWinEvent;

	DRAWTHEMEPARENTBACKGROUND m_pfDrawThemeBackground;
};

extern BCGCONTROLBARDLLEXPORT GLOBAL_DATA globalData;

/////////////////////////////////////////////////////////////////////////////

#endif // __GLOBAL_DATA_H
