// ProcessClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ProcessClient.h"
#include "ProcessServer.h"

#include "Debug/Log.h"
#include "Standard/CommandLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessClientApp

BEGIN_MESSAGE_MAP(CProcessClientApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CProcessClientApp construction

CProcessClientApp::CProcessClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CProcessClientApp object

CProcessClientApp theApp;


// CProcessClientApp initialization

BOOL CProcessClientApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();
	AfxEnableControlContainer();

	new FileReactor( "ProcessClient.log" );
#if defined(_DEBUG)
	new DebugReactor();
#endif

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Palestar"));

	CommandLine arguments( m_lpCmdLine );
	if ( arguments.argumentCount() != 3 )
	{
		//MessageBox( NULL, _T("Usage: ProcessClient.exe <Server> <Port> <SID>"), _T("Invalid Arguments"), MB_OK );
		return FALSE;
	}
	
	CharString	sAddress		= arguments.argument( 0 );
	int			nPort			= arguments.argumentInt( 1 );
	dword		nSID			= arguments.argumentDWORD( 2 );

	CProcessServer dialog;
	if (! dialog.m_Client.open( sAddress, nPort ) || !dialog.m_Client.login( nSID ) )
	{
		MessageBox( NULL, CharString().format( "Failed to connect to %s:%d with SID of %u", 
			sAddress, nPort, nSID), "Failed to connect", MB_OK );
		return FALSE;
	}

	dialog.m_sName = CharString().format("%s:%d", sAddress, nPort );
	m_pMainWnd = &dialog;

	dialog.DoModal();

	return FALSE;
}
