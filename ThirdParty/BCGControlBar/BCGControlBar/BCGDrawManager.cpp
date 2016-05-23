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
// BCGDrawManager.cpp: implementation of the CBCGDrawManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "bcgcontrolbar.h"
#include "globals.h"
#include "BCGDrawManager.h"
#include "BCGToolBarImages.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const double PI = 3.1415926;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGDrawManager::CBCGDrawManager(CDC& m_dc) :
	m_dc (m_dc)
{
}
//*************************************************************************************
CBCGDrawManager::~CBCGDrawManager()
{
}
//*************************************************************************************
BOOL CBCGDrawManager::HighlightRect (CRect rect, int nPercentage, COLORREF clrTransparent,
									  int nTolerance, COLORREF clrBlend)
{
	if (nPercentage == 100)
	{
		// Nothing to do
		return TRUE;
	}

	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return TRUE;
	}

	if (globalData.m_nBitsPerPixel <= 8)
	{
		CBCGToolBarImages::FillDitheredRect (&m_dc, rect);
		return TRUE;
	}

	if (clrBlend != (COLORREF)-1 && nPercentage > 100)
	{
		return FALSE;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ASSERT (pOldBmp != NULL);

	BITMAPINFO bi;

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = cx * cy;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateDIBSection (
		dcMem.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits,
		NULL, NULL);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != -1)
	{
		clrTransparent = RGB (GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

	for (int pixel = 0; pixel < cx * cy; pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;

		BOOL bIgnore = FALSE;

		if (nTolerance > 0)
		{
			bIgnore = (	abs (GetRValue (color) - GetRValue (clrTransparent)) < nTolerance &&
						abs (GetGValue (color) - GetGValue (clrTransparent)) < nTolerance &&
						abs (GetBValue (color) - GetBValue (clrTransparent)) < nTolerance);
		}
		else
		{
			bIgnore = color == clrTransparent;
		}

		if (!bIgnore)
		{
			if (nPercentage == -1)
			{
				*pBits = RGB (
					min (255, (2 * GetRValue (color) + GetBValue (globalData.clrBtnHilite)) / 3),
					min (255, (2 * GetGValue (color) + GetGValue (globalData.clrBtnHilite)) / 3),
					min (255, (2 * GetBValue (color) + GetRValue (globalData.clrBtnHilite)) / 3));
			}
			else
			{
				if (clrBlend == (COLORREF)-1)
				{
					*pBits = PixelAlpha (color, 
						.01 * nPercentage, .01 * nPercentage, .01 * nPercentage);
				}
				else
				{
					long R = GetRValue (color);
					long G = GetGValue (color);
					long B = GetBValue (color);

					*pBits = RGB (
						min (255, R + ::MulDiv (GetBValue (clrBlend) - R, nPercentage, 100)),
						min (255, G + ::MulDiv (GetGValue (clrBlend) - G, nPercentage, 100)),
						min (255, B + ::MulDiv (GetRValue (clrBlend) - B, nPercentage, 100))
						);
				}
			}
		}
	}

	//-------------------------------------------
	// Copy highligted bitmap back to the screen:
	//-------------------------------------------
	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
}
//*********************************************************************************
void CBCGDrawManager::MirrorRect (CRect rect, BOOL bHorz/* = TRUE*/)
{
	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		ASSERT (FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ASSERT (pOldBmp != NULL);

	BITMAPINFO bi;

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = cx * cy;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateDIBSection (
		dcMem.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits,
		NULL, NULL);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (bHorz)
	{
		for (int y = 0; y < cy; y++)
		{
			for (int x = 0; x < cx / 2; x++)
			{
				int xRight = cx - x - 1;
				int y1 = cy - y;

				COLORREF* pColorLeft = (COLORREF*) (pBits + cx * y1 + x);
				COLORREF colorSaved = *pColorLeft;

				COLORREF* pColorRight = (COLORREF*) (pBits + cx * y1 + xRight);

				*pColorLeft = *pColorRight;
				*pColorRight = colorSaved;
			}
		}
	}
	else
	{
		for (int y = 0; y <= cy / 2; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				int yBottom = cy - y - 1;

				COLORREF* pColorTop = (COLORREF*) (pBits + cx * y + x);
				COLORREF colorSaved = *pColorTop;

				COLORREF* pColorBottom = (COLORREF*) (pBits + cx * yBottom + x);

				*pColorTop = *pColorBottom;
				*pColorBottom = colorSaved;
			}
		}
	}

	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}
//*************************************************************************************
BOOL CBCGDrawManager::GrayRect (CRect rect, int nPercentage, COLORREF clrTransparent,
								 COLORREF clrDisabled)
{
	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return TRUE;
	}

	if (globalData.m_nBitsPerPixel <= 8)
	{
		CBCGToolBarImages::FillDitheredRect (&m_dc, rect);
		return TRUE;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ASSERT (pOldBmp != NULL);

	LPBITMAPINFO lpbi;

	// Fill in the BITMAPINFOHEADER
	lpbi = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biWidth = cx;
	lpbi->bmiHeader.biHeight = cy;
	lpbi->bmiHeader.biPlanes = 1;
	lpbi->bmiHeader.biBitCount = 32;
	lpbi->bmiHeader.biCompression = BI_RGB;
	lpbi->bmiHeader.biSizeImage = cx * cy;
	lpbi->bmiHeader.biXPelsPerMeter = 0;
	lpbi->bmiHeader.biYPelsPerMeter = 0;
	lpbi->bmiHeader.biClrUsed = 0;
	lpbi->bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateDIBSection (
		dcMem.m_hDC, lpbi, DIB_RGB_COLORS, (void **)&pBits,
		NULL, NULL);

	if (hmbpDib == NULL || pBits == NULL)
	{
		delete lpbi;
		ASSERT (FALSE);
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != (COLORREF)-1)
	{
		clrTransparent = RGB (GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

	if (clrDisabled == (COLORREF)-1)
	{
		clrDisabled = globalData.clrBtnHilite;
	}

	for (int pixel = 0; pixel < cx * cy; pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;
		if (color != clrTransparent)
		{
			double H,S,L;
			RGBtoHSL(color, &H, &S, &L);
			color = HLStoRGB_ONE(H,L,0);
			
			if (nPercentage == -1)
			{
				*pBits = RGB (
					min (255, GetRValue (color) + ((GetBValue (clrDisabled) -
					GetRValue (color)) / 2)),
					min (255, GetGValue (color) + ((GetGValue (clrDisabled) -
					GetGValue (color)) / 2)),
					min (255, GetBValue(color) + ((GetRValue (clrDisabled) -
					GetBValue (color)) / 2)));
			}
			else
			{
				*pBits = PixelAlpha (color, .01 * nPercentage, .01 * nPercentage, .01 * nPercentage);
			}
		}
	}
	
	//-------------------------------------------
	// Copy highligted bitmap back to the screen:
	//-------------------------------------------
	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
	delete lpbi;

	return TRUE;
} 
//*************************************************************************************
void CBCGDrawManager::FillGradient (CRect rect, 
									COLORREF colorStart, COLORREF colorFinish, 
									BOOL bHorz/* = TRUE*/,
									int nStartFlatPercentage/* = 0*/,
									int nEndFlatPercentage/* = 0*/)
{
	if (colorStart == colorFinish)
	{
		CBrush br (colorStart);
		m_dc.FillRect (rect, &br);
		return;
	}

	if (nStartFlatPercentage > 0)
	{
		ASSERT (nStartFlatPercentage <= 100);

		if (bHorz)
		{
			CRect rectTop = rect;
			rectTop.bottom = rectTop.top + 
				rectTop.Height () * nStartFlatPercentage / 100;
			rect.top = rectTop.bottom;

			CBrush br (colorFinish);
			m_dc.FillRect (rectTop, &br);
		}
		else
		{
			CRect rectLeft = rect;
			rectLeft.right = rectLeft.left + 
				rectLeft.Width () * nStartFlatPercentage / 100;
			rect.left = rectLeft.right;

			CBrush br (colorStart);
			m_dc.FillRect (rectLeft, &br);
		}
	}

	if (nEndFlatPercentage > 0)
	{
		ASSERT (nEndFlatPercentage <= 100);

		if (bHorz)
		{
			CRect rectBottom = rect;
			rectBottom.top = rectBottom.bottom - 
				rectBottom.Height () * nEndFlatPercentage / 100;
			rect.bottom = rectBottom.top;

			CBrush br (colorStart);
			m_dc.FillRect (rectBottom, &br);
		}
		else
		{
			CRect rectRight = rect;
			rectRight.left = rectRight.right - 
				rectRight.Width () * nEndFlatPercentage / 100;
			rect.right = rectRight.left;

			CBrush br (colorFinish);
			m_dc.FillRect (rectRight, &br);
		}
	}

	if (nEndFlatPercentage + nStartFlatPercentage > 100)
	{
		ASSERT (FALSE);
		return;
	}

    // this will make 2^6 = 64 fountain steps
    int nShift = 6;
    int nSteps = 1 << nShift;

    for (int i = 0; i < nSteps; i++)
    {
        // do a little alpha blending
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) +
                   GetRValue(colorFinish) * i) >> nShift);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) +
                   GetGValue(colorFinish) * i) >> nShift);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) +
                   GetBValue(colorFinish) * i) >> nShift);

		CBrush br (RGB(bR, bG, bB));

        // then paint with the resulting color
        CRect r2 = rect;
        if (bHorz)
        {
            r2.bottom = rect.bottom - 
                ((i * rect.Height()) >> nShift);
            r2.top = rect.bottom - 
                (((i + 1) * rect.Height()) >> nShift);
            if (r2.Height() > 0)
                m_dc.FillRect(r2, &br);
        }
        else
        {
            r2.left = rect.left + 
                ((i * rect.Width()) >> nShift);
            r2.right = rect.left + 
                (((i + 1) * rect.Width()) >> nShift);
            if (r2.Width() > 0)
                m_dc.FillRect(r2, &br);
        }
    }
}
//************************************************************************************
void CBCGDrawManager::FillGradient2 (CRect rect, COLORREF colorStart, COLORREF colorFinish, 
					int nAngle)
{
	if (colorStart == colorFinish)
	{
		CBrush br (colorStart);
		m_dc.FillRect (rect, &br);
		return;
	}

	//----------------------
	// Process simple cases:
	//----------------------
	switch (nAngle)
	{
	case 0:
	case 360:
		FillGradient (rect, colorStart, colorFinish, FALSE);
		return;

	case 90:
		FillGradient (rect, colorStart, colorFinish, TRUE);
		return;

	case 180:
		FillGradient (rect, colorFinish, colorStart, FALSE);
		return;

	case 270:
		FillGradient (rect, colorFinish, colorStart, TRUE);
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return;
	}

	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, rect.Width (), rect.Height ()))
	{
		ASSERT (FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject (&bmpMem);
	ASSERT (pOldBmp != NULL);

	CPen* pOldPen = (CPen*) dcMem.SelectStockObject (NULL_PEN);

    int nShift = 6;
    int nSteps = 1 << nShift;

	const double fAngle = PI * (nAngle + 180) / 180;
	const int nOffset = (int) (cos (fAngle) * rect.Height ());
	const int nTotalWidth = rect.Width () + abs (nOffset);

	const int xStart = nOffset > 0 ? - nOffset : 0;

    for (int i = 0; i < nSteps; i++)
    {
        // do a little alpha blending
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) +
                   GetRValue(colorFinish) * i) >> nShift);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) +
                   GetGValue(colorFinish) * i) >> nShift);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) +
                   GetBValue(colorFinish) * i) >> nShift);

		CBrush br (RGB (bR, bG, bB));

        int x11 = xStart + ((i * nTotalWidth) >> nShift);
        int x12 = xStart + (((i + 1) * nTotalWidth) >> nShift);

		if (x11 == x12)
		{
			continue;
		}

		int x21 = x11 + nOffset;
		int x22 = x21 + (x12 - x11);

		POINT points [4];
		points [0].x = x11;
		points [0].y = 0;
		points [1].x = x12;
		points [1].y = 0;
		points [2].x = x22;
		points [2].y = rect.Height ();
		points [3].x = x21;
		points [3].y = rect.Height ();

		CBrush* pOldBrush = dcMem.SelectObject (&br);
		dcMem.Polygon (points, 4);
		dcMem.SelectObject (pOldBrush);
	}

	dcMem.SelectObject (pOldPen);

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	m_dc.BitBlt (rect.left, rect.top, rect.Width (), rect.Height (), &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject (pOldBmp);
}
//************************************************************************************
BOOL CBCGDrawManager::DrawGradientRing (CRect rect,
					   COLORREF colorStart, COLORREF colorFinish,
					   COLORREF colorBorder,
					   int nAngle /* 0 - 360 */,
					   int nWidth,
					   COLORREF clrFace /* = -1 */)
{
	int cx = rect.Width ();
	int cy = rect.Height ();

	if (cx <= 4 || cy <= 4)
	{
		//--------------------
		// Rectangle too small
		//--------------------
		return FALSE;
	}

	int xOrig = rect.left;
	int yOrig = rect.top;

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ASSERT (pOldBmp != NULL);

	LPBITMAPINFO lpbi;

	// Fill in the BITMAPINFOHEADER
	lpbi = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) ];
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biWidth = cx;
	lpbi->bmiHeader.biHeight = cy;
	lpbi->bmiHeader.biPlanes = 1;
	lpbi->bmiHeader.biBitCount = 32;
	lpbi->bmiHeader.biCompression = BI_RGB;
	lpbi->bmiHeader.biSizeImage = cx * cy;
	lpbi->bmiHeader.biXPelsPerMeter = 0;
	lpbi->bmiHeader.biYPelsPerMeter = 0;
	lpbi->bmiHeader.biClrUsed = 0;
	lpbi->bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateDIBSection (
		dcMem.m_hDC, lpbi, DIB_RGB_COLORS, (void **)&pBits,
		NULL, NULL);

	if (hmbpDib == NULL || pBits == NULL)
	{
		delete lpbi;
		ASSERT (FALSE);
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	rect.OffsetRect (-xOrig, -yOrig);

	const int xCenter = (rect.left + rect.right) / 2;
	const int yCenter = (rect.top + rect.bottom) / 2;

	const int nSteps = 360;
	const double fDelta = 2. * PI / nSteps;
	const double fStart = PI * nAngle / 180;
	const double fFinish = fStart + 2. * PI;

	double rDelta = (double) (.5 + GetRValue (colorFinish) - GetRValue (colorStart)) / nSteps * 2;
	double gDelta = (double) (.5 + GetGValue (colorFinish) - GetGValue (colorStart)) / nSteps * 2;
	double bDelta = (double) (.5 + GetBValue (colorFinish) - GetBValue (colorStart)) / nSteps * 2;

	for (int nLevel = 0; nLevel < nWidth; nLevel++)
	{
		int i = 0;
		const int nRadius = min (rect.Width (), rect.Height ()) / 2;
		const int nRectDelta = rect.Width () - rect.Height ();

		if (clrFace != (COLORREF) -1 && nLevel == 0)
		{
			//---------------
			// Fill interior:
			//---------------
			CBrush brFill (clrFace);
			CBrush* pOldBrush = dcMem.SelectObject (&brFill);
			CPen* pOldPen = (CPen*) dcMem.SelectStockObject (NULL_PEN);

			if (nRectDelta == 0)	// Circle
			{
				dcMem.Ellipse (rect);
			}
			else if (nRectDelta > 0)	// Horizontal
			{
				dcMem.Ellipse (rect.left, rect.top, rect.left + rect.Height (), rect.bottom);
				dcMem.Ellipse (rect.right - rect.Height (), rect.top, rect.right, rect.bottom);
				dcMem.Rectangle (rect.left + rect.Height () / 2, rect.top, rect.right - rect.Height () / 2, rect.bottom);
			}
			else	// Vertical
			{
				dcMem.Ellipse (rect.left, rect.top, rect.right, rect.top + rect.Width ());
				dcMem.Ellipse (rect.left, rect.bottom - rect.Width (), rect.right, rect.bottom);
				dcMem.Rectangle (rect.left, rect.top + rect.Width () / 2, rect.right, rect.bottom - rect.Width () / 2);
			}

			dcMem.SelectObject (pOldBrush);
			dcMem.SelectObject (pOldPen);
		}

		int xPrev = -1;
		int yPrev = -1;

		for (double fAngle = fStart; fAngle < fFinish + fDelta; fAngle += fDelta, i ++)
		{
			const int nStep = fAngle <= (fFinish + fStart) / 2 ? i : nSteps - i;

			const BYTE bR = (BYTE) max (0, min (255, (.5 + rDelta * nStep + GetRValue (colorStart))));
			const BYTE bG = (BYTE) max (0, min (255, (.5 + gDelta * nStep + GetGValue (colorStart))));
			const BYTE bB = (BYTE) max (0, min (255, (.5 + bDelta * nStep + GetBValue (colorStart))));

			COLORREF color = nLevel == 0 && colorBorder != -1 ? 
				colorBorder : RGB (bR, bG, bB);

			double dx = /*(fAngle >= 0 && fAngle <= PI / 2) || (fAngle >= 3 * PI / 2) ?
				.5 : -.5*/0;
			double dy = /*(fAngle <= PI) ? .5 : -.5*/0;

			int x = xCenter + (int) (dx + cos (fAngle) * nRadius);
			int y = yCenter + (int) (dy + sin (fAngle) * nRadius);

			if (nRectDelta > 0)
			{
				if (x > xCenter)
				{
					x += (int) (.5 * nRectDelta);
				}
				else
				{
					x -= (int) (.5 * nRectDelta);
				}

				if (xPrev != -1 && (xPrev > xCenter) != (x > xCenter))
				{
					for (int x1 = min (x, xPrev); x1 < max (x, xPrev); x1++)
					{
						SetPixel (pBits, cx, cy, x1, y, color);
					}
				}
			}
			else if (nRectDelta < 0)
			{
				if (y > yCenter)
				{
					y -= (int) (.5 * nRectDelta);
				}
				else
				{
					y += (int) (.5 * nRectDelta);
				}

				if (yPrev != -1 && (yPrev > yCenter) != (y > yCenter))
				{
					for (int y1 = min (y, yPrev); y1 < max (y, yPrev); y1++)
					{
						SetPixel (pBits, cx, cy, x, y1, color);
					}
				}
			}

			SetPixel (pBits, cx, cy, x, y, color);

			xPrev = x;
			yPrev = y;
		}

		rect.DeflateRect (1, 1);
	}


	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	m_dc.BitBlt (xOrig, yOrig, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
	delete lpbi;

	return TRUE;
}
//*************************************************************************************
BOOL CBCGDrawManager::DrawShadow (CRect rect, int nDepth,
								  int iMinBrightness, int iMaxBrightness,
								  CBitmap* pBmpSaveBottom,
								  CBitmap* pBmpSaveRight,
								  BOOL bRightShadow,
								  COLORREF clrBase)
// ==================================================================
// 
// FUNCTION :  DrawShadows ()
// 
// * Description : Draws the shadow for a rectangular screen element
// 
// * Authors: [Stas Levin ]
//			  [Timo Hummel], Modified: [8/11/99 5:06:59 PM]
//			  
// * Function parameters : 
// [rect] -		The CRect of the rectangular region to draw the
//			    shadow around (altough the CDC needs to be big enough
//				to hold the shadow)
// ==================================================================
{
	ASSERT (nDepth >= 0);

	if (nDepth == 0 || rect.IsRectEmpty ())
	{
		return TRUE;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	const BOOL bIsLeft = !bRightShadow;

	if (pBmpSaveRight != NULL && pBmpSaveRight->GetSafeHandle () != NULL &&
		pBmpSaveBottom != NULL && pBmpSaveBottom->GetSafeHandle () != NULL)
	{
		//---------------------------------------------------
		// Shadows are already implemented, put them directly
		// to the DC:
		//---------------------------------------------------
		m_dc.DrawState (CPoint (
						bIsLeft ? rect.left - nDepth : rect.right, 
						rect.top),
					CSize (nDepth, cy + nDepth),
					pBmpSaveRight, DSS_NORMAL);

		m_dc.DrawState (CPoint (
					bIsLeft ? rect.left - nDepth : rect.left, 
					rect.bottom),
					CSize (cx + nDepth, nDepth),
					pBmpSaveBottom, DSS_NORMAL);
		return TRUE;
	}

	ASSERT (pBmpSaveRight == NULL || pBmpSaveRight->GetSafeHandle () == NULL);
	ASSERT (pBmpSaveBottom == NULL || pBmpSaveBottom->GetSafeHandle () == NULL);

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx + nDepth, cy + nDepth))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ASSERT (pOldBmp != NULL);

	BITMAPINFO bi;

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx + nDepth;
	bi.bmiHeader.biHeight = cy + nDepth;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = (cx + nDepth) * (cy + nDepth);
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateDIBSection (
		dcMem.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits,
		NULL, NULL);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx + nDepth, cy + nDepth, &m_dc, 
		bIsLeft ? rect.left - nDepth : rect.left, 
		rect.top, SRCCOPY);

	//----------------------------------------------------------------------------
	// Process shadowing:
	// For having a very nice shadow effect, its actually hard work. Currently,
	// I'm using a more or less "hardcoded" way to set the shadows (by using a
	// hardcoded algorythm):
	//
	// This algorythm works as follows:
	// 
	// It always draws a few lines, from left to bottom, from bottom to right,
	// from right to up, and from up to left). It does this for the specified
	// shadow width and the color settings.
	//-----------------------------------------------------------------------------

	// For speeding up things, iShadowOffset is the
	// value which is needed to multiply for each shadow step
	int iShadowOffset = (iMaxBrightness - iMinBrightness) / nDepth;

	// Loop for drawing the shadow
	// Actually, this was simpler to implement than I thought
	for (int c = 0; c < nDepth; c++)
	{
		// Draw the shadow from left to bottom
		for (int y = cy; y < cy + (nDepth - c); y++)
		{
			SetAlphaPixel (pBits, rect, c + nDepth, y, 
				iMaxBrightness - ((nDepth  - c) * (iShadowOffset)), nDepth, clrBase, bIsLeft);
		}

		// Draw the shadow from left to right
		for (int x = nDepth + (nDepth - c); x < cx + c; x++)
		{
			SetAlphaPixel(pBits, rect,x, cy + c,
				iMaxBrightness - ((c) * (iShadowOffset)),nDepth, clrBase, bIsLeft);
		}

		// Draw the shadow from top to bottom
		for (int y1 = nDepth + (nDepth - c); y1 < cy + c + 1; y1++)
		{
			SetAlphaPixel(pBits, rect, cx+c, y1, 
				iMaxBrightness - ((c) * (iShadowOffset)),
				nDepth, clrBase, bIsLeft);
		}
		
		// Draw the shadow from top to left
		for (int x1 = cx; x1 < cx + (nDepth - c); x1++)
		{
			SetAlphaPixel (pBits, rect, x1, c + nDepth,
				iMaxBrightness - ((nDepth - c) * (iShadowOffset)),
				nDepth, clrBase, bIsLeft);
		}
	}

	//-----------------------------------------
	// Copy shadowed bitmap back to the screen:
	//-----------------------------------------
	m_dc.BitBlt (bIsLeft ? rect.left - nDepth : rect.left, 
		rect.top, 
		cx + nDepth, cy + nDepth, &dcMem, 0, 0, SRCCOPY);

	//------------------------------------
	// Save shadows in the memory bitmaps:
	//------------------------------------
	if (pBmpSaveRight != NULL)
	{
		pBmpSaveRight->CreateCompatibleBitmap (&m_dc, nDepth + 1, cy + nDepth);
		
		dcMem.SelectObject (pBmpSaveRight);
		dcMem.BitBlt (0, 0, nDepth, cy + nDepth,
			&m_dc, bIsLeft ? 0 : rect.right, rect.top, SRCCOPY);
	}

	if (pBmpSaveBottom != NULL)
	{
		pBmpSaveBottom->CreateCompatibleBitmap (&m_dc, cx + nDepth, nDepth + 1);

		dcMem.SelectObject (pBmpSaveBottom);
		dcMem.BitBlt (0, 0, cx + nDepth, nDepth, &m_dc,
						bIsLeft ? rect.left - nDepth : rect.left, 
						rect.bottom, SRCCOPY);
	}

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
}

