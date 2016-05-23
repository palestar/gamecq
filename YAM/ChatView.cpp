// ChatView.cpp : implementation file
//

#include "stdafx.h"
#include "ChatView.h"
#include "FilterText.h"

#include "Standard/Time.h"
#include <mmsystem.h>
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

//const char * BODY_HEAD = "<table border=\\\"0\\\" width=\\\"100%\\\" style=\\\"font-family: Verdana; font-size: %dpt\\\">";
const char * BODY_HEAD = "<table border=\"0\" width=\"100%%\" style=\"font-family: Verdana; font-size: %dpt\">";
const char * BODY_TAIL = "</table><a name=\"EOF\"></a>";
const char * MESSAGE_HEAD = "<tr><td width=\"100%\">";
const char * MESSAGE_TAIL = "</tr></td>";

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CChatView

IMPLEMENT_DYNCREATE(CChatView, CHtmlView)

CChatView::CChatView()
{
	m_bMemoryMode=FALSE;
	m_lpstrBodyContent=NULL;
	m_lpstrCharset=NULL;
	m_lpstrTitle=NULL;
	m_pHtmlDoc2=NULL;
	m_pParentWindow = NULL;

	//{{AFX_DATA_INIT(CChatView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CChatView::~CChatView()
{}

void CChatView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatView, CHtmlView)
	//{{AFX_MSG_MAP(CChatView)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatView diagnostics

#ifdef _DEBUG
void CChatView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CChatView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChatView message handlers

void CChatView::OnInitialUpdate() 
{
	CHtmlView::OnInitialUpdate();

	NavigateMemory();
}

int CChatView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CHtmlView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


//----------------------------------------------------------------------------

void CChatView::ShowMemoryHtml()
{
	// show all stuff of HTML if it's already stored
	SetTitle(NULL);
	SetCharset( NULL);
	PutBodyContent(NULL);
}

BOOL CChatView::PutBodyContent(const char * lpstrContent)
{
	//store body content
	if( lpstrContent) 
		m_lpstrBodyContent = lpstrContent;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2)
	{
		HRESULT hr = S_OK;
		IHTMLElement *pBodyElement;
		//get body element
		hr=m_pHtmlDoc2->get_body( &pBodyElement);
		//put content to body element
		_bstr_t pbBody( m_lpstrBodyContent);
		hr=pBodyElement->put_innerHTML( pbBody);

		if( hr==S_FALSE) return FALSE;
		else return TRUE;
	}
	else return FALSE;
}

BOOL CChatView::SetTitle(LPSTR lpstrTitle)
{
	//store 
	if( lpstrTitle) m_lpstrTitle=lpstrTitle;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2)
	{
		HRESULT hr = S_OK;
		//set title
		_bstr_t pbTitle( lpstrTitle);
		hr=m_pHtmlDoc2->put_title( pbTitle);
	
		if( hr==S_FALSE) return FALSE;
		else return TRUE;
	}
	else return FALSE;
}

BOOL CChatView::SetCharset(LPSTR lpstrCharset)
{
	//store 
	if( lpstrCharset) m_lpstrCharset=lpstrCharset;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2)
	{
		HRESULT hr = S_OK;
		//put charset
		_bstr_t bpCharset( lpstrCharset);
		hr=m_pHtmlDoc2->put_charset( bpCharset);

		if( hr==S_FALSE) return FALSE;
		else return TRUE;
	}
	else return FALSE;
}

