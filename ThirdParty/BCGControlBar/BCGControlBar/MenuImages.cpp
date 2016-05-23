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

// MenuImages.cpp: implementation of the CMenuImages class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MenuImages.h"
#include "bcglocalres.h"
#include "bcgbarres.h"

static const COLORREF clrTransparent = RGB (255, 0, 255);
static const int iImageWidth = 9;
static const int iImageHeight = 9;

CBCGToolBarImages CMenuImages::m_Images;

BOOL CMenuImages::Initialize ()
{
	if (!m_Images.IsValid ())
	{
		CBCGLocalResource locaRes;
		m_Images.SetImageSize (CSize (iImageWidth, iImageHeight));
		if (!m_Images.Load (IDB_BCGBARRES_MENU_IMAGES))
		{
			TRACE(_T("CMenuImages. Can't load menu images %x\n"), IDB_BCGBARRES_MENU_IMAGES);
			return FALSE;
		}
		
		if (m_Images.IsRTL ())
		{
			m_Images.Mirror ();
		}

		m_Images.SetTransparentColor (clrTransparent);
	}

	return TRUE;
}
//****************************************************************************************
void CMenuImages::Draw (CDC* pDC, IMAGES_IDS id, const CPoint& ptImage,
						const CSize& sizeImage/* = CSize (0, 0)*/)
{
	if (!Initialize ())
	{
		return;
	}

	CBCGDrawState ds;

	m_Images.PrepareDrawImage (ds, sizeImage);
	m_Images.Draw (pDC, ptImage.x, ptImage.y, id);
	m_Images.EndDrawImage (ds);
}
//****************************************************************************************
void CMenuImages::Draw (CDC* pDC, CMenuImages::IMAGES_IDS id, const CRect& rectImage,
					  const CSize& sizeImageDest/* = CSize (0, 0)*/)
{
	const CSize sizeImage = 
		(sizeImageDest == CSize (0, 0)) ? Size () : sizeImageDest;

	CPoint ptImage (
		rectImage.left + (rectImage.Width () - sizeImage.cx) / 2 + ((rectImage.Width () - sizeImage.cx) % 2), 
		rectImage.top + (rectImage.Height () - sizeImage.cy) / 2 + ((rectImage.Height () - sizeImage.cy) % 2));

	Draw (pDC, id, ptImage, sizeImageDest);
}
//*************************************************************************************
void CMenuImages::CleanUp ()
{
	if (m_Images.GetCount () > 0)
	{
		m_Images.Clear ();
	}
}