// ==================================================================
// 
// FUNCTION :  static void SetAlphaPixel ()
// 
// * Description : Draws an alpha blended pixel
// 
// * Author : [Timo Hummel], Created : [8/11/99 5:04:38 PM]
// 
// * Function parameters : 
// [pBits] -	The DIB bits
// [x] -		X-Coordinate
// [y] -		Y-Coordinate
// [percent] -	Percentage to blit (100 = hollow, 0 = solid)
// 
// ==================================================================
inline void CBCGDrawManager::SetAlphaPixel (COLORREF* pBits, 
											 CRect rect, int x, int y, 
											 int percent, int m_iShadowSize,
											 COLORREF clrBase,
											 BOOL bIsRight)
{
	// Our direct bitmap access swapped the y coordinate...
	y = (rect.Height()+m_iShadowSize)- y;

	COLORREF* pColor = (COLORREF*) (bIsRight ? 
		(pBits + (rect.Width () + m_iShadowSize) * (y + 1) - x) : 
		(pBits + (rect.Width () + m_iShadowSize) * y + x));

	*pColor = PixelAlpha (*pColor, percent);

	if (clrBase == (COLORREF)-1)
	{
		return;
	}

	*pColor = RGB (	min (255, (3 * GetRValue (*pColor) + GetBValue (clrBase)) / 4),
					min (255, (3 * GetGValue (*pColor) + GetGValue (clrBase)) / 4),
					min (255, (3 * GetBValue (*pColor) + GetRValue (clrBase)) / 4));
}

