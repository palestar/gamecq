// yamDlg.cpp : Declare the yam dialog

#include "stdafx.h"
#include "yam.h"
#include "YamDlg.h"

#include "SystemTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CYamDlg, CResizingDialog)
	//{{AFX_MSG_MAP(CYamDlg)
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenConsole)
	//}}AFX_MSG_MAP
	ON_WM_SYSCOMMAND()
	ON_COMMAND(IDC_ST_RESTORE, OnSTRestore)
	ON_COMMAND(IDC_ST_EXIT, OnSTExit)
END_MESSAGE_MAP();


////////////////////////////////////////////////////////////////////////
// Constants 

#define FRIENDLIST_BOTTOM_OFFSET 50
const char* kpcTrayNotificationMsg_ = "yamtray";

Array<MetaClient::ShortProfile>		CYamDlg::s_friends;
Semaphore							CYamDlg::s_lockFriends;

////////////////////////////////////////////////////////////////////////
// Ctor

CYamDlg::CYamDlg(CWnd* pParent) : 
CResizingDialog(CYamDlg::IDD, pParent),
m_bMinimized(false),
m_pTrayIcon(0),
m_nTrayNotificationMsg(RegisterWindowMessage(kpcTrayNotificationMsg_))
{
	//{{AFX_DATA_INIT(CYamDlg)
	m_Status = _T("");
	//}}AFX_DATA_INIT

	m_hIconNormal = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon = m_hIconNormal;
	m_hIconNewMsg = AfxGetApp()->LoadIcon(IDI_ACTIVITY);

	m_bMsgWaiting = false;
}


//// DoDataExchange ///////////////////////////////////////////////////

void CYamDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizingDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYamDlg)
	DDX_Text(pDX, IDC_STATUS, m_Status);
	//}}AFX_DATA_MAP
}


//// SetupTrayIcon /////////////////////////////////////////////////////
// If we're minimized, create an icon in the systray.  Otherwise, remove
// the icon, if one is present.

void CYamDlg::SetupTrayIcon( bool bShow )
{
	if( bShow && (m_pTrayIcon == 0) )
	{
		m_pTrayIcon = new CSystemTray;
		m_pTrayIcon->Create( 0, m_nTrayNotificationMsg, "YAM",	m_hIcon, IDR_SYSTRAY_MENU );
	}
	else
	{
		delete m_pTrayIcon;
		m_pTrayIcon = 0;
	}
}


//// SetupTaskBarButton ////////////////////////////////////////////////
// Show or hide the taskbar button for this app, depending on whether
// we're minimized right now or not.

void CYamDlg::SetupTaskBarButton( bool bShow )
{
	// Show or hide this window appropriately
	if ( bShow )
		ShowWindow(SW_SHOW);
	else
		ShowWindow( SW_HIDE );
}


////////////////////////////////////////////////////////////////////////
// CYamDlg message handlers

BOOL CYamDlg::OnInitDialog()
{
	CResizingDialog::OnInitDialog();

	CRect rect;
	GetClientRect( &rect );
	
	if (! m_wndFriendList.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, 0 ) )
		return FALSE;

	m_wndFriendList.MoveWindow( rect.left, rect.top, rect.right, rect.bottom - FRIENDLIST_BOTTOM_OFFSET, true );
	m_wndFriendList.EnableScrollBar( SB_HORZ, ESB_DISABLE_BOTH );

	// Set up window icons
	SetIcon(m_hIcon, TRUE);		// big icon
	SetIcon(m_hIcon, FALSE);		// small icon

	// Add standard behavior for controls when resizing
	AddControl( IDC_STATUS, sizeResize, sizeRepos );


