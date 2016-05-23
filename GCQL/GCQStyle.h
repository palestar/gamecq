// FunnyStyle.h: interface for the CGCQStyle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNNYSTYLE_H__B895C338_1BB4_415B_B79A_BF53BC9BB036__INCLUDED_)
#define AFX_FUNNYSTYLE_H__B895C338_1BB4_415B_B79A_BF53BC9BB036__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGCQStyle : public CBCGVisualManager  
{
public:
	CGCQStyle();
	virtual ~CGCQStyle();

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, 
									CControlBar* pBar);
	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnHighlightMenuItem (CDC* pDC, CBCGToolbarMenuButton* pButton,
									CRect rect, COLORREF& clrText);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bHorz);
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);

	virtual void OnFillCaptionBackground( CDC * pDC, CRect rect, const CBCGCaptionBar * pCaptionBar );

	virtual void GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
};

#endif // !defined(AFX_FUNNYSTYLE_H__B895C338_1BB4_415B_B79A_BF53BC9BB036__INCLUDED_)
