// Ignores.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "Ignores.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIgnores dialog


CIgnores::CIgnores(CWnd* pParent /*=NULL*/)
	: CDialog(CIgnores::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIgnores)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIgnores::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIgnores)
	DDX_Control(pDX, IDC_LIST1, m_IgnoreList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIgnores, CDialog)
	//{{AFX_MSG_MAP(CIgnores)
	ON_BN_CLICKED(IDC_BUTTON1, OnRemove)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIgnores message handlers

void CIgnores::OnRemove() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;

	int selected = m_IgnoreList.GetNextItem( -1, LVNI_SELECTED );
	while( selected >= 0 )
	{
		dword userId = m_IgnoreList.GetItemData( selected );
		client.deleteIgnore( userId );

		m_IgnoreList.DeleteItem( selected );

		selected = m_IgnoreList.GetNextItem( -1, LVNI_SELECTED );
	}
}

BOOL CIgnores::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_IgnoreList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	// Init the ResultList
	CRect rect;
	m_IgnoreList.GetWindowRect( &rect );

	m_IgnoreList.InsertColumn( 0, _T("Name"),		LVCFMT_LEFT, rect.Width() / 3, 0 ); 		
	m_IgnoreList.InsertColumn( 1, _T("User Id"),	LVCFMT_LEFT, rect.Width() / 3, 1 ); 		
	m_IgnoreList.InsertColumn( 2, _T("Status"),		LVCFMT_LEFT, rect.Width() / 3, 2 ); 		
	
	OnUpdate();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIgnores::OnUpdate() 
{
	MetaClient & client = CGCQLApp::sm_MetaClient;

	m_IgnoreList.DeleteAllItems();

	// get the ignores
	Array< MetaClient::ShortProfile > ignores;
	if ( client.getIgnores( ignores ) < 0 )
		MessageBox( _T("Failed to get ignore list!") );

	for(int i=0;i<ignores.size();i++)
	{
		MetaClient::ShortProfile & player = ignores[ i ];

		int item = m_IgnoreList.InsertItem( 0, CString( player.name ) );
		m_IgnoreList.SetItemText( item, 1, CString( CharString().format("%u", player.userId) ) );
		m_IgnoreList.SetItemText( item, 2, CString( player.status ) );
		m_IgnoreList.SetItemData( item, player.userId );
	}
}

void CIgnores::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
	delete this;
}

void CIgnores::OnClose() 
{
	CDialog::OnClose();
	DestroyWindow();
}