//	if (::AfxGetApp()->m_nCmdShow & SW_MINIMIZE) {
		// User wants app to start minimized, but that only affects the 
		// main window.  So, minimize this window as well.
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
//	}
	// position in lower right screencorner (but don't overlap taskbar)
	RECT r;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	SetWindowPos( 0, r.right - (rect.right + 7), r.bottom - (rect.bottom + 26), 0, 0, SWP_NOSIZE );


	m_pTrayIcon = new CSystemTray;
	m_pTrayIcon->Create( 0, m_nTrayNotificationMsg, "YAM",	m_hIcon, IDR_SYSTRAY_MENU );

	m_Status = "* Disconnected *";
	MetaClient & client = CYamApp::s_MetaClient;
	if( client.loggedIn() )
	{
		s_lockFriends.lock();
		client.getFriends( s_friends );
		s_lockFriends.unlock();
		
		m_Status = String().format( "Nick: \"%s\"", client.profile().name );
	}
	
	UpdateData( false );
	
	m_TopChatMessage = 0;
	
	SetTimer( 0x08, 15 * 1000, NULL );	// start checking for changes in the friendlist
	SetTimer( 0x10, 30 * 1000, NULL );	// update userstatus in open chatwindows
	SetTimer( 0x20, 0.5 * 1000, NULL );	// update chat

	return TRUE;
}

HCURSOR CYamDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}

void CYamDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Decide if minimize state changed
	bool bOldMin = m_bMinimized;
	if (nID == SC_MINIMIZE)
	{
		m_bMinimized = true;
		setMsgStatusNormal();
	}
	else
		if (nID == SC_RESTORE)
		{
			m_bMinimized = false;
			setMsgStatusNormal();
		}

	CResizingDialog::OnSysCommand(nID, lParam);

	if (bOldMin != m_bMinimized && m_bMinimized)
	{
//		SetupTrayIcon( true );
		SetupTaskBarButton( false );
	}
}

void CYamDlg::OnSTExit()
{
	CResizingDialog::OnCancel();
}

void CYamDlg::OnSTRestore()
{
	ShowWindow(SW_RESTORE);
	m_bMinimized = false;
//	SetupTrayIcon( false );
	SetupTaskBarButton( true );
}


BOOL CYamDlg::DestroyWindow() 
{
	// Get rid of systray icon
	SetupTrayIcon( false );
	
	return CResizingDialog::DestroyWindow();
}

void CYamDlg::OnSize(UINT nType, int cx, int cy) 
{
	CResizingDialog::OnSize(nType, cx, cy);
	
	if ( ::IsWindow( m_wndFriendList.m_hWnd ) )
		m_wndFriendList.MoveWindow( 0, 0, cx, cy - FRIENDLIST_BOTTOM_OFFSET, true );
	
}

void CYamDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 0x8 )
	{
		MetaClient & client = CYamApp::s_MetaClient;
		if( client.loggedIn() )
		{
			s_lockFriends.lock();
			client.getFriends( s_friends );
			s_lockFriends.unlock();

			if( client.profile().status != "Using 'YAM'" )
				client.sendStatus( 0, "Using 'YAM'" );
		}
	}
	else if ( nIDEvent == 0x10 )
	{
		MetaClient & client = CYamApp::s_MetaClient;
		if( client.loggedIn() )
		{
			for( int i = 0; i < m_msgWnds.size() ; i++ )
				if( !m_msgWnds[i].closed )
					updateChatWindowTitle( m_msgWnds[i].userId, false );	// slow update, so request from metaclient
		}
		else
			CYamApp::openCon(false);	// try to reconnect if disconnected
	}
	else if ( nIDEvent == 0x20 )
	{
		MetaClient & client = CYamApp::s_MetaClient;
		if( client.loggedIn() )
		{
			client.lock();
			if ( m_TopChatMessage < client.chatCount() )
			{
				for(;m_TopChatMessage<client.chatCount();m_TopChatMessage++)
					processNewChat( client.chat( m_TopChatMessage ) );
			}
			client.unlock();
		}
		else
			m_Status = "* Disconnected *";

	}

}


void CYamDlg::openChatWindow(dword userId)
{
	setMsgStatusNormal();

	int wndId = getChatWindow( userId );
	if( wndId >= 0 )						// window for that user already exists / existed
	{
		if( !m_msgWnds[ wndId ].closed )	// is currently open
		{
			m_msgWnds[ wndId ].pWnd->SetFocus();	// bring to front
			return;
		}

		reopenChatWindow( userId );	// was closed, needs to be reopened
		return;
	}

	// no window for that user before, so create a new one
	newChatWindow( userId );
}

