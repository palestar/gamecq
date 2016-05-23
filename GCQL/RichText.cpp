// RichText.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "RichText.h"

#include "Standard/Limits.h"



#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRichText

IMPLEMENT_DYNCREATE(CRichText, CScrollView)

CRichText::CRichText()
{
	//m_FontName = "Arial";
	m_FontName = "Verdana";
	m_FontSize = 100;
	m_ForgroundColor = 0xffffff;	// white
	m_BackgroundColor = 0x000000;	// black
	m_ScrollToBottom = false;
	m_ContentHeight = 0;
}

CRichText::~CRichText()
{}


BEGIN_MESSAGE_MAP(CRichText, CScrollView)
	//{{AFX_MSG_MAP(CRichText)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRichText drawing

void CRichText::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

	m_ContentHeight = 0;
}

inline CString GetToken( CString & tokenize, const TCHAR * pWhiteSpace )
{
	tokenize.TrimLeft( pWhiteSpace );	// remove any leading whitespaces

	// find the next whitespace
	int whiteIndex = tokenize.FindOneOf( pWhiteSpace );
	ASSERT( whiteIndex != 0 );

	CString token;
	if ( whiteIndex < 0 )
	{
		token = tokenize;
		tokenize.Empty();
	}
	else
	{
		token = tokenize.Left( whiteIndex );
		tokenize = tokenize.Right( tokenize.GetLength() - whiteIndex );
	}

	token.TrimLeft();
	token.TrimRight();
	return token;
}

inline char	GetChar( CString & text, int index )
{
	if ( index < text.GetLength() )
		return (char)text[ index ];
	return 0;
}

const TCHAR *	TAG_WHITE = _T(";");
const TCHAR *	CMD_COLOR = _T("COLOR");						// set the color, usage: <Color;FF00FFFF>
const TCHAR *	CMD_DEFAULT_COLOR = _T("/COLOR");				// restore default color, usage <DefaultColor>
const TCHAR *	CMD_X = _T("X");								// set the current x position, usage: <X;50>
const TCHAR *	CMD_Y = _T("Y");								// set the current Y position, usage: <Y;50>
const TCHAR *	CMD_BOLD = _T("B");
const TCHAR *	CMD_NOT_BOLD = _T("/B");
const TCHAR *	CMD_INC = _T("+");
const TCHAR *	CMD_DEC = _T("-");
const int		TAB_SIZE = 60;

