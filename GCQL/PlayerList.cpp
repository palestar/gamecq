// PlayerList.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "PlayerList.h"
#include "Message.h"
#include "Profile.h"
#include "WebView.h"
#include "Options.h"
#include "ChatFrame.h"

#include "Standard/Settings.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

#undef RGB
#define RGB(r,g,b)					((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

// THIS IS A HUGE NO NO, DO NOT EVER DO DEFINES LIKE THIS ... CODE IS COMMENTED OUT AND REMOVED
//#define ERROR_PLAYER_LEFT_ROOM()	client.sendLocalChat( "/Error, the selected player has left the room." )

//----------------------------------------------------------------------------

CPlayerList::CPlayerList() : m_nRoomId( 0 )
{
	m_OldY = 0;
}

CPlayerList::~CPlayerList()
{}

const int ID_EMOTION_COUNT = 256;
const int ID_EMOTION_BEGIN = WM_USER+1024;
const int ID_EMOTION_END = ID_EMOTION_BEGIN + ID_EMOTION_COUNT;

BEGIN_MESSAGE_MAP(CPlayerList, CListCtrl)
	//{{AFX_MSG_MAP(CPlayerList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_PLAYER_WHO, OnPlayerWho)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PLAYER_WHISPER, OnPlayerWhisper)
	ON_COMMAND(ID_PLAYER_SEND, OnPlayerSend)
	ON_COMMAND(ID_PLAYER_ADDFRIEND, OnPlayerAddfriend)
	ON_COMMAND(ID_PLAYER_ADDIGNORE, OnPlayerAddignore)
	ON_COMMAND(ID_PLAYER_EDIT, OnPlayerEdit)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_EDIT, OnUpdatePlayerEdit)
	ON_COMMAND(ID_PLAYER_MUTE, OnPlayerMute)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_MUTE, OnUpdatePlayerMute)
	ON_COMMAND(ID_PLAYER_UNMUTE, OnPlayerUnmute)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_UNMUTE, OnUpdatePlayerUnmute)
	ON_COMMAND(ID_PLAYER_KICK, OnPlayerKick)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_KICK, OnUpdatePlayerKick)
	ON_COMMAND(ID_PLAYER_BAN, OnPlayerBan)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_BAN, OnUpdatePlayerBan)
	ON_COMMAND(ID_PLAYER_CLONES, OnPlayerClones)
	ON_COMMAND(ID_PLAYER_CHECK, OnPlayerCheck)
	ON_COMMAND(ID_PLAYER_SESSION, OnPlayerSession)
	ON_COMMAND(ID_PLAYER_MODSEND, OnPlayerModsend)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_EMOTION_BEGIN, ID_EMOTION_END, OnEmotion )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayerList message handlers

int CPlayerList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	m_AvatarIcons.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_HIDDEN ) ) ;		// 0
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_AWAY ) ) ;		// 1
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR ) );				// 2
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_SUB ) );			// 3
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_DEV ) );			// 4
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_MOD ) );			// 5
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_ADMIN ) );		// 6
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_INGAME ) );		// 7

	int index = m_AvatarIcons.Add( AfxGetApp()->LoadIcon(IDI_MUTED) );
	m_AvatarIcons.SetOverlayImage(index, 1);

	SetImageList( &m_AvatarIcons, LVSIL_SMALL );
	ModifyStyle( LVS_ICON|LVS_LIST|LVS_ALIGNTOP|LVS_SMALLICON, 
		LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|LVS_SINGLESEL|LVS_ALIGNLEFT);
	SetExtendedStyle( LVS_EX_FULLROWSELECT );
	SetBkColor( RGB(0,0,0) );
	SetTextBkColor( RGB(0,0,0) );
	SetTextColor(RGB(255,255,255) );

	// setup the report columns
	InsertColumn(0,_T("Room Members"),LVCFMT_LEFT, 100 );
	// start checking for changes in the players
	SetTimer( 0x8, 1000, NULL );
	
	return 0;
}

void CPlayerList::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);

	// only update the header if the control didn't get resized vertically,
	// else the "blank link" bug occurs
	if( m_OldY == cy )
	{
		// update the header
		SetColumnWidth( 0, cx );
	}
	m_OldY = cy;
}

inline int GetAvatar( dword flags )
{
	int avatar = 2;
	if ( flags & MetaClient::SUBSCRIBED )
		avatar = 3;

	if ( flags & MetaClient::HIDDEN )
		avatar = 0;
	else if ( flags & MetaClient::AWAY )
		avatar = 1;
	else if ( flags & MetaClient::ADMINISTRATOR )
		avatar = 6;
	else if ( flags & MetaClient::MODERATOR )
		avatar = 5;
	else if ( flags & MetaClient::DEVELOPER )
		avatar = 4;

	return avatar;
}

const int MAX_NAME = 128;

