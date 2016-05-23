// MainFrame.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "MainFrame.h"

#include "ChatFrame.h"
#include "CacheFrame.h"
#include "WebFrame.h"
#include "ServerFrame.h"

#include "Standard/Time.h"
#include "Standard/Settings.h"
#include "Network/MirrorClient.h"
#include "File/FileDisk.h"
#include "SelfUpdate/ClientUpdate.h"

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

const UINT					MSG_GCQL_COMMAND = RegisterWindowMessage( _T( "{1ACC295B-36B1-4b57-8C9C-D748A34B33D0}" ) );


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

CMainFrame::CMainFrame() : 
	m_bPlacementRestored( false ), 
	m_hIcon( NULL ),
	m_bUpdateCheck( false ),
	m_nLastUpdateCheck( Time::seconds() )
{}

CMainFrame::~CMainFrame()
{}

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE( MSG_GCQL_COMMAND, OnGCQLCommand )
END_MESSAGE_MAP()

// CMainFrame message handlers
int CMainFrame::OnCreateClient(LPCREATESTRUCT lpCreateStruct, CCreateContext* pContext)
{
	if (CFrameWnd::OnCreateClient( lpCreateStruct, pContext ) == -1)
		return -1;

	// try to prevent flickering of child-windows by hiding this window during the creation process...
	ShowWindow( SW_HIDE );

	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

	// add our icon to the taskbar
    NOTIFYICONDATA tnid; 
    tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = GetSafeHwnd(); 
    tnid.uID = ID_TASKBAR_ICON; 
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    tnid.uCallbackMessage = ID_TASKBAR_ICON;
    tnid.hIcon = m_hIcon; 
	strncpy(tnid.szTip, CGCQLApp::sm_Game.name,sizeof(tnid.szTip));

    Shell_NotifyIcon(NIM_ADD, &tnid); 

	CRect r;
	GetClientRect( &r );

	if (! m_wndTabs.Create( CBCGTabWnd::STYLE_3D_VS2005, r, this, ID_TAB_CONTROL, CBCGTabWnd::LOCATION_TOP ) )
	{
		TRACE( "Failed to create tab control!" );
		return -1;
	}
	m_wndTabs.SetTabBorderSize( 0 );

	m_TabImages.Create( 24,24,ILC_COLOR32 | ILC_MASK,0,0); 
	m_TabImages.Add( AfxGetApp()->LoadIcon( IDI_ROOM ) );				// 0
	m_wndTabs.SetImageList( m_TabImages.GetSafeHandle() );
	//m_wndTabs.SetActiveTabColor( CGCQLApp::s_ButtonHighlight );
	//m_wndTabs.AutoHideTabsScroll( true );

	if (!m_wndCaptionBar.Create (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this,
		ID_VIEW_CAPTIONBAR, -1))
	{
		//TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	m_wndCaptionBar.SetFlatBorder ();
	m_wndCaptionBar.SetBorderSize( 1 );
	m_wndCaptionBar.SetText (_T("GameCQ (c)2001-2011 PaleStar, Inc.") );
	m_bmpCaption.LoadBitmap (IDB_CAPTION);
	m_wndCaptionBar.SetBitmap (m_bmpCaption, RGB (255, 0, 255));
	m_wndCaptionBar.m_clrBarBackground = CGCQLApp::sm_BackgroundColor1;
	m_wndCaptionBar.m_clrBarBorder = CGCQLApp::sm_BackgroundColor1;
	m_wndCaptionBar.m_clrBarText = CGCQLApp::sm_TextColor;

	//---------------------------------------------------------------------------------------------------

	CChatFrame * pChatFrame = (CChatFrame *)RUNTIME_CLASS( CChatFrame )->CreateObject();
	if (! pChatFrame->LoadFrame( IDR_MAINFRAME, WS_CHILDWINDOW|FWS_ADDTOTITLE, &m_wndTabs, pContext ) )
	{
		TRACE( "Failed to create chat frame!" );
		return -1;
	}
	pChatFrame->InitialUpdateFrame( pContext->m_pCurrentDoc, FALSE );
	m_wndTabs.AddTab( pChatFrame, _T("Chat "), 0 );
	m_wndTabs.SetTabBkColor( 0, CGCQLApp::sm_BackgroundColor1 );

	CGCQLApp::sm_pChatFrame = pChatFrame;

	//---------------------------------------------------------------------------------------------------

	CWebFrame * pWebFrame = (CWebFrame *)RUNTIME_CLASS( CWebFrame )->CreateObject();
	if (! pWebFrame->LoadFrame( IDR_MAINFRAME, WS_CHILDWINDOW|FWS_ADDTOTITLE, &m_wndTabs, pContext ) )
	{
		TRACE( "Failed to create web frame!" );
		return -1;
	}
	pWebFrame->InitialUpdateFrame( pContext->m_pCurrentDoc, FALSE );
	m_wndTabs.AddTab( pWebFrame, _T("Browser "), 0 );
	m_wndTabs.SetTabBkColor( 1, CGCQLApp::sm_BackgroundColor1 );
	CGCQLApp::sm_pWebFrame = pWebFrame;

	//---------------------------------------------------------------------------------------------------

	CCacheFrame * pCacheFrame = (CCacheFrame *)RUNTIME_CLASS( CCacheFrame )->CreateObject();
	if (! pCacheFrame->LoadFrame( IDR_MAINFRAME, WS_CHILDWINDOW|FWS_ADDTOTITLE, &m_wndTabs, pContext ) )
	{
		TRACE( "Failed to create cache frame!" );
		return -1;
	}
	pCacheFrame->InitialUpdateFrame( pContext->m_pCurrentDoc, FALSE );
	m_wndTabs.AddTab( pCacheFrame, _T("Launch "), 0 );
	m_wndTabs.SetTabBkColor( 2, CGCQLApp::sm_BackgroundColor1 );
	CGCQLApp::sm_pCacheFrame = pCacheFrame;

	//---------------------------------------------------------------------------------------------------

	CServerFrame * pServerFrame = (CServerFrame *)RUNTIME_CLASS( CServerFrame )->CreateObject();
	if (! pServerFrame->LoadFrame( IDR_MAINFRAME, WS_CHILDWINDOW|FWS_ADDTOTITLE, &m_wndTabs, pContext ) )
	{
		TRACE( "Failed to create server frame!" );
		return -1;
	}
	pServerFrame->InitialUpdateFrame( pContext->m_pCurrentDoc, FALSE );
	m_wndTabs.AddTab( pServerFrame, _T("Servers "), 0 );
	m_wndTabs.SetTabBkColor( 3, CGCQLApp::sm_BackgroundColor1 );
	CGCQLApp::sm_pServerFrame = pServerFrame;

	//---------------------------------------------------------------------------------------------------

	ShowWindow( SW_SHOW );
	SetTimer( 0x3, 1000, NULL );

	SetActiveTab( TAB_CHAT );

	return 1;
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);

	if (! m_bPlacementRestored )
	{
		m_bPlacementRestored = true;

        WINDOWPLACEMENT * pWP = NULL;
        UINT nBytes = 0;

        if( AfxGetApp()->GetProfileBinary("CMainFrame", "WP", (LPBYTE*)&pWP, &nBytes) )
        {
            SetWindowPlacement( pWP );
			::free( pWP );
        }
	}

	//m_wndTabs.EnableAutoColor( true );
	//m_wndTabs.SetActiveTabColor( RGB( 180, 0, 0 ) );
	//m_wndTabs.SetActiveTabTextColor( RGB( 255,255,0 ) );
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect rCaptionBar;
	m_wndCaptionBar.GetClientRect( &rCaptionBar );
	int nCaptionHeight = rCaptionBar.bottom - rCaptionBar.top;

	m_wndTabs.MoveWindow( 0, nCaptionHeight, cx, cy - nCaptionHeight, TRUE );
}

