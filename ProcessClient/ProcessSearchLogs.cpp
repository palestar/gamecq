// ProcessSearchLogs.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessSearchLogs.h"

#include "File/FileDisk.h"
#include "Standard/RegExpM.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessSearchLogs dialog


CProcessSearchLogs::CProcessSearchLogs( ProcessClient * pClient, CWnd* pParent /*=NULL*/)
	: CDialog(CProcessSearchLogs::IDD, pParent), m_pClient( pClient ) 
{
	//{{AFX_DATA_INIT(CProcessSearchLogs)
	m_FileMask = _T("");
	m_SearchString = _T("");
	m_UseRegExp = FALSE;
	m_ResultText = _T("");
	m_ResolveClientId = FALSE;
	//}}AFX_DATA_INIT
}


void CProcessSearchLogs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessSearchLogs)
	DDX_Control(pDX, IDC_COMBO1, m_ResultSettings);
	DDX_Text(pDX, IDC_EDIT1, m_FileMask);
	DDX_Text(pDX, IDC_EDIT4, m_SearchString);
	DDX_Check(pDX, IDC_CHECK1, m_UseRegExp);
	DDX_Text(pDX, IDC_EDIT2, m_ResultText);
	DDX_Check(pDX, IDC_CHECK10, m_ResolveClientId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessSearchLogs, CDialog)
	//{{AFX_MSG_MAP(CProcessSearchLogs)
	ON_BN_CLICKED(IDC_BUTTON1, OnSearch)
	ON_BN_CLICKED(IDC_BUTTON6, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessSearchLogs message handlers

void CProcessSearchLogs::OnSearch() 
{
	UpdateData( true );

	if( m_FileMask.Find('/') >= 0 || m_FileMask.Find('\\') >= 0 )
	{
		MessageBox( _T("The filemask may not contain slashes or backslashes.") );
		return;
	}
	
	ProcessClient::SearchLogRequest req;
	req.filemask = m_FileMask;
	req.isRegExp = m_UseRegExp == 1 ? true : false;
	req.searchLevel = m_ResultSettings.GetCurSel();
	req.searchString = m_SearchString;
	req.resolveClientId = m_ResolveClientId == 1 ? true : false;
	
	if( req.isRegExp )
	{
		RegExpM re;
		if( !re.regComp( req.searchString ) )
		{
			MessageBox( _T("The regular expression contains errors.") );
			return;
		}
	}


	m_ResultText = _T("Searching...");
	UpdateData( false );
	
	CharString result;
	if( m_pClient->searchLogs( req, result ) )
		m_ResultText = result;
	else
		m_ResultText = _T("Search failed...");

	m_ResultText = result;
	UpdateData( false );	
}

void CProcessSearchLogs::OnSave() 
{
	CFileDialog save( false, _T("log") );
	FileDisk saveFile;
	if ( save.DoModal() == IDOK )
		if ( saveFile.open( CharString( save.GetPathName() ), FileDisk::WRITE ) )
		{
			saveFile.write( m_ResultText, m_ResultText.GetLength() );
			saveFile.close();
		}

}

BOOL CProcessSearchLogs::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ResultSettings.SetCurSel( 0 );
	m_FileMask = _T("*");

	CEdit *cEdtSearch = (CEdit *)GetDlgItem( IDC_EDIT4 );	
	cEdtSearch->SetFocus();

	UpdateData( false );	
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessSearchLogs::OnOK()
{
	OnSearch();
}
