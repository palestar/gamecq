// FilterText.h: interface for the CFilterText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILTERTEXT_H__B0320C04_9058_11D6_9411_00001CDB2E9A__INCLUDED_)
#define AFX_FILTERTEXT_H__B0320C04_9058_11D6_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Standard/String.h"



class CFilterText  
{
public:
	CFilterText();
	virtual ~CFilterText();

	static CString enhance( CString sText );		// modifies the text in suitable way to be displayed in a HTMLView
	static String limitColorRange( String sText );	// strips invalid BBCode colors out of the string
	static int checkColor( unsigned long color );	// -1: too dark, 0: ok, 1: too bright

private: 
	static long hexToColorLong( String hex );
	static String convertPairs( String text, String regExp1, String subExpr1, String regExp2, String subExpr2 );

};

#endif // !defined(AFX_FILTERTEXT_H__B0320C04_9058_11D6_9411_00001CDB2E9A__INCLUDED_)
