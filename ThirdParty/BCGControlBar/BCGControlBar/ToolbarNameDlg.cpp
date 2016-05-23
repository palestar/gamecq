//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

// ToolbarNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ToolbarNameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolbarNameDlg dialog


CToolbarNameDlg::CToolbarNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToolbarNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CToolbarNameDlg)
	m_strToolbarName = _T("");
	//}}AFX_DATA_INIT
}


void CToolbarNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolbarNameDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Text(pDX, IDC_BCGBARRES_TOOLBAR_NAME, m_strToolbarName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolbarNameDlg, CDialog)
	//{{AFX_MSG_MAP(CToolbarNameDlg)
	ON_EN_UPDATE(IDC_BCGBARRES_TOOLBAR_NAME, OnUpdateToolbarName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolbarNameDlg message handlers

void CToolbarNameDlg::OnUpdateToolbarName() 
{
	UpdateData ();
	m_btnOk.EnableWindow (!m_strToolbarName.IsEmpty ());
}
//**********************************************************************************
BOOL CToolbarNameDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_btnOk.EnableWindow (!m_strToolbarName.IsEmpty ());
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
