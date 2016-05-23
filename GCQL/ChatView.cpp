// ChatView.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "ChatView.h"
#include "ChatEdit.h"
#include "WebView.h"
#include "ChatLog.h"
#include "FilterText.h"

#include "Standard/RegExpM.h"
#include "Standard/Time.h"
#include <mmsystem.h>
#include <atlbase.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

const TCHAR * STYLE_TAG = _T("<style>\n")
						_T("a.normal:link { color : White; text-decoration : none; }\n")
						_T("a.normal:Visited {color : White; text-decoration : none; }\n")
						_T("a.normal:Hover {color: Orange; text-decoration : none; }\n")
						_T("</style>\n");
const TCHAR * BODY_HEAD = _T("<table border=\"0\" width=\"100%%\" style=\"font-family: 'Segoe UI', Verdana; font-size: %dpt\">");
const TCHAR * BODY_TAIL = _T("</table><a name=\"EOF\"></a>");
const TCHAR * MESSAGE_HEAD = _T("<tr><td width=\"100%\">");
const TCHAR * MESSAGE_TAIL = _T("</tr></td>");

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
	m_UpdateBlockedTime = 0;

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
	ON_WM_TIMER()
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

	// set the top chat message
	m_TopChatMessage = 0;
	// check for new text
	SetTimer( 0x2, 500, NULL );

}

