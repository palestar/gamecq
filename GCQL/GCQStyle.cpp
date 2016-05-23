// FunnyStyle.cpp: implementation of the CGCQStyle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GCQL.h"
#include "GCQStyle.h"
#include "Globals.h"		// BBGControlBar globals

//----------------------------------------------------------------------------

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

//----------------------------------------------------------------------------

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGCQStyle::CGCQStyle()
{
	// control the style of this application
	globalData.clrGrayedText = CGCQLApp::sm_GrayedTextColor;
	globalData.clrWindowText = globalData.clrCaptionText = globalData.clrMenuText = 
		globalData.clrBarText = globalData.clrBtnText = CGCQLApp::sm_TextColor;
	globalData.clrWindowFrame = CGCQLApp::sm_FrameColor;
	globalData.clrBtnDkShadow = CGCQLApp::sm_ButtonFrame1;
	globalData.clrBtnLight = CGCQLApp::sm_ButtonFrame2;
	//globalData.clrBtnFace = RGB( 0,0,255);
	globalData.clrBtnShadow = CGCQLApp::sm_BackgroundColor1;
	globalData.clrBtnHilite = CGCQLApp::sm_ButtonHighlight;
	globalData.clrWindow = CGCQLApp::sm_BackgroundColor1;

	/*
	int colorElements[] = 
	{
		COLOR_BACKGROUND,
		COLOR_BTNTEXT,
	};
	const int elements = sizeof(colorElements) / sizeof(int);
	COLORREF elementColors[ elements ] = 
	{
		CGCQLApp::sm_BackgroundColor1,
		CGCQLApp::sm_TextColor,
	};

	SetSysColors( elements, colorElements, elementColors );
	*/
}

//****************************************************************************************
CGCQStyle::~CGCQStyle()
{

}

//****************************************************************************************
void CGCQStyle::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CControlBar* pBar)
{
	if (bHorz)
		rectGripper.DeflateRect (3, 4);
	else
		rectGripper.DeflateRect (4, 3);

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rectGripper, CGCQLApp::sm_GripperColor1, CGCQLApp::sm_GripperColor2, bHorz);

	if (bHorz)
		rectGripper.InflateRect (1, 0);
	else
		rectGripper.InflateRect (0, 1);

	dm.DrawShadow (rectGripper, 3);
}
//****************************************************************************************
void CGCQStyle::OnFillBarBackground  (CDC* pDC, CControlBar* pBar,
										CRect rectClient, CRect rectClip,
										BOOL bNCArea)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (!bNCArea)
	{
		CRgn rgn;
		rgn.CreateRectRgnIndirect (&rectClient);

		pDC->SelectClipRgn (&rgn);
	}

	CBCGDrawManager dm (*pDC);

	CRect rectFill = rectClient;

	if (!pBar->IsFloating () &&
		!pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
	{
		CRect rectMainFrame;
		pBar->GetTopLevelFrame ()->GetWindowRect (rectMainFrame);

		pBar->ScreenToClient (&rectMainFrame);
		rectFill = rectMainFrame;

		if (bNCArea)
		{
			CRect rectWindow;
			pBar->GetWindowRect (rectWindow);

			pBar->ScreenToClient (rectWindow);

			CRect rectClientActual;
			pBar->GetClientRect (rectClientActual);

			rectFill.left += rectClientActual.left - rectWindow.left;
			rectFill.top += rectClientActual.top - rectWindow.top;
			rectFill.right += 10;
		}
	}

	dm.FillGradient (rectFill, CGCQLApp::sm_BackgroundColor1, CGCQLApp::sm_BackgroundColor2, FALSE);

	if (!bNCArea)
	{
		pDC->SelectClipRgn (NULL);
	}

	//pDC->SetTextColor( RGB(255,255,255) );
}
//************************************************************************************
void CGCQStyle::OnHighlightMenuItem (CDC* pDC, CBCGToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	ASSERT_VALID (pDC);

	CBCGDrawManager dm (*pDC);

	rect.DeflateRect (1, 2);
	dm.FillGradient (rect, CGCQLApp::sm_BackgroundColor2 , CGCQLApp::sm_BackgroundColor1, FALSE);

	rect.InflateRect (0, 1);
	dm.DrawShadow (rect, 3);

	clrText = RGB (255, 255, 0);
}
//**************************************************************************************
void CGCQStyle::OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bHorz)
{
	rect.DeflateRect (2, 2);
	CBCGVisualManager::OnDrawSeparator (pDC, pBar, rect, bHorz);
}
//**************************************************************************************
void CGCQStyle::OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* /*pTabWnd*/)
{
	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rect, CGCQLApp::sm_BackgroundColor1, CGCQLApp::sm_BackgroundColor2, FALSE);
}

void CGCQStyle::OnFillCaptionBackground( CDC * pDC, CRect rect, const CBCGCaptionBar * pCaptionBar )
{
	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rect, CGCQLApp::sm_BackgroundColor1, CGCQLApp::sm_BackgroundColor2, FALSE);
}

void CGCQStyle::GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				COLORREF& clrDark,
				COLORREF& clrBlack,
				COLORREF& clrHighlight,
				COLORREF& clrFace,
				COLORREF& clrDarkShadow,
				COLORREF& clrLight,
				CBrush*& pbrFace,
				CBrush*& pbrBlack)
{
	CBCGVisualManager::GetTabFrameColors( pTabWnd, clrDark, clrBlack, clrHighlight, clrFace,
		clrDarkShadow, clrLight, pbrFace, pbrBlack );

	clrDark = CGCQLApp::sm_BackgroundColor1;
	clrBlack = CGCQLApp::sm_BackgroundColor2;
	clrHighlight = CGCQLApp::sm_ButtonHighlight;
	clrFace = CGCQLApp::sm_StatusColor;
	clrDarkShadow = CGCQLApp::sm_ButtonFrame1;
	clrLight = CGCQLApp::sm_ButtonFrame2;
}

