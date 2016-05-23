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

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "multimon.h"
#include "globals.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "bcgsound.h"
#include "BCGToolBar.h"
#include "BCGKeyboardManager.h"
#include "MenuImages.h"
#include "MenuHash.h"
#include "BCGVisualManager.h"
#include "BCGSkinManager.h"
#include "BCGURLLinkButton.h"
#include "bcgcbver.h"

extern CObList	gAllToolbars;

BOOL CBCGMemDC::m_bUseMemoryDC = TRUE;

static const CString strOfficeFontName	= _T("Tahoma");
static const CString strDefaultFontName = _T("MS Sans Serif");
static const CString strVertFontName	= _T("Arial");
static const CString strMarlettFontName	= _T("Marlett");

static int CALLBACK FontFamalyProcFonts (const LOGFONT FAR* lplf,
									const TEXTMETRIC FAR* /*lptm*/,
									ULONG /*ulFontType*/,
									LPARAM /*lParam*/)
{
	ASSERT (lplf != NULL);

	CString strFont = lplf->lfFaceName;
	return strFont.CollateNoCase (strOfficeFontName) == 0 ? 0 : 1;
}
/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc

GLOBAL_DATA globalData;

// Initialization code
GLOBAL_DATA::GLOBAL_DATA()
{
	//---------------------------------------------------------
	// Cached system values (updated in CWnd::OnSysColorChange)
	//---------------------------------------------------------
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	hbrWindow = NULL;

	UpdateSysColors();

	m_hcurStretch = NULL;
	m_hcurStretchVert = NULL;
	m_hcurHand = NULL;
	m_hcurSizeAll = NULL;
	m_hiconTool = NULL;
	m_hcurMoveTab = NULL;
	m_hcurNoMoveTab = NULL;
	m_hcurSelectRow = NULL;

	UpdateFonts();
	OnSettingChange ();

	//------------------
	// Small icon sizes:
	//------------------
	m_sizeSmallIcon.cx	= ::GetSystemMetrics (SM_CXSMICON);
	m_sizeSmallIcon.cy	= ::GetSystemMetrics (SM_CYSMICON);

	//-----------------------
	// Detect the kind of OS:
	//-----------------------
	OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx (&osvi);
	bIsWindowsNT4 = ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
					(osvi.dwMajorVersion < 5));

	bIsWindows9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

	bIsOSAlphaBlendingSupport = (osvi.dwMajorVersion > 4) ||
		((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0));

	if (bIsWindowsNT4)
	{
		bIsOSAlphaBlendingSupport = FALSE;
	}

	m_nDragFrameThiknessFloat = 4;  // pixels
	m_nDragFrameThiknessDock = 3;   // pixels

	m_nAutoHideToolBarSpacing = 14; // pixels
	m_nAutoHideToolBarMargin  = 4;  // pixels

	m_nCoveredMainWndClientAreaPercent = 50; // percents

	bIsRemoteSession = FALSE;

	if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		(osvi.dwMajorVersion >= 5))
	{
		bIsRemoteSession = GetSystemMetrics (0x1000 /* SM_REMOTESESSION */);
	}

	m_hinstUXThemeDLL = LoadLibrary (_T("UxTheme.dll"));
	if (m_hinstUXThemeDLL != NULL)
	{
		m_pfDrawThemeBackground = 
			(DRAWTHEMEPARENTBACKGROUND)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeParentBackground");
	}
	else
	{
		m_pfDrawThemeBackground = NULL;
	}

	if (!bIsOSAlphaBlendingSupport)
	{
		m_hinstUser32 = NULL;
		m_pfSetLayeredWindowAttributes = NULL;
	}
	else
	{
		if ((m_hinstUser32 = LoadLibrary (_T("USER32.DLL"))) == NULL)
		{
			ASSERT (FALSE);
		}
		else
		{
			m_pfSetLayeredWindowAttributes = 
				(SETLAYEATTRIB)::GetProcAddress (
					m_hinstUser32, "SetLayeredWindowAttributes");
		}
	}


	m_nMaxToolTipWidth = -1;
	m_bIsBlackHighContrast = FALSE;
	m_bIsWhiteHighContrast = FALSE;

	m_bUseBuiltIn32BitIcons = TRUE;

	m_pGetAccObjectRes = NULL;
	m_hinstAccDLL = NULL;
	m_pfNotifyWinEvent = NULL;
}
//*******************************************************************************************
GLOBAL_DATA::~GLOBAL_DATA()
{
	CleanUp ();
}
//************************************************************************************
void GLOBAL_DATA::UpdateFonts()
{
	if (fontRegular.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontRegular.Detach ());
	}

	if (fontTooltip.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontTooltip.Detach ());
	}

	if (fontBold.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontBold.Detach ());
	}

	if (fontDefaultGUIBold.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontDefaultGUIBold.Detach ());
	}

	if (fontUnderline.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontUnderline.Detach ());
	}

	if (fontDefaultGUIUnderline.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontDefaultGUIUnderline.Detach ());
	}

	if (fontVert.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontVert.Detach ());
	}

	if (fontVertCaption.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontVertCaption.Detach ());
	}

	if (fontMarlett.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontMarlett.Detach ());
	}

	if (fontSmall.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontSmall.Detach ());
	}

	//------------------
	// Initialize fonts:
	//------------------
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	::SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	CWindowDC dc (NULL);
	lf.lfCharSet = (BYTE) GetTextCharsetInfo (dc.GetSafeHdc (), NULL, 0);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	//////////////////////////////////////////////
	// 	// Check if we should use system font
	//--------------------------------------------
	_tcscpy (lf.lfFaceName, info.lfMenuFont.lfFaceName);

	BOOL fUseSystemFont = (info.lfMenuFont.lfCharSet > SYMBOL_CHARSET);
	if (!fUseSystemFont)
	{
		//----------------------------------
		// Check for "Tahoma" font existance:
		//----------------------------------
		if (::EnumFontFamilies (dc.GetSafeHdc (), NULL, FontFamalyProcFonts, 0) == 0)
		{
			//---------------------------
			// Found! Use MS Office font!
			//---------------------------
			_tcscpy (lf.lfFaceName, strOfficeFontName);
		}
		else
		{
			//-----------------------------
			// Not found. Use default font:
			//-----------------------------
			_tcscpy (lf.lfFaceName, strDefaultFontName);
		}
	}
	//--------------------------------------------
	//////////////////////////////////////////////

	fontRegular.CreateFontIndirect (&lf);

	//-------------------
	// Create small font:
	//-------------------
	LONG lfHeightSaved = lf.lfHeight;

	lf.lfHeight = (long) ((1. + abs (lf.lfHeight)) * 2 / 3);
	if (lfHeightSaved < 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}

	fontSmall.CreateFontIndirect (&lf);

	lf.lfHeight = lfHeightSaved;

	//---------------------
	// Create tooltip font:
	//---------------------
	NONCLIENTMETRICS ncm;
	memset (&ncm, 0, sizeof (NONCLIENTMETRICS));
	ncm.cbSize = sizeof (NONCLIENTMETRICS);

	::SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICS),
		&ncm, 0);

	lf.lfItalic = ncm.lfStatusFont.lfItalic;
	lf.lfWeight = ncm.lfStatusFont.lfWeight;
	fontTooltip.CreateFontIndirect (&lf);

	lf.lfItalic = info.lfMenuFont.lfItalic;
	lf.lfWeight = info.lfMenuFont.lfWeight;

	//-------------------------
	// Create "underline" font:
	//-------------------------
	lf.lfUnderline = TRUE;
	fontUnderline.CreateFontIndirect (&lf);
	lf.lfUnderline = FALSE;

	//------------------
	// Create bold font:
	//------------------
	lf.lfWeight = FW_BOLD;
	fontBold.CreateFontIndirect (&lf);

	//---------------------
	// Create Marlett font:
	//---------------------
	BYTE bCharSet = lf.lfCharSet;
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = ::GetSystemMetrics (SM_CYMENUCHECK) - 1;
	_tcscpy (lf.lfFaceName, strMarlettFontName);

	fontMarlett.CreateFontIndirect (&lf);

	lf.lfCharSet = bCharSet;	// Restore charset

	//----------------------
	// Create vertical font:
	//----------------------
	CFont font;
	if (font.CreateStockObject (DEFAULT_GUI_FONT))
	{
		if (font.GetLogFont (&lf) != 0)
		{
			lf.lfOrientation = 900;
			lf.lfEscapement = 2700;

			lf.lfHeight = info.lfMenuFont.lfHeight;
			lf.lfWeight = info.lfMenuFont.lfWeight;
			lf.lfItalic = info.lfMenuFont.lfItalic;
			
			{
				_tcscpy (lf.lfFaceName, strVertFontName);
			}
			//--------------------------------------------
			//////////////////////////////////////////////

			fontVert.CreateFontIndirect (&lf);

			lf.lfEscapement = 900;
			fontVertCaption.CreateFontIndirect (&lf);
		}
	}

	//----------------------------------------
	// Create dialog underline and bold fonts:
	//----------------------------------------
	CFont* pDefaultGUIFont = CFont::FromHandle ((HFONT) GetStockObject (DEFAULT_GUI_FONT));
	ASSERT_VALID (pDefaultGUIFont);
	pDefaultGUIFont->GetLogFont (&lf);

	lf.lfUnderline = TRUE;
	fontDefaultGUIUnderline.CreateFontIndirect (&lf);
	lf.lfUnderline = FALSE;

	lf.lfWeight = FW_BOLD;
	fontDefaultGUIBold.CreateFontIndirect (&lf);

	UpdateTextMetrics();

	//-------------------------------------
	// Notify toolbars about font changing:
	//-------------------------------------
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			pToolBar->OnGlobalFontsChanged ();
		}
	}
}
//*******************************************************************************************
static BOOL CALLBACK InfoEnumProc (	HMONITOR hMonitor, HDC /*hdcMonitor*/,
										LPRECT /*lprcMonitor*/, LPARAM dwData)
{
	CRect* pRect = (CRect*) dwData;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);

	if (GetMonitorInfo (hMonitor, &mi))
	{
		CRect rectMon = mi.rcWork;

		pRect->left = min (pRect->left, rectMon.left);
		pRect->right = max (pRect->right, rectMon.right);
		pRect->top = min (pRect->top, rectMon.top);
		pRect->bottom = max (pRect->bottom, rectMon.bottom);
	}

	return TRUE;
}
//*******************************************************************************************
void GLOBAL_DATA::OnSettingChange ()
{
	m_rectVirtual.SetRectEmpty ();

	if (!EnumDisplayMonitors (NULL, NULL, InfoEnumProc, (LPARAM) &m_rectVirtual))
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &m_rectVirtual, 0);
	}
}
//*******************************************************************************************
void GLOBAL_DATA::UpdateSysColors()
{
	m_bIsBlackHighContrast = 
		::GetSysColor (COLOR_3DLIGHT) == RGB (255, 255, 255) &&
		::GetSysColor (COLOR_3DFACE) == RGB (0, 0, 0);

	m_bIsWhiteHighContrast = 
		::GetSysColor (COLOR_3DDKSHADOW) == RGB (0, 0, 0) &&
		::GetSysColor (COLOR_3DFACE) == RGB (255, 255, 255);

	CWindowDC dc (NULL);
	m_nBitsPerPixel = dc.GetDeviceCaps (BITSPIXEL);

	clrBarFace = clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBarShadow = clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBarDkShadow = clrBtnDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
	clrBarLight = clrBtnLight = ::GetSysColor(COLOR_3DLIGHT);
	clrBarHilite = clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBarText = clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrGrayedText = ::GetSysColor (COLOR_GRAYTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	clrHilite = ::GetSysColor(COLOR_HIGHLIGHT);
	clrTextHilite = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

	clrBarWindow = clrWindow = ::GetSysColor (COLOR_WINDOW);
	clrWindowText = ::GetSysColor (COLOR_WINDOWTEXT);

	clrCaptionText = ::GetSysColor (COLOR_CAPTIONTEXT);
	clrMenuText = ::GetSysColor (COLOR_MENUTEXT);

	clrActiveCaption = ::GetSysColor (COLOR_ACTIVECAPTION);
	clrInactiveCaption = ::GetSysColor (COLOR_INACTIVECAPTION);

	clrActiveBorder = ::GetSysColor (COLOR_ACTIVEBORDER);
	clrInactiveBorder = ::GetSysColor (COLOR_INACTIVEBORDER);

	clrInactiveCaptionText = ::GetSysColor (COLOR_INACTIVECAPTIONTEXT);

	OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx (&osvi);

	if (m_bIsBlackHighContrast)
	{
		clrHotText = clrWindowText;
		clrHotLinkText = clrWindowText;
	}
	else
	{
		if (osvi.dwMajorVersion >= 5)
		{
			clrHotText = ::GetSysColor (26 /*COLOR_HOTLIGHT*/);
			clrHotLinkText = RGB (0, 0, 255);	// Light blue
		}
		else
		{
			clrHotText = RGB (0, 0, 255);		// Light blue
			clrHotLinkText = RGB (255, 0, 255);	// Violet
		}
	}

	hbrBtnShadow = ::GetSysColorBrush (COLOR_BTNSHADOW);
	ASSERT(hbrBtnShadow != NULL);

	hbrBtnHilite = ::GetSysColorBrush (COLOR_BTNHIGHLIGHT);
	ASSERT(hbrBtnHilite != NULL);

	hbrWindow = ::GetSysColorBrush (COLOR_WINDOW);
	ASSERT(hbrWindow != NULL);

	brBtnFace.DeleteObject ();
	brBtnFace.CreateSolidBrush (clrBtnFace);

	brBarFace.DeleteObject ();
	brBarFace.CreateSolidBrush (clrBarFace);

	brActiveCaption.DeleteObject ();
	brActiveCaption.CreateSolidBrush (clrActiveCaption);

	brInactiveCaption.DeleteObject ();
	brInactiveCaption.CreateSolidBrush (clrInactiveCaption);

	brHilite.DeleteObject ();
	brHilite.CreateSolidBrush (clrHilite);

	brBlack.DeleteObject ();
	brBlack.CreateSolidBrush (clrBtnDkShadow);

	brWindow.DeleteObject ();
	brWindow.CreateSolidBrush (clrWindow);

	penHilite.DeleteObject ();
	penHilite.CreatePen (PS_SOLID, 1, globalData.clrHilite);

	penBarFace.DeleteObject ();
	penBarFace.CreatePen (PS_SOLID, 1, globalData.clrBarFace);

	penBarShadow.DeleteObject ();
	penBarShadow.CreatePen (PS_SOLID, 1, globalData.clrBarShadow);

	if (brLight.GetSafeHandle ())
	{
		brLight.DeleteObject ();
	}
	
	if (m_nBitsPerPixel > 8)
	{
		COLORREF clrLight = RGB (
			GetRValue(clrBtnFace) + ((GetRValue(clrBtnHilite) -
				GetRValue(clrBtnFace)) / 2 ),
			GetGValue(clrBtnFace) + ((GetGValue(clrBtnHilite) -
				GetGValue(clrBtnFace)) / 2),
			GetBValue(clrBtnFace) + ((GetBValue(clrBtnHilite) -
				GetBValue(clrBtnFace)) / 2)
			);

		brLight.CreateSolidBrush (clrLight);
	}
	else
	{
		HBITMAP hbmGray = CreateDitherBitmap (dc.GetSafeHdc ());
		ASSERT (hbmGray != NULL);

		CBitmap bmp;
		bmp.Attach (hbmGray);

		brLight.CreatePatternBrush (&bmp);
	}

	CMenuImages::CleanUp ();
}
//************************************************************************************
BOOL GLOBAL_DATA::SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz)
{
	ASSERT (lpLogFont != NULL);

	if (bHorz)
	{
		//---------------------
		// Create regular font:
		//---------------------
		fontRegular.DeleteObject ();
		if (!fontRegular.CreateFontIndirect (lpLogFont))
		{
			ASSERT (FALSE);
			return FALSE;
		}

		//-----------------------
		// Create underline font:
		//-----------------------
		lpLogFont->lfUnderline = TRUE;
		fontUnderline.DeleteObject ();
		fontUnderline.CreateFontIndirect (lpLogFont);
		lpLogFont->lfUnderline = FALSE;

		//---------------------------------------------------
		// Create bold font (used in the default menu items):
		//---------------------------------------------------
		long lSavedWeight = lpLogFont->lfWeight;
		lpLogFont->lfWeight = 700;

		fontBold.DeleteObject ();
		BOOL bResult = fontBold.CreateFontIndirect (lpLogFont);

		lpLogFont->lfWeight = lSavedWeight;	// Restore weight

		if (!bResult)
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}
	else	// Vertical font
	{
		fontVert.DeleteObject ();
		if (!fontVert.CreateFontIndirect (lpLogFont))
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}

	UpdateTextMetrics();
	return TRUE;
}
//************************************************************************************
void GLOBAL_DATA::UpdateTextMetrics ()
{
	CWindowDC dc (NULL);

	CFont* pOldFont = dc.SelectObject (&fontRegular);
	ASSERT (pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	int nExtra = tm.tmHeight < 15 ? 2 : 5;

	m_nTextHeightHorz = tm.tmHeight + nExtra;
	m_nTextWidthHorz = tm.tmMaxCharWidth + nExtra;

	dc.SelectObject (&fontVert);
	dc.GetTextMetrics (&tm);

	nExtra = tm.tmHeight < 15 ? 2 : 5;

	m_nTextHeightVert = tm.tmHeight + nExtra;
	m_nTextWidthVert = tm.tmMaxCharWidth + nExtra;

	dc.SelectObject (pOldFont);
}
//*******************************************************************************
HBITMAP GLOBAL_DATA::CreateDitherBitmap (HDC hDC)
{
	struct  // BITMAPINFO with 16 colors
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD      bmiColors[16];
	} 
	bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 8;
	bmi.bmiHeader.biHeight = 8;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	COLORREF clr = globalData.clrBtnFace;

	bmi.bmiColors[0].rgbBlue = GetBValue(clr);
	bmi.bmiColors[0].rgbGreen = GetGValue(clr);
	bmi.bmiColors[0].rgbRed = GetRValue(clr);

	clr = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	bmi.bmiColors[1].rgbBlue = GetBValue(clr);
	bmi.bmiColors[1].rgbGreen = GetGValue(clr);
	bmi.bmiColors[1].rgbRed = GetRValue(clr);

	// initialize the brushes
	long patGray[8];
	for (int i = 0; i < 8; i++)
	   patGray[i] = (i & 1) ? 0xAAAA5555L : 0x5555AAAAL;

	HBITMAP hbm = CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT,
		(LPBYTE)patGray, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	return hbm;
}
//*************************************************************************************
void GLOBAL_DATA::CleanUp ()
{
	// Terminate sound thread:
	BCGPlaySystemSound (BCGSOUND_TERMINATE);

	if (brLight.GetSafeHandle ())
	{
		brLight.DeleteObject ();
	}
	
	// cleanup fonts:
	fontRegular.DeleteObject ();
	fontBold.DeleteObject ();
	fontUnderline.DeleteObject ();
	fontVert.DeleteObject ();
	fontVertCaption.DeleteObject ();
	fontTooltip.DeleteObject ();

	if (m_hinstUXThemeDLL != NULL)
	{
		::FreeLibrary (m_hinstUXThemeDLL);
		m_hinstUXThemeDLL = NULL;
	}

	if (m_hinstUser32 != NULL)
	{
		::FreeLibrary (m_hinstUser32);
		m_hinstUser32 = NULL;
	}

	if (m_hinstAccDLL != NULL)
	{
		FreeLibrary (m_hinstAccDLL);
		m_hinstAccDLL = NULL;
	}

	m_pfSetLayeredWindowAttributes = NULL;
	m_pGetAccObjectRes = NULL;
	m_pfNotifyWinEvent = NULL;
}
//*************************************************************************************
BCGCONTROLBARDLLEXPORT void BCGCBCleanUp ()
{
	globalData.CleanUp ();
	g_menuHash.CleanUp ();

	CBCGToolBar::CleanUpImages ();
	CMenuImages::CleanUp ();

	if (GetCmdMgr () != NULL)
	{
		GetCmdMgr ()->CleanUp ();
	}

	CBCGKeyboardManager::CleanUp ();

	// Destroy visualization manager:
	CBCGVisualManager::DestroyInstance (TRUE /* bAutoDestroyOnly */);
}
//****************************************************************************************
BOOL GLOBAL_DATA::DrawParentBackground (CWnd* pWnd, CDC* pDC, LPRECT rectClip)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pWnd);

	BOOL bRes = FALSE;

	CRgn rgn;
	if (rectClip != NULL)
	{
		rgn.CreateRectRgnIndirect (rectClip);
		pDC->SelectClipRgn (&rgn);
	}

	CWnd* pParent = pWnd->GetParent ();
	ASSERT_VALID (pParent);

	// In Windows XP, we need to call DrawThemeParentBackground function to implement
	// transparent controls
	if (m_pfDrawThemeBackground != NULL)
	{
		bRes = (*m_pfDrawThemeBackground) (pWnd->GetSafeHwnd (), 
			pDC->GetSafeHdc (), rectClip) == S_OK;
	}

	if (!bRes)
	{
		CPoint pt (0, 0);
		pWnd->MapWindowPoints (pParent, &pt, 1);
		pt = pDC->OffsetWindowOrg (pt.x, pt.y);

		bRes = (BOOL) pParent->SendMessage (WM_ERASEBKGND, (WPARAM)pDC->m_hDC);

		pDC->SetWindowOrg(pt.x, pt.y);
	}

	pDC->SelectClipRgn (NULL);

	return bRes;
}

