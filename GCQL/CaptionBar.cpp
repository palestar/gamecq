// CaptionBar.cpp: implementation of the CCaptionBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GCQL.h"
#include "GCQStyle.h"
#include "CaptionBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCaptionBar::CCaptionBar() 
{}

CCaptionBar::~CCaptionBar()
{}


void CCaptionBar::OnDrawBackground(CDC*pDC,CRect rect)
{
	((CGCQStyle *)CBCGVisualManager::GetInstance())->OnFillCaptionBackground( pDC, rect, this );
}

void CCaptionBar::UpdateText( const TCHAR * pText )
{
	SetWindowText( pText );

	m_strText = pText;
	Invalidate();
}
