// Login.cpp : implementation file
//

#include "stdafx.h"
#include "GCQL.h"
#include "Login.h"
#include "PWCipher.h"

#include "Standard/Settings.h"



#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogin dialog


CLogin::CLogin(CWnd* pParent /*=NULL*/)
	: CBCGDialog(CLogin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogin)
	m_PW = _T("");
	m_UID = _T("");
	m_RememberPW = FALSE;
	m_AutoLogin = FALSE;
	//}}AFX_DATA_INIT
}


void CLogin::DoDataExchange(CDataExchange* pDX)
{
	CBCGDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogin)
	DDX_Control(pDX, IDC_COMBO1, m_UIDControl);
	DDX_Text(pDX, IDC_EDIT2, m_PW);
	DDX_CBString(pDX, IDC_COMBO1, m_UID);
	DDX_Check(pDX, IDC_CHECK1, m_RememberPW);
	DDX_Check(pDX, IDC_CHECK9, m_AutoLogin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogin, CBCGDialog)
	//{{AFX_MSG_MAP(CLogin)
	ON_BN_CLICKED(IDC_BUTTON1, OnNewLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogin message handlers

void CLogin::OnNewLogin() 
{
	EndDialog( IDYES );
}

const char * REG_SECTION = "Login";

BOOL CLogin::OnInitDialog() 
{
	CBCGDialog::OnInitDialog();

	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );
	
	//SetBackgroundImage( LoadBitmap( pApp->m_hInstance, MAKEINTRESOURCE( IDB_LOGIN ) ) );

	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );
	CPWCipher * pwdecode = new CPWCipher();
	
	m_bIsAutomatedLogin = FALSE;

	int loginCount = settings.get( "loginCount", (dword)0 );
	for(int i=0;i<loginCount;i++)
		m_UIDControl.AddString( CString( settings.get( CharString().format("Login%d", i), "") ) );

	m_UID = (CString)settings.get( "previousUID", "" );
	if ( settings.get( "rememberPW", (dword)0 ) )
	{
		CString sTempPw = (CString)settings.get( "previousPW", "" );
		if( sTempPw.Find(_T("[ECM]") ) == 0 && sTempPw.GetLength() > 37 )	// is stored PW already encryped ?
		{
			sTempPw = sTempPw.Right( sTempPw.GetLength() - 5 );	// cut off the tag
			sTempPw = CPWCipher().Decode( sTempPw );
		}
		
		m_PW = sTempPw;
		m_RememberPW = true;
	}
	
	m_AutoLogin = m_PW.GetLength() > 0 ? CGCQLApp::sm_AutoLogin : 0;	// don't auto-login if there is no password

	UpdateData( false );
	
	if( m_AutoLogin )
	{
		m_bIsAutomatedLogin = TRUE;
		OnOK();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLogin::OnOK() 
{
	UpdateData( true );

	if( m_AutoLogin )
	{
		if( !m_RememberPW )
		{
			MessageBox( _T("\"Remember Password\" must be checked to perform an automated login.") );
			m_AutoLogin = false;
			return;
		}

		if( m_PW.GetLength() == 0 )
		{
			MessageBox( _T("A password must be entered to perform an automated login.") );
			m_AutoLogin = false;
			return;
		}
	}	

	
	if ( m_UIDControl.FindStringExact( -1, m_UID ) < 0 )
		m_UIDControl.AddString( m_UID );

	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );

	settings.put( "loginCount", m_UIDControl.GetCount() );
	for(int i=0;i<m_UIDControl.GetCount();i++)
	{
		CString sUID;
		m_UIDControl.GetLBText( i, sUID );

		settings.put( CharString().format("Login%d", i), CharString(sUID) );
	}

	if ( m_RememberPW )
	{
		settings.put( "rememberPW", 1 );

		CString	sTempPw = m_PW;
		if( sTempPw.GetLength() <= 40 )
		{
			sTempPw = CPWCipher().Encode( sTempPw );
			sTempPw = "[ECM]" + sTempPw;
		}
		else
			MessageBox(_T("Your password is too long and will be stored unencrypted.\nConsider reducing it to 40 characters."));

		settings.put( "previousPW", CharString(sTempPw) );
	}
	else
	{
		settings.put( "rememberPW", (dword)0 );
		settings.put( "previousPW", "" );
	}

	settings.put( "previousUID", CharString(m_UID) );
	CGCQLApp::sm_AutoLogin = m_AutoLogin == 0 ? false : true;

	if( !m_bIsAutomatedLogin && m_AutoLogin != 0 )
		MessageBox(_T("You can turn off the Auto-Login option in\nthe View->Options menu if you no longer need it."));

	CBCGDialog::OnOK();
}

