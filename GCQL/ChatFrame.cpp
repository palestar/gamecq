/*
	ChatFrame.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "stdafx.h"
#include "GCQL.h"
#include "ChatFrame.h"
#include "FilterText.h"
#include "ChatEdit.h"
#include "ChatView.h"
#include "PlayerView.h"
#include "CreateRoom.h"
#include "RoomPassword.h"

#include "Resource.h"
#include "WebView.h"

#include "Standard/Time.h"
#include <mmsystem.h>

//---------------------------------------------------------------------------------------------------

const int PLAYER_VIEW_WIDTH = 200;

const int MAX_ROOMS = 64;
const int ID_ROOM_BEGIN = WM_USER + 33;
const int ID_ROOM_END = ID_ROOM_BEGIN + MAX_ROOMS;

const int MAX_FRIENDS = 100;

const int ID_FRIEND_MSG_BEGIN = ID_ROOM_END + 1;
const int ID_FRIEND_MSG_END = ID_FRIEND_MSG_BEGIN + MAX_FRIENDS;
const int ID_FRIEND_DEL_BEGIN = ID_FRIEND_MSG_END + 1;
const int ID_FRIEND_DEL_END = ID_FRIEND_DEL_BEGIN + MAX_FRIENDS;

const int MAX_GAMES = 256;

const int ID_GAMES_BEGIN = ID_FRIEND_DEL_END + 1;
const int ID_GAMES_END = ID_GAMES_BEGIN + MAX_GAMES;

const int MAX_CLAN = 64;
const int ID_CLAN_MSG_BEGIN = ID_GAMES_END + 1;
const int ID_CLAN_MSG_END = ID_CLAN_MSG_BEGIN + MAX_CLAN;

const int MAX_STAFF = 64;
const int ID_STAFF_MSG_BEGIN = ID_CLAN_MSG_END + 1;
const int ID_STAFF_MSG_END = ID_STAFF_MSG_BEGIN + MAX_STAFF;


#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

//---------------------------------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CChatFrame, CFrameWnd)

CChatFrame::CChatFrame() : 
	m_nRoomId( 0 ),
	m_bAutoAway( false ), 
	m_bLoginActive( false ),
	m_TimeIdle( 0 ), 
	m_ReconnectTime( 0 ), 
	m_GameButton( -1 ), 
	m_RoomsButton( -1 ),
	m_FriendsButton( -1 ), 
	m_ClanButton( -1 ), 
	m_StaffButton( -1 ), 
	m_bSplitterInit( false ), 
	m_hIcon( NULL ), 
	m_hMessageIcon( NULL ), 
	m_bPrivateMessage( false )
{
	if( FilterText::checkColor( CGCQLApp::sm_ChatColor ) != 0 )	// if chattextcolor is invalid then change it to
		CGCQLApp::sm_ChatColor = 0x00FFFF; // yellow
}

CChatFrame::~CChatFrame()
{}


BEGIN_MESSAGE_MAP(CChatFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_CHAT_SENDCHAT, OnChatSendchat)
	ON_UPDATE_COMMAND_UI(ID_CHAT_EDIT, OnUpdateChatEdit)
	ON_COMMAND(ID_CHAT_COLOR, OnChatColor)
	ON_COMMAND(ID_CHAT_CREATEROOM, OnChatCreateRoom)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)

	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_COMMAND_RANGE(ID_ROOM_BEGIN, ID_ROOM_END, OnJoinRoom )
	ON_COMMAND_RANGE(ID_FRIEND_MSG_BEGIN, ID_FRIEND_MSG_END, OnFriendMessage )
	ON_COMMAND_RANGE(ID_FRIEND_DEL_BEGIN, ID_FRIEND_DEL_END, OnFriendDelete )
	ON_COMMAND_RANGE(ID_GAMES_BEGIN, ID_GAMES_END, OnLobby )
	ON_COMMAND_RANGE(ID_CLAN_MSG_BEGIN, ID_CLAN_MSG_END, OnClanMessage )
	ON_COMMAND_RANGE(ID_STAFF_MSG_BEGIN, ID_STAFF_MSG_END, OnStaffMessage )
END_MESSAGE_MAP()


// CChatFrame message handlers

BOOL CChatFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (! CFrameWnd::OnCreateClient( lpcs, pContext ) )
		return FALSE;

	int nRightSize = PLAYER_VIEW_WIDTH;
	int nLeftSize =  lpcs->cx - nRightSize;

	m_wndSplitter.CreateStatic( this, 1, 2 );
	m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CChatView ), CSize( nLeftSize, lpcs->cy), pContext );
	m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS( CPlayerView ), CSize( nRightSize, lpcs->cy), pContext );
	m_bSplitterInit = true;

	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	m_hMessageIcon = AfxGetApp()->LoadIcon( IDI_PMESSAGE );

	CDocument * pDoc = GetActiveDocument();
	if ( pDoc != NULL )
		pDoc->SetTitle( CGCQLApp::sm_Game.name );

	SetTimer( 0x1, 1000, NULL );

	return TRUE;
}

int CChatFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndChatBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
		| /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1), AFX_IDW_TOOLBAR) )
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndChatBar.LoadToolBar(IDR_CHATBAR);
	m_wndChatBar.SetWindowText(_T("Chat"));

	if (!m_wndTopChatBar.CreateEx(this, TBSTYLE_FLAT/*|TBSTYLE_TRANSPARENT*/, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1), AFX_IDW_TOOLBAR))
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndTopChatBar.LoadToolBar( IDR_TOPCHATBAR, 0, 0, true );
	m_wndTopChatBar.EnableTextLabels();
	m_wndTopChatBar.SetWindowText(_T("Game"));

	return 0;
}

void CChatFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	if ( m_bSplitterInit )
	{
		int nRightSize = PLAYER_VIEW_WIDTH;
		int nLeftSize =  cx - nRightSize;

		if ( nLeftSize > 0 )
		{
			m_wndSplitter.SetColumnInfo( 0, nLeftSize, 0 );
			m_wndSplitter.SetColumnInfo( 1, nRightSize, 0 );
			m_wndSplitter.RecalcLayout();
		}
	}
	clearIdleTime();
}

BOOL CChatFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndChatBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		return TRUE;
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CChatFrame::OnTimer(UINT nIDEvent)
{
	if ( nIDEvent == 0x1 )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;
		if ( client.loggedIn() )
		{
			if ( m_nRoomId == 0 )
			{
				m_nRoomId = client.joinBestRoom();
				if ( m_nRoomId != 0 && CGCQLApp::sm_RoomAnnounce )
					client.sendChat( m_nRoomId, CharString().format("/me has entered the room...", client.profile().name) );

				m_bAutoAway = (client.profile().flags & MetaClient::AWAY) != 0;
			}

			// We only want to check auto away if we're not away already the user wants it
			if ( !m_bAutoAway && CGCQLApp::sm_bEnableAutoAway )
			{
				// We only want to increment the timer if the window is visible
				if ( CGCQLApp::sm_bMinimized && ( !AfxGetMainWnd()->IsWindowVisible() || AfxGetMainWnd()->IsIconic() ) )
				{
					client.sendChat( m_nRoomId, "/away" );
					m_bAutoAway = true;
				} 
				else
				{
					// Start incrementing the timer, if it reaches the set time set the client as away
					m_TimeIdle++;
					if ( m_TimeIdle >= CGCQLApp::sm_AutoAwayTime )
					{
						client.sendChat( m_nRoomId, "/away" );
						m_bAutoAway = true;
					}
				}
			}
			
			if ( m_bPrivateMessage )
			{
				m_bPrivateMessage = false;

				// restore the shell icon
				NOTIFYICONDATA tnid; 
				memset( &tnid, 0, sizeof(tnid) );
				tnid.cbSize = sizeof(NOTIFYICONDATA); 
				tnid.hWnd = AfxGetMainWnd()->GetSafeHwnd(); 
				tnid.uID = ID_TASKBAR_ICON; 
				tnid.uFlags = NIF_ICON; 
				tnid.hIcon = m_hIcon; 

				Shell_NotifyIcon( NIM_MODIFY, &tnid );
			}
		}
		else if (! m_bLoginActive )
		{
			if ( m_ReconnectTime == 0 )
			{
				client.sendLocalChat( "/Connection lost, attempting reconnect in 30 seconds..." );
				m_ReconnectTime = 30;
				m_nRoomId = 0;
			}
			else
			{
				m_ReconnectTime--;
				if ( m_ReconnectTime == 0 )
				{
					client.sendLocalChat( "/Connecting..." );
					if ( client.open() > 0 )
					{
						// prevent doLogin() from getting called multiple times, set a flag when we 
						// enter the modal loop, and clear the flag afterwards..
						m_bLoginActive = true;
						if ( CGCQLApp::doLogin() )
						{
							CGCQLApp::selectGame( CGCQLApp::sm_Game.id, true );
							client.sendLocalChat( "/Connected..." );
						}
						m_bLoginActive = false;
					}
				}
			}
		}
	}
	else
	{
		CFrameWnd::OnTimer(nIDEvent);
	}
}

