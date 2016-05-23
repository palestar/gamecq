// Process.cpp : implementation file
//

#include "stdafx.h"
#include "Process.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcess dialog


CProcess::CProcess(CWnd* pParent /*=NULL*/)
	: CDialog(CProcess::IDD, pParent)
	, m_bRunOnce(FALSE)
{
	//{{AFX_DATA_INIT(CProcess)
	m_Name = _T("");
	m_Disabled = FALSE;
	m_Exec = _T("");
	m_Arguments = _T("");
	m_Config = _T("");
	m_Log = _T("");
	//}}AFX_DATA_INIT
}


void CProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcess)
	DDX_Text(pDX, IDC_EDIT1, m_Name);
	DDX_Check(pDX, IDC_CHECK2, m_Disabled);
	DDX_Text(pDX, IDC_EDIT2, m_Exec);
	DDX_Text(pDX, IDC_EDIT3, m_Arguments);
	DDX_Text(pDX, IDC_EDIT6, m_Config);
	DDX_Text(pDX, IDC_EDIT7, m_Log);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK3, m_bRunOnce);
}


BEGIN_MESSAGE_MAP(CProcess, CDialog)
	//{{AFX_MSG_MAP(CProcess)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcess message handlers