// ==================================================================
// 
// FUNCTION :  PixelAlpha ()
// 
// * Description : Shades a color value with a specified percentage
// 
// * Author : [Timo Hummel], Created : [8/11/99 2:37:04 PM]
// 
// * Returns : [COLORREF] - The result pixel
// 
// * Function parameters : 
// [srcPixel] - The source pixel
// [percent] -  Percentage (amount of shadow)
//
// Example: percent = 10    makes the pixel around 10 times darker
//          percent = 50    makes the pixel around 2 times darker
// 
// ==================================================================
COLORREF CBCGDrawManager::PixelAlpha (COLORREF srcPixel, int percent)
{
	// My formula for calculating the transpareny is as
	// follows (for each single color):
	//
	//							   percent
	// destPixel = sourcePixel * ( ------- )
	//                               100
	//
	// This is not real alpha blending, as it only modifies the brightness,
	// but not the color (a real alpha blending had to mix the source and
	// destination pixels, e.g. mixing green and red makes yellow).
	// For our nice "menu" shadows its good enough.

	COLORREF clrFinal = RGB ( min (255, (GetRValue (srcPixel) * percent) / 100), 
							  min (255, (GetGValue (srcPixel) * percent) / 100), 
							  min (255, (GetBValue (srcPixel) * percent) / 100));

	return (clrFinal);

}