COLORREF GLOBAL_DATA::GetColor (int nColor)
{
	switch (nColor)
	{
	case COLOR_BTNFACE:             return clrBtnFace;
	case COLOR_BTNSHADOW:           return clrBtnShadow;			
	case COLOR_3DDKSHADOW:          return clrBtnDkShadow;			
	case COLOR_3DLIGHT:             return clrBtnLight;				
	case COLOR_BTNHIGHLIGHT:        return clrBtnHilite;			
	case COLOR_BTNTEXT:             return clrBtnText;				
	case COLOR_GRAYTEXT:            return clrGrayedText;			
	case COLOR_WINDOWFRAME:         return clrWindowFrame;			
	                          
	case COLOR_HIGHLIGHT:           return clrHilite;				
	case COLOR_HIGHLIGHTTEXT:       return clrTextHilite;			
	                          
	case COLOR_WINDOW:				return clrWindow;				
	case COLOR_WINDOWTEXT:			return clrWindowText;			
	                          
	case COLOR_CAPTIONTEXT:			return clrCaptionText;			
	case COLOR_MENUTEXT:			return clrMenuText;				
	                          
	case COLOR_ACTIVECAPTION:		return clrActiveCaption;		
	case COLOR_INACTIVECAPTION:		return clrInactiveCaption;		
	                          
	case COLOR_ACTIVEBORDER:		return clrActiveBorder;			
	case COLOR_INACTIVEBORDER:		return clrInactiveBorder;
	                          
	case COLOR_INACTIVECAPTIONTEXT:	return clrInactiveCaptionText;
	}

	return ::GetSysColor (nColor);
}
//******************************************************************************
BOOL GLOBAL_DATA::SetLayeredAttrib (HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	if (m_pfSetLayeredWindowAttributes == NULL)
	{
		return FALSE;
	}

	return (*m_pfSetLayeredWindowAttributes) (hwnd, crKey, bAlpha, dwFlags);
}
//**********************************************************************************
void GLOBAL_DATA::EnableAccessibilitySupport (BOOL bEnable/* = TRUE*/)
{
	OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx (&osvi);

	if ((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) || (osvi.dwMajorVersion < 5))
	{
		return;
	}

	if (bEnable)
	{
		if (m_hinstAccDLL == NULL)
		{
			if ((m_hinstAccDLL = LoadLibrary (_T("BCGOleAcc.dll"))) == NULL)
			{
				m_pGetAccObjectRes = NULL;
				ASSERT (FALSE);
				return;
			}

			m_pGetAccObjectRes = (BCGGETACCOBJECTLRESULT)
				GetProcAddress (m_hinstAccDLL, "BCGGetAccObjectLresult");
			ASSERT (m_pGetAccObjectRes != NULL);
		}

		if (m_hinstUser32 == NULL)
		{
			if ((m_hinstUser32 = LoadLibrary (_T("USER32.DLL"))) == NULL)
			{
				m_pfNotifyWinEvent = NULL;
				ASSERT (FALSE);
				return;
			}
		}

		m_pfNotifyWinEvent = (NOTIFYWINEVENT)
				GetProcAddress (m_hinstUser32, "NotifyWinEvent");
		ASSERT (m_pfNotifyWinEvent != NULL);
	}
	else
	{
		if (m_hinstAccDLL != NULL)
		{
			FreeLibrary (m_hinstAccDLL);
			m_hinstAccDLL = NULL;
		}

		m_pGetAccObjectRes = NULL;
	}
}
//**********************************************************************************
LRESULT GLOBAL_DATA::GetAccObjectRes (BCGACCDATA* pAccData, IAccessible* pAccess)
{
	if (m_pGetAccObjectRes == NULL)
	{
		return 0;
	}

	return (*m_pGetAccObjectRes) (pAccData, pAccess);
}
//**********************************************************************************
BOOL GLOBAL_DATA::NotifyWinEvent (DWORD event, HWND hwnd, LONG idObject, LONG idChild)
{
	if (m_pfNotifyWinEvent == NULL)
	{
		return FALSE;
	}

	return (*m_pfNotifyWinEvent) (event, hwnd, idObject, idChild);
}
//**********************************************************************************
CString GLOBAL_DATA::RegisterWindowClass (LPCTSTR lpszClassNamePrefix)
{
	ASSERT (lpszClassNamePrefix != NULL);

	//-----------------------------
	// Register a new window class:
	//-----------------------------
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT uiClassStyle = CS_DBLCLKS;
	HCURSOR hCursor = ::LoadCursor (NULL, IDC_ARROW);
	HBRUSH hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

	CString strClassName;
	strClassName.Format (_T("%s:%x:%x:%x:%x"), 
		lpszClassNamePrefix,
		(UINT_PTR)hInst, uiClassStyle, (UINT_PTR)hCursor, (UINT_PTR)hbrBackground);

	//---------------------------------
	// See if the class already exists:
	//---------------------------------
	WNDCLASS wndcls;
	if (::GetClassInfo (hInst, strClassName, &wndcls))
	{
		//-----------------------------------------------
		// Already registered, assert everything is good:
		//-----------------------------------------------
		ASSERT (wndcls.style == uiClassStyle);
	}
	else
	{
		//-------------------------------------------
		// Otherwise we need to register a new class:
		//-------------------------------------------
		wndcls.style = uiClassStyle;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = hCursor;
		wndcls.hbrBackground = hbrBackground;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = strClassName;
		
		if (!AfxRegisterClass (&wndcls))
		{
			AfxThrowResourceException();
		}
	}

	return strClassName;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGAboutDlg dialog

class CBCGAboutDlg : public CDialog
{
// Construction
public:
	CBCGAboutDlg(LPCTSTR lpszAppName, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGAboutDlg)
	enum { IDD = IDD_BCGBARRES_ABOUT_DLG };
	CBCGURLLinkButton	m_wndURL;
	CButton	m_wndPurchaseBtn;
	CStatic	m_wndAppName;
	CBCGURLLinkButton	m_wndEMail;
	CString	m_strAppName;
	CString	m_strVersion;
	CString	m_strYear;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGAboutDlg)
	afx_msg void OnBcgbarresPurchase();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont	m_fontBold;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGAboutDlg dialog

CBCGAboutDlg::CBCGAboutDlg(LPCTSTR lpszAppName, CWnd* pParent /*=NULL*/)
	: CDialog(CBCGAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBCGAboutDlg)
	m_strAppName = _T("");
	m_strVersion = _T("");
	m_strYear = _T("");
	//}}AFX_DATA_INIT

	m_strVersion.Format (_T("%d.%d"), _BCGCB_VERSION_MAJOR, _BCGCB_VERSION_MINOR);

	CString strCurrDate = __DATE__;
	m_strYear.Format (_T("1998-%s"), strCurrDate.Right (4));

	m_strAppName = lpszAppName;
}

void CBCGAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGAboutDlg)
	DDX_Control(pDX, IDC_BCGBARRES_URL, m_wndURL);
	DDX_Control(pDX, IDC_BCGBARRES_PURCHASE, m_wndPurchaseBtn);
	DDX_Control(pDX, IDC_BCGBARRES_NAME, m_wndAppName);
	DDX_Control(pDX, IDC_BCGBARRES_MAIL, m_wndEMail);
	DDX_Text(pDX, IDC_BCGBARRES_NAME, m_strAppName);
	DDX_Text(pDX, IDC_BCGBARRES_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_BCGBARRES_YEAR, m_strYear);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CBCGAboutDlg)
	ON_BN_CLICKED(IDC_BCGBARRES_PURCHASE, OnBcgbarresPurchase)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGAboutDlg message handlers

void CBCGAboutDlg::OnBcgbarresPurchase() 
{
	CString strURL = _T("http://www.bcgsoft.com/register1.htm");
	::ShellExecute (NULL, _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL);

	EndDialog (IDOK);
}

BOOL CBCGAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//------------------
	// Create bold font:
	//------------------
	CFont* pFont = m_wndAppName.GetFont ();
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);

	m_wndAppName.SetFont (&m_fontBold);

	//-----------------------------
	// Setup URL and e-mail fields:
	//-----------------------------
	m_wndEMail.SetURLPrefix (_T("mailto:"));
	m_wndEMail.SetURL (_T("info@bcgsoft.com"));
	m_wndEMail.SizeToContent ();
	m_wndEMail.SetTooltip (_T("Send mail to us"));
	m_wndEMail.m_bDrawFocus = FALSE;

	m_wndURL.m_bDrawFocus = FALSE;
	m_wndURL.SizeToContent ();

	//--------------------
	// Set dialog caption:
	//--------------------
	CString strCaption;
	strCaption.Format (_T("About %s"), m_strAppName);

	SetWindowText (strCaption);

	//------------------------------------------
	// Hide "Purchase" button in retail version:
	//------------------------------------------
#ifndef _BCGCB_EVAL_
	m_wndPurchaseBtn.EnableWindow (FALSE);
	m_wndPurchaseBtn.ShowWindow (SW_HIDE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BCGShowAboutDlg (LPCTSTR lpszAppName)
{
	CBCGLocalResource locaRes;

	CBCGAboutDlg dlg (lpszAppName);
	dlg.DoModal ();
}

void BCGShowAboutDlg (UINT uiAppNameResID)
{
	CString strAppName;
	strAppName.LoadString (uiAppNameResID);

	BCGShowAboutDlg (strAppName);
}
