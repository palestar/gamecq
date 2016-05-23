// GCQL.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GCQL.h"

#include "GCQLDoc.h"
#include "GCQStyle.h"

#include "ProgDlg.h"
#include "Eula.h"
#include "Login.h"
#include "NewLogin.h"
#include "WebView.h"
#include "ChatFrame.h"
#include "ChangePassword.h"
#include "ChangeName.h"
#include "ChangeEmail.h"
#include "WebFrame.h"
#include "FindUser.h"
#include "Options.h"
#include "Ignores.h"
#include "CacheList.h"
#include "MainFrame.h"

#include "SelfUpdate/ClientUpdate.h"

#include "Debug/Trace.h"
#include "Standard/Process.h"
#include "Standard/Time.h"
#include "Standard/CommandLine.h"
#include "Standard/Settings.h"
#include "System/Locale.h"
#include "File/FileDisk.h"

#include "Resource.h"

//----------------------------------------------------------------------------

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

const UINT					MSG_GCQL_COMMAND = RegisterWindowMessage( _T( "{1ACC295B-36B1-4b57-8C9C-D748A34B33D0}" ) );

//----------------------------------------------------------------------------

CChatFrame *		CGCQLApp::sm_pChatFrame = NULL;
CWebFrame *			CGCQLApp::sm_pWebFrame = NULL;
CServerFrame *		CGCQLApp::sm_pServerFrame = NULL;
CCacheFrame *		CGCQLApp::sm_pCacheFrame = NULL;

MetaClient			CGCQLApp::sm_MetaClient;				// our connection to the meta client
dword				CGCQLApp::sm_SessionID;				// our session id
MetaClient::Game	CGCQLApp::sm_Game;					// our game information
dword				CGCQLApp::sm_LoginTime;
CChatLog			CGCQLApp::s_ChatLog;				// reference to the ChatLog

BOOL				CGCQLApp::sm_bBtnDown;
POINT				CGCQLApp::sm_ptMouseDown;				


// Settings
int					CGCQLApp::sm_ChatBufferSize = 1024 * 32;
int					CGCQLApp::sm_TextSize = 8;
bool				CGCQLApp::sm_bEnableWordFilter = true;
COLORREF			CGCQLApp::sm_ChatColor = RGB( 255, 255, 0 );
bool				CGCQLApp::sm_bChatSound = true;
bool				CGCQLApp::sm_bMessageSound = true;
bool				CGCQLApp::sm_bTaskBarMessages = true;
int					CGCQLApp::sm_RefreshGameListTime = 60;
bool				CGCQLApp::sm_RoomAnnounce = false;
bool				CGCQLApp::sm_AlwaysLog = false;
dword				CGCQLApp::sm_AutoAwayTime = 900;
dword				CGCQLApp::sm_Language = 0;
bool				CGCQLApp::sm_AutoLogin = false;
bool				CGCQLApp::sm_bMinimized = true;
bool				CGCQLApp::sm_bEnableAutoAway = true;
dword				CGCQLApp::sm_nUpdateCheckInterval = 60 * 60;
CharString			CGCQLApp::sConfigName = "GCQL";

COLORREF			CGCQLApp::sm_BackgroundColor1 = RGB (32, 32, 32);
COLORREF			CGCQLApp::sm_BackgroundColor2 = RGB (48, 48, 48);
COLORREF			CGCQLApp::sm_FrameColor = RGB(64,64,64);		// WindowFrame
COLORREF			CGCQLApp::sm_ButtonFrame1 = RGB(16,16,16);	// DkShadow
COLORREF			CGCQLApp::sm_ButtonFrame2 = RGB(32,32,32);	// BtnLight
COLORREF			CGCQLApp::sm_ButtonHighlight = RGB (64, 64, 64);	// BtnHilight
COLORREF			CGCQLApp::sm_GripperColor1 = RGB (32, 32, 32);
COLORREF			CGCQLApp::sm_GripperColor2 = RGB (255, 255, 255);
COLORREF			CGCQLApp::sm_TextColor = RGB( 255, 255, 255 );
COLORREF			CGCQLApp::sm_GrayedTextColor = RGB( 150, 64, 64 );
COLORREF			CGCQLApp::sm_StatusColor = RGB( 128, 128, 128 );