void CMainFrame::OnClose()
{
	ShowWindow( SW_HIDE );
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	MetaClient & client = CGCQLApp::sm_MetaClient;

	client.update();

	if ( client.loggedIn() )
	{
		if ( ::IsWindow( m_wndCaptionBar.m_hWnd ) )
		{
			// Update the caption once a second
			dword online = Time::seconds() - CGCQLApp::sm_LoginTime;
			dword hours = online / 3600;
			dword minutes = (online / 60) % 60;
			dword seconds = online % 60;

			// display some important flags
			dword clientFlags = client.profile().flags;

			CString flags;

			if ( clientFlags & MetaClient::HIDDEN )
				flags += _T("[HIDDEN] ");
			else if ( clientFlags & MetaClient::AWAY )
				flags += _T("[AWAY] ");

			if ( clientFlags & MetaClient::MUTED )
				flags += _T("[MUTED] ");
				
			if ( clientFlags & MetaClient::ADMINISTRATOR )
				flags += _T("[ADMIN] ");
			else if ( clientFlags & MetaClient::MODERATOR )
				flags += _T("[MOD] ");		

			if ( clientFlags & MetaClient::EVENT )
				flags += _T("[EVENT] ");

			if ( clientFlags & MetaClient::DEVELOPER )
				flags += _T("[DEV] ");

			if ( clientFlags & MetaClient::SERVER )
				flags += _T("[SERVER] ");

			if ( clientFlags & MetaClient::SUBSCRIBED )
				flags += _T("[SUBSCRIBED] ");
			else
				flags += _T("[FREE] ");

			if ( clientFlags & MetaClient::BETA )
				flags += _T("[BETA]");

			// update the caption
			CString captionText;
			captionText.Format( _T("Logged in as '%s'    Status: '%s'   Flags: %s  Online: %2.2u:%2.2u:%2.2u"), 
				CString( client.profile().name ),
				CString( client.profile().status ), flags,
				hours, minutes, seconds );

			CString previousText;
			m_wndCaptionBar.GetWindowText( previousText );
			m_wndCaptionBar.UpdateText( captionText );

			if ( previousText.GetLength() != captionText.GetLength() )
				RecalcLayout();
		}

		if ( CGCQLApp::sm_nUpdateCheckInterval > 0 && Time::seconds() > (m_nLastUpdateCheck + CGCQLApp::sm_nUpdateCheckInterval) )
		{
			m_nLastUpdateCheck = Time::seconds();

			if (! m_bUpdateCheck )
			{
				m_bUpdateCheck = true;

				Settings settings( CGCQLApp::sConfigName );
				CharString sMirrorAddress( settings.get( "mirrorAddress", "mirror-server.palestar.com" ) );
				int nMirrorPort( settings.get("mirrorPort", 9100 ) );

				MirrorClient client;
				if ( client.open( sMirrorAddress, nMirrorPort, FileDisk::home(), NULL, true ) )
				{
					dword remoteCRC = client.getCRC();
					if ( remoteCRC != 0 && remoteCRC != client.getLocalCRC() )
					{
						client.close();

						// files have been changed, pop up a message box asking the user if they want to update..
						if ( MessageBox( "A new update for GCQL is available, do you wish to update your client?", "Update Available", MB_YESNO ) == IDYES )
						{
							if (! ClientUpdate::updateSelf( CGCQLApp::sConfigName, sMirrorAddress, nMirrorPort ) )
								MessageBox( _T("Failed to check for updates."), _T("Failure"), MB_OK );
						}
					}
				}

				m_bUpdateCheck = false;
			}
		}
	}

	//CFrameWnd::OnTimer(nIDEvent);
}

