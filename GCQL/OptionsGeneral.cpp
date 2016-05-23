// OptionsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "OptionsGeneral.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral property page

IMPLEMENT_DYNCREATE(COptionsGeneral, CPropertyPage)

COptionsGeneral::COptionsGeneral() : CPropertyPage(COptionsGeneral::IDD)
, m_bCheckForUpdates(false)
, m_nCheckUpdateInterval(0)
{
	//{{AFX_DATA_INIT(COptionsGeneral)
	m_ChatSound = FALSE;
	m_RoomAnnounce = FALSE;
	m_AlwaysLog = FALSE;
	m_Language = -1;
	m_ChatBuffer = 0;
	m_TextSize = 0;
	m_AutoLogin = FALSE;
	m_bMessageSound = FALSE;
	m_bTaskBarMessages = FALSE;
	m_AutoAwayTime = 0;
	m_bMinimized = TRUE;
	m_bAway = TRUE;
	m_bEnableWordFilter = TRUE;
	//}}AFX_DATA_INIT
}

COptionsGeneral::~COptionsGeneral()
{
}

void COptionsGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsGeneral)
	DDX_Check(pDX, IDC_CHECK1, m_ChatSound);
	DDX_Check(pDX, IDC_CHECK2, m_RoomAnnounce);
	DDX_Check(pDX, IDC_CHECK3, m_AlwaysLog);
	DDX_Check(pDX, IDC_CHECK4, m_bMessageSound);
	DDX_Check(pDX, IDC_CHECK5, m_bTaskBarMessages);
	DDX_Check(pDX, IDC_CHECK6, m_AutoLogin);
	DDX_Check(pDX, IDC_CHECK7, m_bMinimized);
	DDX_Check(pDX, IDC_CHECK11, m_bAway);
	DDX_Check(pDX, IDC_CHECK8, m_bCheckForUpdates);
	DDX_Check(pDX, IDC_ENABLE_WORD_FILTER, m_bEnableWordFilter );
	DDX_Text(pDX, IDC_EDIT4, m_nCheckUpdateInterval);
	DDX_CBIndex(pDX, IDC_COMBO1, m_Language);
	DDX_Text(pDX, IDC_EDIT2, m_ChatBuffer);
	DDX_Text(pDX, IDC_EDIT5, m_AutoAwayTime);
	DDX_Text(pDX, IDC_EDIT6, m_TextSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsGeneral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral message handlers

BOOL COptionsGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_ChatBuffer = CGCQLApp::sm_ChatBufferSize;
	m_TextSize = CGCQLApp::sm_TextSize;
	m_ChatSound = CGCQLApp::sm_bChatSound;
	m_bMessageSound = CGCQLApp::sm_bMessageSound;
	m_bTaskBarMessages = CGCQLApp::sm_bTaskBarMessages;
	m_RoomAnnounce = CGCQLApp::sm_RoomAnnounce;
	m_Language = MetaClient::LCIDToLanguage( CGCQLApp::sm_Language );
	m_AlwaysLog = CGCQLApp::sm_AlwaysLog;
	m_AutoLogin = CGCQLApp::sm_AutoLogin;
	m_AutoAwayTime = CGCQLApp::sm_AutoAwayTime / 60;
	m_bMinimized = CGCQLApp::sm_bMinimized;
	m_bAway = CGCQLApp::sm_bEnableAutoAway;
	m_bCheckForUpdates = CGCQLApp::sm_nUpdateCheckInterval > 0;
	m_nCheckUpdateInterval = CGCQLApp::sm_nUpdateCheckInterval / (60 * 60);
	if ( m_nCheckUpdateInterval <= 0 )
		m_nCheckUpdateInterval = 1;		// always default to an hour even if disabled..
	m_bEnableWordFilter = CGCQLApp::sm_bEnableWordFilter;

	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsGeneral::OnOK() 
{
	if ( UpdateData() )
	{
		CGCQLApp::sm_AutoAwayTime = m_AutoAwayTime * 60;
		CGCQLApp::sm_ChatBufferSize = m_ChatBuffer;
		CGCQLApp::sm_TextSize = m_TextSize;
		CGCQLApp::sm_bChatSound = m_ChatSound ? true : false;
		CGCQLApp::sm_bMessageSound = m_bMessageSound ? true : false;
		CGCQLApp::sm_bTaskBarMessages = m_bTaskBarMessages ? true : false;
		CGCQLApp::sm_bMinimized = m_bMinimized ? true : false;
		CGCQLApp::sm_bEnableAutoAway = m_bAway ? true : false;
		CGCQLApp::sm_nUpdateCheckInterval = m_bCheckForUpdates ? (m_nCheckUpdateInterval * 60 * 60) : 0;
		CGCQLApp::sm_RoomAnnounce = m_RoomAnnounce ? true : false;
		CGCQLApp::sm_AlwaysLog = m_AlwaysLog ? true : false;
		CGCQLApp::sm_Language = MetaClient::LanguageToLCID( (MetaClient::Language)m_Language );
		CGCQLApp::sm_bEnableWordFilter = m_bEnableWordFilter != 0;

		if( !m_AutoLogin )
			CGCQLApp::sm_AutoLogin = false;	// can only be switched off here

		if ( m_bEnableWordFilter )
			CGCQLApp::sm_MetaClient.loadWordFilter( "WordFilter.txt" );
		else
			CGCQLApp::sm_MetaClient.clearWordFilter();

		CPropertyPage::OnOK();
	}
}