//----------------------------------------------------------------------------

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp

BEGIN_MESSAGE_MAP(CGCQLApp, CWinApp)
	//{{AFX_MSG_MAP(CGCQLApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_GAMECQ_CHANGEPASSWORD, OnGamecqChangepassword)
	ON_COMMAND(ID_GAMECQ_CHANGEUSERNAME, OnGamecqChangeusername)
	ON_COMMAND(ID_GAMECQ_CHANGEEMAIL, OnGamecqChangeemail)
	ON_COMMAND(ID_GAMECQ_SAVECHAT, OnGamecqSavechat)
	ON_UPDATE_COMMAND_UI(ID_GAMECQ_SAVECHAT, OnUpdateGamecqSavechat)
	ON_COMMAND(ID_GAMECQ_SAVECHAT_AS, OnGamecqSavechatAs)
	ON_UPDATE_COMMAND_UI(ID_GAMECQ_SAVECHAT_AS, OnUpdateGamecqSavechatAs)
	ON_COMMAND(ID_GAMECQ_STOPCHATLOG, OnGamecqStopchatlog)
	ON_UPDATE_COMMAND_UI(ID_GAMECQ_STOPCHATLOG, OnUpdateGamecqStopchatlog)
	ON_COMMAND(ID_EDIT_FINDUSERS, OnEditFindusers)
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_IGNORES, OnViewIgnores)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_VIEW_HOME, OnViewHome)
	ON_COMMAND(ID_VIEW_CHAT, OnViewChat)
	ON_COMMAND(ID_VIEW_GAMES, OnViewGames)
	ON_COMMAND(ID_VIEW_SERVERS, OnViewServers)
	ON_COMMAND(ID_HELP_MANUALONLINE, OnViewManual)
	ON_COMMAND(ID_HELP_FAQ, OnViewFAQ)
	ON_COMMAND(ID_HELP_SUPPORTONLINE, OnViewSupport)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp construction

CGCQLApp::CGCQLApp() //: CBCGWorkspace (TRUE /* m_bResourceSmartUpdate */)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CGCQLApp object

CGCQLApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp initialization

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