//---------------------------------------------------------------------------------------------------

void CChatFrame::clearIdleTime()
{
	m_TimeIdle = 0;

	if ( m_bAutoAway && CGCQLApp::sm_bEnableAutoAway )
	{
		CGCQLApp::sm_MetaClient.sendChat( m_nRoomId, "/back" );
		m_bAutoAway = false;
	}
}

void CChatFrame::setChatLineText( const TCHAR * pNewText )
{
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );
	
	pEdit->setChatLine( pNewText );
}

void CChatFrame::onPrivateMessage( dword authorId, const char * pAuthor,  const char * pMessage )
{
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );
	
	if ( CGCQLApp::sm_bMessageSound )
		PlaySound( MAKEINTRESOURCE(IDW_MESSAGE01), AfxGetResourceHandle(), SND_RESOURCE|SND_ASYNC );

	pEdit->setLastUserId( authorId );

	if ( CGCQLApp::sm_bTaskBarMessages && (!AfxGetMainWnd()->IsWindowVisible() || AfxGetMainWnd()->IsIconic()) )
	{
		if ( pMessage[0] == '/' )
			++pMessage;

		CharString sCleanMessage = FilterText::ansi( pMessage );

		// blink the taskbar icon if private message has been received..
		NOTIFYICONDATA tnid; 
		memset( &tnid, 0, sizeof(tnid) );
		tnid.cbSize = sizeof(NOTIFYICONDATA); 
		tnid.hWnd = AfxGetMainWnd()->GetSafeHwnd(); 
		tnid.uID = ID_TASKBAR_ICON; 
		tnid.uFlags = NIF_ICON | NIF_INFO; 
		tnid.hIcon = m_hMessageIcon; 
		strncpy( tnid.szInfo, sCleanMessage, sizeof(tnid.szInfo) );
		tnid.uTimeout = 1000 * 2;

		Shell_NotifyIcon( NIM_MODIFY, &tnid );
		m_bPrivateMessage = true;
	}
}

CChatEdit *	CChatFrame::getChatEdit()
{
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );

	return pEdit;
}

//---------------------------------------------------------------------------------------------------

void CChatFrame::OnChatSendchat()
{
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );

	pEdit->sendChat();

	// clear the idle timer
	clearIdleTime();
}

void CChatFrame::OnUpdateChatEdit(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CGCQLApp::sm_MetaClient.loggedIn() && m_nRoomId > 0 );
}

