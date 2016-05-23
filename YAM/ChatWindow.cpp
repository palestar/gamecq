// ChatWindow.cpp : implementation file
//

#include "stdafx.h"
#include "yam.h"
#include "YamDlg.h"
#include "ChatWindow.h"

#include "Standard/Time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatWindow dialog


CChatWindow::CChatWindow(CWnd* pParent /*=NULL*/, bool bEcho /*=TRUE*/)
	: CDialog(CChatWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatWindow)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bEcho = bEcho;
}


void CChatWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatWindow)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatWindow, CDialog)
	//{{AFX_MSG_MAP(CChatWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatWindow message handlers

void CChatWindow::OnOK()	// prevent enter key from closing this window
{
}

void CChatWindow::OnCancel()
{
	String oldText;
	oldText.copy( m_wndChatView->m_ChatBuffer.GetBuffer(0) );
	CDialog::OnCancel();

	((CYamDlg*)GetParent())->chatWindowClosed( this, oldText );
}

BOOL CChatWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect rect;
	GetClientRect( &rect );
	rect.bottom -= 30;

	CRuntimeClass *ret = RUNTIME_CLASS(CChatView);
	CObject *newobj = ret->CreateObject();

	if( newobj == NULL )
		return FALSE;

	if( newobj->GetRuntimeClass()->IsDerivedFrom( RUNTIME_CLASS(CChatView) ) == FALSE )
		return FALSE;

	m_wndChatView = (CChatView *)newobj;
	m_wndChatView->Create( NULL, "HTMLView", WS_CHILD|WS_VISIBLE, rect, this, 0 );
	m_wndChatView->OnInitialUpdate();
	
	rect.left += 11;
	rect.right -= 11;
	rect.top = rect.bottom + 8;
	rect.bottom += 24;

	if (! m_wndChatEdit.Create( ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP, rect, this, 0 ) )
		return FALSE;

	m_wndChatEdit.LimitText( 486 );
	m_wndChatEdit.SetLimitText( 486 );
	m_wndChatEdit.SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChatWindow::putOldText(String oldText)
{
	m_wndChatView->m_ChatBuffer = oldText;
	m_wndChatView->updateBodyContent();
}

void CChatWindow::addNewMessage( const MetaClient::Chat chat )
{
	m_wndChatView->addNewMessage( chat );	
}

void CChatWindow::sendChat(CString text)
{
	String sText;
	sText.copy( text.GetBuffer(0) );
	
	((CYamDlg*)GetParent())->chatWindowSendText( this, sText );
	
	if( m_bEcho && !sText.beginsWith("/") )
	{
		MetaClient::Chat chat;
		chat.time = Time::seconds();
		chat.text = String().format("/You sent: %s", text );
		addNewMessage( chat );
	}
}
