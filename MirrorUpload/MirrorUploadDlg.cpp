// MirrorUploadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MirrorUpload.h"
#include "MirrorUploadDlg.h"
#include "EditProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadDlg dialog

CMirrorUploadDlg::CMirrorUploadDlg( CWnd* pParent /*=NULL*/)
	: CDialog(CMirrorUploadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMirrorUploadDlg)
	//}}AFX_DATA_INIT

	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


CMirrorUploadDlg::~CMirrorUploadDlg()
{
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp != NULL );

	// save all profiles, kill any running threads...
	pApp->WriteProfileInt( _T("MirrorUpload"), _T("ProfileCount"), m_Profiles.size() );
	for(int i=0;i<m_Profiles.size();i++)
	{
		CharString sProfileBase;
		sProfileBase.format( "Profile%d", i );

		if ( m_Profiles[i].pThread != NULL )
		{
			m_Profiles[i].pThread->kill();
			delete m_Profiles[i].pThread;
			m_Profiles[i].pThread = NULL;
		}

		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sName" ), CString( m_Profiles[i].sName) );
		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sLogFile" ), CString( m_Profiles[i].sLogFile) );
		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sPath" ), CString( m_Profiles[i].sPath) );
		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sAddress" ), CString( m_Profiles[i].sAddress) );
		pApp->WriteProfileInt( _T("MirrorUpload"), CString( sProfileBase + "nPort" ), m_Profiles[i].nPort );
		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sUID" ), CString( m_Profiles[i].sUID) );
		pApp->WriteProfileString( _T("MirrorUpload"), CString( sProfileBase + "sPW" ), CString( m_Profiles[i].sPW) );
	}
}

void CMirrorUploadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMirrorUploadDlg)
	DDX_Control(pDX, IDC_LIST1, m_cProfileList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMirrorUploadDlg, CDialog)
	//{{AFX_MSG_MAP(CMirrorUploadDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnNewProfile)
	ON_BN_CLICKED(IDC_BUTTON2, OnDeleteProfile)
	ON_BN_CLICKED(IDC_BUTTON3, OnUpload)
	ON_BN_CLICKED(IDC_BUTTON8, OnEditProfile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnListOpen)
	ON_BN_CLICKED(IDC_BUTTON7, OnOpenLog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadDlg message handlers

BOOL CMirrorUploadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp != NULL );

	// load profiles from registry
	int nProfileCount = pApp->GetProfileInt( _T("MirrorUpload"), _T("ProfileCount"), 0 );
	for(int i=0;i<nProfileCount;i++)
	{
		CharString sProfileBase;
		sProfileBase.format( "Profile%d", i );

		Profile & profile = m_Profiles.push();
		profile.sName = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sName" ), _T("") );
		profile.sLogFile = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sLogFile" ), _T("") );
		profile.sPath = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sPath" ), _T("") );
		profile.sAddress = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sAddress" ), _T("") );
		profile.nPort = pApp->GetProfileInt( _T("MirrorUpload"), CString( sProfileBase + "nPort" ), 9000 );
		profile.sUID = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sUID" ), _T("") );
		profile.sPW = pApp->GetProfileString( _T("MirrorUpload"), CString( sProfileBase + "sPW" ), _T("") );
		profile.pThread = NULL;
	}

	// now initialize our list control with the profiles
	CRect rect;
	m_cProfileList.GetClientRect( &rect );
	m_cProfileList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	int cWidth = rect.Width() / 5;
	m_cProfileList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, cWidth * 2, 0 );
	m_cProfileList.InsertColumn( 1, _T("Server"), LVCFMT_LEFT, cWidth * 2, 1 );
	m_cProfileList.InsertColumn( 2, _T("Status"), LVCFMT_LEFT, cWidth, 2 );
	m_cProfileList.InsertColumn( 3, _T("Path"), LVCFMT_LEFT, cWidth * 3, 3 );
	m_cProfileList.InsertColumn( 4, _T("Log"), LVCFMT_LEFT, cWidth * 3, 4 );

	// update our list control now..
	OnTimer( 0 );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMirrorUploadDlg::OnTimer(UINT nIDEvent) 
{
	bool bUploading = false;

	// check the status of all profiles, and update the status field...
	for(int i=0;i<m_Profiles.size();i++)
	{
		Profile & profile = m_Profiles[i];
		if ( m_cProfileList.GetItemCount() <= i )
			m_cProfileList.InsertItem( i, CString( profile.sName ) );
		else
			m_cProfileList.SetItemText( i, 0, CString( profile.sName ) );

		m_cProfileList.SetItemText( i, 1, CString( CharString().format("%s:%d", profile.sAddress, profile.nPort)) );
		m_cProfileList.SetItemText( i, 3, CString( profile.sPath ) );
		m_cProfileList.SetItemText( i, 4, CString( profile.sLogFile ) );

		if ( profile.pThread != NULL )
		{
			if ( profile.pThread->done() )
			{
				if ( profile.pThread->error() )
					m_cProfileList.SetItemText( i, 2, _T("Error") );
				else
					m_cProfileList.SetItemText( i, 2, _T("Success") );

				// remove the thread object now..
				delete profile.pThread;
				profile.pThread = NULL;
			}
			else
			{
				m_cProfileList.SetItemText( i, 2, _T("Uploading") );
				bUploading = true;
			}
		}
	}

	// remove extra list items
	while( m_cProfileList.GetItemCount() > m_Profiles.size() )
		m_cProfileList.DeleteItem( m_cProfileList.GetItemCount()-1 );

	if ( nIDEvent == 0x1 )
		CDialog::OnTimer(nIDEvent);

	if (! bUploading )
		KillTimer( 0x1 );		// stop the updates if nothing is uploading...
}