void CChatFrame::OnChatColor()
{
	// get the selected color
	if ( CGCQLApp::sm_ChatColor == CBCGColorMenuButton::GetColorByCmdID( ID_CHAT_COLOR ) )
	{
		CBCGColorDialog dialog( CGCQLApp::sm_ChatColor, 0, this, NULL );
		if ( dialog.DoModal() == IDOK )
		{
			unsigned long color = dialog.GetColor();
			switch( FilterText::checkColor( color ) )
			{
				case 0:
					{
						CGCQLApp::sm_ChatColor = color;
					
						CBCGColorMenuButton * pButton = (CBCGColorMenuButton *)m_wndChatBar.GetButton( 2 );
						ASSERT( pButton );
					
						pButton->SetColor( CGCQLApp::sm_ChatColor );
					}
					break;

				case 1:	
					MessageBox( _T("The color you have choosen is too white. White is reserved for private messages and emotes.\n")
						_T("Please choose another textcolor.") ); 
					break;
				
				case -1:
					MessageBox( _T("The color you have choosen is too dark. Other players would have problems reading it.\n")
							_T("Please choose another textcolor.") );
					break;
			}
		}
	}
	else
		CGCQLApp::sm_ChatColor = CBCGColorMenuButton::GetColorByCmdID( ID_CHAT_COLOR );

	clearIdleTime();
}

void CChatFrame::OnChatCreateRoom() 
{
	clearIdleTime();
	CCreateRoom().DoModal();	
}

//---------------------------------------------------------------------------------------------------

LRESULT CChatFrame::OnToolbarReset(WPARAM wp,LPARAM)
{
	UINT uiToolBarId = (UINT) wp;

	switch (uiToolBarId)
	{
	case IDR_TOPCHATBAR:
		{
			m_RoomsButton = m_wndTopChatBar.CommandToIndex( ID_GAMEBAR_ROOMS );
			m_FriendsButton = m_wndTopChatBar.CommandToIndex( ID_GAMEBAR_FRIENDS );
			m_GameButton = m_wndTopChatBar.CommandToIndex( ID_GAMEBAR_LOBBY );
			m_ClanButton = m_wndTopChatBar.CommandToIndex( ID_GAMEBAR_CLAN );
			m_StaffButton = m_wndTopChatBar.CommandToIndex( ID_GAMEBAR_STAFF );

			CMenu roomsMenu;
			roomsMenu.CreatePopupMenu();
			CString roomLabel;
			roomLabel.LoadString( ID_GAMEBAR_ROOMS );

			m_wndTopChatBar.ReplaceButton( ID_GAMEBAR_ROOMS, 
				CBCGToolbarMenuButton(-1, roomsMenu, m_RoomsButton, roomLabel ));

			CMenu friendMenu;
			friendMenu.CreatePopupMenu();
			CString friendLabel;
			friendLabel.LoadString( ID_GAMEBAR_FRIENDS );

			m_wndTopChatBar.ReplaceButton( ID_GAMEBAR_FRIENDS, 
				CBCGToolbarMenuButton(-1, friendMenu, m_FriendsButton, friendLabel ));

			CMenu gamesMenu;
			gamesMenu.CreatePopupMenu();
			CString lobbyLabel;
			lobbyLabel.LoadString( ID_GAMEBAR_LOBBY );

			m_wndTopChatBar.ReplaceButton( ID_GAMEBAR_LOBBY, 
				CBCGToolbarMenuButton(-1, gamesMenu, m_GameButton, lobbyLabel ));

			CMenu clanMenu;
			clanMenu.CreatePopupMenu();
			CString clanLabel;
			clanLabel.LoadString( ID_GAMEBAR_CLAN );

			m_wndTopChatBar.ReplaceButton( ID_GAMEBAR_CLAN,
				CBCGToolbarMenuButton(-1, clanMenu, m_ClanButton, clanLabel) );


			CMenu staffMenu;
			staffMenu.CreatePopupMenu();
			CString staffLabel;
			staffLabel.LoadString( ID_GAMEBAR_STAFF );

			m_wndTopChatBar.ReplaceButton( ID_GAMEBAR_STAFF,
				CBCGToolbarMenuButton(-1, staffMenu, m_StaffButton, staffLabel) );

			CString sFindLabel;
			sFindLabel.LoadString( ID_EDIT_FINDUSERS );

			m_wndTopChatBar.SetButtonText( m_wndTopChatBar.CommandToIndex( ID_EDIT_FINDUSERS ), sFindLabel );
		}
		break;
	case IDR_CHATBAR:
		{
			m_wndChatBar.ReplaceButton( ID_CHAT_EDIT, CChatEditButton() );

			CBCGColorMenuButton colorButton( ID_CHAT_COLOR, _T("Chat Color"), NULL );
			colorButton.SetColor( CGCQLApp::sm_ChatColor );
			colorButton.EnableOtherButton (_T("More Colors..."));
			m_wndChatBar.ReplaceButton( ID_CHAT_COLOR, colorButton );
		}
		break;
	}

	return 0;
}

