/*
	FilterText.h
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef FILTERTEXT_H
#define FILTERTEXT_H

#include "Standard/CharString.h"

//----------------------------------------------------------------------------

class FilterText  
{
public:
	static CharString		ansi( const char * pText );					// strip all HTML codes from text, make it plain ansi...
	static CharString		enhance( const char * pText );				// modifies the text in suitable way to be displayed in a HTMLView
	static CharString		limitColorRange( const char * pText );		// strips invalid BBCode colors out of the string
	static int				checkColor( unsigned long color );			// -1: too dark, 0: ok, 1: too bright

private: 
	static long				hexToColorLong( const char * pHex );

	static CharString		convertPairs( const char * pText, 
								const char * regExp1, 
								const char * subExpr1, 
								const char * regExp2, 
								const char * subExpr2 );
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