//---------------------------------------------------------------------------------------------------

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == ID_TASKBAR_ICON)
	{
		switch(lParam)
		{
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			{
				// get the current cursor position
				CPoint point;
				GetCursorPos( &point );

				// bring up context menu
				CMenu ctmenu;		// context menu
				//ctmenu.CreatePopupMenu();
				ctmenu.LoadMenu( IDR_POPUP_GAMECQ );
				ctmenu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTALIGN,point.x,point.y, this);
				ctmenu.DestroyMenu();
			}
			break;
		}
	}
	
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

BOOL CMainFrame::DestroyWindow()
{
	WINDOWPLACEMENT wp;
	if ( GetWindowPlacement( &wp ) )
		AfxGetApp()->WriteProfileBinary("CMainFrame", "WP", (LPBYTE)&wp, sizeof(wp));

	// remove icon from taskbar
    NOTIFYICONDATA tnid;  
    tnid.cbSize = sizeof(NOTIFYICONDATA);     
	tnid.hWnd = GetSafeHwnd(); 
    tnid.uID = ID_TASKBAR_ICON;  
	Shell_NotifyIcon(NIM_DELETE, &tnid); 

	//SaveBarState( _T("ChatFrame") );
	return CFrameWnd::DestroyWindow();
}

BOOL CMainFrame::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	CBCGFrameWnd * pFrame = (CBCGFrameWnd *)m_wndTabs.GetActiveWnd();
	return pFrame->OnShowPopupMenu( pMenuPopup );
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CBCGFrameWnd * pFrame = (CBCGFrameWnd *)m_wndTabs.GetActiveWnd();
	return pFrame->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

//---------------------------------------------------------------------------------------------------

bool CMainFrame::SetActiveTab( Tabs a_nTab )
{
	m_wndTabs.SetActiveTab( a_nTab );

	ShowWindow( SW_SHOWNA );
	SetForegroundWindow();

	return true;
}

LRESULT CMainFrame::OnGCQLCommand(WPARAM, LPARAM lParam)
{
	// unhide this window if hidden...
	ShowWindow( SW_SHOWNORMAL );
	SetForegroundWindow();

	dword nGameID = 0;
	CharString sLaunchProgram;
	CharString sLaunchAddress;
	int nLaunchPort = 0;

	// process the command line
	//CommandLine commands( (const char *)lParam );
	//for(int i=0;i<commands.argumentCount();i++)
	//{
	//	const char * pArg = commands.argument( i );
	//	if ( pArg[0] == '-' )
	//	{
	//		switch( tolower( pArg[1] ) )
	//		{
	//		case 'g':	// GameId
	//			if ( (i + 1 ) < commands.argumentCount() )
	//			{
	//				nGameID = commands.argumentDWORD( i + 1 );
	//				++i;
	//			}
	//			break;
	//		case 'r':	// Run 
	//			if ( (i + 1 ) < commands.argumentCount() )
	//			{
	//				sLaunchProgram = commands.argument( i + 1 );
	//				++i;
	//			}
	//			break;
	//		case 'a':	// Address
	//			if ( (i + 1 ) < commands.argumentCount() )
	//			{
	//				sLaunchAddress = commands.argument( i + 1 );
	//				++i;
	//			}
	//			break;
	//		case 'p':	// Port
	//			if ( (i + 1 ) < commands.argumentCount() )
	//			{
	//				nLaunchPort = commands.argumentInt( i + 1 );
	//				++i;
	//			}
	//			break;
	//		}
	//	}
	//}
	
	return 0;
} 


//---------------------------------------------------------------------------------------------------