static int userOnlineSort( MetaClient::ShortProfile p1, MetaClient::ShortProfile p2 )
{
	if( p1.status != "Offline" )
	{
		if( p2.status != "Offline" )
			return p1.name.compareNoCase( p2.name );// both are online, result is name-compare
		else
			return -1;								// p1 is online, p2 is not, so p1 > p2
	}
	else
	{
		if( p2.status != "Offline" )
			return 1;								// p1 is offline, p2 is not, so p2 > p1
		else
			return p1.name.compareNoCase( p2.name );// both are offine, result is name-compare
	}
}

BOOL CChatFrame::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	clearIdleTime();

	//---------------------------------------------------------
	// Replace ID_VIEW_TOOLBARS menu item to the toolbars list:
	//---------------------------------------------------------
    CFrameWnd::OnShowPopupMenu (pMenuPopup);

    if (pMenuPopup != NULL &&
		pMenuPopup->GetMenuBar ()->CommandToIndex (ID_VIEW_TOOLBARS) >= 0)
    {
		if (CBCGToolBar::IsCustomizeMode ())
		{
			//----------------------------------------------------
			// Don't show toolbars list in the cuztomization mode!
			//----------------------------------------------------
			return FALSE;
		}

		pMenuPopup->RemoveAllItems ();

		CMenu menu;
		VERIFY(menu.LoadMenu (IDR_POPUP_TOOLBAR));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		pMenuPopup->GetMenuBar ()->ImportFromMenu (*pPopup, TRUE);
    }
	
	if ( pMenuPopup != NULL && pMenuPopup->GetParentToolBar() == &m_wndTopChatBar )
	{
		if ( m_wndTopChatBar.GetButton( m_GameButton ) == pMenuPopup->GetParentButton() )
		{
			CWaitCursor wait;

			MetaClient & client = CGCQLApp::sm_MetaClient;
			if ( client.getGames( m_Games ) < 0 )
			{
				//MessageBox( "Failed to get game list." );
				return FALSE;
			}

			pMenuPopup->RemoveAllItems();

			CMenu gameMenu;
			gameMenu.CreatePopupMenu();

			for(int i=0;i<m_Games.size() && i < MAX_GAMES;i++)
				gameMenu.AppendMenu( MF_STRING, ID_GAMES_BEGIN + i, CString(m_Games[i].name) );

			pMenuPopup->GetMenuBar()->ImportFromMenu (gameMenu, TRUE);
		}
		if ( m_wndTopChatBar.GetButton( m_RoomsButton ) == pMenuPopup->GetParentButton() )
		{
			CWaitCursor wait;

			MetaClient & client = CGCQLApp::sm_MetaClient;
			if ( client.getRooms( m_Rooms ) < 0 )
				return FALSE;

			pMenuPopup->RemoveAllItems();

			CMenu roomMenu;
			roomMenu.CreatePopupMenu();
			roomMenu.AppendMenu( MF_STRING, ID_CHAT_CREATEROOM, _T("New Room") );
			roomMenu.AppendMenu( MF_SEPARATOR );

			for(int i=0;i<m_Rooms.size() && i < MAX_ROOMS;i++)
			{
				MetaClient::Room & room = m_Rooms[i];
				
				CString sName;
				sName.Format( _T("%s "), CString( room.name ) );
				if ( room.flags & MetaClient::FLAG_ROOM_MODERATED )
					sName += _T("[MODERATED]");
				if ( room.flags & MetaClient::FLAG_ROOM_PASSWORD )
					sName += _T("[PRIVATE]");
				if ( room.flags & MetaClient::FLAG_ROOM_PRIVATE )
					sName += _T("[HIDDEN]");

				CString sMembers;
				sMembers.Format( _T("\t%u"), room.members );

				sName += sMembers;

				roomMenu.AppendMenu( MF_STRING, ID_ROOM_BEGIN + i, sName );
			}

			pMenuPopup->GetMenuBar()->ImportFromMenu (roomMenu, TRUE);
		}
		if ( m_wndTopChatBar.GetButton( m_FriendsButton ) == pMenuPopup->GetParentButton() )
		{
			CWaitCursor wait;

			MetaClient & client = CGCQLApp::sm_MetaClient;
			if ( client.getFriends( m_Friends ) < 0 )
			{
				//MessageBox( "Failed to get room list." );
				return FALSE;
			}
			
			// let friends who are online appear on top of the list
			m_Friends.qsort( userOnlineSort );

			pMenuPopup->RemoveAllItems();

			CMenu friendsMenu;
			friendsMenu.CreatePopupMenu();

			for(int i=0;i<m_Friends.size() && i < MAX_FRIENDS;i++)
			{
				MetaClient::ShortProfile & profile = m_Friends[ i ];

				CString sFriend;
				sFriend.Format(_T("%s\t%s"), CString(profile.name), CString(profile.status) );

				CMenu friendMenu;
				friendMenu.CreateMenu();
				friendMenu.AppendMenu( MF_STRING, ID_FRIEND_MSG_BEGIN + i, _T("Message") );
				friendMenu.AppendMenu( MF_STRING, ID_FRIEND_DEL_BEGIN + i, _T("Delete") );
				friendsMenu.AppendMenu( MF_POPUP, (UINT)friendMenu.Detach(), sFriend );
			}

			pMenuPopup->GetMenuBar()->ImportFromMenu( friendsMenu, TRUE );
		}
		if ( m_wndTopChatBar.GetButton( m_StaffButton ) == pMenuPopup->GetParentButton() )
		{
			CWaitCursor wait;

			MetaClient & client = CGCQLApp::sm_MetaClient;
			if ( client.getStaffOnline( m_Staff ) < 0 )
				return FALSE;
			
			dword userId = client.profile().userId;
			for( int i = 0 ; i < m_Staff.size() ; i++ )
				if( m_Staff[ i ].userId == userId )
				{
					m_Staff.remove(i);
					break;
				}

			pMenuPopup->RemoveAllItems();

			CMenu staffMenu;
			staffMenu.CreatePopupMenu();

			for(int i=0;i<m_Staff.size() && i < MAX_STAFF;i++)
			{
				MetaClient::ShortProfile & profile = m_Staff[ i ];

				CString sType;
				if ( profile.flags & MetaClient::ADMINISTRATOR )
					sType = "[ADMIN]";
				else if ( profile.flags & MetaClient::MODERATOR )
					sType = "[MOD]";

				CString sName;
				sName = profile.name;
				CString sStaffMember;
				sStaffMember.Format( _T("%s: %s\t%s"), sType, sName, CString(profile.status) );

				CMenu stMenu;
				stMenu.CreateMenu();
				stMenu.AppendMenu( MF_STRING, ID_STAFF_MSG_BEGIN + i, _T("Message") );

				bool bInserted = false;
				for(unsigned int j=0;j<staffMenu.GetMenuItemCount() && !bInserted;j++)
				{
					CString sCompare;
					staffMenu.GetMenuString( j, sCompare, MF_BYPOSITION );

					if ( _tcscmp( sCompare, sName ) > 0 )
					{
						staffMenu.InsertMenu( j, MF_BYPOSITION|MF_POPUP, (UINT)stMenu.Detach(), sStaffMember ) ? true : false;
						bInserted = true;
					}
				}

				if (!bInserted )
					staffMenu.AppendMenu( MF_POPUP, (UINT)stMenu.Detach(), sStaffMember );
			}

			pMenuPopup->GetMenuBar()->ImportFromMenu( staffMenu, TRUE );
		}
		if ( m_wndTopChatBar.GetButton( m_ClanButton ) == pMenuPopup->GetParentButton() )
		{
			CWaitCursor wait;

			pMenuPopup->RemoveAllItems();

			MetaClient & client = CGCQLApp::sm_MetaClient;

			CMenu clanMenu;
			clanMenu.CreatePopupMenu();
			//clanMenu.AppendMenu( MF_STRING, ID_GAMEBAR_CLAN, _T("Fleet Home") );

			dword clanId = client.profile().clanId;
			if ( clanId != 0 )
			{
				clanMenu.AppendMenu( MF_SEPARATOR );
				if ( client.getClan( clanId, m_Clan ) < 0 )
				{
					//MessageBox( "Failed to get clan members" );
					return FALSE;
				}
				
				// Remove self from clanlist
				for( int i = 0 ; i < m_Clan.size() ; i++ )
					if( m_Clan[i].userId == client.profile().userId )
					{
						m_Clan.remove(i);
						break;
					}

				// let members who are online appear on top of the list
				m_Clan.qsort( userOnlineSort );

				for( int i = 0 ; i < m_Clan.size() && i < MAX_CLAN ; i++ )
				{
					MetaClient::ShortProfile & profile = m_Clan[ i ];

					CString sClanMember;
					sClanMember.Format( _T("%s\t%s"), CString(profile.name), CString(profile.status) );

					CMenu subMenu;
					subMenu.CreateMenu();
					subMenu.AppendMenu( MF_STRING, ID_CLAN_MSG_BEGIN + i, _T("Message") );

					clanMenu.AppendMenu( MF_POPUP, (UINT)subMenu.Detach(), sClanMember );
				}
			}

			pMenuPopup->GetMenuBar()->ImportFromMenu( clanMenu, TRUE );
		}

	}

	return TRUE;
}

