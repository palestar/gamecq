// ChangeName.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "ChangeName.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeName dialog


CChangeName::CChangeName(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeName)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}


void CChangeName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeName)
	DDX_Text(pDX, IDC_EDIT1, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeName, CDialog)
	//{{AFX_MSG_MAP(CChangeName)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeName message handlers
