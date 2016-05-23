// CaptionBar.h: interface for the CCaptionBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAPTIONBAR_H__3BB1C28D_74AC_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CAPTIONBAR_H__3BB1C28D_74AC_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCaptionBar : public CBCGCaptionBar  
{
public:
	CCaptionBar();
	virtual ~CCaptionBar();

	// Overridables
	void OnDrawBackground(CDC*pDC,CRect rect);

	// Mutators
	void	UpdateText( const TCHAR * pText );
};

#endif // !defined(AFX_CAPTIONBAR_H__3BB1C28D_74AC_11D5_BA96_00C0DF22DE85__INCLUDED_)