static inline int AdjustChannel (int nValue, double nPercent)
{
	int nNewValue = (int) (.5 + nPercent * nValue);
	if (nValue == 0 && nPercent > 1.)
	{
		nNewValue = (int) (.5 + (nPercent - 1.) * 255);
	}

	return min (nNewValue, 255);
}

COLORREF CBCGDrawManager::PixelAlpha (COLORREF srcPixel, double percentR, double percentG, double percentB)
{
	COLORREF clrFinal = RGB ( AdjustChannel (GetRValue (srcPixel), percentR), 
							  AdjustChannel (GetGValue (srcPixel), percentG), 
							  AdjustChannel (GetBValue (srcPixel), percentB));

	return (clrFinal);

}

static inline int AdjustChannelFast (int nValue, int nPercent)
{
	int nNewValue = nPercent * nValue / 100;
	if (nValue == 0 && nPercent > 100)
	{
		nNewValue = (nPercent - 100) * 255 / 100;
	}

	return min (nNewValue, 255);
}

// ==================================================================
// 
// FUNCTION :  PixelAlpha ()
// 
// * Description : Shades a color value with a specified percentage
// 
// * Author : [Guillaume Nodet]
// 
// * Returns : [COLORREF] - The result pixel
// 
// * Function parameters : 
// [srcPixel] - The source pixel
// [dstPixel] - The destination pixel
// [percent] -  Percentage (amount of shadow)
//
// ==================================================================
COLORREF CBCGDrawManager::PixelAlpha (COLORREF srcPixel, int percentR, int percentG, int percentB)
{
	COLORREF clrFinal = RGB ( AdjustChannelFast (GetRValue (srcPixel), percentR), 
							  AdjustChannelFast (GetGValue (srcPixel), percentG), 
							  AdjustChannelFast (GetBValue (srcPixel), percentB));

	return clrFinal;
}