int CYamDlg::getChatWindow(dword userId)
{
	for( int i = 0 ; i < m_msgWnds.size() ; i++ )
		if( m_msgWnds[i].userId == userId )
			return i;
		
	return -1;
}

int CYamDlg::getChatWindow(CChatWindow *wnd)
{
	for( int i = 0 ; i < m_msgWnds.size() ; i++ )
		if( m_msgWnds[i].pWnd == wnd )
			return i;
		
	return -1;
}

void CYamDlg::newChatWindow(dword userId)
{
	MsgWnd msgWnd;
	msgWnd.closed = false;
	msgWnd.userId = userId;
	msgWnd.pWnd = new CChatWindow( this, userId != 0 );	// don't echo if console window
	msgWnd.oldText = "";
	
	//Check if new succeeded and we got a valid pointer to a dialog object
	if(msgWnd.pWnd != NULL)
	{
		BOOL ret = msgWnd.pWnd->Create(CChatWindow::IDD,this);
		if(ret)	// succeeded ?
		{
			msgWnd.pWnd->ShowWindow( SW_SHOW );
			m_msgWnds.push( msgWnd );

			// position somewhere in the middle of the screen
			RECT r;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
			r.right = ( ( r.right / 2 ) - 200 ) + rand() % 200;
			r.bottom = ( ( r.bottom / 2 ) - 200 ) + rand() % 200;
			msgWnd.pWnd->SetWindowPos( 0, r.right, r.bottom, 0, 0, SWP_NOSIZE );

			updateChatWindowTitle( userId, true );
		}
	}
}


void CYamDlg::OnOK()	
{
}

