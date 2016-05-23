// ChatEdit.cpp : implementation file
//

#include "stdafx.h"
#include "ChatEdit.h"

#include "ChatWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatEdit

CChatEdit::CChatEdit()
{
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
		switch (pMsg->wParam)
		{
			case VK_ESCAPE:
				SetWindowText("");
				return TRUE;

			case VK_RETURN:
				sendChat();
				return TRUE;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

void CChatEdit::sendChat()
{
	if ( GetLimitText() != 486 )
		SetLimitText( 486 );
	
	CString sendText;
	GetWindowText( sendText );
	
	if ( sendText.GetLength() > 486 )
	{
		MessageBox( "Can not send message: Line too long..." );
		return;
	}

	((CChatWindow *)GetParent())->sendChat( sendText );

	SetWindowText( "" );
	SetFocus();
}


//----------------------------------------------------------------------------



