// Eula.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "Eula.h"

#include "File/FileDisk.h"


#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEula dialog


CEula::CEula(CWnd* pParent /*=NULL*/)
	: CDialog(CEula::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEula)
	m_Eula = _T("");
	//}}AFX_DATA_INIT
}


void CEula::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEula)
	DDX_Control(pDX, IDC_EULA, m_EulaControl);
	DDX_Text(pDX, IDC_EULA, m_Eula);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEula, CDialog)
	//{{AFX_MSG_MAP(CEula)
	ON_BN_CLICKED(IDC_BUTTON1, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEula message handlers

BOOL CEula::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HRSRC hR = FindResource( NULL, MAKEINTRESOURCE(IDR_EULA), _T("RT_EULA") );
	HGLOBAL hG = LoadResource( NULL, hR );

	m_Eula = (char *)LockResource( hG );

	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	int year = aTime->tm_year + 1900;
	m_Eula.Append( CharString().format( "© 2000 - %d PaleStar Inc. All rights reserved worldwide.", year ) );
	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEula::OnSave() 
{
	CFileDialog dialog( false, _T("TXT"), _T("EULA.TXT"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		_T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"), this );

	if ( dialog.DoModal() == IDOK )
	{
		// save the file to disk
		FileDisk::saveFile( CharString(dialog.GetPathName()), m_Eula, m_Eula.GetLength() );
	}
}