COLORREF CBCGDrawManager::PixelAlpha (COLORREF srcPixel, COLORREF dstPixel, int percent)
{
	int ipercent = 100 - percent;
	COLORREF clrFinal = RGB ( (GetRValue (srcPixel) * percent + GetRValue (dstPixel) * ipercent) / 100, 
							  (GetGValue (srcPixel) * percent + GetGValue (dstPixel) * ipercent) / 100, 
							  (GetBValue (srcPixel) * percent + GetBValue (dstPixel) * ipercent) / 100);

	return (clrFinal);

}

void CBCGDrawManager::SetPixel (COLORREF* pBits, int cx, int cy, int x, int y,
								COLORREF color)
{
	// Our direct bitmap access swapped the y coordinate...
	y = cy - y;

	COLORREF* pColor = (COLORREF*) (pBits + cx * y + x);
	*pColor = RGB (GetBValue(color), GetGValue(color), GetRValue(color));
}

//----------------------------------------------------------------------
// Conversion between the HSL (Hue, Saturation, and Luminosity) 
// and RBG color model.
//----------------------------------------------------------------------
// The conversion algorithms presented here come from the book by 
// Fundamentals of Interactive Computer Graphics by Foley and van Dam. 
// In the example code, HSL values are represented as floating point 
// number in the range 0 to 1. RGB tridrants use the Windows convention 
// of 0 to 255 of each element. 
//----------------------------------------------------------------------

