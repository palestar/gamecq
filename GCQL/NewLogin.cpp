// NewLogin.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "NewLogin.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewLogin dialog


CNewLogin::CNewLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CNewLogin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewLogin)
	m_UID = _T("");
	m_PW = _T("");
	m_PW2 = _T("");
	m_EMail = _T("");
	//}}AFX_DATA_INIT
}


void CNewLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewLogin)
	DDX_Text(pDX, IDC_EDIT1, m_UID);
	DDX_Text(pDX, IDC_EDIT2, m_PW);
	DDX_Text(pDX, IDC_EDIT3, m_PW2);
	DDX_Text(pDX, IDC_EDIT4, m_EMail);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewLogin, CDialog)
	//{{AFX_MSG_MAP(CNewLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewLogin message handlers

BOOL CNewLogin::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit *cEdtName = (CEdit *)GetDlgItem( IDC_EDIT1 );
	cEdtName->SetLimitText( 37 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
