// GameList.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "GCQLDoc.h"
#include "ServerList.h"
#include "MainFrame.h"

#include "Standard/Process.h"
#include "File/FileDisk.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define	LIST_TIMEOUT	10000	// Max time in ms for pingGames thread to wait for all responses
#define PING_DELAY	50	// Time in ms to wait between ping sends for pacing and accuracy
#define MAX_LATENCY	10000	// Value to put in latency field to signify "NR" ("Not Responding")

//----------------------------------------------------------------------------

static HKEY GetKey( HKEY root, const TCHAR * key )
{
	HKEY hKey;
	if ( RegOpenKeyEx( root, key, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
		return NULL;

	return hKey;
}

static void CloseKey( HKEY key )
{
	RegCloseKey( key );
}

static CString GetString( HKEY hKey, const TCHAR * key )
{
	TCHAR value[ MAX_PATH ];
	value[ 0 ] = 0;

	dword valueSize = MAX_PATH;
	dword valueType = REG_SZ;

	RegQueryValueEx( hKey, key, NULL, &valueType, (byte *)value, &valueSize );
	return CString( value );
}

static dword GetInteger( HKEY hKey, const TCHAR * key )
{
	dword value = 0;
	dword keySize = sizeof(dword);
	dword keyType = REG_DWORD;

	RegQueryValueEx( hKey, key, NULL, &keyType, (byte *)&value, &keySize);
	return value;
}

//----------------------------------------------------------------------------

bool CServerList::isServerSelected()
{
	return GetListCtrl().GetNextItem( -1, LVNI_SELECTED ) >= 0;
}

//----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CServerList, CListView)
 
CServerList::CServerList() : m_bInitialized( false )
{
	m_nGameIdFilter = 0;
	m_nServerTypeFilter = 0; //MetaClient::GAME_SERVER;
	m_RefreshTimer = 0;
	m_SortColumn = 2;
	m_SortAscending = true;
}

CServerList::~CServerList()
{}

BEGIN_MESSAGE_MAP(CServerList, CListView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnGamesJoinDC)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_TIMER()
//	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_SERVER_CONNECT, OnServerConnect)
	ON_UPDATE_COMMAND_UI(ID_SERVER_CONNECT, OnUpdateServerConnect)
	ON_COMMAND(ID_SERVER_REFRESH, OnServerRefresh)
	ON_COMMAND(ID_SERVER_KICK, OnServerKick)
	ON_UPDATE_COMMAND_UI(ID_SERVER_KICK, OnUpdateServerKick)
	ON_COMMAND(ID_SERVER_BAN, OnServerBan)
	ON_UPDATE_COMMAND_UI(ID_SERVER_BAN, OnUpdateServerBan)
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerList drawing

void CServerList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	CListView::OnDraw( pDC );
}

/////////////////////////////////////////////////////////////////////////////
// CServerList diagnostics

#ifdef _DEBUG
void CServerList::AssertValid() const
{
	CListView::AssertValid();
}

void CServerList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CServerList message handlers

BOOL CServerList::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	if ( !CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext) )
		return FALSE;

	m_Tip.Create( this );

	return TRUE;
}

void CServerList::OnInitialUpdate() 
{
	if ( m_bInitialized )
		return;

	GetListCtrl().ModifyStyle( LVS_ICON|LVS_SMALLICON|LVS_LIST, 
		LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|LVS_SINGLESEL);
	GetListCtrl().SetExtendedStyle( LVS_EX_FULLROWSELECT );

	CListView::OnInitialUpdate();

	GetListCtrl().SetBkColor( RGB(0,0,0) );
	GetListCtrl().SetTextBkColor( RGB(0,0,0) );
	GetListCtrl().SetTextColor(RGB(255,255,255) );

	CRect r;
	GetClientRect( &r );

	// setup the report columns
	int	cwidth = r.Width() / 5;
	GetListCtrl().InsertColumn(0,_T("Name"),LVCFMT_LEFT,cwidth );
	GetListCtrl().InsertColumn(1,_T("Description"),LVCFMT_LEFT,cwidth * 2);
	GetListCtrl().InsertColumn(2,_T("Population"),LVCFMT_LEFT,cwidth);
	GetListCtrl().InsertColumn(3,_T("Game"),LVCFMT_LEFT,cwidth);

	// start auto refresh timer
	m_RefreshTimer = SetTimer( 0x6, 1000 * CGCQLApp::sm_RefreshGameListTime, NULL );

	OnServerRefresh();
	m_bInitialized = true;
}