//---------------------------------------------------------------------------------------------------

void CChatFrame::OnJoinRoom( UINT nID )
{
	clearIdleTime();

	int roomIndex = nID - ID_ROOM_BEGIN;
	if ( roomIndex < 0 || roomIndex >= m_Rooms.size() )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;

	CharString password;
	if ( (m_Rooms[ roomIndex ].flags & MetaClient::FLAG_ROOM_PASSWORD) != 0 )
		if ( (client.profile().flags & MetaClient::MODERATOR) == 0 )
		{
			CRoomPassword dialog( this );
			if ( dialog.DoModal() != IDOK )
				return;

			password = dialog.m_Password;
		}

	// leave our previous room..
	if ( m_nRoomId != 0 )
	{
		client.leaveRoom( m_nRoomId );
		m_nRoomId = 0;
	}

	// join chat room
	if ( (m_nRoomId = client.joinRoom( m_Rooms[ roomIndex ].roomId, password ) ) != 0 )
	{
		// post local message, so the client can know that they have joined a room
		client.sendLocalChat( CharString().format("/Joining '%s'...", m_Rooms[ roomIndex ].name ) );

		if ( CGCQLApp::sm_RoomAnnounce )
			client.sendChat( m_nRoomId, CharString().format("/me has entered the room...", client.profile().name) );

		// set my status
		client.sendStatus( CharString().format("Chatting in '%s'", m_Rooms[ roomIndex ].name.cstr() ) );
	}
	else
	{
		client.sendLocalChat( "Failed to join chat room!" );
	}
}