BOOL CGCQLApp::InitInstance()
{
	m_bSettingsLoaded = false;	// haven't loaded our settings yet

	AfxEnableControlContainer();

	new FileReactor( "GCQL.log" );
#if defined(_DEBUG)
	new DebugReactor();
#endif

	//// HACK: Some directories got renamed during development, this is to just make
	//// it easier for people so they don't have to redownload all the files again.
	//FileDisk::renameFile( "Cache/", ".Cache/" );
	//FileDisk::renameFile( "ChatLogs/", ".ChatLogs/" );

	dword gameId = 1;			// default to the DarkSpace lobby

	CharString sLaunchProgram;
	CharString sLaunchAddress;
	int nLaunchPort = 0;
	bool bMultiInstance = false;

	// process the command line
	CommandLine commands = CharString( m_lpCmdLine );
	for(int i=0;i<commands.argumentCount();i++)
	{
		const char * pArg = commands.argument( i );
		if ( pArg[0] == '-' )
		{
			switch( tolower( pArg[1] ) )
			{
			case 'm':	// multiple instance
				bMultiInstance = true;
				break;
			case 's':	// SessionId
				if ( (i + 1 ) < commands.argumentCount() )
				{
					sm_SessionID = commands.argumentDWORD( i + 1 );
					++i;
				}
				break;
			case 'g':	// GameId
				if ( (i + 1 ) < commands.argumentCount() )
				{
					gameId = commands.argumentDWORD( i + 1 );
					++i;
				}
				break;
			case 'r':	// Run 
				if ( (i + 1 ) < commands.argumentCount() )
				{
					sLaunchProgram = commands.argument( i + 1 );
					++i;
				}
				break;
			case 'a':	// Address
				if ( (i + 1 ) < commands.argumentCount() )
				{
					sLaunchAddress = commands.argument( i + 1 );
					++i;
				}
				break;
			case 'p':	// Port
				if ( (i + 1 ) < commands.argumentCount() )
				{
					nLaunchPort = commands.argumentInt( i + 1 );
					++i;
				}
				break;
			case 'c':	// config
				if ( (i + 1 ) < commands.argumentCount() )
				{
					sConfigName = commands.argument( i + 1 );
					++i;
				}
				break;					
			}
		}
	}

	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	gotoHomeDirectory();
	Settings settings( sConfigName );
	
	//SetDialogBkColor( sm_StatusColor, sm_TextColor );
#if !defined(_DEBUG)
	HANDLE hMutex = ::CreateMutex( NULL, FALSE, "GCQL-MUTEX" );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS && !bMultiInstance )
	{
		SendMessage( HWND_BROADCAST, MSG_GCQL_COMMAND, 0, (LPARAM)m_lpCmdLine );
		return FALSE;
	}
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Palestar"));
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	if ( settings.get( "AcceptedEULA", (dword)0 ) != 1 )
	{
		if ( CEula().DoModal() != IDOK )
			return FALSE;

		// user agreed to EULA
		settings.put( "AcceptedEULA", 1 );
	}

	sm_ChatBufferSize = settings.get( "ChatBufferSize", 32768 );
	sm_TextSize = settings.get( "TextSize", 8 );
	sm_bEnableWordFilter = settings.get( "enableWordFilter", 1 ) != 0;
	sm_ChatColor = settings.get( "ChatColor", RGB( 255, 255, 0 ) );
	sm_bChatSound = settings.get( "ChatSound", 1 ) != 0 ? true : false;
	sm_bMessageSound = settings.get( "MessageSound", 1 ) != 0 ? true : false;
	sm_bTaskBarMessages = settings.get( "TaskBarMessages", 1 ) != 0 ? true : false;
	sm_bMinimized = settings.get( "Minimized", 1 ) != 0 ? true : false;
	sm_bEnableAutoAway = settings.get( "Away", 1 ) != 0 ? true : false;
	sm_nUpdateCheckInterval = settings.get( "UpdateCheckInterval", 60 * 60 );
	sm_RefreshGameListTime = settings.get( "RefreshGameListTime", 60 );
	sm_RoomAnnounce = settings.get( "RoomAnnounce", (dword)0 ) != 0;
	sm_AlwaysLog = settings.get( "AlwaysLog", (dword)0 ) != 0 ? true : false;
	sm_AutoAwayTime = settings.get( "AutoAwayTime", 900 );
	sm_Language = settings.get( "Language", (dword)0 );
	sm_AutoLogin = settings.get( "AutoLogin", (dword)0 ) != 0 ? true : false;
	sm_BackgroundColor1 = settings.get( "SkinColor1", RGB(32,32,32) );
	sm_BackgroundColor2 = settings.get( "SkinColor2", RGB(48,48,48) );
	sm_FrameColor = settings.get( "SkinColor3", RGB(64,64,64) );
	sm_ButtonFrame1 = settings.get( "SkinColor4", RGB(16,16,16) );
	sm_ButtonFrame2 = settings.get( "SkinColor5", RGB(32,32,32) );
	sm_ButtonHighlight = settings.get( "SkinColor6", RGB(64,64,64) );
	sm_GripperColor1 = settings.get( "GripperColor1", RGB (32, 32, 32) );
	sm_GripperColor2 = settings.get( "GripperColor2", RGB (255, 255, 255) );
	sm_TextColor = settings.get( "TextColor", RGB (255, 255, 255) );
	sm_GrayedTextColor = settings.get( "GrayedTextColor", RGB( 150, 64, 64 ) );
	sm_StatusColor = settings.get( "StatusColor", RGB( 128, 128, 128 ) );
	m_bSettingsLoaded = true;

	if ( sm_bEnableWordFilter )
		sm_MetaClient.loadWordFilter( "WordFilter.txt" );

	// Do not allow autoawaytime to be lower then 60.
	// Having such a low time could lead to massive "/back" flooding
	if( sm_AutoAwayTime < 60 )
		sm_AutoAwayTime = 60;

	// Create the CBCGVisualManager object
	if (CBCGVisualManager::GetInstance () != NULL)
		 delete CBCGVisualManager::GetInstance ();
	new CGCQStyle();

	if ( sm_Language == 0 )
		sm_Language = Locale::getDefaultLCID();
	Locale::locale().setLCID( sm_Language );

	//----------------------------------------------------------------------------
	// Connect to the MirrorServer

