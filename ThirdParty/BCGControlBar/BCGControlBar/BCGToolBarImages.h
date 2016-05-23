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

// BCGToolBarImages.h: interface for the CBCGToolBarImages class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
#define AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"

// for custom drawing
typedef struct tagDrawState
{
	HBITMAP hbmMono;
	HBITMAP hbmMonoOld;
	HBITMAP hbmOldGlyphs;
}
CBCGDrawState;

typedef BOOL (__stdcall * ALPHABLEND)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);

class BCGCONTROLBARDLLEXPORT CBCGToolBarImages : public CObject
{
	friend class CBCGDropDownFrame;
	friend class CBCGImageEditDlg;

public:
	CBCGToolBarImages();
	virtual ~CBCGToolBarImages();

// Operations:
public:
	static BOOL Is32BitTransparencySupported ();

	BOOL IsValid () const
	{
		return (m_hbmImageWell != NULL);
	}

	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	HBITMAP GetImageWell () const
	{
		return m_hbmImageWell;
	}

	HBITMAP GetImageWellLight () const
	{
		return m_hbmImageLight;
	}

	BOOL IsUserImagesList () const
	{
		return m_bUserImagesList;
	}

	void SetImageSize (SIZE sizeImage)
	{
		m_sizeImage = sizeImage;
	}

	SIZE GetImageSize (BOOL bDest = FALSE) const
	{
		return bDest ? m_sizeImageDest : m_sizeImage;
	}

	int GetCount () const
	{
		return m_iCount;
	}

	int GetResourceOffset (UINT uiResId) const
	{
		int iOffset = -1;
		if (m_mapOrigResOffsets.Lookup (uiResId, iOffset))
		{
			return iOffset;
		}

		return -1;
	}

	COLORREF SetTransparentColor (COLORREF clrTransparent)
	{
		COLORREF clrOld = m_clrTransparent;

		if (clrTransparent != clrOld)
		{
			m_clrTransparent = clrTransparent;
			
			UpdateInternalImage (0);
			UpdateInternalImage (1);
		}

		return clrOld;
	}

	COLORREF GetTransparentColor () const
	{
		return m_clrTransparent;
	}

	BOOL Load (UINT uiResID, HINSTANCE hinstRes = NULL, BOOL bAdd = FALSE);
	BOOL Load (LPCTSTR lpszBmpFileName);
	BOOL Save (LPCTSTR lpszBmpFileName = NULL);

	void Clear ();

	BOOL PrepareDrawImage (CBCGDrawState& ds,
						CSize sizeImageDest = CSize (0, 0),
						BOOL bFadeInactive = FALSE);

	BOOL Draw (CDC* pDC, 
				int x, int y,
				int iImageIndex,
				BOOL bHilite = FALSE, 
				BOOL bDisabled = FALSE,
				BOOL bIndeterminate = FALSE,
				BOOL bShadow = FALSE,
				BOOL bInactive = FALSE);
	void EndDrawImage (CBCGDrawState& ds);

	int AddImage (HBITMAP hbmp, BOOL bSetBitPerPixel = FALSE);
	int AddIcon (HICON hIcon, BOOL bAlphaBlend = FALSE);
	BOOL UpdateImage (int iImage, HBITMAP hbmp);
	BOOL DeleteImage (int iImage);

	HICON ExtractIcon (int nIndex);

	BOOL CreateFromImageList (const CImageList& imageList);

	BOOL CopyTo (CBCGToolBarImages& imageList);
	BOOL CopyImageToClipboard (int iImage);

	BOOL GrayImages (int nGrayPercentage);

	HBITMAP GetMask (int iImage);

	void OnSysColorChange();

	BOOL MapTo3dColors (BOOL bUseRGBQUAD = TRUE,
						COLORREF clrSrc = (COLORREF)-1,
						COLORREF clrDest = (COLORREF)-1);
	static COLORREF MapToSysColorAlpha (COLORREF color);
	static COLORREF MapToSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static COLORREF MapFromSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static void FillDitheredRect (CDC* pDC, const CRect& rect);

	static void CleanUp ();

	static void SetDisabledImageAlpha (BYTE nValue)
	{
		ASSERT (nValue > 0);
		ASSERT (nValue <= 255);

		m_nDisabledImageAlpha = nValue;
	}

	static BYTE GetDisabledImageAlpha ()
	{
		return m_nDisabledImageAlpha;
	}

	static void SetFadedImageAlpha (BYTE nValue)
	{
		ASSERT (nValue > 0);
		ASSERT (nValue <= 255);

		m_nFadedImageAlpha = nValue;
	}

	static BYTE GetFadedImageAlpha ()
	{
		return m_nFadedImageAlpha;
	}