void CChatFrame::OnFriendMessage( UINT nID )
{
	clearIdleTime();

	int friendIndex = nID - ID_FRIEND_MSG_BEGIN;
	if ( friendIndex < 0 || friendIndex >= m_Friends.size() )
		return;

	const MetaClient::ShortProfile & player = m_Friends[ friendIndex ];

	CString sSendTemplate = CharString().format("/send @%d ", player.userId );
	setChatLineText( sSendTemplate );
}

void CChatFrame::OnLobby( UINT nID )
{
	clearIdleTime();

	int game = nID - ID_GAMES_BEGIN;
	if ( game < 0 || game >= m_Games.size() )
		return;

	// set the current game
	CGCQLApp::sm_Game = m_Games[ game ];
	CGCQLApp::selectGame( CGCQLApp::sm_Game.id, false );

	CDocument * pDoc = GetActiveDocument();
	if ( pDoc != NULL )
		pDoc->SetTitle( CGCQLApp::sm_Game.name );

	//// set timer for updates
	////SetTimer( 0x2, 5000, NULL );
}

void CChatFrame::OnClanMessage( UINT nID )
{
	clearIdleTime();

	int clanIndex = nID - ID_CLAN_MSG_BEGIN;
	if ( clanIndex < 0 || clanIndex >= m_Clan.size() )
		return;

	const MetaClient::ShortProfile & player = m_Clan[ clanIndex ];

	CString sSendTemplate = CharString().format("/send @%d ", player.userId );
	setChatLineText( sSendTemplate );
}

