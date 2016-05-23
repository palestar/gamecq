// FieldValue.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "FieldValue.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldValue dialog


CFieldValue::CFieldValue(CWnd* pParent /*=NULL*/)
	: CDialog(CFieldValue::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFieldValue)
	m_Value = _T("");
	//}}AFX_DATA_INIT
}


void CFieldValue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldValue)
	DDX_Text(pDX, IDC_EDIT1, m_Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFieldValue, CDialog)
	//{{AFX_MSG_MAP(CFieldValue)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldValue message handlers
