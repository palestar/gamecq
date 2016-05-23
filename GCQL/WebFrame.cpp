/*
	WebFrame.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "stdafx.h"
#include "GCQL.h"
#include "WebFrame.h"
#include "WebView.h"
#include ".\webframe.h"

//---------------------------------------------------------------------------------------------------

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
};

//---------------------------------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CWebFrame, CFrameWnd)

CWebFrame::CWebFrame() : m_bPlacementRestored( false )
{}

CWebFrame::~CWebFrame()
{}


BEGIN_MESSAGE_MAP(CWebFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_NAVBAR_HOME, OnNavbarHome)
	ON_UPDATE_COMMAND_UI(ID_NAVBAR_HOME, OnUpdateNavbarHome)
	ON_COMMAND(ID_NAVBAR_NEWS, OnNavbarNews)
	ON_UPDATE_COMMAND_UI(ID_NAVBAR_NEWS, OnUpdateNavbarNews)
	ON_COMMAND(ID_NAVBAR_FORUM, OnNavbarForum)
	ON_UPDATE_COMMAND_UI(ID_NAVBAR_FORUM, OnUpdateNavbarForum)
	ON_COMMAND(ID_NAVBAR_DOWNLOADS, OnNavbarDownloads)
	ON_UPDATE_COMMAND_UI(ID_NAVBAR_DOWNLOADS, OnUpdateNavbarDownloads)
	ON_COMMAND(ID_NAVBAR_PROFILE, OnNavbarProfile)
	ON_UPDATE_COMMAND_UI(ID_NAVBAR_PROFILE, OnUpdateNavbarProfile)
END_MESSAGE_MAP()


int CWebFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableDocking(CBRS_ALIGN_TOP);

	if (!m_wndNavBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1)) ||
		!m_wndNavBar.LoadToolBar(IDR_NAVBAR, 0, 0, true))
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndNavBar.SetWindowText(_T("Navgiation"));
	m_wndNavBar.EnableTextLabels();

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

BOOL CWebFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	pContext->m_pNewViewClass = RUNTIME_CLASS(CWebView);
	if (!CFrameWnd::OnCreateClient(lpcs, pContext) )
		return FALSE;

	return TRUE;
}

void CWebFrame::OnNavbarHome() 
{
	CWebView * pView = (CWebView *)GetActiveView();
	pView->Navigate2( CString( CGCQLApp::sm_Game.home + CharString().format("&sid=%u", CGCQLApp::sm_SessionID) ) );
}

void CWebFrame::OnUpdateNavbarHome(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( CGCQLApp::sm_Game.home.length() > 0 );
}

void CWebFrame::OnNavbarNews() 
{
	CWebView * pView = (CWebView *)GetActiveView();
	pView->Navigate2( CString( CGCQLApp::sm_Game.news + CharString().format("&sid=%u", CGCQLApp::sm_SessionID) ) );
}

void CWebFrame::OnUpdateNavbarNews(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( CGCQLApp::sm_Game.news.length() > 0 );
}

void CWebFrame::OnNavbarForum() 
{
	CWebView * pView = (CWebView *)GetActiveView();
	pView->Navigate2( CString( CGCQLApp::sm_Game.forum + CharString().format("&sid=%u", CGCQLApp::sm_SessionID) ) );
}

void CWebFrame::OnUpdateNavbarForum(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( CGCQLApp::sm_Game.forum.length() > 0 );
}

void CWebFrame::OnNavbarDownloads() 
{
	CWebView * pView = (CWebView *)GetActiveView();
	pView->Navigate2( CString( CGCQLApp::sm_Game.download + CharString().format("&sid=%u", CGCQLApp::sm_SessionID) ) );
}

void CWebFrame::OnUpdateNavbarDownloads(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( CGCQLApp::sm_Game.download.length() > 0 );
}

void CWebFrame::OnNavbarProfile() 
{
	CWebView * pView = (CWebView *)GetActiveView();
	pView->Navigate2( CString( CGCQLApp::sm_Game.profile + CharString().format("&sid=%u", CGCQLApp::sm_SessionID ) ) );
}

void CWebFrame::OnUpdateNavbarProfile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( CGCQLApp::sm_Game.profile.length() > 0 );
}

