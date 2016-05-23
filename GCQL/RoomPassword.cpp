// RoomPassword.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "RoomPassword.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRoomPassword dialog


CRoomPassword::CRoomPassword(CWnd* pParent /*=NULL*/)
	: CDialog(CRoomPassword::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoomPassword)
	m_Password = _T("");
	//}}AFX_DATA_INIT
}


void CRoomPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomPassword)
	DDX_Text(pDX, IDC_EDIT1, m_Password);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomPassword, CDialog)
	//{{AFX_MSG_MAP(CRoomPassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomPassword message handlers

BOOL CRoomPassword::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit *cEdtUsr = (CEdit *)GetDlgItem( IDC_EDIT1 );	
	cEdtUsr->SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}