static int CALLBACK SortGameList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CServerList * pServerList = (CServerList *)lParamSort;

	Array< MetaClient::Server > & games = pServerList->m_Servers;
	Array< CCacheList::Program * > & programs = pServerList->m_Programs;

	int result = 0;
	switch( pServerList->m_SortColumn )
	{
	case 0:
		result = games[ lParam1 ].name.compareNoCase( games[ lParam2 ].name );
		break;
	case 1:
		result = games[ lParam1 ].shortDescription.compareNoCase( games[ lParam2 ].shortDescription );
		break;
	case 2:
		result = games[ lParam2 ].clients - games[ lParam1 ].clients;
		break;
	case 3:
		result = programs[ lParam1 ]->m_sDescription.compareNoCase( programs[ lParam2 ]->m_sDescription );
		break;
	}

	if (! pServerList->m_SortAscending )
		result = -result;

	return result;
}

void CServerList::OnGamesJoinDC(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnServerConnect();
	*pResult = 0;
}

void CServerList::OnMouseMove(UINT nFlags, CPoint point) 
{
	int selected = GetListCtrl().HitTest( point );
	if ( selected >= 0 && selected == GetListCtrl().GetNextItem( -1, LVNI_SELECTED) )
	{
		int game = GetListCtrl().GetItemData( selected );
		MetaClient::Server & info = m_Servers[ game ];

		CString sDesc;
		sDesc.Format( _T("%s\n\n%s"), CString(info.name), CString(info.description) );

		m_Tip.On( true );
		m_Tip.Set( point, sDesc );
	}
	else
		m_Tip.On( false );

	CListView::OnMouseMove(nFlags, point);
}

void CServerList::OnContextMenu(CWnd* pWnd, CPoint pt) 
{
	CMenu gameMenu;
	gameMenu.LoadMenu( IDR_POPUP_GAME );

	CMenu * pPopup = gameMenu.GetSubMenu(0);
	ASSERT( pPopup != NULL );
 
	pPopup->TrackPopupMenu(TPM_LEFTALIGN, pt.x,pt.y, GetParentFrame());
}

void CServerList::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	if ( m_SortColumn != pNMListView->iSubItem )
	{
		m_SortColumn = pNMListView->iSubItem;
		m_SortAscending = true;
	}
	else
		m_SortAscending = !m_SortAscending;

	sortServers();
	*pResult = 0;
}

void CServerList::sortServers()
{
	GetListCtrl().SortItems( SortGameList, (dword)this );
}

void CServerList::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == m_RefreshTimer && IsWindowVisible() )
	{
		// refresh the game list
		OnServerRefresh();
		//CListView::OnTimer(nIDEvent);
	}
}

CServerList * CServerList::getServerList()
{
	return (CServerList *)CGCQLApp::sm_pServerFrame->GetActiveView();
}

//---------------------------------------------------------------------------------------------------

void CServerList::OnServerConnect()
{
	int selected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient::Server & server = m_Servers[ GetListCtrl().GetItemData( selected ) ];
	MetaClient & client = CGCQLApp::sm_MetaClient;

	// check the flags, make sure this client can join this server
	dword clientFlags = client.profile().flags & MetaClient::REGISTRATION;
	dword gameFlags = server.flags & MetaClient::REGISTRATION;

	if ( (clientFlags & gameFlags) == 0 )
	{
		if ( gameFlags & MetaClient::SUBSCRIBED )
		{
			if ( clientFlags & MetaClient::BETA )
				MessageBox( _T("You cannot join this server; it is not available to beta accounts...") );
			else
				MessageBox( _T("You cannot join this server; it is not available to free accounts...") );
		}
		else
			MessageBox( _T("You cannot join this server; it is restricted...") );
		return;
	}

	CCacheList::Program * pProgram = m_Programs[ GetListCtrl().GetItemData( selected ) ];
	ASSERT( pProgram );
	CCacheList * pCache = CCacheList::getCacheList();
	ASSERT( pCache );

	// handle connecting to servers behind a NAT...
	CharString myAddress;
	client.getAddress( myAddress );
	if ( server.address == myAddress )
		server.address = "localhost";

	pCache->launchProgram( pProgram, server.address, server.port );

	((CGCQLApp *)AfxGetApp())->OnViewGames();
}