#ifndef _DEBUG
	if (! IsDebuggerPresent() )
	{
		if (! ClientUpdate::updateSelf( CGCQLApp::sConfigName, 
			settings.get( "mirrorAddress", "mirror-server.palestar.com" ), 
			settings.get("mirrorPort", 9100 ) ) )
		{
			if ( MessageBox( NULL, _T("Failed to check for updates, continue?"), _T("Failure"), MB_YESNO ) != IDYES )
				return FALSE;
		}
	}
#endif

	//----------------------------------------------------------------------------
	// Connect to the MetaServer
	
	CProgressDlg progress;
	progress.Create();
	progress.SetRange( 0, 6 );
	progress.SetStep( 1 );
	progress.SetStatus( _T("Connecting to GameCQ server...") );

	if ( sm_MetaClient.open() < 0 )
	{
		MessageBox( NULL, _T("Failed to connect to the GameCQ server; please try again later."), _T("Failed"), MB_OK );
		return FALSE;
	}

	progress.StepIt();
	progress.SetStatus( _T("Logging in...") );
	// login to server
	if (! doLogin() )
		return FALSE;

	// save my session id
	sm_SessionID = sm_MetaClient.profile().sessionId;

	progress.StepIt();
	progress.SetStatus( _T("Initializing logfile...") );	
	
	// initialize ChatLog instance using current NickName
	s_ChatLog.initialize( CString( sm_MetaClient.profile().name ) );
	if ( sm_AlwaysLog && !s_ChatLog.startLogging() )
		sm_MetaClient.sendLocalChat("/Error opening Logfile, make sure it is not currently in use...");

	progress.StepIt();
	progress.SetStatus( _T("Selecting game...") );
	
	// select the game
	if (! selectGame( gameId, false ) )
		return FALSE;

	progress.StepIt();
	progress.SetStatus( _T("Syncing...") );

	// set the login time
	sm_LoginTime = Time::seconds();
	sm_MetaClient.getTime();		// request time, so the metaclient can sync to the servertime
	
	progress.StepIt();
	progress.SetStatus( _T("Launching GCQL...") );	

	m_pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGCQLDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		NULL );
	AddDocTemplate(m_pDocTemplate);

	//m_nCmdShow = SW_SHOWMAXIMIZED;
	m_pDocTemplate->OpenDocumentFile( NULL );
	if (!m_pMainWnd )
		return FALSE;
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->SetFocus();

	if ( sLaunchProgram.length() > 0 )
	{
		CCacheList * pCache = CCacheList::getCacheList();
		CCacheList::Program * pProgram = pCache->findProgram( sLaunchProgram );
		if ( pProgram != NULL )
		{
			if (! pCache->launchProgram( pProgram, sLaunchAddress, nLaunchPort ) )
				MessageBox( NULL, CharString().format("Failed to launch %s!", pProgram->m_sDescription), "Error", MB_OK );
		}
		else
		{
			MessageBox( NULL, CharString().format("Invalid game %s!", sLaunchProgram.cstr()), "Error", MB_OK );
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp message handlers

int CGCQLApp::ExitInstance() 
{
	BCGCBCleanUp();

	//----------------------------------------------------------------------------
	// Save Settings
	if( m_bSettingsLoaded )	// but only if they were loaded before. So no custom setting will be
	{						// overwritten using default values.
		gotoHomeDirectory();
		Settings settings( sConfigName );
		settings.put( "ChatBufferSize", sm_ChatBufferSize );
		settings.put( "TextSize", sm_TextSize );
		settings.put( "enableWordFilter", (dword)sm_bEnableWordFilter );
		settings.put( "ChatColor", sm_ChatColor );
		settings.put( "ChatSound", sm_bChatSound );
		settings.put( "MessageSound", sm_bMessageSound );
		settings.put( "TaskBarMessages", sm_bTaskBarMessages );
		settings.put( "Minimized", sm_bMinimized );
		settings.put( "Away", sm_bEnableAutoAway );
		settings.put( "UpdateCheckInterval", sm_nUpdateCheckInterval );
		settings.put( "RefreshGameListTime", sm_RefreshGameListTime );
		settings.put( "RoomAnnounce", sm_RoomAnnounce );
		settings.put( "AlwaysLog", sm_AlwaysLog );
		settings.put( "AutoAwayTime", sm_AutoAwayTime );
		settings.put( "Language", sm_Language );
		settings.put( "AutoLogin", sm_AutoLogin );
		settings.put( "SkinColor1", sm_BackgroundColor1 );
		settings.put( "SkinColor2", sm_BackgroundColor2 );
		settings.put( "SkinColor3", sm_FrameColor );
		settings.put( "SkinColor4", sm_ButtonFrame1 );
		settings.put( "SkinColor5", sm_ButtonFrame2 );
		settings.put( "SkinColor6", sm_ButtonHighlight );
		settings.put( "GripperColor1", sm_GripperColor1 );
		settings.put( "GripperColor2", sm_GripperColor2 );
		settings.put( "TextColor", sm_TextColor );
		settings.put( "GrayedTextColor", sm_GrayedTextColor );
		settings.put( "StatusColor", sm_StatusColor );
	}

	return CWinApp::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CBCGURLLinkButton	m_btnURL;
	CStatic				m_txtCopywrite;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_COMPANY_URL, m_btnURL);
	DDX_Control(pDX, IDC_COPYWRITE, m_txtCopywrite);
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	int year = aTime->tm_year + 1900;
	m_txtCopywrite.SetWindowTextA( CharString().format( "© 2000 - %d PaleStar Inc. All rights reserved worldwide.", year ) );

   return true;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGCQLApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp message handlers

int CGCQLApp::selectBestServer( Array< MetaClient::Server > & servers )
{
	// TODO: write routine which not only checks for the closest server

	int bestServer = -1;
	float bestClients = 1.0f;

	for(int i=0;i<servers.size();i++)
	{
		if ( servers[i].clients < servers[i].maxClients )
		{
			float clients = float(servers[i].clients) / servers[i].maxClients;
			if ( clients < bestClients )
			{
				bestServer = i;
				bestClients = clients;
			}
		}
	}

	return bestServer;
}

bool CGCQLApp::selectGame( dword gameId, bool a_bLeaveAllRooms )
{
	Array< MetaClient::Game > games;
	if ( sm_MetaClient.getGames( games ) < 0 || games.size() < 1 )
	{
		MessageBox( NULL, _T("Failed to get game information!"), _T("Failed"), MB_OK );
		sm_MetaClient.logoff();
		return false;
	}

	bool gameFound = false;
	for(int i=0;i<games.size();i++)
		if ( games[i].id == gameId )
		{
			gameFound = true;
			sm_Game = games[i];
			break;
		}

	if (! gameFound )
		sm_Game = games[ 0 ];

	// select the game
	if ( sm_MetaClient.selectGame( sm_Game.id ) < 0 )
		return false;

	if ( a_bLeaveAllRooms )
		sm_MetaClient.leaveRoom();

	// join the default room
	if ( sm_pChatFrame != NULL )
	{
		if ( sm_pChatFrame->m_nRoomId != 0 )
			sm_MetaClient.leaveRoom( sm_pChatFrame->m_nRoomId );
		sm_pChatFrame->m_nRoomId = sm_MetaClient.joinBestRoom();
	}

	if ( sm_pWebFrame != NULL )
		sm_pWebFrame->OnNavbarHome();

	return true;
}

bool CGCQLApp::openURL( const char * pURL )
{
	((CGCQLApp *)AfxGetApp())->OnViewHome();
	// navigate to a specific URL
	CWebView::getWebView()->Navigate2( pURL );

	return true;
}

bool CGCQLApp::doLogin()
{
	gotoHomeDirectory();
	Settings settings( sConfigName );
	
	if ( sm_SessionID != 0 )
		sm_MetaClient.login( sm_SessionID );

	dword attempts = 0;
	while( !sm_MetaClient.loggedIn() )
	{
		CLogin loginDialog;
		switch( loginDialog.DoModal() )
		{
		case IDOK:
			{
				switch( sm_MetaClient.login( CharString(loginDialog.m_UID), CharString(loginDialog.m_PW) ) )
				{
				case MetaClient::LOGIN_OKAY:
					break;
				case MetaClient::LOGIN_BANNED:
					MessageBox( NULL, _T("This account is currently banned from GameCQ"), _T("Banned"), MB_OK );
					sm_MetaClient.logoff();
					sm_AutoLogin = false;
					return false;
				case MetaClient::LOGIN_DUPLICATE_LOGIN:
					MessageBox( NULL, _T("This account is already connected to GameCQ"), _T("Duplicate Login"), MB_OK );
					sm_MetaClient.logoff();
					return false;
				case MetaClient::LOGIN_FAILED:
					sm_AutoLogin = false;
					attempts++;
					if ( attempts > 3 )
						return false;
					MessageBox( NULL, _T("Invalid username or password"), _T("Login Failed"), MB_OK );
					break;
				default:
					sm_AutoLogin = false;
					MessageBox( NULL, _T("A error occured on the server, please try again later"), _T("Error"), MB_OK );
					sm_MetaClient.logoff();
					return false;
				}
			}
			break;
		case IDYES:
			{
				if ( sm_Game.newlogin.length() > 0 )
				{
					// open browser to newlogin URL
					ShellExecute(NULL, _T("open"), CString(sm_Game.newlogin), _T(""), _T(""), SW_SHOWNORMAL );
					break;
				}

				CNewLogin newLogin;

				bool accountCreated = false;
				while( !accountCreated )
				{
					if ( newLogin.DoModal() == IDCANCEL )
					{
						sm_MetaClient.logoff();
						return false;
					}

					if ( newLogin.m_PW != newLogin.m_PW2 )
					{
						MessageBox( NULL, _T("The passwords you entered do not match, please retype the passwords"), _T("Password Mismatch"), MB_OK );
						newLogin.m_PW = newLogin.m_PW2 = "";
						continue;
					}

					MetaClient::Profile newProfile;
					newProfile.name = newLogin.m_UID;
					newProfile.email = newLogin.m_EMail;
					newProfile.userId = 0;
					newProfile.sessionId = 0;
					newProfile.clanId = 0;
					newProfile.flags = 0;
					newProfile.score = 0.0f;

					// attempt to create the new account
					switch( sm_MetaClient.create( newProfile, CharString(newLogin.m_PW) ) )
					{
					case MetaClient::LOGIN_ILLEGAL:
						MessageBox( NULL, _T("Failed to create account, name contains illegal words or characters!\n\n")
										_T("- The account name may not contain whitespaces as first or last character.\n")
										_T("- Neither '[' nor ']' may be used within the first 3 characters.\n")
										_T("- The name may not include < or >.\n")
										_T("- The name must be atleast 2 characters in length.") , _T("Illegal Name"), MB_OK );
						break;
					case MetaClient::LOGIN_DUPLICATE_LOGIN:
						MessageBox( NULL, _T("That user ID is already being used, please select a different username."), _T("Duplicate Login"), MB_OK );
						break;
					case MetaClient::LOGIN_OKAY:
						settings.put( "previousUID", CharString(newLogin.m_UID) );
						accountCreated = true;
						break;
					default:	// we don't explicitly tell the user if he is banned
						MessageBox( NULL, _T("A error occured on the server, please try again later"), _T("Error"), MB_OK );
						sm_MetaClient.logoff();
						return false;
					}
				}
			}
			break;
		case IDCANCEL:
			sm_MetaClient.logoff();
			return false;
		default:
			return false;
		}
	}
	return true;
}

void CGCQLApp::gotoHomeDirectory()
{
#if !defined(_DEBUG)
	TCHAR szBuffer[_MAX_PATH]; 
	VERIFY(::GetModuleFileName( AfxGetInstanceHandle(), szBuffer, _MAX_PATH) );
	CString sPath = (CString)szBuffer;
	sPath = sPath.Left( sPath.ReverseFind('\\') );

	SetCurrentDirectory( sPath );
#endif
}

BOOL CGCQLApp::PreTranslateMessage(MSG* pMsg) 
{
	if ( sm_pChatFrame != NULL && (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_KEYDOWN) )
		sm_pChatFrame->clearIdleTime();

	if ( m_pMainWnd != NULL && (pMsg->message == WM_LBUTTONDOWN ) )
	{	// remember the mousestatus so other classes can use it more easy
		sm_bBtnDown = true;
		sm_ptMouseDown = pMsg->pt;
	}

	if ( m_pMainWnd != NULL && (pMsg->message == WM_LBUTTONUP ) )
		sm_bBtnDown = false;

	return CWinApp::PreTranslateMessage(pMsg);
}

void CGCQLApp::OnGamecqChangepassword()
{
	CChangePassword dialog;
	if ( dialog.DoModal() == IDOK )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;
		if ( client.changePassword( CharString(dialog.m_NewPassword) ) < 0 )
			AfxGetMainWnd()->MessageBox(  _T("Failed to change password!") );
		else
			AfxGetMainWnd()->MessageBox( _T("Password changed!") );
	}
}

void CGCQLApp::OnGamecqChangeusername()
{
	CChangeName dialog;
	if ( dialog.DoModal() == IDOK )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;

		int result = client.changeName( CharString(dialog.m_Name) );
		if ( result == MetaClient::LOGIN_OKAY )
		{
			CString sNameChanged;
			sNameChanged.Format( _T("Name changed to '%s'; use this next time you login!"), CString(client.profile().name) );

			sm_pChatFrame->MessageBox( sNameChanged );
		}
		else if ( result == MetaClient::LOGIN_ILLEGAL )
		{
			sm_pChatFrame->MessageBox( _T("Failed to change name, name contains illegal words or characters!\n\n")
				_T("- The account name may not contain whitespaces as first or last character.\n")
				_T("- Neither '[' nor ']' may be used within the first 3 characters.\n")
				_T("- The name may not include < or >.\n")
				_T("- The name must be atleast 2 characters in length."));
		}
		else if ( result == MetaClient::LOGIN_DUPLICATE_LOGIN )
			sm_pChatFrame->MessageBox( _T("Failed to change name, name is already being used!") );
		else 
			sm_pChatFrame->MessageBox( _T("Failed to change name, try again later!") );
	}
}

