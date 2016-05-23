// EditDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditDialog.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditDialog dialog


CEditDialog::CEditDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEditDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditDialog)
	m_Text = _T("");
	//}}AFX_DATA_INIT
}


void CEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDialog)
	DDX_Control(pDX, IDC_EDIT1, m_TextControl);
	DDX_Text(pDX, IDC_EDIT1, m_Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditDialog, CDialog)
	//{{AFX_MSG_MAP(CEditDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDialog message handlers

BOOL CEditDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_TextControl.LineScroll( m_TextControl.GetLineCount() );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
