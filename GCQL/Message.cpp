// Message.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "Message.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessage dialog


CMessage::CMessage(CWnd* pParent /*=NULL*/)
	: CDialog(CMessage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessage)
	m_To = _T("");
	m_Message = _T("");
	//}}AFX_DATA_INIT
}


void CMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessage)
	DDX_Control(pDX, IDCANCEL, m_CancelControl);
	DDX_Control(pDX, IDC_EDIT1, m_MessageControl);
	DDX_Control(pDX, IDOK, m_SendControl);
	DDX_Text(pDX, IDC_TO, m_To);
	DDX_Text(pDX, IDC_EDIT1, m_Message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessage, CDialog)
	//{{AFX_MSG_MAP(CMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessage message handlers

BOOL CMessage::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_CancelControl.SetIcon( AfxGetApp()->LoadIcon( IDI_CANCEL ) );
	m_SendControl.SetIcon( AfxGetApp()->LoadIcon( IDI_MESSAGE ) );
	m_MessageControl.SetFocus();

	return FALSE;	
}
