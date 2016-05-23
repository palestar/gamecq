// PlayerView.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "PlayerView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayerView

IMPLEMENT_DYNCREATE(CPlayerView, CView)

CPlayerView::CPlayerView()
{
}

CPlayerView::~CPlayerView()
{
}


BEGIN_MESSAGE_MAP(CPlayerView, CView)
	//{{AFX_MSG_MAP(CPlayerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayerView drawing

void CPlayerView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CPlayerView diagnostics

#ifdef _DEBUG
void CPlayerView::AssertValid() const
{
	CView::AssertValid();
}

void CPlayerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPlayerView message handlers

int CPlayerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (! m_wndPlayerList.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, 0 ) )
		return -1;

	return 0;
}

void CPlayerView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if ( ::IsWindow( m_wndPlayerList.m_hWnd ) )
		m_wndPlayerList.MoveWindow( 0, 0, cx, cy, true );
}

BOOL CPlayerView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if ( m_wndPlayerList.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		return TRUE;
	
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


int CPlayerView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	//return MA_ACTIVATE;	
	return CView::OnMouseActivate( pDesktopWnd, nHitTest, message);
}