	void SetLightPercentage (int nValue)
	{
		m_nLightPercentage = nValue;
	}

	int GetLightPercentage () const
	{
		return m_nLightPercentage;
	}

	// RTL support:
	static void EnableRTL (BOOL bIsRTL = TRUE);
	static BOOL IsRTL ()
	{
		return m_bIsRTL;
	}

	BOOL Mirror ();
	static BOOL MirrorBitmap (HBITMAP& hbmp, int cxImage);
	static HRGN CreateRegionFromImage (HBITMAP bmp, COLORREF clrTransparent);

	void AddaptColors (COLORREF clrBase, COLORREF clrTone);

	void SetSingleImage ();

	BOOL GetMapTo3DColors () const
	{
		return m_bMapTo3DColors;
	}

	void SetMapTo3DColors (BOOL bMapTo3DColors)
	{
		m_bMapTo3DColors = bMapTo3DColors;
	}

	BOOL GetAlwaysLight () const
	{
		return m_bAlwaysLight;
	}

	void SetAlwaysLight (BOOL bAlwaysLight = TRUE)
	{
		m_bAlwaysLight = bAlwaysLight;
	}

	CRect GetLastImageRect () const
	{
		return m_rectLastDraw;
	}

protected:
	static void TransparentBlt (HDC hdcDest, int nXDest, int nYDest, int nWidth, 
								int nHeight, CDC* pDcSrc, int nXSrc, int nYSrc,
								COLORREF colorTransparent,
								int nWidthDest = -1, int nHeightDest = -1);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void CreateMask(int iImage,
				BOOL bHilite, 
				BOOL bHiliteShadow);
	static HBITMAP CreateDitherBitmap();
	void UpdateCount ();
	BOOL UpdateInternalImage (int nIndex);

	BOOL PreMultiplyAlpha (HBITMAP hbmp);
	void CopyTemp (CBCGToolBarImages& imagesDest);

	static BOOL MapBmpTo3dColors (	HBITMAP& hBmp, BOOL bUseRGBQUAD = TRUE,
									COLORREF clrSrc = (COLORREF)-1,
									COLORREF clrDest = (COLORREF)-1);

// Attributes:
public:
	static BOOL			m_bDisableTrueColorAlpha;

protected:
	CSize				m_sizeImage;        // size of glyph
	CSize				m_sizeImageDest;	// destination size glyph
	HBITMAP				m_hbmImageWell;		// glyphs only
	HBITMAP				m_hbmImageLight;	// "faded" version (hi-color only)
	HBITMAP				m_hbmImageShadow;	// "shadow" version (hi-color only)
	BOOL				m_bUserImagesList;	// is user-defined images list?
	CString				m_strUDLPath;		// user-defined images path
	BOOL				m_bModified;		// is image modified?
	int					m_iCount;			// image counter
	CList<UINT, UINT>	m_lstOrigResIds;	// original resource ids
	CList<HINSTANCE, HINSTANCE>	m_lstOrigResInstances;	// original resource instances
	CMap<UINT, UINT, int, int> m_mapOrigResOffsets;// original resource offsets
	BOOL				m_bStretch;			// stretch images
	CDC					m_dcMem;			// DC for the drawing in memory
	CBitmap				m_bmpMem;			// bitmap for the drawing in memory
	CBitmap*			m_pBmpOriginal;
	COLORREF			m_clrTransparent;	// Transparent color
	BOOL				m_bReadOnly;		// Loaded from read-only file
	BOOL				m_bIsTemporary;		// Temporary copy of another image
	COLORREF			m_clrImageShadow;	// Color of the shadow
	BOOL				m_bFadeInactive;	// Inactive image will be drawn with the "fading" effect
	int					m_nBitsPerPixel;	// Bitmap color depth
	static HINSTANCE	m_hinstMSIMGDLL;	// MSIMG.DLL instance
	static ALPHABLEND	m_pfAlphaBlend;		// Pointer to AlphaBlend method from MSIMG.DLL
	static BYTE			m_nDisabledImageAlpha;	// Used in the alpha-blending only
	static BYTE			m_nFadedImageAlpha;	// Used in the alpha-blending only
	BOOL				m_bIsGray;			// Image is grayed
	int					m_nGrayPercentage;	// Grayed image brightness
	static BOOL			m_bIsRTL;			// RTL languages support
	int					m_nLightPercentage;	// Light image percentage value
	BOOL				m_bMapTo3DColors;	// Map image colors to system
	BOOL				m_bAlwaysLight;		// Always use light image in all color resolutions
	CRect				m_rectLastDraw;		// Last drawn image location
};

#endif // !defined(AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
