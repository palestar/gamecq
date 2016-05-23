// EditProfile.cpp : implementation file
//

#include "stdafx.h"
#include "MirrorUpload.h"
#include "EditProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditProfile dialog


CEditProfile::CEditProfile(CWnd* pParent /*=NULL*/)
	: CDialog(CEditProfile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditProfile)
	m_sName = _T("");
	m_sLocalPath = _T("");
	m_sAddress = _T("");
	m_nPort = 9000;
	m_sUser = _T("");
	m_sPassword = _T("");
	m_sConfirmPassword = _T("");
	m_sLogFile = _T("");
	//}}AFX_DATA_INIT
}


void CEditProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditProfile)
	DDX_Text(pDX, IDC_EDIT4, m_sName);
	DDX_Text(pDX, IDC_EDIT1, m_sLocalPath);
	DDX_Text(pDX, IDC_EDIT2, m_sAddress);
	DDX_Text(pDX, IDC_EDIT3, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT5, m_sUser);
	DDX_Text(pDX, IDC_EDIT6, m_sPassword);
	DDX_Text(pDX, IDC_EDIT7, m_sConfirmPassword);
	DDX_Text(pDX, IDC_EDIT8, m_sLogFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditProfile, CDialog)
	//{{AFX_MSG_MAP(CEditProfile)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowsePath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditProfile message handlers

void CEditProfile::OnOK() 
{
	if ( UpdateData() )
	{
		if ( m_sPassword != m_sConfirmPassword )
		{
			MessageBox( _T("Passwords do not match!"), _T("Invalid Passwords"), MB_OK );
			m_sPassword = m_sConfirmPassword = "";
			UpdateData( false );
			return;
		}
		
		CDialog::OnOK();
	}
}

void CEditProfile::OnBrowsePath() 
{
	// TODO: Add your control notification handler code here
	
}
