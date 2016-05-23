// FriendList.cpp : implementation file
//

#include "stdafx.h"
#include "Yam.h"
#include "FriendList.h"
#include "YamDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

#undef RGB
#define RGB(r,g,b)					((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

//----------------------------------------------------------------------------

CFriendList::CFriendList()
{
	m_OldY = 0;
}

CFriendList::~CFriendList()
{
}

BEGIN_MESSAGE_MAP(CFriendList, CListCtrl)
	//{{AFX_MSG_MAP(CFriendList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFriendList message handlers

int CFriendList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	m_AvatarIcons.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_HIDDEN ) ) ;		// 0
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_AWAY ) ) ;		// 1
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR ) );				// 2
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_MOD ) );			// 3
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_ADMIN ) );		// 4

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
	InsertColumn(0,"Friends",LVCFMT_LEFT, 100 );
	
	// create tooltips
	m_Tip.Create( this );

	// start checking for changes in the players
	SetTimer( 0x8, 2 * 1000, NULL );

	return 0;
}

void CFriendList::OnSize(UINT nType, int cx, int cy) 
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

inline int GetAvatar( dword flags, String status )
{
	if( status == "Offline" )
		return 0;
	
	int avatar = 2;
	if ( flags & MetaClient::AWAY )
		avatar = 1;
	else if ( flags & MetaClient::ADMINISTRATOR )
		avatar = 4;
	else if ( flags & MetaClient::MODERATOR )
		avatar = 3;

	return avatar;
}

const int MAX_NAME = 128;

int CALLBACK SortFriendList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Array<MetaClient::ShortProfile> & friends = CYamDlg::s_friends;

	MetaClient::ShortProfile player1, player2;
	bool p1found = false, p2found = false;
	for(int i=0;i<friends.size();i++)
	{
		if ( !p1found && friends[i].userId == lParam1 )
		{
			player1 = friends[i];
			if( p2found ) break;
		}
		if ( !p2found && friends[i].userId == lParam2 )
		{
			player2 = friends[i];
			if( p1found ) break;
		}
	}

	int avatar1 = GetAvatar( player1.flags, player1.status );
	int avatar2 = GetAvatar( player2.flags, player2.status );
	// sort by status first
	if ( avatar1 != avatar2 )
		return avatar2 - avatar1;

	// sort by name last
	return stricmp( player1.name, player2.name );
}

void CFriendList::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 0x8 )
	{
		int oldItemCount = GetItemCount();
		
		CYamDlg::s_lockFriends.lock();
		Array<MetaClient::ShortProfile> & friends = CYamDlg::s_friends;
			
		Array< bool > playerFound( GetItemCount() );
		for(int i=0;i<playerFound.size();i++)
			playerFound[ i ] = false;
		
		for(i=0;i<friends.size();i++)
		{
			const MetaClient::ShortProfile & player = friends[i];
				
			CString		name( player.name );
			int			avatar( GetAvatar( player.flags, player.status ) );
			dword		userId = player.userId;
		
			// search the list for this player
			bool addPlayer = true;
			for(int j=0;j<GetItemCount() && addPlayer;j++)
				if ( name == GetItemText( j, 0 ) )
				{
					addPlayer = false;
					playerFound[j] = true;
					
					// player found, update their information
					SetItem( j, 0, LVIF_IMAGE | LVIF_STATE | LVIF_TEXT | LVIF_PARAM, name, 
						avatar, 0, LVIS_OVERLAYMASK, userId );
					
				}
				
				if ( addPlayer )
				{
					// player not found in list, insert the player
					int item = InsertItem( GetItemCount(), name, avatar );
					SetItemData( item, userId );
					SetItemState( item, 0, LVIS_OVERLAYMASK );
					
					playerFound.push( true );
				}
		}
		
		// remove players no longer in room
		for(i=GetItemCount()-1;i>=0;i--)
			if ( !playerFound[i] )
				DeleteItem( i );
			
		// resort the list
		SortItems( SortFriendList, (dword)this );

		CYamDlg::s_lockFriends.unlock();
	}
}


void CFriendList::checkForToolTip(CPoint point)
{
	int selected = HitTest( point );
	if ( selected >= 0 )
	{
		String tooltipText;
		dword userId = GetItemData( selected );

		bool bFound = false;

		CYamDlg::s_lockFriends.lock();
		Array<MetaClient::ShortProfile> & friends = CYamDlg::s_friends;
		
		for(int i=0;i<friends.size();i++)
			if ( friends[i].userId == userId )
				{
					 tooltipText = friends[i].status;
					 bFound = true;
					 break;
				}
		CYamDlg::s_lockFriends.unlock();
		
		if( !bFound )	// friendlist could've been updated in between and a friend removed
			return;

		m_Tip.On( true );
		m_Tip.SetOffset( -8 * tooltipText.length(), 0 );
		m_Tip.Set( point, (const char *)tooltipText );
	}
	else
		m_Tip.On( false );
}

void CFriendList::OnMouseMove(UINT nFlags, CPoint point) 
{
	checkForToolTip( point );

	CListCtrl::OnMouseMove(nFlags, point);
}

void CFriendList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int selected = HitTest( point );
	if ( selected >= 0 )
	{
		dword userId = GetItemData( selected );
		((CYamDlg*)GetParent())->openChatWindow( userId );
	}
	
//	CResizingDialog::OnLButtonDblClk(nFlags, point);
}