void CRichText::OnDraw(CDC* pDC)
{
	// create the default font
	CFont font;
	if (! font.CreatePointFont( m_FontSize, m_FontName, pDC ) )
		return;

	// get the text to display
	CString text;
	GetWindowText( text );

	int previousHeight = m_ContentHeight;
	m_ContentHeight = 0;
	
	// get the bounds of this component
	CRect textWindow;
	GetClientRect( textWindow );

	// default the font size to a 10 point
	CFont * pPreviousFont = pDC->SelectObject( &font );

	// set the current font
	pDC->SetTextColor( m_ForgroundColor );
	pDC->SetBkColor( m_BackgroundColor );

	// current line position
	CPoint			linePosition( 0, 0 ); //-GetDeviceScrollPosition().y );
	long			lineWidth = 0;
	long			lineHeight = pDC->GetTextExtent( _T(" ") ).cy;
	CSize 			windowSize( textWindow.Size() );
	
	bool			flushLine = false;
	bool			nextLine = false;
	bool			done = false;
	bool			parseTag = false;
	CString			tag = _T("");
	CString			line = _T("");
	
	int textIndex = 0;
	while(!done)
	{
		char character = GetChar( text, textIndex++ );
		while( true )
		{
			if ( character == 0 )
				break;
			if ( character == '<' )
			{
				if ( GetChar( text, textIndex ) != '<' )
					break;		// parse the tag
				textIndex++;	
			}
			if ( character == '\n' )
				break;
			
			if ( character != '\r' )
			{
				// add character to output line
				line += character;

				CSize extents( pDC->GetTextExtent( line ) );
				lineWidth = extents.cx;
				lineHeight = Max( extents.cy, lineHeight );
			
				// check the total line width, break if we need to word-wrap
				if ( (linePosition.x + lineWidth) > windowSize.cx )
					break;
			}
			
			character = GetChar( text, textIndex++ );
		}

		switch( character )
		{
		case 0:		// null terminator
			flushLine = true;
			done = true;
			break;
		case '\n':	// newline
			nextLine = true;
			break;
		case '<':	// begin tag
			{
				while( true )
				{
					character = GetChar( text, textIndex++ );
					if ( character == '>' )	// find end tag
					{
						parseTag = true;
						flushLine = true;	// flush the line before we process the tag
						break;
					}
					else if ( character == 0 )
					{
						flushLine = true;
						done = true;
						break;
					}
					else
						tag += character;
				}
			}
			break;
		}

		bool overflow = (linePosition.x + lineWidth) > windowSize.cx;
		if ( flushLine || nextLine || overflow )
		{
			CString wrapped = _T("");		// characters to be moved to the next line

			// handle word wrapping
			if ( overflow )
			{
				int lastWhite = line.ReverseFind( ' ' );
				if ( lastWhite > 0 )
				{
					wrapped = line.Mid( lastWhite );
					// terminate the line at the last whitespace
					line = line.Mid( 0, lastWhite );

					// recalculate the lineWidth
					CSize extents( pDC->GetTextExtent( line ) );
					lineWidth = extents.cx;
					lineHeight = Max( extents.cy, lineHeight );
				}
				nextLine = true;
			}

			// draw the text, this will advance linePosition automatically
			pDC->TextOut( linePosition.x, linePosition.y, line );
			linePosition.x += lineWidth;

			if ( nextLine )
			{
				linePosition.x = 0;
				if ( overflow )
					linePosition.x += TAB_SIZE;
				linePosition.y += lineHeight;
				m_ContentHeight += lineHeight;

				lineHeight = pDC->GetTextExtent( _T(" ") ).cy;
			}

			line = wrapped;

			CSize extents( pDC->GetTextExtent( line ) );
			lineWidth = extents.cx;
			lineHeight = Max( extents.cy, lineHeight );

			flushLine = nextLine = false;
		}

		if ( parseTag )
		{
			while ( tag.GetLength() > 0 )
			{
				CString cmd = GetToken( tag, TAG_WHITE );
				
				if ( cmd.CompareNoCase( CMD_X ) == 0 )
				{
					if ( tag.GetLength() > 0 )
					{
						CString arg = GetToken( tag, TAG_WHITE );
						linePosition.x = _ttoi( arg );
					}
				}
				else if ( cmd.CompareNoCase( CMD_Y ) == 0 )
				{
					if ( tag.GetLength() > 0 )
					{
						CString arg = GetToken( tag, TAG_WHITE );
						linePosition.y = _ttoi( arg );
					}
				}
				else if ( cmd.CompareNoCase( CMD_COLOR ) == 0 )		
				{
					if ( tag.GetLength() > 0 )
					{
						CString arg = GetToken( tag, TAG_WHITE );

						unsigned long color = _tcstoul( arg, NULL, 16 );
						pDC->SetTextColor( color );
					}
				}
				else if ( cmd.CompareNoCase( CMD_DEFAULT_COLOR ) == 0 )	
				{
					// restore default color
					pDC->SetTextColor( 0xffffff );
				}
				else if ( cmd.CompareNoCase( CMD_BOLD ) == 0 ||
					cmd.CompareNoCase( CMD_INC ) == 0)
				{
					LOGFONT lf;
					pDC->GetCurrentFont()->GetLogFont( &lf );

					lf.lfWeight = FW_BOLD;

					pDC->SelectObject( pPreviousFont );
					font.DeleteObject();
					font.CreateFontIndirect( &lf );
					pPreviousFont = pDC->SelectObject( &font );
				}
				else if ( cmd.CompareNoCase( CMD_NOT_BOLD ) == 0 ||
					cmd.CompareNoCase( CMD_DEC ) == 0)
				{
					LOGFONT lf;
					pDC->GetCurrentFont()->GetLogFont( &lf );

					lf.lfWeight = FW_NORMAL;

					pDC->SelectObject( pPreviousFont );
					font.DeleteObject();
					font.CreateFontIndirect( &lf );
					pPreviousFont = pDC->SelectObject( &font );
				}
			}
		}
	}
	
	// restore the original font
	pDC->SelectObject( pPreviousFont );

	// update the scroll bar
	if ( m_ContentHeight != previousHeight )
	{
		CSize contentSize( 100, m_ContentHeight );
		SetScrollSizes(MM_TEXT, contentSize);

		if ( m_ScrollToBottom )
		{
			if ( windowSize.cy < m_ContentHeight )
				SetScrollPos( SB_VERT, m_ContentHeight );
			else
				SetScrollPos( SB_VERT, 0 );
			RedrawWindow();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRichText diagnostics

#ifdef _DEBUG
void CRichText::AssertValid() const
{
	CScrollView::AssertValid();
}

void CRichText::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRichText message handlers

BOOL CRichText::OnEraseBkgnd(CDC* pDC) 
{
	CRect fillRect;
	GetClientRect( &fillRect );

	pDC->FillSolidRect( fillRect, m_BackgroundColor );
	return true;
	//return CScrollView::OnEraseBkgnd(pDC);
}
