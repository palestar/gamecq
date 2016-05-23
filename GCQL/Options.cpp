// Options.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "Options.h"
#include "EditEmotion.h"

#include "Standard/Settings.h"



#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent /*=NULL*/)
	: CPropertySheet(_T("Options"), pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_psh.dwFlags |= PSP_USEHICON;
	m_psh.hIcon = m_hIcon;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;    // Lose the Apply Now button
	m_psh.dwFlags &= ~PSH_HASHELP;  // Lose the Help button

	AddPage( &m_General );
	AddPage( &m_Emotions );
	AddPage( &m_Colors );

	//{{AFX_DATA_INIT(COptions)
	//}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(COptions, CPropertySheet)
	//{{AFX_MSG_MAP(COptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::RestoreDefaultEmotions()
{
	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );

	int count = settings.get( "EmotionNum", (dword)0 );
	if ( count == 0 )
	{
		// write default emotions to configuration file
		const char * DEFAULT_EMOTIONS[] = 
		{
			"Clap",		"/me begins applauding $d...",
			"Slap",		"/me slaps $d in the head...",
			"Smile",	"/me smiles at $d...",		
			"Wink",		"/me winks suspiciously at $d...",
			"Laugh",	"/me laughs at $d...",		
			"Giggle",	"/me giggles at $d...",		
			"Nod",		"/me nods at $d...",			
			"Wave",		"/me waves to $d...",		
			"Hug",		"/me gives $d a great big hug...",
			"Kiss",		"/me kisses $d...",			
			NULL
		};

		int count = 0;
		while ( DEFAULT_EMOTIONS[ count ] != NULL )
		{
			settings.put( CharString().format("Emotion%d", count), DEFAULT_EMOTIONS[ count ] );
			count++;
		}
		settings.put( "EmotionNum", count );
	}
}

//----------------------------------------------------------------------------