double CBCGDrawManager::HuetoRGB(double m1, double m2, double h )
{
	if( h < 0 ) h += 1.0;
	if( h > 1 ) h -= 1.0;
	if( 6.0*h < 1 )
		return (m1+(m2-m1)*h*6.0);
	if( 2.0*h < 1 )
		return m2;
	if( 3.0*h < 2.0 )
		return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
	return m1;
}

BYTE CBCGDrawManager::HueToRGB(float rm1, float rm2, float rh)
{
	if (rh > 360.0f)
		rh -= 360.0f;
	else if (rh < 0.0f)
		rh += 360.0f;
	
	if (rh <  60.0f)
		rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f)
		rm1 = rm2;
	else if (rh < 240.0f)
		rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      
	
	return static_cast<BYTE>(rm1 * 255);
}

COLORREF CBCGDrawManager::HLStoRGB_ONE( double H, double L, double S )
{
	double r, g, b;
	double m1, m2;
	
	if(S==0) {
		r=g=b=L;
	} else {
		if(L <=0.5)
			m2 = L*(1.0+S);
		else
			m2 = L+S-L*S;
		m1 = 2.0*L-m2;
		r = HuetoRGB(m1, m2, H+1.0/3.0);
		g = HuetoRGB(m1, m2, H);
		b = HuetoRGB(m1, m2, H-1.0/3.0);
	}
	return RGB((BYTE)(r*255), (BYTE)(g*255), (BYTE)(b*255));
}

