// ChatEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "ChatEdit.h"
#include "ChatFrame.h"
#include "FilterText.h"
#include "Standard/CharString.h"
#include "Standard/RegExpM.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------

const int MAX_CHAT_LINE = 486;

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CChatEdit

CChatEdit::CChatEdit()
{
	m_ChatPosition = NULL;
	m_LastUserId = NULL;
	m_AcceptInput = true;
}

CChatEdit::~CChatEdit()
{
}


BEGIN_MESSAGE_MAP(CChatEdit, CEdit)
	//{{AFX_MSG_MAP(CChatEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatEdit message handlers

BOOL CChatEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if( !m_AcceptInput )
			return TRUE;
		
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			SetWindowText(_T(""));
			return TRUE;
		case VK_RETURN:
			sendChat();
			return TRUE;
		case VK_UP:
			{
				if ( m_ChatPosition != NULL )
					m_ChatHistory.GetNext( m_ChatPosition ); 
				if ( m_ChatPosition == NULL )
					if ( (m_ChatPosition = m_ChatHistory.GetHeadPosition()) == NULL )
						break;

				CString text( m_ChatHistory.GetAt( m_ChatPosition ) );
				SetWindowText(text);
				SetFocus();
				SetSel( 0, -1, true );				}
			return TRUE;
		case VK_DOWN:
			{
				if ( m_ChatPosition != NULL )
					m_ChatHistory.GetPrev( m_ChatPosition );
				if ( m_ChatPosition == NULL )
					if ( (m_ChatPosition = m_ChatHistory.GetHeadPosition()) == NULL )
						break;

				CString text( m_ChatHistory.GetAt( m_ChatPosition ) );
				SetWindowText(text);
				SetFocus();
				SetSel( 0, -1, true );
			}
			return TRUE;

		case VK_SPACE:	// begin empty line with the space key to reply to last priv msg received
			{
				if( m_LastUserId != NULL )
				{
					CString sText;
					GetWindowText( sText );
					if( sText.GetLength() == 0 )
					{
						CString sNewSelection;
						sNewSelection.Format( _T("/send @%u "), m_LastUserId );

						SetSel( 0, -1 );		// workaround for SetWindowText and LineScroll not working in combination.
						ReplaceSel( sNewSelection );	
						return true;
					}
				}
			}
			break;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

void CChatEdit::sendChat()
{
	if ( GetLimitText() != 486 )
		SetLimitText( 486 );
	
	MetaClient & metaClient = CGCQLApp::sm_MetaClient;

	CString sSendText;
	GetWindowText( sSendText );
	
	sendChat( sSendText, true );

	SetWindowText( _T("") );
	SetFocus();
}

void CChatEdit::sendChat( const TCHAR * pSendText, bool addHistory )
{
	MetaClient & metaClient = CGCQLApp::sm_MetaClient;

	CharString sSendText = pSendText;
	if ( sSendText.length() > 0 && sSendText.length() < MAX_CHAT_LINE )
	{
		if( addHistory )
		{
			m_ChatPosition = NULL;
			m_ChatHistory.AddHead( pSendText );
		}
		
		// remove sid's from URL's
		sSendText.replace( CharString().format("sid=%u", CGCQLApp::sm_SessionID), "sid=" );
		// filter out linebreaks if a user managed to get one into the chatline using another application
		sSendText.replace( "\r", " " );
		sSendText.replace( "\n", " " );
		
		// filter some HTML, users are supposed to use the BBCode equivalent.
		sSendText.replace( "</font>", "" );
		RegExpM::regSearchReplace( sSendText, "<(/)?(b|B|i|I)>", "" );
		RegExpM::regSearchReplace( sSendText, "<font color=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]>", "" );

		if( (( metaClient.profile().flags & MetaClient::MODERATOR ) + ( metaClient.profile().flags & MetaClient::DEVELOPER )) == 0 	// don't spamcheck staff
			&& !m_SpamBlock.check( sSendText ) )	
		{
			m_AcceptInput = false;	// yep he is, disable message processing
		}

		if ( sSendText[ 0 ] != '/' )
		{
			dword color = CGCQLApp::sm_ChatColor;
			dword rgb = (color & 0xff) << 16 | (color & 0xff00) | (color & 0xff0000) >> 16;
			
			if( FilterText::checkColor( rgb ) != 0 )	// if chattextcolor is invalid then change it to
				rgb = 0x00FFFF; // yellow

			metaClient.sendChat( ((CChatFrame *)GetParentFrame())->m_nRoomId, 
				CharString().format( "<font color=%6.6x>%s</font>", rgb, sSendText ) );
		}
		else
		{
			// user entered a / command, do not add formatting for the font color
			metaClient.sendChat( ((CChatFrame *)GetParentFrame())->m_nRoomId, sSendText );
		}
	}
	else if ( sSendText.length() > 0 )
	{
		metaClient.sendLocalChat( "/Error, can not send message: Line too long..." );
		if( addHistory )
		{
			m_ChatPosition = NULL;
			m_ChatHistory.AddHead( pSendText );
		}
	}
}


void CChatEdit::setChatLine(const TCHAR * pNewText)
{
	CString sCurrText;
	GetWindowText( sCurrText );
	
	// if there is already text in the input line
	if ( sCurrText.GetLength() > 0 )
	{		
		// save it into the chatHistory
		m_ChatPosition = NULL;
		m_ChatHistory.AddHead( sCurrText );
	}

	SetFocus();
	SetSel( 0, -1 );		// workaround for SetWindowText and LineScroll not working in combination.
	ReplaceSel( pNewText );		
}

void CChatEdit::setLastUserId(dword userId)
{
	m_LastUserId = userId;
}

bool CChatEdit::acceptsInput()
{
	return m_AcceptInput;
}

void CChatEdit::enableInput()
{
	m_AcceptInput = true;
	SetWindowText(_T(""));
}

//----------------------------------------------------------------------------

IMPLEMENT_SERIAL(CChatEditButton, CBCGToolbarEditBoxButton, 1)

CEdit * CChatEditButton::CreateEdit(CWnd* pWndParent, const CRect& rect)
{
	CChatEdit* pWndEdit = new CChatEdit;

	if (!pWndEdit->Create(m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	return pWndEdit;
}

BOOL CChatEditButton::NotifyCommand (int iNotifyCode)
{
	BOOL bRes = CBCGToolbarEditBoxButton::NotifyCommand (iNotifyCode);
	
	switch (iNotifyCode)
	{
	case CBN_KILLFOCUS:
		bRes = TRUE;
		break;

	case CBN_SETFOCUS:
		bRes = TRUE;
		break;
	}

	return bRes;
}

//----------------------------------------------------------------------------



