// Profile.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "Profile.h"
#include "FieldValue.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProfile dialog


CProfile::CProfile(CWnd* pParent /*=NULL*/)
	: CDialog(CProfile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProfile)
	m_ProfileID = 0;
	m_ClanID = 0;
	m_Server = FALSE;
	m_Moderator = FALSE;
	m_Alias = _T("");
	m_Email = _T("");
	m_Administrator = FALSE;
	m_News = FALSE;
	//}}AFX_DATA_INIT
}


void CProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfile)
	DDX_Control(pDX, IDC_LIST1, m_Fields);
	DDX_Text(pDX, IDC_EDIT2, m_ProfileID);
	DDX_Text(pDX, IDC_EDIT3, m_ClanID);
	DDX_Check(pDX, IDC_CHECK2, m_Server);
	DDX_Check(pDX, IDC_CHECK3, m_Moderator);
	DDX_Text(pDX, IDC_EDIT1, m_Alias);
	DDX_Text(pDX, IDC_EDIT4, m_Email);
	DDX_Check(pDX, IDC_CHECK4, m_Administrator);
	DDX_Check(pDX, IDC_CHECK5, m_News);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProfile, CDialog)
	//{{AFX_MSG_MAP(CProfile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnEditField)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfile message handlers

BOOL CProfile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ProfileID = m_Profile.userId;
	m_ClanID = m_Profile.clanId;
	m_Alias = (CString)m_Profile.name;
	m_Administrator = m_Profile.flags & MetaClient::ADMINISTRATOR ? true : false;
	m_Server = m_Profile.flags & MetaClient::SERVER ? true : false;
	m_Moderator = m_Profile.flags & MetaClient::MODERATOR ? true : false;
	m_News = m_Profile.flags & MetaClient::NEWS_ADMIN ? true : false;
	m_Email = (CString)m_Profile.email;
	UpdateData( false );

	RECT rect;
	m_Fields.GetWindowRect(&rect);
	int columnWidth = (rect.right - rect.left) / 2;

	m_Fields.InsertColumn(0,_T("Name"), LVCFMT_LEFT, columnWidth );
	m_Fields.InsertColumn(2,_T("Value"), LVCFMT_LEFT, columnWidth );

	for(int i=0;i<m_Profile.fields.size();i++)
	{
		int item = m_Fields.InsertItem( 0, (CString)m_Profile.fields[i].name );
		m_Fields.SetItemData( item, i );
		m_Fields.SetItemText( item, 1, (CString)m_Profile.fields[i].value );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProfile::OnOK() 
{
	if ( UpdateData( true ) )
	{
		m_Profile.flags &= ~(MetaClient::ADMINISTRATOR|MetaClient::SERVER|MetaClient::MODERATOR);
		m_Profile.flags |= m_Administrator ? MetaClient::ADMINISTRATOR : 0;
		m_Profile.flags |= m_Server ? MetaClient::SERVER : 0;
		m_Profile.flags |= m_Moderator ? MetaClient::MODERATOR : 0;
		m_Profile.flags |= m_News ? MetaClient::NEWS_ADMIN : 0;

		for(int i=0;i<m_Fields.GetItemCount();i++)
		{
			int field = m_Fields.GetItemData( i );
			m_Profile.fields[ field ].value = m_Fields.GetItemText( i, 1 );
		}

		CDialog::OnOK();
	}
}

void CProfile::OnEditField(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int selected = m_Fields.GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	CFieldValue dialog;
	dialog.m_Value = m_Fields.GetItemText( selected, 1 );
	if (dialog.DoModal() == IDOK )
		m_Fields.SetItemText( selected, 1, dialog.m_Value );

	*pResult = 0;
}