COLORREF CBCGDrawManager::HLStoRGB_TWO( double H, double L, double S)
{
	WORD R, G, B; // RGB component values
	
	if (S == 0.0)
		R = G = B = unsigned char(L * 255.0);
	else
	{
		float rm1, rm2;
		
		if (L <= 0.5f)
			rm2 = (float)(L + L * S);
		else
			rm2 = (float)(L + S - L * S);
		
		rm1 = (float)(2.0f * L - rm2);
		
		R = HueToRGB(rm1, rm2, (float)(H + 120.0f));
		G = HueToRGB(rm1, rm2, (float)(H));
		B = HueToRGB(rm1, rm2, (float)(H - 120.0f));
	}
	
	return RGB(R, G, B);
}

void CBCGDrawManager::RGBtoHSL( COLORREF rgb, double *H, double *S, double *L )
{   
	double delta;
	double r = (double)GetRValue(rgb)/255;
	double g = (double)GetGValue(rgb)/255;
	double b = (double)GetBValue(rgb)/255;   
	double cmax = max(r, max(g, b));
	double cmin = min(r, min(g, b));   
	*L=(cmax+cmin)/2.0;   
	
	if(cmax==cmin) 
	{
		*S = 0;      
		*H = 0; // it's really undefined   
	} 
	else 
	{
		if(*L < 0.5) 
			*S = (cmax-cmin)/(cmax+cmin);      
		else
			*S = (cmax-cmin)/(2.0-cmax-cmin);      
		
		delta = cmax - cmin;
		if(r==cmax) 
			*H = (g-b)/delta;      
		else if(g==cmax)
			*H = 2.0 +(b-r)/delta;
		else          
			*H=4.0+(r-g)/delta;
		*H /= 6.0; 
		if(*H < 0.0)
			*H += 1;  
	}
}