void CServerList::OnUpdateServerConnect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( GetListCtrl().GetNextItem( -1, LVNI_SELECTED ) >= 0 );
}

void CServerList::OnServerRefresh()
{
	CCacheList * pCache = CCacheList::getCacheList();
	if (! pCache )
		return;

	CWaitCursor waitCursor;	

	CStatusBar * pStatusBar = (CStatusBar *)GetParentFrame()->GetDlgItem( AFX_IDW_STATUS_BAR );
	ASSERT( pStatusBar );

	pStatusBar->SetPaneText( 0, _T("Getting game list...") );

	//ASSERT( pCache );

	m_Servers.release();
	m_Programs.release();

	GetListCtrl().SetImageList( &pCache->m_ProgramIcons, LVSIL_SMALL );

	MetaClient & client = CGCQLApp::sm_MetaClient;
	dword clientFlags = client.profile().flags;
	if ( client.getServers( "", m_nGameIdFilter, m_nServerTypeFilter, m_Servers ) )
	{
		m_Programs.allocate( m_Servers.size() );
		for(int i=0;i<m_Programs.size();++i)
		{
			MetaClient::Server & server = m_Servers[i];
			m_Programs[i] = pCache->findProgram( server.gameId, server.type );
		}

		CListCtrl & list = GetListCtrl();

		list.DeleteAllItems();
		for(int i=0;i<m_Servers.size();++i)
		{
			MetaClient::Server & server = m_Servers[i];
			CCacheList::Program * pProgram = m_Programs[ i ];
			if (! pProgram || !pProgram->m_bCanUse )
				continue;		// unknown or unusable program, skip this server..

			CString sClients;
			if ( clientFlags & MetaClient::ADMINISTRATOR )
				sClients.Format("%d / %d", server.clients, server.maxClients);
			else
				sClients = MetaClient::populationText( server.clients, server.maxClients );

			int item = list.InsertItem( i, CString(server.name), pProgram->m_nIndex );
			list.SetItemText(item, 1, CString(server.shortDescription) );
			list.SetItemText(item, 2, sClients );
			list.SetItemText(item, 3, m_Programs[i]->m_sDescription );
			list.SetItemData( item, i );
		}

		if ( m_Servers.size() > 0 )
			sortServers();

		CString sActiveServers;
		sActiveServers.Format(_T("%d Active Servers"), list.GetItemCount() );
		pStatusBar->SetPaneText( 0, sActiveServers );
	}
	else
		MessageBox( _T("Failed to get game list from server; please try again later!") );
}

void CServerList::OnServerKick()
{
	int selected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	if ( (client.profile().flags & MetaClient::MODERATOR) == 0 )
		return;

	MetaClient::Server & game = m_Servers[ GetListCtrl().GetItemData( selected ) ];
	if ( MessageBox( _T("Confirm Kick?"), CString(game.name), MB_YESNO ) == IDYES )
		client.banServer( game.id, 60 * 10 );
}

void CServerList::OnUpdateServerKick(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( GetListCtrl().GetNextItem( -1, LVNI_SELECTED ) >= 0 && 
		(CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CServerList::OnServerBan()
{
	int selected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	MetaClient & client = CGCQLApp::sm_MetaClient;
	if ( (client.profile().flags & MetaClient::MODERATOR) == 0 )
		return;

	MetaClient::Server & game = m_Servers[ GetListCtrl().GetItemData( selected ) ];
	if ( MessageBox( _T("Confirm Ban?"), CString(game.name), MB_YESNO ) == IDYES )
		client.banServer( game.id, (60 * 60) * 24 );
}

void CServerList::OnUpdateServerBan(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( GetListCtrl().GetNextItem( -1, LVNI_SELECTED ) >= 0 && 
		(CGCQLApp::sm_MetaClient.profile().flags & MetaClient::MODERATOR) != 0 );
}

void CServerList::OnSize(UINT nType, int cx, int cy)
{
	CListView::OnSize(nType, cx, cy);
	
	if (::IsWindow(GetListCtrl().m_hWnd))
	{
		int	cwidth = cx / 5;
		GetListCtrl().SetColumnWidth(0 , cwidth );
		GetListCtrl().SetColumnWidth(1 , cwidth * 2);
		GetListCtrl().SetColumnWidth(2 , cwidth );
		GetListCtrl().SetColumnWidth(3 , cwidth );
	}
}
