// FindUser.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "FindUser.h"
#include "Profile.h"
#include ".\finduser.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

/////////////////////////////////////////////////////////////////////////////
// CFindUser dialog


CFindUser::CFindUser(CWnd* pParent /*=NULL*/)
	: CDialog(CFindUser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindUser)
	m_UserID = _T("");
	//}}AFX_DATA_INIT
}


void CFindUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindUser)
	DDX_Text(pDX, IDC_EDIT1, m_UserID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindUser, CDialog)
	//{{AFX_MSG_MAP(CFindUser)
	ON_BN_CLICKED(IDC_BUTTON1, OnFind)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, OnContextMenu)
	ON_COMMAND(ID_PLAYER_EDIT, OnPlayerEdit)
	ON_COMMAND(ID_PLAYER_ADDFRIEND, OnPlayerAddfriend)
	ON_COMMAND(ID_PLAYER_ADDIGNORE, OnPlayerAddignore)
	ON_COMMAND(ID_PLAYER_WHO, OnPlayerWho)
	ON_COMMAND(ID_PLAYER_SEND, OnPlayerSend)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_EDIT, OnUpdatePlayerEdit)
	ON_BN_CLICKED(IDOK, OnFind)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_PLAYER_BAN, OnPlayerBan)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_BAN, OnUpdatePlayerBan)
	ON_COMMAND(ID_PLAYER_KICK, OnPlayerKick)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_KICK, OnUpdatePlayerKick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindUser message handlers

/**
 *  Converts userflags to avatar icon number in listview
 *  Also used in CPlayerList so apply any changes there too.
 **/
inline int GetAvatarStatus( dword flags )
{
	int avatar = 0;
	if ( flags & MetaClient::SUBSCRIBED )
		avatar = 1;

	if ( flags & MetaClient::ADMINISTRATOR )
		avatar = 3;
	else if ( flags & MetaClient::MODERATOR )
		avatar = 2;
	else if ( flags & MetaClient::DEVELOPER )
		avatar = 4;

	return avatar;
}

/**
 *	Uses the search pattern entered in the editBox to find all matching users.
 *  Entered value can be either @userID or (partial) username.
 *  Prepares an array of shortProfile´s containing the results for futher processing
 **/
void CFindUser::OnFind() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;				// Pointer for MetaClient calls
	CListCtrl *cList = (CListCtrl *)GetDlgItem( IDC_LIST2 );	// Grab a reference to the output list
	
	cList->DeleteAllItems();

	CString sUserID;
	GetDlgItemText(IDC_EDIT1, sUserID);	// grab reference to the inputfield
	sUserID.TrimRight();				// No whitespaces allowed
	sUserID.TrimLeft();
	
	if ( ! sUserID.IsEmpty() )
	{
		if ( sUserID.GetLength() < 2 )
		{
			MessageBox(_T("Error, search pattern must be atleast 2 characters...") );
			return;
		}
		
		if ( sUserID.GetAt(0) == '@' )				// numeric userID was entered
		{
			sUserID.Remove('@');
			dword userId = _ttoi( sUserID );
			
			MetaClient::Profile profile;			// retrieve the users profile
			MetaClient::ShortProfile shortProfile;	// and convert to short profile afterwards
			if ( client.getProfile( userId, profile ) > 0 )
			{
				shortProfile.name = profile.name;
				shortProfile.flags = profile.flags;
				shortProfile.status = profile.status;
				shortProfile.userId = profile.userId;
				
				found.push( shortProfile );
			} 
		
		}
		else
		{
			// no numeric ID, so it must be a (partial) username
			client.getProfiles( CharString().format("%%%s%%", CharString(sUserID)), found );	// do pattern search
		}
	}
	
	
	/* ShortProfileArray >> ListCtrl */
	CString text;
	if ( found.size() > 0 )			// print the results found if any
	{
		int actItem; /* , avatar; */
		cList->DeleteAllItems();	// clear list first
		for(int i=0;i<found.size();i++)
		{
		    int avatar = GetAvatarStatus( found[i].flags ) ;
			actItem = cList->InsertItem( cList->GetItemCount(), CString( found[i].name ), avatar );
			cList->SetItem(actItem, 1, LVIF_TEXT, CString( CharString().format("%d",found[i].userId) ), 0, LVIF_TEXT, 0, 0 );
			cList->SetItem(actItem, 2, LVIF_TEXT, CString( found[i].status ), 0, LVIF_TEXT, 0, 0 );
		}
	
	}

	// restore focus to input field
	CEdit *cEdtUsr = (CEdit *)GetDlgItem( IDC_EDIT1 );	
	cEdtUsr->SetFocus();
}