void CGCQLApp::OnGamecqChangeemail()
{
	CChangeEmail dialog;
	if ( dialog.DoModal() == IDOK )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;
		if ( client.changeEmail( CharString(dialog.m_Email) ) >= 0 )
		{
			CString sEmailChanged;
			sEmailChanged.Format( _T("Email changed to '%s'!"), CString(client.profile().email ) );

			sm_pChatFrame->MessageBox( sEmailChanged );
		}
		else
			sm_pChatFrame->MessageBox( _T("Failed to change email!") );
	}
}

void CGCQLApp::OnGamecqSavechat()
{
	CChatLog & cChatLog = CGCQLApp::s_ChatLog;
	MetaClient & client = CGCQLApp::sm_MetaClient;

	if ( cChatLog.startLogging() )	
		client.sendLocalChat( CharString( "/Logging to \"" + cChatLog.getLogFileName() + "\" ..." ) );
	else
		client.sendLocalChat( "/Error opening Logfile, make sure it is not currently in use..." );
}

void CGCQLApp::OnUpdateGamecqSavechat(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( !CGCQLApp::s_ChatLog.isLogging() );
}

void CGCQLApp::OnGamecqSavechatAs()
{
	CFileDialog saveFile(FALSE, _T("html"), CTime::GetCurrentTime().Format(_T("%Y.%m.%d")),
					OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, 
					_T("HTML Files (*.html; *.htm)|*.html; *.htm||"));

	CChatLog & chatlog = CGCQLApp::s_ChatLog;
	CString logPath = chatlog.getLogPath();
	saveFile.m_ofn.lpstrInitialDir = logPath;

	if( saveFile.DoModal() != IDOK )
		return;

	if( chatlog.isLogging() )
		chatlog.stopLogging();
	
	if( !chatlog.startLogging( saveFile.GetFileName(), saveFile.GetPathName() ) )
		CGCQLApp::sm_MetaClient.sendLocalChat("/Error opening Logfile, make sure it is not currently in use...");
}