int CALLBACK SortPlayerList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CPlayerList * pPlayerList = (CPlayerList *)lParamSort;

	MetaClient & client = CGCQLApp::sm_MetaClient;

	MetaClient::ShortProfile player1;
	client.player( pPlayerList->m_nRoomId, lParam1, player1 );
	MetaClient::ShortProfile player2;
	client.player( pPlayerList->m_nRoomId, lParam2, player2 );

	int avatar1 = GetAvatar( player1.flags );
	int avatar2 = GetAvatar( player2.flags );
	// sort by status first
	if ( avatar1 != avatar2 )
		return avatar2 - avatar1;

	// sort by name last
	return player1.name.compareNoCase( player2.name );
}

void CPlayerList::OnTimer(UINT nIDEvent) 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	int oldItemCount = GetItemCount();

	m_nRoomId = ((CChatFrame *)GetParentFrame())->m_nRoomId;

	if ( client.waitPlayers( m_nRoomId, false ) )
	{
		AutoLock lock( client.autoLock() );

		Array< bool > playerFound( GetItemCount() );
		for(int i=0;i<playerFound.size();i++)
			playerFound[ i ] = false;

		for(int i=0;i<client.playerCount( m_nRoomId );i++)
		{
			const MetaClient::ShortProfile & player = client.player( m_nRoomId, i );
			if ( (client.profile().flags & MetaClient::ADMINISTRATOR) == 0 )
				if ( (player.flags & MetaClient::HIDDEN) != 0 && player.userId != client.profile().userId )	// show yourself hidden, only show other hidden players if your an admin
					continue;

			CString		name( player.name );
			int			avatar( GetAvatar( player.flags ) );
			dword		userId = player.userId;

			dword state = (player.flags & MetaClient::MUTED) != 0 ? INDEXTOOVERLAYMASK(1) : 0;

			// search the list for this player
			bool addPlayer = true;
			for(int j=0;j<GetItemCount() && addPlayer;++j)
				if ( userId	== GetItemData( j ) )
				{
					addPlayer = false;
					playerFound[j] = true;

					// player found, update their information
					SetItem( j, 0, LVIF_IMAGE | LVIF_STATE | LVIF_TEXT | LVIF_PARAM, name, 
						avatar, state, LVIS_OVERLAYMASK, userId );
				}

			if ( addPlayer )
			{
				// player not found in list, insert the player
				int item = InsertItem( GetItemCount(), name, avatar );
				SetItemData( item, userId );
				SetItemState( item, state, LVIS_OVERLAYMASK );
				
				playerFound.push( true );
			}
		}

		// remove players no longer in room
		for(int i=GetItemCount()-1;i>=0;--i)
			if ( !playerFound[i] )
				DeleteItem( i );

		// resort the list
		SortItems( SortPlayerList, (dword)this );

		lock.release();

		// if a player is removed and the list is scrolled to the right it may get empty and
		// the scrollbar will disappear. This is prevented by checking if there is an item
		// in the upper left corner
		if( oldItemCount > GetItemCount() )
		{
			LVHITTESTINFO lvhti;
			POINT pt;
			pt.x = 8;
			pt.y = 5;
			lvhti.pt = pt;
			if( SubItemHitTest( & lvhti ) == -1 && GetItemCount() > 0 )
				EnsureVisible( GetItemCount(), true );
		}
	}
	//CListCtrl::OnTimer(nIDEvent);
}

void CPlayerList::OnContextMenu(CWnd* pWnd, CPoint pt) 
{
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	// restore default emotes
	COptions::RestoreDefaultEmotions();

	m_EmotionNames.release();
	m_EmotionText.release();

	// load the emotion commands from the settings file
	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );

	int count = settings.get( "EmotionNum", (dword)0 ) & 0xfe;
	for(int i=0;i<count;i+=2)
	{
		m_EmotionNames.push( CString( settings.get( CharString().format("Emotion%d", i), "" ) ) );
		m_EmotionText.push( CString( settings.get( CharString().format("Emotion%d", i + 1), "" ) ) );
		if ( m_EmotionNames.size() >= ID_EMOTION_COUNT )
			break;
	}

	CMenu playerMenu;
	if ( ( CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR ) == 0 )
		playerMenu.LoadMenu( IDR_POPUP_PLAYER );	// normal contextmenu for players
	else
		playerMenu.LoadMenu( IDR_POPUP_PLAYERM );	// additional commands for moderators

	CMenu * pPopup = playerMenu.GetSubMenu(0);
	ASSERT( pPopup != NULL );
 
	// add emotion commands
	for(int i=0;i<m_EmotionNames.size();i++)
		pPopup->AppendMenu( MF_STRING, ID_EMOTION_BEGIN + i, m_EmotionNames[ i ] );

	pPopup->TrackPopupMenu(TPM_LEFTALIGN, pt.x,pt.y, GetParentFrame() );
}

//----------------------------------------------------------------------------