BOOL CFindUser::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init the ResultList
	CRect rect;
	CListCtrl *cList = (CListCtrl *)GetDlgItem( IDC_LIST2 );	// Grab a reference to the output list
	cList->GetWindowRect( &rect );
	cList->SetExtendedStyle( LVS_EX_FULLROWSELECT );
	cList->InsertColumn( 0, _T("User"),		LVCFMT_LEFT, rect.Width() * 3/10, 0 ); 		
	cList->InsertColumn( 1, _T("ID"),		LVCFMT_LEFT, rect.Width() * 1/10, 1 ); 		
	cList->InsertColumn( 2, _T("Status"),	LVCFMT_LEFT, rect.Width() * 6/10 - 19, 2 );		// leave room for scrollbar


	// load avatars
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	m_AvatarIcons.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR ) );				// 0
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_SUB ) );			// 1
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_MOD ) );			// 2
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_ADMIN ) );		// 3
	m_AvatarIcons.Add( pApp->LoadIcon( IDI_AVATAR_DEV ) );			// 4

	cList->SetImageList( &m_AvatarIcons, LVSIL_SMALL );
	cList->SetBkColor( RGB(0,0,0) );
	cList->SetTextBkColor( RGB(0,0,0) );
	cList->SetTextColor(RGB(255,255,255) );

	CEdit *cEdtUsr = (CEdit *)GetDlgItem( IDC_EDIT1 );	
	cEdtUsr->SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindUser::OnContextMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CListCtrl *cList = (CListCtrl *)GetDlgItem( IDC_LIST2 );	// Grab a reference to the output list
	selected = cList->GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 ) 
		return;
	
	// as this routine doesn't receive the cursor pos -> approximate a pos for the menu
	CPoint pt;
	GetCursorPos( &pt );
	
	// Display the context menu
	CMenu playerMenu;
	playerMenu.LoadMenu( IDR_POPUP_FINDUSER );

	CMenu * pPopup = playerMenu.GetSubMenu(0);
	ASSERT( pPopup != NULL );
 
	pPopup->TrackPopupMenu( TPM_LEFTALIGN , pt.x, pt.y, this);

	*pResult = 0;
}

void CFindUser::OnPlayerEdit() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	const MetaClient::ShortProfile & player = found[selected];
	
	CProfile editDialog;
	if ( client.getProfile( player.userId, editDialog.m_Profile ) > 0 )
	{
		if ( editDialog.DoModal() == IDOK )
			if ( client.putProfile( editDialog.m_Profile ) < 0 )
				MessageBox(_T("Failed to upload player profile"));
	}
	else
		MessageBox(_T("Failed to download player profile"));

}

void CFindUser::OnPlayerAddfriend() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	const MetaClient::ShortProfile & player = found[selected];
	if ( client.addFriend( player.userId ) < 0 )
		MessageBox(_T("Failed to add player to friend list"));
}

void CFindUser::OnPlayerAddignore() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	const MetaClient::ShortProfile & player = found[selected];
	if ( client.addIgnore( player.userId ) < 0 )
		MessageBox(_T("Failed to add player to ignore list"));
}

void CFindUser::OnPlayerWho() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	
	const MetaClient::ShortProfile & player = found[selected];
	CGCQLApp::openURL( CString( CGCQLApp::sm_Game.profile + CharString().format("&sid=%u&view=%u", CGCQLApp::sm_SessionID, player.userId)) );
}

void CFindUser::OnPlayerSend() 
{
	const MetaClient::ShortProfile & player = found[selected];

	CString sSend;
	sSend.Format( _T("/send @%d "), player.userId );

	CGCQLApp::sm_pChatFrame->setChatLineText( sSend );
}

void CFindUser::OnUpdatePlayerEdit(CCmdUI* pCmdUI) 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	pCmdUI->Enable( (client.profile().flags & MetaClient::ADMINISTRATOR) != 0 );
}

void CFindUser::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
	delete this;
}

void CFindUser::OnClose() 
{
	CDialog::OnClose();
	DestroyWindow();
}

void CFindUser::OnPlayerBan()
{
	const MetaClient::ShortProfile & player = found[selected];

	CString sSend;
	sSend.Format( _T("/ban @%d "), player.userId );

	CGCQLApp::sm_pChatFrame->setChatLineText( sSend );
}

void CFindUser::OnUpdatePlayerBan(CCmdUI *pCmdUI)
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	pCmdUI->Enable( (client.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CFindUser::OnPlayerKick()
{
	const MetaClient::ShortProfile & player = found[selected];

	CString sSend;
	sSend.Format( _T("/kick @%d "), player.userId );

	CGCQLApp::sm_pChatFrame->setChatLineText( sSend );
}

void CFindUser::OnUpdatePlayerKick(CCmdUI *pCmdUI)
{
	MetaClient & client = CGCQLApp::sm_MetaClient;
	pCmdUI->Enable( (client.profile().flags & MetaClient::MODERATOR) != 0 );
}
