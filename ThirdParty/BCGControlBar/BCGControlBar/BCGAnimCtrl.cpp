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

// BCGAnimCtrl.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_ANIMCONTROL

#include "BCGAnimCtrl.h"
#include "BCGPopupMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iAnimEventId = 1;

IMPLEMENT_DYNAMIC(CBCGAnimCtrl, CStatic)

/////////////////////////////////////////////////////////////////////////////
// CBCGAnimCtrl

CBCGAnimCtrl::CBCGAnimCtrl()
{
	m_clrBack = (COLORREF)-1;
	m_sizeFrame = CSize (0, 0);
	m_iCurrFrame = 0;
	m_iFrameCount = -1;
	m_pImagesAnim = NULL;
	m_bIsRunning = FALSE;
	m_uiFrameRate = 500;
}

CBCGAnimCtrl::~CBCGAnimCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGAnimCtrl, CStatic)
	//{{AFX_MSG_MAP(CBCGAnimCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGAnimCtrl message handlers

BOOL CBCGAnimCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//**************************************************************************************
void CBCGAnimCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == iAnimEventId)
	{
		if (++ m_iCurrFrame == m_iFrameCount)
		{
			m_iCurrFrame = 0;
		}

		Invalidate (FALSE);
		UpdateWindow ();
	}

	CRect rectScreen;
	GetWindowRect (&rectScreen);
	CBCGPopupMenu::UpdateAllShadows (rectScreen);

	CStatic::OnTimer(nIDEvent);
}
//**************************************************************************************
void CBCGAnimCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CDC*		pDC = &dc;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	CRect rect;
	GetClientRect (rect);

	if (dcMem.CreateCompatibleDC (&dc) &&
		bmp.CreateCompatibleBitmap (&dc, rect.Width (),
								  rect.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

	if (m_clrBack != (COLORREF)-1)
	{
		CBrush brBack (m_clrBack);
		pDC->FillRect (rect, &brBack);
	}
	else
	{
		globalData.DrawParentBackground (this, pDC);
	}

	if (m_pImagesAnim != NULL &&
		m_pImagesAnim->GetSafeHandle () != NULL)
	{
		m_pImagesAnim->Draw (pDC, m_iCurrFrame, 
			CPoint ((rect.Width () - m_sizeFrame.cx) / 2,
					(rect.Height () - m_sizeFrame.cy) / 2),
			ILD_NORMAL);
	}

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		CRect rectClip;
		int nClipType = dc.GetClipBox (rectClip);
		if (nClipType != NULLREGION)
		{
			if (nClipType != SIMPLEREGION)
			{
				rectClip = rect;
			}

			dc.BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
						   &dcMem, rectClip.left, rectClip.top, SRCCOPY);
		}

		dcMem.SelectObject(pOldBmp);
	}
}
//**************************************************************************************
BOOL CBCGAnimCtrl::SetBitmap (UINT uiBmpId, int nFrameWidth,
							  COLORREF clrTransparent,
							  BOOL bSizeToContent)
{
	ASSERT_VALID (this);

	if (nFrameWidth <= 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		return FALSE;
	}

	if (m_imagesAnim.GetSafeHandle () != NULL)
	{
		m_imagesAnim.DeleteImageList ();
	}

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_imagesAnim.Create (nFrameWidth, bmpObj.bmHeight, nFlags, 0, 0);
	m_imagesAnim.Add (&bmp, clrTransparent);

	return SetBitmap (&m_imagesAnim, bSizeToContent);
}
//**************************************************************************************
BOOL CBCGAnimCtrl::SetBitmap (CImageList* pImagesAnim, BOOL bSizeToContent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pImagesAnim);

	if (m_imagesAnim.GetSafeHandle () != NULL &&
		m_imagesAnim.GetSafeHandle () != pImagesAnim->GetSafeHandle ())
	{
		::DeleteObject (m_imagesAnim.Detach ());
	}

	if (m_bIsRunning)
	{
		KillTimer (iAnimEventId);
		m_bIsRunning = FALSE;
	}

	m_pImagesAnim = pImagesAnim;

	IMAGEINFO imageInfo;
	pImagesAnim->GetImageInfo (0, &imageInfo);

	CRect rectImage = imageInfo.rcImage;

	m_sizeFrame.cx = rectImage.Width ();
	m_sizeFrame.cy = rectImage.Height ();

	m_iFrameCount = pImagesAnim->GetImageCount ();

	if (bSizeToContent)
	{
		SetWindowPos (NULL, -1, -1, m_sizeFrame.cx, m_sizeFrame.cy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	Invalidate ();
	UpdateWindow ();

	return TRUE;
}
//**************************************************************************************
BOOL CBCGAnimCtrl::Play (UINT uiFrameRate)
{
	ASSERT_VALID (this);

	if (uiFrameRate != 0)
	{
		m_uiFrameRate = uiFrameRate;
	}

	if (m_pImagesAnim == NULL ||
		m_pImagesAnim->GetSafeHandle () == NULL ||
		m_bIsRunning)
	{
		return FALSE;
	}

	SetTimer (iAnimEventId, m_uiFrameRate, NULL);
	m_bIsRunning = TRUE;

	return TRUE;
}
//**************************************************************************************
BOOL CBCGAnimCtrl::Stop ()
{
	if (m_pImagesAnim == NULL ||
		m_pImagesAnim->GetSafeHandle () == NULL ||
		!m_bIsRunning)
	{
		return FALSE;
	}

	KillTimer (iAnimEventId);
	m_iCurrFrame = 0;
	m_bIsRunning = FALSE;

	Invalidate ();
	UpdateWindow ();

	return TRUE;
}
//*******************************************************************************
void CBCGAnimCtrl::SetFrameRate (UINT uiFrameRate)
{
	ASSERT_VALID (this);

	if (uiFrameRate == 0)
	{
		ASSERT (FALSE);
		return;
	}

	if (m_uiFrameRate != uiFrameRate)
	{
		m_uiFrameRate = uiFrameRate;

		if (m_bIsRunning)
		{
			KillTimer (iAnimEventId);
			SetTimer (iAnimEventId, uiFrameRate, NULL);
		}
	}
}

#endif // BCG_NO_ANIMCONTROL
