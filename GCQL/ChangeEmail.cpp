// ChangeEmail.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "ChangeEmail.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeEmail dialog


CChangeEmail::CChangeEmail(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeEmail::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeEmail)
	m_Email = _T("");
	//}}AFX_DATA_INIT
}


void CChangeEmail::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeEmail)
	DDX_Text(pDX, IDC_EDIT1, m_Email);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeEmail, CDialog)
	//{{AFX_MSG_MAP(CChangeEmail)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeEmail message handlers
