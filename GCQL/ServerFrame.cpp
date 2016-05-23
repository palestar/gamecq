// ServerFrame.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "ServerFrame.h"
#include "ServerList.h"
#include ".\serverframe.h"

//---------------------------------------------------------------------------------------------------

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
};

const int MAX_FILTERS = 64;
const int ID_FILTER_MSG_BEGIN = WM_USER + 1000;
const int ID_FILTER_MSG_END = ID_FILTER_MSG_BEGIN + MAX_FILTERS;

//---------------------------------------------------------------------------------------------------

// CServerFrame
IMPLEMENT_DYNCREATE(CServerFrame, CFrameWnd)

CServerFrame::CServerFrame() : m_bPlacementRestored( false ), m_FilterButton( -1 )
{}

CServerFrame::~CServerFrame()
{}


BEGIN_MESSAGE_MAP(CServerFrame, CFrameWnd)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_COMMAND_RANGE(ID_FILTER_MSG_BEGIN, ID_FILTER_MSG_END, OnFilter )
END_MESSAGE_MAP()


int CServerFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableDocking(CBRS_ALIGN_TOP);

	if (! m_wndServerBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1), ID_VIEW_NAVIGATIONBAR) ||
		!m_wndServerBar.LoadToolBar(IDR_SERVERBAR, 0, 0, true))
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndServerBar.SetWindowText(_T("Servers"));
	m_wndServerBar.EnableTextLabels();

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		//TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.GetStatusBarCtrl().SetBkColor( CGCQLApp::sm_StatusColor );

	return 0;
}


BOOL CServerFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	pContext->m_pNewViewClass = RUNTIME_CLASS(CServerList);
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CServerFrame::OnClose()
{
	ShowWindow( SW_HIDE );
}

void CServerFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);

	if (! m_bPlacementRestored )
	{
		m_bPlacementRestored = true;

        WINDOWPLACEMENT * pWP = NULL;
        UINT nBytes = 0;

        if( AfxGetApp()->GetProfileBinary("CServerFrame", "WP", (LPBYTE*)&pWP, &nBytes) )
        {
            SetWindowPlacement( pWP );
			::free( pWP );
        }
	}
}

BOOL CServerFrame::DestroyWindow()
{
	WINDOWPLACEMENT wp;
	if ( GetWindowPlacement( &wp ) )
		AfxGetApp()->WriteProfileBinary("CServerFrame", "WP", (LPBYTE)&wp, sizeof(wp));

	return CFrameWnd::DestroyWindow();
}

LRESULT CServerFrame::OnToolbarReset(WPARAM wp,LPARAM)
{
	UINT uiToolBarId = (UINT) wp;

	switch (uiToolBarId)
	{
	case IDR_SERVERBAR:
		{
			m_FilterButton = m_wndServerBar.CommandToIndex( ID_SERVER_FILTER );

			CMenu FilterMenu;
			FilterMenu.CreatePopupMenu();
			CString FilterLabel;
			FilterLabel.LoadString( ID_SERVER_FILTER );

			m_wndServerBar.ReplaceButton( ID_SERVER_FILTER, 
				CBCGToolbarMenuButton(-1, FilterMenu, m_FilterButton, FilterLabel ));
		}
		break;
	}

	return 0;
}

BOOL CServerFrame::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	CFrameWnd::OnShowPopupMenu (pMenuPopup);

	if ( pMenuPopup != NULL && pMenuPopup->GetParentToolBar() == &m_wndServerBar )
	{
		if ( m_wndServerBar.GetButton( m_FilterButton ) == pMenuPopup->GetParentButton() )
		{
			pMenuPopup->RemoveAllItems();

			CMenu FilterMenu;
			FilterMenu.CreatePopupMenu();
			FilterMenu.AppendMenu( MF_STRING, ID_FILTER_MSG_BEGIN, _T("None") );
			FilterMenu.AppendMenu( MF_SEPARATOR );

			CServerList * pServers = (CServerList *)GetActiveView();

			CCacheList * pCache = CCacheList::getCacheList();
			if (! pCache )
				return FALSE;

			for(int i=0;i<pCache->m_Programs.size() && i < (MAX_FILTERS-1);++i)
			{
				CCacheList::Program * pProgram = pCache->m_Programs[ i ];
				if (! pProgram->m_bNeedServer || !pProgram->m_bCanUse )
					continue;

				bool bChecked = pProgram->m_nLobbyId == pServers->getGameIdFilter() && 
					pProgram->m_nServerType == pServers->getServerTypeFilter();

				UINT nID = ID_FILTER_MSG_BEGIN + i + 1;
				FilterMenu.AppendMenu( MF_STRING, nID, pProgram->m_sDescription);

				if ( bChecked )
					FilterMenu.CheckMenuItem( nID, MF_CHECKED );
			}

			pMenuPopup->GetMenuBar()->ImportFromMenu(FilterMenu, TRUE);
		}
	}

	return TRUE;
}


void CServerFrame::OnFilter( UINT nID )
{
	int nGame = (nID - ID_FILTER_MSG_BEGIN) - 1;

	CServerList * pServers = (CServerList *)GetActiveView();
	if ( nGame >= 0 )
	{
		CCacheList * pCache = CCacheList::getCacheList();
		CCacheList::Program * pProgram = pCache->m_Programs[ nGame ];
		pServers->setGameIdFilter( pProgram->m_nLobbyId );
		pServers->setServerTypeFilter( pProgram->m_nServerType );
		pServers->OnServerRefresh();
	}
	else
	{
		// clear all filters
		pServers->setGameIdFilter( 0 );
		pServers->setServerTypeFilter( 0 );
		pServers->OnServerRefresh();
	}
}

