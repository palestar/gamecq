#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "Service.h"

//----------------------------------------------------------------------------

static HANDLE ProcessStart( const char * commandLine )
{
    STARTUPINFO			si;
    PROCESS_INFORMATION pi;

	memset( &si, 0, sizeof(si) );
    si.cb = sizeof(si);

	char mutableCommandLine[ MAX_PATH ];
	strcpy_s( mutableCommandLine, sizeof(mutableCommandLine), commandLine );

	if ( !CreateProcess( NULL,					// No module name (use command line). 
        mutableCommandLine,						// Command line. 
        NULL,									// Process handle not inheritable. 
        NULL,									// Thread handle not inheritable. 
        FALSE,									// Set handle inheritance to FALSE. 
        CREATE_NEW_PROCESS_GROUP,				// Creation flags. 
        NULL,									// Use parent's environment block. 
        NULL,									// Use parent's starting directory. 
        &si,									// Pointer to STARTUPINFO structure.
        &pi ))									// Pointer to PROCESS_INFORMATION structure.
	{
		// failed
		return NULL;
	}

	return( pi.hProcess );
}

static bool ProcessActive( HANDLE hProcess )
{
	DWORD exitCode;
	if ( GetExitCodeProcess( hProcess, &exitCode ) )
		return exitCode == STILL_ACTIVE ? true : false;
	return false;
}

static int ProcessExitCode( HANDLE hProcess )
{
	DWORD exitCode;
	GetExitCodeProcess( hProcess, &exitCode );

	return exitCode;
}

//----------------------------------------------------------------------------

CService::CService()
	: CNTService(TEXT("GameCQ"), TEXT("GameCQ"))
	, m_hStop(0), m_hStopProcess(0)
{
	m_dwControlsAccepted = 0;
	m_dwControlsAccepted |= SERVICE_ACCEPT_STOP;
}


void CService::Run(DWORD dwArgc, LPTSTR * ppszArgv) {
	// report to the SCM that we're about to start
	ReportStatus(SERVICE_START_PENDING);

	m_hStop = ::CreateEvent(0, TRUE, FALSE, 0);
	m_hStopProcess = CreateEvent( 0, TRUE, FALSE, "ProcessServerStop" );
	m_hProcessRunning = CreateEvent( 0, TRUE, FALSE, "ProcessServerRun" );

	// TODO: You might do some initialization here.
	//		 Parameter processing for instance ...
	//		 If this initialization takes a long time,
	//		 don't forget to call "ReportStatus()"
	//		 frequently or adjust the number of milliseconds
	//		 in the "ReportStatus()" above.
	
#ifndef _DEBUG
	char path[ MAX_PATH ];
	GetModuleFileName( GetModuleHandle(NULL), path, MAX_PATH );

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath_s( path, drive, sizeof(drive), dir, sizeof(dir), NULL, 0, NULL, 0 );

	sprintf_s( path, sizeof(path), "%s%s", drive, dir );
	if (! SetCurrentDirectory( path ) )
		AddToMessageLog( "Failed to set current directory!", EVENTLOG_ERROR_TYPE );
#endif

	// report SERVICE_RUNNING immediately before you enter the main-loop
	// DON'T FORGET THIS!
	ReportStatus(SERVICE_RUNNING);

	// don't allow shutdown, if we're just coming back from a shutdown - this prevents looping reboots
	bool bAllowReboot = GetProfileInt( "GAMECQ", "allowReboot", 1 ) != 0;

	// main-loop
	while( ::WaitForSingleObject(m_hStop, 1000) != WAIT_OBJECT_0 ) 
	{
		// start the process server
		if ( m_hProcessServer == NULL )
			m_hProcessServer = ProcessStart( "ProcessServer.exe" );

		// make sure process server is still running
		if (! ProcessActive( m_hProcessServer ) )
		{
			switch( ProcessExitCode( m_hProcessServer ) )
			{
			case -3:
				// try to update the files using MirrorUpdate
				{
					bool bReboot = true;

					HANDLE hUpdate = ProcessStart( "MirrorUpdate.exe" );
					if ( hUpdate )
					{
						DWORD nSeconds = 0;

						// wait for it to finish
						while( ::WaitForSingleObject(m_hStop, 1000) != WAIT_OBJECT_0 && nSeconds < 120 && ProcessActive( hUpdate ) )
							nSeconds++;

						if (! ProcessActive( hUpdate ) && ProcessExitCode( hUpdate ) >= 0 )
							bReboot = false;
						::CloseHandle( hUpdate );
					}

					if (! bReboot )
						break;		// no reboot needed, restart ProcessServer!
					// failure, fall through and let it reboot
				}
			case -4:
				// check reboot flag
				if ( bAllowReboot )
				{
					// stop this service
					::SetEvent( m_hStop );

					// don't allow the machine to reboot again once restarted
					WriteProfileString( "GAMECQ", "allowReboot", "0" );
					// restart this machine
					shutdown();
				}
				else
					AddToMessageLog( "Reboot aborted, allow reboot flag not cleared!", EVENTLOG_INFORMATION_TYPE );
				break;
			default:
				break;
			}

			// restart the process server
			::CloseHandle( m_hProcessServer );
			m_hProcessServer = NULL;
		}

		// if the process server started ok, then clear the reboot flag
		if ( WaitForSingleObject( m_hProcessRunning, 0 ) == WAIT_OBJECT_0 )
		{
			AddToMessageLog( "Process Server running, clearing reboot flag...", EVENTLOG_INFORMATION_TYPE );

			WriteProfileString( "GAMECQ", "allowReboot", "1" );
			bAllowReboot = true;

			::ResetEvent( m_hProcessRunning );
		}
	}

	::SetEvent( m_hStopProcess );
	::CloseHandle( m_hProcessServer );
	::CloseHandle( m_hProcessRunning );
	::CloseHandle(m_hStop);
	::CloseHandle(m_hStopProcess);
}


void CService::Stop() {
	// report to the SCM that we're about to stop

	// TODO: Adjust the number of milliseconds you think
	//		 the stop-operation may take.
	ReportStatus(SERVICE_STOP_PENDING, 5000);

	if( m_hStop )
		::SetEvent(m_hStop);
}

//----------------------------------------------------------------------------

void CService::shutdown()
{
	AddToMessageLog( "Shutting down system to update locked files", EVENTLOG_INFORMATION_TYPE );

	HANDLE hToken; 
	if (! OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ) 
	{
		AddToMessageLog( "Failed to get process token for shutdown!", EVENTLOG_ERROR_TYPE );
		return;
	}

	// Get the LUID for the shutdown privilege. 
	TOKEN_PRIVILEGES tkp; 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1; // one privilege to set 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

	// Cannot test the return value of AdjustTokenPrivileges. 
	if (GetLastError() != ERROR_SUCCESS) 
	{
		AddToMessageLog( "Failed to adjust token privileges for system shutdown!", EVENTLOG_ERROR_TYPE );
		return;
	}

	// Shut down the system and force all applications to close. 
	if (! ExitWindowsEx(EWX_REBOOT, 0) ) 
		AddToMessageLog( "Failed to shutdown system!", EVENTLOG_ERROR_TYPE );
}

//----------------------------------------------------------------------------
