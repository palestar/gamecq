/*
	CacheList.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/


#include "stdafx.h"
#include "GCQL.h"
#include "CacheList.h"
#include "ServerList.h"
#include "MainFrame.h"
#include "tinyxml.h"

#include "File/FileDisk.h"
#include "Standard/Process.h"
#include "Standard/Time.h"

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))


//---------------------------------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CCacheList, CListView)

CCacheList::CCacheList() : m_SortColumn( 0 ), m_SortAscending( true ), m_bInitialized( false )
{}

CCacheList::~CCacheList()
{}

//---------------------------------------------------------------------------------------------------

static bool ConfirmProgramInstall( CCacheList::Program * pProgram )
{
	return MessageBox( NULL, CharString().format( "Are you sure you wish to install %s?", pProgram->m_sName ), 
		"Confirm Install", MB_YESNO|MB_SETFOREGROUND|MB_TOPMOST ) == IDYES;
}

static bool ConfirmProgramDelete( CCacheList::Program * pProgram )
{
	return MessageBox( NULL, CharString().format( "Are you sure you wish to uninstall %s?", pProgram->m_sName ), 
		"Confirm Delete", MB_YESNO|MB_SETFOREGROUND|MB_TOPMOST ) == IDYES;
}

static const char * GetAttribute( TiXmlElement * pElement, const char * pName )
{
	const char * pValue = pElement->Attribute( pName );
	if ( pValue != NULL )
		return pValue;
	return "";
}

static dword GetDWORDAttribute( TiXmlElement * pElement, const char * pName,dword nDefault = 0 )
{
	const char * pValue = pElement->Attribute( pName );
	if ( pValue != NULL )
		return strtoul( pValue, NULL, 10 );
	return nDefault;
}

//---------------------------------------------------------------------------------------------------

bool CCacheList::initializeCache()
{
	if (! m_CacheConfig.LoadFile( "CacheConfig.xml" ) )
		return false;

	TiXmlElement * pRoot = m_CacheConfig.FirstChildElement();
	CharString sDirectory = pRoot->Attribute( "Directory" );
	m_sCacheDirectory = FileDisk::currentDirectory() + "\\" + sDirectory;

	// ensure the cache directory exists!
	FileDisk::createDirectory( m_sCacheDirectory );

	TiXmlElement * pElement = pRoot->FirstChildElement();
	while( pElement != NULL )
	{
		CCacheList::Program * pProgram = new Program();
		m_Programs.push( pProgram );

		pProgram->m_nIndex = m_Programs.size() - 1;
		pProgram->m_pElement = pElement;
		pProgram->m_pUpdate = NULL;
		pProgram->m_nLastUpdate = 0;
		
		pProgram->m_sName = GetAttribute( pElement, "Name" );
		pProgram->m_sExecutable = m_sCacheDirectory + GetAttribute( pElement, "Executable" );
		pProgram->m_bNeedServer = GetDWORDAttribute( pElement, "NeedServer", 0 ) != 0;
		pProgram->m_bTrackProcess = GetDWORDAttribute( pElement, "TrackProcess", 1 ) != 0;
		pProgram->m_sDescription = GetAttribute( pElement, "Description" );
		pProgram->m_sCommandLine = GetAttribute( pElement, "CommandLine" );
		pProgram->m_sMirror = GetAttribute( pElement, "Mirror" );
		pProgram->m_sDirectory = m_sCacheDirectory + GetAttribute( pElement, "Directory" );
		
		const char * pDepends = pElement->Attribute( "Depends" );
		if ( pDepends != NULL )
		{
			for(int i=0;i<m_Programs.size();i++)
				if ( strcmp( m_Programs[i]->m_sName, pDepends ) == 0 )
				{
					pProgram->m_pDepends = m_Programs[i];
					break;
				}
		}

		pProgram->m_nServerType = GetDWORDAttribute( pElement, "ServerType" );
		pProgram->m_nLobbyId = GetDWORDAttribute( pElement, "LobbyId" );
		pProgram->m_nUserFlags = GetDWORDAttribute( pElement, "UserFlags" );
		pProgram->m_sCategory = GetAttribute( pElement, "Category" );
		pProgram->m_bInstalled = isInstalled( pProgram );
		pProgram->m_bCanUse = pProgram->m_nUserFlags == 0 || (CGCQLApp::sm_MetaClient.profile().flags & pProgram->m_nUserFlags) != 0;

		if ( pProgram->m_bCanUse )
		{
			int n = m_Programs.size() - 1;
			int nItem = GetListCtrl().InsertItem( n, pProgram->m_sDescription, n );
			GetListCtrl().SetItemData( nItem, n );
			GetListCtrl().SetItemText( nItem, 1, pProgram->m_sStatus );
			GetListCtrl().SetItemText( nItem, 2, pProgram->m_sCategory );
		}

		pElement = pElement->NextSiblingElement();
	}

	updateProgramIcons();

	return true;
}

bool CCacheList::shutdownCache()
{
	GetListCtrl().DeleteAllItems();
	for(int i=0;i<m_Programs.size();i++)
	{
		CCacheList::Program * pProgram = m_Programs[ i ];
		if ( pProgram->m_pUpdate != NULL )
			stopUpdate( pProgram );
		if ( pProgram->m_hIcon != NULL )
			::DestroyIcon( pProgram->m_hIcon );
		delete pProgram;
	}
	m_Programs.release();

	return true;
}

CCacheList::Program * CCacheList::findProgram( const char * pName )
{
	for(int i=0;i<m_Programs.size();i++)
	{
		CCacheList::Program * pProgram = m_Programs[ i ];
		if ( stricmp( pProgram->m_sName, pName ) == 0 )
			return pProgram;
	}

	return NULL;
}

CCacheList::Program * CCacheList::findProgram( dword nGameId, dword nServerType )
{
	for(int i=0;i<m_Programs.size();i++)
	{
		CCacheList::Program * pProgram = m_Programs[ i ];
		if ( pProgram->m_nServerType == nServerType && (pProgram->m_nLobbyId == 0 || pProgram->m_nLobbyId == nGameId) )
			return pProgram;
	}

	return NULL;
}

bool CCacheList::isInstalled( Program * pProgram )
{
	if (! pProgram )
		return false;
	return FileDisk::fileExists( pProgram->m_sExecutable );
}

bool CCacheList::isUpdating( Program * pProgram )
{
	if (! pProgram )
		return false;
	if ( pProgram->m_pDepends != NULL )
		return isUpdating( pProgram->m_pDepends );
	return pProgram->m_pUpdate != NULL && !pProgram->m_pUpdate->isDone();	
}

bool CCacheList::isUpdated( Program * pProgram )
{
	if (! pProgram )
		return false;
	if ( pProgram->m_pDepends != NULL )
		return isUpdated( pProgram->m_pDepends );
	return pProgram->m_pUpdate != NULL && pProgram->m_pUpdate->isDone();
}

bool CCacheList::startUpdate( Program * pProgram )
{
	if (! pProgram )
		return false;
	if ( pProgram->m_pDepends != NULL )
		return startUpdate( pProgram->m_pDepends );
	if ( pProgram->m_pUpdate != NULL && !pProgram->m_pUpdate->isDone() )
		return true;		// already updating...

	stopUpdate( pProgram );

	if ( pProgram->m_sDirectory.length() == 0 || pProgram->m_sMirror.length() == 0 )
		return true;		// this program does not update

	FileDisk::createDirectory( pProgram->m_sDirectory );

	pProgram->m_pUpdate = new ClientUpdate( pProgram->m_sDirectory, pProgram->m_sMirror, 
		0, 0, this, CGCQLApp::sm_SessionID, false );
	pProgram->m_pUpdate->Create( ClientUpdate::IDD, this );
	pProgram->m_pUpdate->ShowWindow( SW_HIDE );		// make sure the dialog is invisible
	pProgram->m_nLastUpdate = Time::seconds();
	pProgram->m_sStatus = "Updating...";

	return true;
}

bool CCacheList::deleteFiles( Program * pProgram )
{
	if (! pProgram )
		return false;
	if (! ConfirmProgramDelete( pProgram ) )
		return false;
	if (! stopUpdate( pProgram ) )
		return false;

	FileDisk::removeDirectory( pProgram->m_sDirectory, true );

	pProgram->m_bInstalled = false;
	updateProgramIcons();
	return true;
}

bool CCacheList::stopUpdate( Program * pProgram )
{
	if (! pProgram )
		return false;
	if ( pProgram->m_pDepends != NULL )
		return stopUpdate( pProgram->m_pDepends );

	if ( pProgram->m_pUpdate != NULL )
	{
		delete pProgram->m_pUpdate;
		pProgram->m_pUpdate = NULL;
	}
	return true;
}

bool CCacheList::launchProgram( Program * pProgram, const char * pAddress, unsigned int nPort )
{
	if (! pProgram )
		return false;

	if (! isInstalled( pProgram ) && !ConfirmProgramInstall( pProgram ) )
		return false;		// user declined to install the program..

	if ( pProgram->m_bNeedServer && strlen( pAddress ) == 0 )
	{
		//if ( MessageBox( "This program requires a server, would you like to show the available servers?", "Confirm", MB_YESNO ) == IDYES )
		{
			// user needs to select a server, just show the server list..
			((CGCQLApp *)AfxGetApp())->OnViewServers();

			CServerList * pServers = CServerList::getServerList();
			pServers->setGameIdFilter( pProgram->m_nLobbyId );
			pServers->setServerTypeFilter( pProgram->m_nServerType );
			pServers->OnServerRefresh();
		}
		return true;
	}

	pProgram->m_bLaunch = true;
	pProgram->m_sAddress = pAddress;
	pProgram->m_nPort = nPort;
	startUpdate( pProgram );

	// program will be launched once update is completed..
	return true;
}

bool CCacheList::startProcess( Program * pProgram )
{
	if (! pProgram )
		return false;
	if (! pProgram->m_bLaunch )
		return false;
	pProgram->m_bLaunch = false;

	if ( pProgram->m_pProcess != NULL )
		return true;		// process is already running

	CharString sCommandLine = pProgram->m_sCommandLine;
	sCommandLine.replace( "$DIR", pProgram->m_sDirectory );
	sCommandLine.replace( "$SID", CharString().format("%u",CGCQLApp::sm_SessionID) );
	sCommandLine.replace( "$ADDRESS", pProgram->m_sAddress );
	sCommandLine.replace( "$PORT", CharString().format("%u", pProgram->m_nPort) );
	
	void * pProcess = Process::start( pProgram->m_sExecutable + " " + sCommandLine, pProgram->m_sDirectory );
	if (! pProcess )
		return false;
	if ( pProgram->m_bTrackProcess )
		pProgram->m_pProcess = pProcess;

	return true;
}

//---------------------------------------------------------------------------------------------------

static int CALLBACK SortProgramList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CCacheList * pList = (CCacheList *)lParamSort;

	Array< CCacheList::Program * > & programs = pList->m_Programs;

	int result = 0;
	switch( pList->m_SortColumn )
	{
	case 0:
		result = programs[ lParam1 ]->m_sDescription.compareNoCase( programs[ lParam2 ]->m_sDescription );
		break;
	case 1:
		result = programs[ lParam1 ]->m_sStatus.compareNoCase( programs[ lParam2 ]->m_sStatus );
		break;
	case 2:
		result = programs[ lParam1 ]->m_sCategory.compareNoCase(programs[ lParam2 ]->m_sCategory );
		break;
	}

	if (! pList->m_SortAscending )
		result = -result;

	return result;
}

void CCacheList::sortPrograms()
{
	GetListCtrl().SortItems( SortProgramList, (dword)this );
}

void CCacheList::updateProgramIcons()
{
	GetListCtrl().SetImageList( NULL, LVSIL_SMALL );
	m_ProgramIcons.DeleteImageList();
	m_ProgramIcons.Create( 16, 16, ILC_COLOR32 | ILC_MASK, 0, 0 );

	for(int i=0;i<m_Programs.size();++i)
	{
		CCacheList::Program * pProgram = m_Programs[i];
		if ( pProgram->m_hIcon )
		{
			DestroyIcon( pProgram->m_hIcon );
			pProgram->m_hIcon = NULL;
		}

		pProgram->m_hIcon = ::ExtractIcon( AfxGetApp()->m_hInstance, pProgram->m_sExecutable, 0 );
		if (! pProgram->m_hIcon )
			pProgram->m_hIcon = AfxGetApp()->LoadIcon( IDI_PROGRAM );
		m_ProgramIcons.Add( pProgram->m_hIcon );
	}

	GetListCtrl().SetImageList( &m_ProgramIcons, LVSIL_SMALL );
}

//---------------------------------------------------------------------------------------------------

CCacheList * CCacheList::getCacheList()
{
	return (CCacheList *)CGCQLApp::sm_pCacheFrame->GetActiveView();
}

//---------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CCacheList, CListView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_TIMER()
	ON_COMMAND(ID_GAMES_INSTALL, OnGamesInstall)
	ON_UPDATE_COMMAND_UI(ID_GAMES_INSTALL, OnUpdateGamesInstall)
	ON_COMMAND(ID_GAMES_LAUNCH, OnGamesLaunch)
	ON_UPDATE_COMMAND_UI(ID_GAMES_LAUNCH, OnUpdateGamesLaunch)
	ON_COMMAND(ID_GAMES_DELETE, OnGamesDelete)
	ON_UPDATE_COMMAND_UI(ID_GAMES_DELETE, OnUpdateGamesDelete)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
//	ON_WM_SIZE()
	ON_COMMAND(ID_GAMES_UPDATE, OnGamesUpdate)
	ON_UPDATE_COMMAND_UI(ID_GAMES_UPDATE, OnUpdateGamesUpdate)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CCacheList diagnostics

#ifdef _DEBUG
void CCacheList::AssertValid() const
{
	CListView::AssertValid();
}

void CCacheList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

void CCacheList::OnInitialUpdate()
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
	GetClientRect( r );

	// setup the report columns
	int	cwidth = r.Width() / 4;
	GetListCtrl().InsertColumn(0,_T("Name"),LVCFMT_LEFT,cwidth );
	GetListCtrl().InsertColumn(1,_T("Status"),LVCFMT_LEFT,cwidth * 2);
	GetListCtrl().InsertColumn(2,_T("Category"),LVCFMT_LEFT,cwidth);

	initializeCache();
	SetTimer( 0x7, 500, NULL );	
	m_bInitialized = true;
}

void CCacheList::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{}

void CCacheList::OnTimer(UINT nIDEvent)
{
	if ( nIDEvent == 0x7 )
	{
		bool bUpdateIcons = false;

		// update all program status
		for(int i=0;i<m_Programs.size();++i)
		{
			Program * pProgram = m_Programs[ i ];
			if ( pProgram->m_pProcess != NULL )
			{
				if (! Process::active( pProgram->m_pProcess ) )
				{
					// program has exited...
					Process::close( pProgram->m_pProcess );
					pProgram->m_pProcess = NULL;

					// rejoin the player to their chat room...
					if ( pProgram->m_nLobbyId != 0 )
						CGCQLApp::selectGame( pProgram->m_nLobbyId, true );
				}
				else if ( pProgram->m_sStatus != "Running." )
				{
					pProgram->m_sStatus = "Running.";
					CGCQLApp::sm_MetaClient.sendStatus( CharString().format( "Playing %s.", pProgram->m_sName.cstr() ) );
				}	
			}
			else if ( isUpdating( pProgram ) )
			{
				if ( pProgram->m_pUpdate != NULL )
					pProgram->m_sStatus = pProgram->m_pUpdate->getStatus();
				else
					pProgram->m_sStatus = "Updating.";
			}
			else if ( isInstalled( pProgram ) )
			{
				if ( pProgram->m_pUpdate != NULL && pProgram->m_pUpdate->isError() )
					pProgram->m_sStatus = pProgram->m_pUpdate->getStatus();				// continue to display the error if one occured..
				else
					pProgram->m_sStatus = "Ready.";
				if (! pProgram->m_bInstalled )
				{
					pProgram->m_bInstalled = true;
					bUpdateIcons = true;
				}
				if ( pProgram->m_bLaunch )
					startProcess( pProgram );
			}
			else
			{
				pProgram->m_sStatus = "Not Installed.";
			}
		}

		for(int k=0;k<GetListCtrl().GetItemCount();++k)
		{
			Program * pProgram = m_Programs[ GetListCtrl().GetItemData( k ) ];
			if ( GetListCtrl().GetItemText( k, 1 ) != pProgram->m_sStatus )
				GetListCtrl().SetItemText( k, 1, pProgram->m_sStatus );
		}

		if ( bUpdateIcons )
			updateProgramIcons();
		if ( m_SortColumn == 1 )
			sortPrograms();			// resort if the current column is program status

		return;
	}

	CListView::OnTimer(nIDEvent);
}

void CCacheList::OnGamesInstall()
{
	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		if ( ConfirmProgramInstall( pProgram ) )
			startUpdate( pProgram );
	}
}

void CCacheList::OnUpdateGamesInstall(CCmdUI *pCmdUI)
{
	bool bEnabled = false;

	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		bEnabled = !isInstalled( pProgram ) && !isUpdating( pProgram );
	}

	pCmdUI->Enable( bEnabled );
}


void CCacheList::OnGamesUpdate()
{
	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
		startUpdate( m_Programs[ GetListCtrl().GetItemData( nSelected ) ] );
}

void CCacheList::OnUpdateGamesUpdate(CCmdUI *pCmdUI)
{
	bool bEnabled = false;

	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		bEnabled = isInstalled( pProgram ) && !isUpdating( pProgram );
	}

	pCmdUI->Enable( bEnabled );
}

void CCacheList::OnGamesLaunch()
{
	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		launchProgram( pProgram, "", 0 );
	}
}

void CCacheList::OnUpdateGamesLaunch(CCmdUI *pCmdUI)
{}

void CCacheList::OnGamesDelete()
{
	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		deleteFiles( pProgram );
	}
}

void CCacheList::OnUpdateGamesDelete(CCmdUI *pCmdUI)
{
	bool bEnabled = false;

	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		bEnabled = isInstalled( pProgram ) || isUpdating( pProgram );
	}

	pCmdUI->Enable( bEnabled );
}

BOOL CCacheList::DestroyWindow()
{
	shutdownCache();
	return CListView::DestroyWindow();
}

void CCacheList::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
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

	sortPrograms();
	*pResult = 0;
}

void CCacheList::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	bool bEnabled = false;

	int nSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected >= 0 )
	{
		Program * pProgram = m_Programs[ GetListCtrl().GetItemData( nSelected ) ];
		launchProgram( pProgram, "", 0 );
	}

	*pResult = 0;
}


void CCacheList::OnSize(UINT nType, int cx, int cy)
{
	CListView::OnSize(nType, cx, cy);

	if (::IsWindow(GetListCtrl().m_hWnd))
	{
		int	cwidth = cx / 4;
		GetListCtrl().SetColumnWidth(0 , cwidth );
		GetListCtrl().SetColumnWidth(1 , cwidth * 2);
		GetListCtrl().SetColumnWidth(2 , cwidth );
	}
}