void CPlayerList::OnPlayerWho() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	CGCQLApp::openURL( CGCQLApp::sm_Game.profile + 
		CharString().format("&sid=%u&view=%u", CGCQLApp::sm_SessionID, GetItemData( selected ) ) );
}

void CPlayerList::OnPlayerWhisper() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	CString sWhisperTemplate = CharString().format( "/whisper @%d ", GetItemData( selected ) );
	((CChatFrame *)GetParentFrame())->setChatLineText( sWhisperTemplate );
}

void CPlayerList::OnPlayerSend() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	CString sSendTemplate = CharString().format( "/send @%d ", GetItemData( selected ) );
	((CChatFrame *)GetParentFrame())->setChatLineText( sSendTemplate );
}

void CPlayerList::OnPlayerAddfriend() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
	{
		if ( client.addFriend( player.userId ) < 0 )
			client.sendLocalChat( CharString().format("/<font color=ff0000>Failed to add \"%s\" to friend list...</font>", player.name ) );
		else
			client.sendLocalChat( CharString().format("/<font color=ffff00>Added \"%s\" to friend list...</font>", player.name ) );
	}
}

void CPlayerList::OnPlayerAddignore() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;
	
	MetaClient & client = CGCQLApp::sm_MetaClient;
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
	{
		if ( client.addIgnore( player.userId ) < 0 )
			client.sendLocalChat( CharString().format("/<font color=ff0000>Failed to add \"%s\" to ignore list...</font>", player.name ) );
		else
			client.sendLocalChat( CharString().format("/<font color=ffff00>Added \"%s\" to ignore list...</font>", player.name ) );
	}
}

void CPlayerList::OnPlayerEdit() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
	{
		client.sendLocalChat( CharString().format("/<b>Downloading profile information on '%s'...</b>", player.name) );
		
		CProfile editDialog;
		if ( client.getProfile( player.userId, editDialog.m_Profile ) > 0 )
		{
			if ( editDialog.DoModal() == IDOK )
			{
				client.sendLocalChat( CharString().format("/<b>Uploading profile information on '%s'...</b>", player.name) );
				if ( client.putProfile( editDialog.m_Profile ) < 0 )
					client.sendLocalChat( "/<font color=ff0000>Failed edit player profile...</font>" );
			}
		}
		else
			client.sendLocalChat( "/<font color=ff0000>Failed to get player profile from server...</font>" );
	}
}

void CPlayerList::OnUpdatePlayerEdit(CCmdUI* pCmdUI) 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	pCmdUI->Enable( (client.profile().flags & MetaClient::ADMINISTRATOR) != 0 );
}

void CPlayerList::OnEmotion( UINT nID )
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;

	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
	{
		CString emotionText( m_EmotionText[ nID - ID_EMOTION_BEGIN ] );
		emotionText.Replace( _T("$s"), CString( client.profile().name ) );
		emotionText.Replace( _T("$d"), CString( player.name ) );
		emotionText.Replace( _T("$g"), CString( "GothThug" ) );
		emotionText.Replace( _T("$t"), CString( "Two Weeks&trade;" ) );
		
		MetaClient & client = CGCQLApp::sm_MetaClient;
		client.sendChat( m_nRoomId, emotionText );
	}
}

void CPlayerList::OnPlayerMute() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
		client.sendChat( m_nRoomId, CharString().format("/mute @%u", player.userId) );
}

void CPlayerList::OnUpdatePlayerMute(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( (CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CPlayerList::OnPlayerUnmute() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
		client.sendChat( m_nRoomId, CharString().format("/unmute @%u", player.userId) );
}

void CPlayerList::OnUpdatePlayerUnmute(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( (CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CPlayerList::OnPlayerKick() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	CString sKickTemplate = CharString().format( "/kick @%d ", GetItemData( selected ) );
	((CChatFrame *)GetParentFrame())->setChatLineText( sKickTemplate );
}

void CPlayerList::OnUpdatePlayerKick(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( (CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CPlayerList::OnPlayerBan() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	CString sKickTemplate = CharString().format( "/ban @%d ", GetItemData( selected ) );
	((CChatFrame *)GetParentFrame())->setChatLineText( sKickTemplate );
}

void CPlayerList::OnUpdatePlayerBan(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( (CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CPlayerList::OnPlayerClones() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
		client.sendChat( m_nRoomId, CharString().format("/clones @%u", player.userId) );
}

void CPlayerList::OnPlayerCheck() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
		client.sendChat( m_nRoomId, CharString().format("/check @%u", player.userId) );
}

void CPlayerList::OnPlayerSession() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	MetaClient::ShortProfile player;
	if ( client.player( m_nRoomId, GetItemData( selected ), player ) )
		client.sendChat( m_nRoomId, CharString().format("/session @%u", player.userId) );
}

void CPlayerList::OnPlayerModsend() 
{
	int selected = GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;

	CString sSendTemplate = CharString().format( "/modsend @%d ", GetItemData( selected ) );
	((CChatFrame *)GetParentFrame())->setChatLineText( sSendTemplate );
}