int CChatView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CHtmlView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CChatView::OnTimer(UINT nIDEvent) 
{
	CChatFrame * pFrame = (CChatFrame *)GetParentFrame();

	//CMainFrame* pMainFrame = (CMainFrame *)AfxGetMainWnd();	
	//
	//// SpamCheck heartbeat
	//CChatEdit* pChatEdit = pMainFrame->getChatEdit();
	//if( pChatEdit->m_SpamBlock.heartbeat() )		// reenable chatbar if spampenality-time expired
	//	if( ! pChatEdit->acceptsInput() )
	//		pChatEdit->enableInput();
	

	// don't update/scroll if the user is currently marking text
	BOOL bAllowUpdate = true;
	if( m_UpdateBlockedTime > 0 )
	{
		m_UpdateBlockedTime--;
		bAllowUpdate = false;
	}

	if ( CGCQLApp::sm_bBtnDown )
	{
		RECT rChatWindow;
		GetWindowRect( &rChatWindow );
		if ( ( rChatWindow.bottom >= CGCQLApp::sm_ptMouseDown.y ) && ( rChatWindow.top <= CGCQLApp::sm_ptMouseDown.y )
			&& ( rChatWindow.left <= CGCQLApp::sm_ptMouseDown.x ) && ( rChatWindow.right >= CGCQLApp::sm_ptMouseDown.x ) )
		{
			bAllowUpdate = false;
			m_UpdateBlockedTime = 5;
		}
	}

	if ( bAllowUpdate )
	{
		MetaClient & client = CGCQLApp::sm_MetaClient;
		
		AutoLock lock( client.autoLock() );
		if ( m_TopChatMessage < client.chatCount() )
		{
			for(;m_TopChatMessage<client.chatCount();m_TopChatMessage++)
			{
				const MetaClient::Chat & chat = client.chat( m_TopChatMessage );
				if( chat.recpId == client.profile().userId )
					pFrame->onPrivateMessage( chat.authorId, chat.author, chat.text );
				else if ( CGCQLApp::sm_bChatSound )
					PlaySound( MAKEINTRESOURCE(IDW_CHAT01), AfxGetResourceHandle(), SND_RESOURCE|SND_ASYNC );
				
				if ( chat.text.length() > 0 && chat.text[ 0 ] == '/' )	// emotes and server messages
				{
					CharString sText;
					
					// mark emotes to prevent message faking
					// /me echos are received with authorId 0 but should be marked as player emote too
					if( ( chat.authorId == 0 && chat.text.find( CharString("/") + client.profile().name ) >= 0 ) ||
						( chat.authorId != 0 && chat.text.find( CharString("/") + chat.author ) >= 0 ) )
						sText.format( "<b><font color=ffa000>*</font></b> %s", FilterText::enhance( chat.text.buffer() + 1 ) );
					else	
						sText.format( "<b>*</b> %s", FilterText::enhance( chat.text.buffer() + 1 ) );

					// Write to ChatLog
					CChatLog & cChatLog = CGCQLApp::s_ChatLog;
					if ( cChatLog.isLogging() )
						cChatLog.write( CString( sText ) );	
					
					// don't include the time stamp, just add the text directly
					m_ChatBuffer += MESSAGE_HEAD;
					
					// add timestamp if message is older than 5 minutes
					//if ( chat.time < (Time::seconds() - 300 ) )
					//	m_ChatBuffer += CTime( chat.time ).FormatGmt( "[%H:%M:%S %d/%m/%y] " );

					m_ChatBuffer += CString( sText );
					m_ChatBuffer += MESSAGE_TAIL;
					m_ChatBuffer += "\n";
					
				}
				else							// normal chatlines
				{
					// prepend the time and author
					CharString sText;
					if( chat.authorId != client.profile().userId )	// make chatlines from other users clickable
					{
						sText.format( "%s <a class=\"normal\" href=\"%u\">%s</a>: \"<b>%s</b>\"",
							Time::format( chat.time, "%H:%M:%S" ), 
							chat.authorId, chat.author, FilterText::enhance( chat.text ) );
					}
					else
					{
						sText.format( "%s %s: \"<b>%s</b>\"", Time::format( chat.time, "%H:%M:%S" ),
							chat.author, FilterText::enhance( chat.text ) );
					}

					// Write to ChatLog
					CChatLog & cChatLog = CGCQLApp::s_ChatLog;
					if ( cChatLog.isLogging() )
						cChatLog.write( CString( sText ) );	

					// add new message to buffer
					m_ChatBuffer += MESSAGE_HEAD;
					m_ChatBuffer += CString( sText );
					m_ChatBuffer += MESSAGE_TAIL;
					m_ChatBuffer += "\n";
				}
			}
			
			// trim the buffer
			int length = m_ChatBuffer.GetLength();
			while( length > CGCQLApp::sm_ChatBufferSize )
			{
				m_ChatBuffer = m_ChatBuffer.Right( m_ChatBuffer.GetLength() - (m_ChatBuffer.Find( '\n' ) + 1) );
				length = m_ChatBuffer.GetLength();
			}

			CString sBodyHeader;
			sBodyHeader.Format( BODY_HEAD, CGCQLApp::sm_TextSize );
			sBodyHeader += STYLE_TAG;
			
			// update the body text
			m_Body = sBodyHeader + m_ChatBuffer + BODY_TAIL; 
			PutBodyContent( m_Body );
			
			// scroll to the bottom of the window
			if ( m_pParentWindow != NULL )
				m_pParentWindow->scrollBy( 0, 1000 );
		}
	}

	//CHtmlView::OnTimer(nIDEvent);
}

//----------------------------------------------------------------------------

void CChatView::ShowMemoryHtml()
{
	// show all stuff of HTML if it's already stored
	SetTitle(NULL);
	SetCharset( NULL);
	PutBodyContent(NULL);
}

BOOL CChatView::PutBodyContent(const TCHAR * lpstrContent)
{
	//store body content
	if( lpstrContent) 
		m_lpstrBodyContent = lpstrContent;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2 && m_lpstrBodyContent )
	{
		HRESULT hr = S_OK;
		IHTMLElement *pBodyElement;
		//get body element
		hr=m_pHtmlDoc2->get_body( &pBodyElement);
		//put content to body element
		_bstr_t pbBody( m_lpstrBodyContent);
		hr=pBodyElement->put_innerHTML( pbBody);

		if( hr==S_FALSE) 
			return FALSE;
		else 
			return TRUE;
	}
	else 
		return FALSE;
}

BOOL CChatView::SetTitle(LPSTR lpstrTitle)
{
	//store 
	if( lpstrTitle) 
		m_lpstrTitle = lpstrTitle;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2 && m_lpstrTitle )
	{
		HRESULT hr = S_OK;
		//set title
		_bstr_t pbTitle( m_lpstrTitle );
		hr=m_pHtmlDoc2->put_title( pbTitle);
	
		if( hr==S_FALSE) return FALSE;
		else return TRUE;
	}
	else return FALSE;
}

