// ChangePassword.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "ChangePassword.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangePassword dialog


CChangePassword::CChangePassword(CWnd* pParent /*=NULL*/)
	: CDialog(CChangePassword::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangePassword)
	m_NewPassword = _T("");
	m_ConfirmPassword = _T("");
	//}}AFX_DATA_INIT
}


void CChangePassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangePassword)
	DDX_Text(pDX, IDC_EDIT4, m_NewPassword);
	DDX_Text(pDX, IDC_EDIT5, m_ConfirmPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangePassword, CDialog)
	//{{AFX_MSG_MAP(CChangePassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangePassword message handlers

void CChangePassword::OnOK() 
{
	if ( UpdateData() ) 
	{
		if ( m_NewPassword != m_ConfirmPassword )
		{
			MessageBox( _T("Password mismatch, please retype your passwords") );
			return;
		}

		CDialog::OnOK();
	}
}
