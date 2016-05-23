// EditEmotion.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "EditEmotion.h"
#include ".\editemotion.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditEmotion dialog


CEditEmotion::CEditEmotion(CWnd* pParent /*=NULL*/)
	: CDialog(CEditEmotion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditEmotion)
	m_Name = _T("");
	m_Text = _T("");
	//}}AFX_DATA_INIT
}


void CEditEmotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditEmotion)
	DDX_Text(pDX, IDC_EDIT1, m_Name);
	DDX_Text(pDX, IDC_EDIT2, m_Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditEmotion, CDialog)
	//{{AFX_MSG_MAP(CEditEmotion)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_EN_MAXTEXT(IDC_EDIT2, OnEnMaxtextEdit1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditEmotion message handlers

BOOL CEditEmotion::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEdit* pEditControl = (CEdit*)GetDlgItem(IDC_EDIT2);
	if(pEditControl)
	{
		pEditControl->SetLimitText(259);
	}
	return TRUE;
}

void CEditEmotion::OnEnMaxtextEdit1()
{
	AfxMessageBox("Emote too long!");
}