void CChatView::addNewMessage(const MetaClient::Chat chat)
{
	CString newText;
	String text;
	text.copy( chat.text );

	CString msgBegin;
	msgBegin.Format( "/<b>%s @%u</b> Sent \"<b>", chat.author, chat.authorId );
	if( text.beginsWith( msgBegin ) )	// is it a standard /send msg ?
	{
		// convert it so it's better readable
		text.right( text.length() - msgBegin.length() );
		if( text.length() > 5 )
			text.left( text.length() - 5 );	

		newText.Format( "%s %s: \"<font color=e01010><b>%s</b></font>\"", CTime( chat.time ).FormatGmt( "%H:%M:%S" ),
		chat.author, CFilterText::enhance( (const char *)text ) );
	}
	else
	{	// could it be a local echo of a /send msg ?
		if( text.beginsWith( "/You sent: " ) )
		{
			// convert it so it's better readable
			text.right( text.length() - 11 );
			
			newText.Format( "%s ==&gt; \"<font color=d0d000><b>%s</b></font>\"", CTime( chat.time ).FormatGmt( "%H:%M:%S" ),
				CFilterText::enhance( (const char *)text ) );
		}
		else
		{
			if( text.beginsWith("/") )
			{
				// no standard sent msg, probably a /modsend
				newText = CFilterText::enhance( (const char *)text + 1 );	// display original ( minus leading / )
			}
			else	// regular roomchat ?
				newText.Format( "%s %s: \"<b>%s</b>\"", CTime( chat.time ).FormatGmt( "%H:%M:%S" ),
					chat.author, CFilterText::enhance( (const char *)text ) );
		}
	}

	
	// add new message to buffer
	m_ChatBuffer += MESSAGE_HEAD;
	m_ChatBuffer += newText;
	m_ChatBuffer += MESSAGE_TAIL;
	m_ChatBuffer += "\n";
				
	// trim the buffer
	int length = m_ChatBuffer.GetLength();
	while( length > 32768 )
	{
		m_ChatBuffer = m_ChatBuffer.Right( m_ChatBuffer.GetLength() - (m_ChatBuffer.Find( '\n' ) + 1) );
		length = m_ChatBuffer.GetLength();
	}
	
	updateBodyContent();
}


void CChatView::NavigateMemory()
{
	//get application name
	CString sAppName = AfxGetApp()->m_pszExeName;
	CString sResourseID;
	//get resourse ID of Empty Html
	sResourseID.Format("%d", IDR_EMPTY_HTML);
	CString sNavigatePath;
	//compile
	sNavigatePath="res://"+sAppName+".exe/"+sResourseID;
	Navigate2( sNavigatePath);
	//Activate memory mode
	m_bMemoryMode=TRUE;
}

void CChatView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel) 
{
	if ( m_bMemoryMode )
	{
		if ( lpszURL != NULL && lpszURL[0] == '#' )
			return;

		if ( strcmp(lpszTargetFrameName,"_blank") )
		{
			*pbCancel = TRUE;
			Navigate2( lpszURL, 0, "_blank" );
			return;
		}
	}

}

void CChatView::OnDocumentComplete(LPCTSTR lpszURL) 
{
	// show html on first loading of document
	if(m_bMemoryMode)
	{
		LPDISPATCH lpDispatch;
		lpDispatch=GetHtmlDocument();
		ASSERT(lpDispatch);

		//get html document from IDispatch
		HRESULT hr=lpDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&m_pHtmlDoc2);
		if( SUCCEEDED(hr)) 
			ShowMemoryHtml();

		// get the parent window
		m_pHtmlDoc2->get_parentWindow( &m_pParentWindow );
	}

	CHtmlView::OnDocumentComplete(lpszURL);
}

void CChatView::OnDownloadComplete() 
{
	//show html on refresh
	if( m_bMemoryMode)
		if( m_pHtmlDoc2)
			ShowMemoryHtml();

	CHtmlView::OnDownloadComplete();
}

//----------------------------------------------------------------------------

void CChatView::OnEditCut() 
{
	ExecWB(OLECMDID_CUT, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL); 
}

void CChatView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( QueryStatusWB(OLECMDID_CUT) & OLECMDF_ENABLED );
}

void CChatView::OnEditCopy() 
{
	ExecWB(OLECMDID_COPY, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL); 
}

void CChatView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( QueryStatusWB(OLECMDID_COPY) & OLECMDF_ENABLED );
}


int CChatView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	//return MA_ACTIVATE;	
	return CHtmlView::OnMouseActivate( pDesktopWnd, nHitTest, message );
}

void CChatView::updateBodyContent()
{
	m_Body = String().format( BODY_HEAD, 10 ) + m_ChatBuffer + BODY_TAIL; 
	PutBodyContent( m_Body );
		
	// scroll to the bottom of the window
	if ( m_pParentWindow != NULL )
		m_pParentWindow->scrollBy( 0, 1000 );
//	else
//		m_pHtmlDoc2->get_parentWindow( &m_pParentWindow );	// if the window got triggered by a msg instead of opened
															// by the user the parentWindow pointer wasn't set yet.
															// first message will not trigger scrolling so it can be set
															// here
}
