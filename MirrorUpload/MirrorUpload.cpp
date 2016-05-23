// MirrorUpload.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MirrorUpload.h"
#include "MirrorUploadDlg.h"

#include "Standard/CommandLine.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadApp

BEGIN_MESSAGE_MAP(CMirrorUploadApp, CWinApp)
	//{{AFX_MSG_MAP(CMirrorUploadApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadApp construction

CMirrorUploadApp::CMirrorUploadApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMirrorUploadApp object

CMirrorUploadApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadApp initialization

BOOL CMirrorUploadApp::InitInstance()
{
	AfxEnableControlContainer();

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	SetRegistryKey( _T("Palestar") );


	CharString sCommandLine = m_lpCmdLine;
	CommandLine cmdLine( sCommandLine );

	if ( cmdLine.argumentCount() > 0 )
	{
		// Command line mode
		if ( cmdLine.argumentCount() != 6 )
		{
			MessageBox( NULL, TEXT("Usage: MirrorUpload <Path> <Address> <Port> <UID> <PW>"), TEXT("Invalid Command Line"), MB_OK );
			return FALSE;
		}

		UploadThread upload( 
			cmdLine.argument( 0 ),						// path
			cmdLine.argument( 1 ),						// address
			cmdLine.argumentInt( 2 ),					// port
			cmdLine.argument( 3 ),						// user
			cmdLine.argument( 4 ));						// password

		upload.resume();

		while( !upload.done() )
			Thread::sleep( 1000 );

		return FALSE;
	}


	// Interactive mode
	CMirrorUploadDlg().DoModal();
	return FALSE;
}
