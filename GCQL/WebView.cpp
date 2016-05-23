// WebView.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "GCQLDoc.h"
#include "WebView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWebView

IMPLEMENT_DYNCREATE(CWebView, CHtmlView)

CWebView::CWebView()
{
	//{{AFX_DATA_INIT(CWebView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CWebView::~CWebView()
{
}

void CWebView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWebView, CHtmlView)
	//{{AFX_MSG_MAP(CWebView)
	ON_COMMAND(ID_NAVIGATION_BACK, OnNavigationBack)
	ON_COMMAND(ID_NAVIGATION_FORWARD, OnNavigationForward)
	ON_COMMAND(ID_NAVIGATION_REFRESH, OnNavigationRefresh)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebView diagnostics

#ifdef _DEBUG
void CWebView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CWebView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

//---------------------------------------------------------------------------------------------------

CWebView * CWebView::getWebView()
{
	return (CWebView *)CGCQLApp::sm_pWebFrame->GetActiveView();
}

//---------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CWebView message handlers

void CWebView::OnNavigationBack() 
{
	GoBack();	
}

void CWebView::OnNavigationForward() 
{
	GoForward();
}

void CWebView::OnNavigationRefresh() 
{
	Refresh();
}

void CWebView::OnInitialUpdate() 
{
	if ( CGCQLApp::sm_Game.home.length() > 0 )
		Navigate2( CString( CGCQLApp::sm_Game.home  + String().format("&sid=%u", CGCQLApp::sm_SessionID ) ),NULL,NULL);
	else
		Navigate2(_T("http://www.palestar.com"),NULL,NULL);
}

void CWebView::OnEditCopy() 
{
	ExecWB(OLECMDID_COPY, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL); 
}

void CWebView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( QueryStatusWB(OLECMDID_COPY) & OLECMDF_ENABLED );
}

void CWebView::OnEditCut() 
{
	ExecWB(OLECMDID_CUT, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL); 
}

void CWebView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( QueryStatusWB(OLECMDID_CUT) & OLECMDF_ENABLED );
}

void CWebView::OnEditPaste() 
{
	ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL); 
}

void CWebView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( QueryStatusWB(OLECMDID_PASTE) & OLECMDF_ENABLED );
}