void CChatFrame::OnStaffMessage( UINT nID )
{
	clearIdleTime();

	int staffIndex = nID - ID_STAFF_MSG_BEGIN;
	if ( staffIndex < 0 || staffIndex >= m_Staff.size() )
		return;

	const MetaClient::ShortProfile & player = m_Staff[ staffIndex ];

	CString sSendTemplate = CharString().format("/send @%d ", player.userId );
	setChatLineText( sSendTemplate );
}

void CChatFrame::OnFriendDelete( UINT nID )
{
	clearIdleTime();

	int friendIndex = nID - ID_FRIEND_DEL_BEGIN;
	if ( friendIndex < 0 || friendIndex >= m_Friends.size() )
		return;
	
	MetaClient::ShortProfile & user = m_Friends[ friendIndex ];

	CString sConfirmMessage;
	sConfirmMessage.Format( _T("Are you sure you want to remove \"%s\" from your friend list?"), CString(user.name) );

	if ( MessageBox( sConfirmMessage, _T("Confirm"), MB_YESNO ) == IDYES )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;
		if ( client.deleteFriend( user.userId ) < 0 )
			client.sendLocalChat( CharString().format("/<font color=ff0000>Failed to remove \"%s\" to friend list...</font>", user.name ) );
		else
			client.sendLocalChat( CharString().format("/<font color=ffff00>Removed \"%s\" from friend list...</font>", user.name ) );
	}
}

//---------------------------------------------------------------------------------------------------

void CChatFrame::OnEditPaste() 
{
	// paste into the chat edit 
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );

	pEdit->Paste();
}

void CChatFrame::OnEditUndo() 
{
	// past into the chat edit 
	CChatEditButton * pButton = dynamic_cast<CChatEditButton *>( m_wndChatBar.GetButton( 0 ) );
	ASSERT( pButton );
	CChatEdit * pEdit = dynamic_cast<CChatEdit *>( pButton->GetEditBox() );
	ASSERT( pEdit );

	pEdit->Undo();
}

//---------------------------------------------------------------------------------------------------
// EOF