//----------------------------------------------------------------------------

void CMirrorUploadDlg::OnNewProfile() 
{
	CEditProfile dialog;
	if ( dialog.DoModal() == IDOK )
	{
		// create a new profile entry
		Profile & profile = m_Profiles.push();
		profile.sName = dialog.m_sName;
		profile.sLogFile = dialog.m_sLogFile;
		profile.sPath = dialog.m_sLocalPath;
		profile.sAddress = dialog.m_sAddress;
		profile.nPort = dialog.m_nPort;
		profile.sUID = dialog.m_sUser;
		profile.sPW = dialog.m_sPassword;
		profile.pThread = NULL;

		OnTimer( 0x0 );
	}
}

void CMirrorUploadDlg::OnDeleteProfile() 
{
	Array< int > remove;

	POSITION pos = m_cProfileList.GetFirstSelectedItemPosition();
	while( pos )
	{
		int nItem = m_cProfileList.GetNextSelectedItem( pos );
		Profile & profile = m_Profiles[ nItem ];
		if ( MessageBox( CString( CharString().format("Confirm delete of profile %s?", profile.sName)), _T("Confirm Delete"), MB_YESNO) != IDYES )
			continue;

		remove.push( nItem );
	}	
	
	for(int i=remove.size()-1;i>=0;i--)
	{
		Profile & profile = m_Profiles[ remove[i] ];
		if ( profile.pThread != NULL )
		{
			profile.pThread->kill();
			delete profile.pThread;
			profile.pThread = NULL;
		}
		m_Profiles.remove( remove[i] );
	}

	OnTimer( 0x0 );
}

void CMirrorUploadDlg::OnUpload() 
{
	POSITION pos = m_cProfileList.GetFirstSelectedItemPosition();
	while( pos )
	{
		int nItem = m_cProfileList.GetNextSelectedItem( pos );
		Profile & profile = m_Profiles[ nItem ];

		if ( profile.pThread != NULL )
			continue;		// upload already in progress
		if ( MessageBox( CString(CharString().format("Please confirm upload from %s to %s:%d", profile.sPath, profile.sAddress, profile.nPort) ),
			CString( profile.sName ), MB_YESNO) != IDYES )
			continue;		// skip

		// start upload then...
		profile.pThread = new UploadThread( profile.sPath, profile.sAddress, 
			profile.nPort, profile.sUID, profile.sPW );
		profile.pThread->resume();

		// start an update timer..
		SetTimer( 0x1, 2500, NULL );
	}
	
}

void CMirrorUploadDlg::OnEditProfile() 
{
	POSITION pos = m_cProfileList.GetFirstSelectedItemPosition();
	while( pos )
	{
		int nItem = m_cProfileList.GetNextSelectedItem( pos );
		Profile & profile = m_Profiles[ nItem ];

		CEditProfile dialog;
		dialog.m_sName = profile.sName;
		dialog.m_sLogFile = profile.sLogFile;
		dialog.m_sLocalPath = profile.sPath;
		dialog.m_sAddress = profile.sAddress;
		dialog.m_nPort = profile.nPort;
		dialog.m_sUser = profile.sUID;
		dialog.m_sConfirmPassword = dialog.m_sPassword = profile.sPW;

		if ( dialog.DoModal() == IDOK )
		{
			// create a new profile entry
			profile.sName = dialog.m_sName;
			profile.sLogFile = dialog.m_sLogFile;
			profile.sPath = dialog.m_sLocalPath;
			profile.sAddress = dialog.m_sAddress;
			profile.nPort = dialog.m_nPort;
			profile.sUID = dialog.m_sUser;
			profile.sPW = dialog.m_sPassword;
			profile.pThread = NULL;

			OnTimer( 0x0 );
		}
	}
}

void CMirrorUploadDlg::OnListOpen(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnEditProfile();
}

void CMirrorUploadDlg::OnOpenLog() 
{
	POSITION pos = m_cProfileList.GetFirstSelectedItemPosition();
	while( pos )
	{
		int nItem = m_cProfileList.GetNextSelectedItem( pos );
		Profile & profile = m_Profiles[ nItem ];

		ShellExecute( 0, _T("open"), CString( profile.sLogFile ), _T(""), _T(""), SW_NORMAL );
	}	
}