void CBCGDrawManager::RGBtoHSV (COLORREF rgb, double *H, double *S, double *V)
// Algoritm by A. R. Smith 
{
	double r = (double) GetRValue (rgb) / 255;
	double g = (double) GetGValue (rgb) / 255;
	double b = (double) GetBValue (rgb) / 255;

	double dblMin = min (r, min (g, b));
	double dblMax = max (r, max (g, b));

	*V = dblMax;				// v
	double delta = dblMax - dblMin;

	if( dblMax != 0 )
		*S = delta / dblMax;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*S = 0;
		*H = -1;
		return;
	}

	if (delta == 0.)
	{
		*H = 1;
	}
	else
	{
		if (r == dblMax)
			*H = (g - b) / delta;		// between yellow & magenta
		else if( g == dblMax )
			*H = 2 + ( b - r ) / delta;	// between cyan & yellow
		else
			*H = 4 + ( r - g ) / delta;	// between magenta & cyan
	}

	*H *= 60;				// degrees

	if (*H < 0)
		*H += 360;
}

COLORREF CBCGDrawManager::HSVtoRGB (double h, double s, double v)
// Algoritm by A. R. Smith
{
	int i;
	double f, p, q, t;
	double r, g, b;

	if( s == 0 ) 
	{
		// achromatic (grey)
		r = g = b = v;
	}
	else
	{
		h /= 60;			// sector 0 to 5
		i = (int) floor( h );
		f = h - i;			// factorial part of h
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );

		switch( i ) 
		{
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		default:		// case 5:
			r = v;
			g = p;
			b = q;
			break;
		}
	}

	return RGB (
		(int) (.5 + r * 255),
		(int) (.5 + g * 255),
		(int) (.5 + b * 255));
}

COLORREF CBCGDrawManager::SmartMixColors (COLORREF color1, COLORREF color2,
		double dblLumRatio, int k1, int k2)
{
	ASSERT (k1 >= 0);
	ASSERT (k2 >= 0);

	if (k1 + k2 == 0)
	{
		ASSERT (FALSE);
		return RGB (0, 0, 0);
	}

	COLORREF color = RGB (
		(GetRValue (color1) * k1 + GetRValue (color2) * k2) / (k1 + k2),
		(GetGValue (color1) * k1 + GetGValue (color2) * k2) / (k1 + k2),
		(GetBValue (color1) * k1 + GetBValue (color2) * k2) / (k1 + k2));

	double h1, s1, v1;
	RGBtoHSV (color, &h1, &s1, &v1);

	double h2, s2, v2;
	RGBtoHSV (color2, &h2, &s2, &v2);

	v1 = v2;
	s1 = (s1 *  k1 + s2 *  k2) /  (k1 + k2);

	color = HSVtoRGB (h1, s1, v1);

	if (dblLumRatio != 1.)
	{
		double H, S, L;
		RGBtoHSL (color, &H, &S, &L);

		color = HLStoRGB_ONE (H, min (1., L * dblLumRatio), S);
	}

	return color;
}
