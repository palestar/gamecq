// CreateRoom.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "CreateRoom.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateRoom dialog


CCreateRoom::CCreateRoom(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateRoom::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateRoom)
	m_Name = _T("");
	m_PasswordProtect = FALSE;
	m_Password = _T("");
	//}}AFX_DATA_INIT
}


void CCreateRoom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateRoom)
	DDX_Control(pDX, IDC_EDIT2, m_PasswordControl);
	DDX_Control(pDX, IDC_EDIT1, m_NameControl);
	DDX_Text(pDX, IDC_EDIT1, m_Name);
	DDX_Check(pDX, IDC_CHECK1, m_PasswordProtect);
	DDX_Control(pDX, IDC_STATIC_CHECKBOX, m_StaticControl);
	DDX_Control(pDX, IDC_HIDDEN_CHECKBOX, m_HiddenControl);
	DDX_Control(pDX, IDC_MOD_CHECKBOX, m_ModeratedControl );
	DDX_Text(pDX, IDC_EDIT2, m_Password);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateRoom, CDialog)
	//{{AFX_MSG_MAP(CCreateRoom)
	ON_BN_CLICKED(IDC_CHECK1, OnPasswordProtect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateRoom message handlers

void CCreateRoom::OnOK() 
{
	if ( UpdateData() && m_Name.GetLength() > 0 )
	{
		CharString sName = m_Name;
		CharString sPassword = m_Password;

		// check the room name, only allow numbers, letters, and spaces
		for(int i=0;i<sName.length();i++)
			if ( !isalnum( sName[i] ) && sName[ i ] != ' ' )
			{
				MessageBox( _T("Room name contains illegal characters; please user only letters and numbers.") );
				return;
			}

		bool bStatic = m_StaticControl.GetCheck() != 0;
		bool bHidden = m_HiddenControl.GetCheck() != 0;
		bool bModerated = m_ModeratedControl.GetCheck() != 0;

		CChatFrame * pChatFrame = CGCQLApp::sm_pChatFrame;
		if (! pChatFrame )
			return;

		MetaClient & client = CGCQLApp::sm_MetaClient;

		if ( pChatFrame->m_nRoomId != 0 )
			client.leaveRoom( pChatFrame->m_nRoomId );

		pChatFrame->m_nRoomId = client.createRoom( sName, sPassword, bStatic, bModerated, bHidden );
		if ( ((int)pChatFrame->m_nRoomId) < 0 )
		{
			MessageBox( _T("Failed to create chat room!") );
			return;
		}

		// post local message, so the client can know that they have joined a room
		client.sendLocalChat( CharString().format("/Created '<b>%s</b>'...", sName ) );
		// set my status
		client.sendStatus( CharString().format("Chatting in '%s'", sName.cstr() ) );
	}

	CDialog::OnOK();
}

BOOL CCreateRoom::OnInitDialog() 
{
	m_PasswordProtect = false;

	CDialog::OnInitDialog();

	m_NameControl.SetFocus();
	m_PasswordControl.EnableWindow( false );

	MetaClient & client = CGCQLApp::sm_MetaClient;
	bool bAdmin = (client.profile().flags & MetaClient::ADMINISTRATOR) != 0;
	m_StaticControl.ShowWindow( bAdmin ? SW_SHOW : SW_HIDE );
	bool bServer = (client.profile().flags & MetaClient::SERVER) != 0;
	m_HiddenControl.ShowWindow( (bAdmin || bServer) ? SW_SHOW : SW_HIDE );
	bool bModerator = (client.profile().flags & MetaClient::MODERATOR) != 0;
	m_ModeratedControl.ShowWindow( (bAdmin || bModerator) ? SW_SHOW : SW_HIDE );

	return FALSE;
}

void CCreateRoom::OnPasswordProtect() 
{
	if ( UpdateData() )
	{
		m_PasswordControl.EnableWindow( m_PasswordProtect );
		if ( !m_PasswordProtect )
			m_PasswordControl.SetWindowText( _T("") );
		else
		{
			CEdit *cEdtPW = (CEdit *)GetDlgItem( IDC_EDIT2 );	
			cEdtPW->SetFocus();
		}
	}
}