BOOL CChatView::SetCharset(LPSTR lpstrCharset)
{
	//store 
	if( lpstrCharset) 
		m_lpstrCharset = lpstrCharset;
	//check if HtmlDocument initialized
	if( m_pHtmlDoc2 && m_lpstrCharset )
	{
		HRESULT hr = S_OK;
		//put charset
		_bstr_t bpCharset( m_lpstrCharset );
		hr=m_pHtmlDoc2->put_charset( bpCharset);

		if( hr==S_FALSE) return FALSE;
		else return TRUE;
	}
	else return FALSE;
}

void CChatView::NavigateMemory()
{
	//get application name
	CString sAppName = AfxGetApp()->m_pszExeName;
	CString sResourseID;
	//get resourse ID of Empty Html
	sResourseID.Format(_T("%d"), IDR_EMPTY_HTML);
	CString sNavigatePath;
	//compile
	sNavigatePath=_T("res://")+sAppName+_T(".exe/")+sResourseID;
	Navigate2( sNavigatePath);
	//Activate memory mode
	m_bMemoryMode=TRUE;
}

void CChatView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel) 
{
	if ( m_bMemoryMode )
	{
		m_UpdateBlockedTime = 0;	// don't block the chat if the user just clicked an url

		if ( lpszURL != NULL && lpszURL[0] == '#' )
			return;

		static TCHAR * VALID_LINKS[] = 
		{
			_T("http://"),
			_T("https://"),
			_T("ftp://"),
			_T("mailto:"),
			NULL
		};

		bool bOpenLink = false;

		// check the the link is for a user chat line, generate a /send command if we are clicking on one of those links
		CString sURL = lpszURL;
		sURL.MakeLower();

		for(int i=0;VALID_LINKS[i] != NULL && !bOpenLink;i++)
			if ( sURL.Left( _tcslen( VALID_LINKS[i] ) ) == VALID_LINKS[i] )
				bOpenLink = true;

		if(! bOpenLink ) 
		{	
			// don't let the link be opened
			*pbCancel = TRUE;

			// this code catches the QuickReply UserId from URLs
			CString sLobbyLink = CString(_T("res://")) + AfxGetApp()->m_pszExeName + _T(".exe/");
			sLobbyLink.MakeLower();

			if( sURL.Left( sLobbyLink.GetLength() ) == sLobbyLink )
			{
				sURL = sURL.Right( sURL.GetLength() - sLobbyLink.GetLength() );
				unsigned int userId = _ttol( sURL );
				if ( userId != 0 )
				{
					CChatEdit * chatEdit = ((CChatFrame *)GetParentFrame())->getChatEdit();
					if( chatEdit->acceptsInput() )
					{
						CString sSend;
						sSend.Format( _T("/send @%u "), userId );

						chatEdit->setChatLine( sSend );
					}
				}
			}
		}
		else if ( _tcscmp(lpszTargetFrameName,_T("_blank")) )
		{
			*pbCancel = TRUE;
			// opens the URL in the users default browser
			if( (int)ShellExecute( NULL, _T("open"), lpszURL, NULL, _T(""), SW_SHOW ) <= 32 )
				MessageBox( _T("Failed to open URL") );
		}

		// let the link be opened inside the lobby...
	}
	
	//CHtmlView::OnBeforeNavigate2(lpszURL, nFlags, lpszTargetFrameName, baPostedData, lpszHeaders, pbCancel);
}

void CChatView::OnStatusTextChange( LPCTSTR strText )
{
	// Check if this link is a QuickReply URL
	CString sReplyLink = CString("res://") + AfxGetApp()->m_pszAppName + _T(".exe/");
	sReplyLink.MakeLower();
	CString sCheck( strText, sReplyLink.GetLength() );
	sCheck.MakeLower();

	if( sCheck == sReplyLink )
		strText = _T("Message user");

	CHtmlView::OnStatusTextChange( strText );
}

void CChatView::OnNavigateComplete2(LPCTSTR strURL) 
{
	//ShowMemoryHtml();
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