void CGCQLApp::OnUpdateGamecqSavechatAs(CCmdUI *pCmdUI)
{}

void CGCQLApp::OnGamecqStopchatlog()
{
	CChatLog & cChatLog = CGCQLApp::s_ChatLog;
	MetaClient & client = CGCQLApp::sm_MetaClient;

	client.sendLocalChat( "/Stopped logging..." );
	cChatLog.stopLogging();
}

void CGCQLApp::OnUpdateGamecqStopchatlog(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CGCQLApp::s_ChatLog.isLogging() );
}

void CGCQLApp::OnEditFindusers()
{
	CFindUser * pDialog = new CFindUser;
	pDialog->Create( CFindUser::IDD, sm_pChatFrame );
}

void CGCQLApp::OnViewOptions()
{
	COptions().DoModal();
}

void CGCQLApp::OnViewIgnores()
{
	CIgnores * pDialog = new CIgnores;
	pDialog->Create( CIgnores::IDD, sm_pChatFrame );
}

void CGCQLApp::OnAppExit()
{
	m_pMainWnd->DestroyWindow();
	PostMessage( m_pMainWnd->GetSafeHwnd(), WM_QUIT, 0, 0 );
}

void CGCQLApp::OnUpdateAdminServers(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( (CGCQLApp::sm_MetaClient.profile().flags & MetaClient::ADMINISTRATOR) != 0 );
}

void CGCQLApp::OnViewHome()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_BROWSE );
}

void CGCQLApp::OnViewChat()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_CHAT );
}

void CGCQLApp::OnViewGames()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_LAUNCH );
}

void CGCQLApp::OnViewServers()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_SERVERS );
}

void CGCQLApp::OnViewManual()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_BROWSE );
	CGCQLApp::openURL( "http://www.darkspace.net/wiki/index.php/Manual" );
}

void CGCQLApp::OnViewFAQ()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_BROWSE );
	CGCQLApp::openURL( "http://www.darkspace.net/wiki/index.php/FAQ" );
}

void CGCQLApp::OnViewSupport()
{
	((CMainFrame *)m_pMainWnd)->SetActiveTab( CMainFrame::TAB_BROWSE );
	CGCQLApp::openURL( "http://www.darkspace.net/index.htm?lang=en&module=document.php&doc_id=5" );
}