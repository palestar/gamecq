// yam.cpp : Defines the class behaviors for the application.

#include "stdafx.h"
#include "YamDlg.h"

#include "PWCipher.h"

#include "GCQ/MetaClient.h"
#include "Standard/Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CYamApp, CWinApp)
	//{{AFX_MSG_MAP(CYamApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP();


////////////////////////////////////////////////////////////////////////
// Globals 

// The one and only CYamApp object
CYamApp theApp;

MetaClient			CYamApp::s_MetaClient;				// our connection to the meta client
dword				CYamApp::s_SessionID;				// our session id
MetaClient::Game	CYamApp::s_Game;					// our game information


////////////////////////////////////////////////////////////////////////
// Ctor

CYamApp::CYamApp()
{
}


//// InitInstance //////////////////////////////////////////////////////
// CYamApp initialization

BOOL CYamApp::InitInstance()
{
	if( !openCon( true ) )
	{
		MessageBox( NULL, "Error during startup, quitting.", "Failed", MB_OK );
		return FALSE;
	}

	TRACE( "Opening mainwindow..." );

	// Create the main window.
	CYamDlg dlg(0);
	m_pMainWnd = &dlg;
	dlg.DoModal();

	TRACE( "Closing..." );

	if( s_MetaClient.loggedIn() )
		s_MetaClient.logoff();

	if( s_MetaClient.connected() )
		s_MetaClient.close();

	return FALSE;
}

bool CYamApp::doLogin()
{
	TRACE( "1..." );
	gotoHomeDirectory();
	Settings settings( "GCQL" );
	TRACE( "2..." );
	Thread::sleep( 4000 );
	if( settings.get( "loginCount", (dword)0 ) < 1 )
		return false;
	TRACE( "3..." );
	
	CString loginName = settings.get( "previousUID", "" );
	if( loginName.GetLength() == 0 )
		return false;
	TRACE( "4..." );

	if ( settings.get( "rememberPW", (dword)0 ) == 0 )
		return false;
	TRACE( "5..." );

	CString sTempPw = settings.get( "previousPW", "" );
	TRACE( "6..." );
	if( sTempPw.Find("[ECM]" ) == 0 && sTempPw.GetLength() > 37 )	// is stored PW already encryped ?
	{
		sTempPw = sTempPw.Right( sTempPw.GetLength() - 5 );	// cut off the tag
		CPWCipher pwdecode;
		sTempPw = pwdecode.Decode( sTempPw );
	}
		
	TRACE( "Right before logging in..." );

	switch( s_MetaClient.login( loginName, sTempPw ) )
	{
		case MetaClient::LOGIN_OKAY:
			break;
		case MetaClient::LOGIN_BANNED:
			MessageBox( NULL, "This account is currently banned from GameCQ", "Banned", MB_OK );
			s_MetaClient.logoff();
			return false;
		case MetaClient::LOGIN_DUPLICATE_LOGIN:
			MessageBox( NULL, "This account is already connected to GameCQ", "Duplicate Login", MB_OK );
			s_MetaClient.logoff();
			return false;
		case MetaClient::LOGIN_FAILED:
			MessageBox( NULL, "Invalid username or password", "Login Failed", MB_OK );
			return false;
		default:
			MessageBox( NULL, "A error occured on the server, please try again later", "Error", MB_OK );
			s_MetaClient.logoff();
			return false;
	}

	TRACE( "Login done..." );

	return true;
}

void CYamApp::gotoHomeDirectory()
{
	char szBuffer[_MAX_PATH]; 
	VERIFY(::GetModuleFileName( AfxGetInstanceHandle(), szBuffer, _MAX_PATH) );
	CString sPath = (CString)szBuffer;
	sPath = sPath.Left( sPath.ReverseFind('\\') );

	SetCurrentDirectory( sPath );
}

bool CYamApp::openCon(bool bWarnings)
{
	TRACE( "Opening..." );
	if ( s_MetaClient.open() < 0 )
	{
		if( bWarnings )
			MessageBox( NULL, "Failed to connect to the GameCQ server; please try again later.", "Failed", MB_OK );
		return FALSE;
	}


	TRACE( "Getting games..." );

	Array< MetaClient::Game > games;
	if ( s_MetaClient.getGames( games ) < 0 || games.size() < 1 )
	{
		if( bWarnings )
			MessageBox( NULL, "Failed to get game information!", "Failed", MB_OK );
		s_MetaClient.logoff();
		return FALSE;
	}

	bool gameFound = false;
	for(int i=0;i<games.size();i++)
		if ( games[i].id == 1 )
		{
			gameFound = true;
			s_Game = games[i];
			break;
		}

	if (! gameFound )
		s_Game = games[ 0 ];
	
	TRACE( "Got game..." );

	TRACE( "Logging in..." );

	if (! doLogin() )
		return FALSE;

	TRACE( "Logged in..." );

	// save my session id
	s_SessionID = s_MetaClient.profile().sessionId;

	TRACE( "Selecting game..." );

	// select the game
	if ( s_MetaClient.selectGame( s_Game.id ) < 0 )
		return FALSE;


	TRACE( "Setting status..." );

	s_MetaClient.sendStatus( 0, "Using 'YAM'" );

	return TRUE;
}