void CYamDlg::OnCancel()	// prevent enter key from closing the window, minimize instead
{
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CYamDlg::chatWindowClosed(CChatWindow *which, String oldText)
{
	int wndId = getChatWindow( which );
	m_msgWnds[wndId].closed = true;
	m_msgWnds[wndId].oldText = oldText;
	m_msgWnds[wndId].pWnd->DestroyWindow();	// kill the window so MFC doesn't complain
	delete m_msgWnds[wndId].pWnd;			// then free it's memory

	setMsgStatusNormal();
}

void CYamDlg::chatWindowSendText(CChatWindow *which, String text)
{
	setMsgStatusNormal();

	int wndId = getChatWindow( which );
	dword userId = m_msgWnds[wndId].userId;

	MetaClient & client = CYamApp::s_MetaClient;
	if( client.loggedIn() )
	{
		if( text.beginsWith("/") )
			client.sendChat( text );
		else
			if( userId == 0 )
					client.sendChat( String().format("/%s", text ) );	// all console commands are prefixed with / to make sure we don't broadcast when roomId is 0
			else
				client.sendChat( String().format("/send @%u %s", userId, text ) );
	}
	else
		MessageBox( String().format("Can not send \"%s\"\nNot connected.", text ) );
}


String CYamDlg::getFriendName(dword userId)
{
	String result;
	s_lockFriends.lock();
	for( int i = 0 ; i < s_friends.size() ; i++ )
		if( s_friends[i].userId == userId )
		{
			result = s_friends[i].name;
			break;
		}

	s_lockFriends.unlock();
	return result;
}

String CYamDlg::getFriendStatus(dword userId)
{
	String result;
	s_lockFriends.lock();
	for( int i = 0 ; i < s_friends.size() ; i++ )
		if( s_friends[i].userId == userId )
		{
			result = s_friends[i].status;
			break;
		}

	s_lockFriends.unlock();
	return result;
}

void CYamDlg::reopenChatWindow(dword userId)
{
	int wndId = getChatWindow( userId );
	if( wndId < 0 )
		return;

	MsgWnd & msgWnd = m_msgWnds[ wndId ];
	msgWnd.pWnd = new CChatWindow( this, userId != 0 ); // don't echo if console
	
	//Check if new succeeded and we got a valid pointer to a dialog object
	if(msgWnd.pWnd != NULL)
	{
		BOOL ret = msgWnd.pWnd->Create(CChatWindow::IDD,this);
		if(ret)	// succeeded ?
		{
			msgWnd.pWnd->ShowWindow( SW_SHOW );
			msgWnd.pWnd->putOldText( msgWnd.oldText );
			msgWnd.closed = false;
			
			// position somewhere in the middle of the screen
			RECT r;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
			r.right = ( ( r.right / 2 ) - 200 ) + rand() % 200;
			r.bottom = ( ( r.bottom / 2 ) - 200 ) + rand() % 200;
			msgWnd.pWnd->SetWindowPos( 0, r.right, r.bottom, 0, 0, SWP_NOSIZE );

			updateChatWindowTitle( userId, true );
		}
	}

}

void CYamDlg::updateChatWindowTitle(dword userId, bool quick)
{
	dword wndId = getChatWindow( userId );
	if( wndId < 0 )
		return;
	
	if( userId == 0 )
	{
		m_msgWnds[wndId].pWnd->SetWindowText("Console");
		return;
	}

	String friendName = getFriendName( userId );	// is the user in our friendlist, so we can get the name quickly ?
	String friendStatus = getFriendStatus( userId );
	if( friendName.length() > 0 && friendStatus.length() > 0 )
		m_msgWnds[wndId].pWnd->SetWindowText(String().format("\"%s\" @%u - [ %s ]", friendName, userId, friendStatus ));
	else
		if( quick )
			m_msgWnds[wndId].pWnd->SetWindowText(String().format("Chatting with @%u", userId));
		else
		{
			MetaClient::Profile profile;
			if( CYamApp::s_MetaClient.getProfile( userId, profile ) )
				m_msgWnds[wndId].pWnd->SetWindowText(String().format("\"%s\" @%u - Status: \"%s\"", profile.name, userId, profile.status ));
		}
}

void CYamDlg::processNewChat( const MetaClient::Chat & chat )
{
	// local echo or reports
	if( chat.recpId == 0 && chat.roomId == 0 && chat.authorId == 0 && chat.author == "SERVER" )
	{
		if( chat.text.beginsWith("/You sent to ") && chat.text.endsWith("</b>\"") )	// local echo for directed msges, no need to display it
			return;
	}
	
	dword authorId = chat.authorId;

	if( chat.roomId != 0 )
		authorId = 0;	// room chat into console
	else if( chat.text.beginsWith( String().format("/<b>%s @%u</b> sent to Friends: \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// put friend sends into the console window
	else if( chat.text.beginsWith( String().format("/<b>%s @%u</b> sent to Clan: \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// put regular sends into the console window
	else if( chat.text.beginsWith( String().format("/<b>%s @%u</b> sent to your fleet: \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// put fleet sends by other players into the console window
	else if( chat.text.beginsWith( String().format("/<b>%s @%u</b> reports \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// be sure reports are handled correctly
	else if( chat.text.beginsWith( String().format("/<b>[MODTALK] %s @%u</b> Sent \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// modtalk into the console
	else if( chat.text.beginsWith( String().format("/<b>[STAFFSEND] %s @%u</b> Sent \"<b>", chat.author, chat.authorId ) ) )
		authorId = 0;	// staffsend into console
	

	openChatWindow( authorId );
	int wndId = getChatWindow( authorId );
	m_msgWnds[wndId].pWnd->addNewMessage( chat );

	setMsgWaiting();		
}

void CYamDlg::setMsgWaiting()
{
	if( !m_bMsgWaiting )
	{
		m_bMsgWaiting = true;
		m_hIcon = m_hIconNewMsg;
		SetupTrayIcon( false );
		SetupTrayIcon( true );
	}
}

void CYamDlg::setMsgStatusNormal()
{
	if( m_bMsgWaiting )
	{
		m_bMsgWaiting = false;
		m_hIcon = m_hIconNormal;
		SetupTrayIcon( false );
		SetupTrayIcon( true );
	}
}


void CYamDlg::OnOpenConsole() 
{
	openChatWindow( 0 );	
}
