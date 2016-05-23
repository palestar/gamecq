// SmartUpdate.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SmartUpdate.h"

#include "SimpleProcessAPI.h"
#include "ProgDlg.h"

#include <stdarg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if _MSC_VER <= 1310
static char * strcpy_s( char * pDst, size_t nSize, const char * pSrc )
{
	return strcpy( pDst, pSrc );
}
static char * strcat_s( char * pDst, size_t nSize, const char * pSrc )
{
	return strcat( pDst, pSrc );
}

#define _vsnprintf_s		_vsnprintf
#define _snprintf_s			_snprintf
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmartUpdateApp

BEGIN_MESSAGE_MAP(CSmartUpdateApp, CWinApp)
	//{{AFX_MSG_MAP(CSmartUpdateApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmartUpdateApp construction

CSmartUpdateApp::CSmartUpdateApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSmartUpdateApp object

CSmartUpdateApp theApp;
CProgressDlg	progressDialog;

#pragma warning( disable: 4996 )

//----------------------------------------------------------------------------

static void Log( const char * pFormat, ... )
{
	va_list va;
	va_start( va, pFormat );

	char pBuffer[ 1024 ];
	_vsnprintf_s( pBuffer, sizeof(pBuffer) - 1, pFormat, va );
	va_end(va);

	FILE * pLog = fopen( "SmartUpdate.log", "a" );
	if ( pLog != NULL )
	{
		fprintf( pLog, "%s\n", pBuffer );
		fclose( pLog );
	}
}

static DWORD StartProcess( const char * pCommandLine )
{
    STARTUPINFO			si;

	memset( &si, 0, sizeof(si) );
    si.cb = sizeof(si);

	char mutableCommandLine[ MAX_PATH ];
	strcpy_s( mutableCommandLine, sizeof( mutableCommandLine), pCommandLine );

	PROCESS_INFORMATION pi;
	if ( !CreateProcess( NULL,					// No module name (use command line). 
        mutableCommandLine,						// Command line. 
        NULL,									// Process handle not inheritable. 
        NULL,									// Thread handle not inheritable. 
        FALSE,									// Set handle inheritance to FALSE. 
        0,										// Creation flags. 
        NULL,									// Use parent's environment block. 
        NULL,									// Use parent's starting directory. 
        &si,									// Pointer to STARTUPINFO structure.
        &pi ))									// Pointer to PROCESS_INFORMATION structure.
	{
		// failed
		return 0;
	}

	return pi.dwProcessId;
}

static bool WaitProcess( DWORD pid, DWORD timeout, DWORD * a_pExitCode = NULL )
{
	bool processDone = true;

	HANDLE hProcess = OpenProcess( SYNCHRONIZE, true, pid );
	if ( hProcess != NULL )
	{
		// wait for the process to exit, timeout after 30 seconds
		if ( WaitForSingleObject( hProcess, timeout ) == WAIT_TIMEOUT)
			processDone = false;

		if ( processDone && a_pExitCode != NULL )
			GetExitCodeProcess( hProcess, a_pExitCode );

		// release the handle
		CloseHandle( hProcess );
	}

	return processDone;
}

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
	DWORD pid;
	GetWindowThreadProcessId( hwnd, &pid );

	// if this is the process we're looking to stop, ask it nicely to quit
	if ( pid == (DWORD)lParam )
		PostMessage( hwnd, WM_QUIT, 0, 0 );

	return TRUE;
}

static bool QuitProcess( DWORD pid )
{
	EnumWindows( EnumWindowsProc, pid );
	return true;
}

static bool KillProcess( DWORD pid )
{
	// firstly make sure the process is still running
	HANDLE hProcess = OpenProcess( PROCESS_TERMINATE, true, pid );
	if ( hProcess != NULL )
	{
		if (! TerminateProcess( hProcess, 0 ) )
		{
			CloseHandle( hProcess );
			return false;		// failed
		}
	
		CloseHandle( hProcess );
		return true;
	}

	// process isn't running
	return true;			
}

static bool StopProcess( DWORD pid )
{
	if ( pid == GetCurrentProcessId() )
		return false;		// don't attempt to kill ourselves

	CSimpleProcessAPI papi;
	CString processName( papi.GetProcessExecutableName( pid ) );

	if ( theApp.m_AskBeforeKill )
	{
		CString ask;
		ask.Format("%s is using a file that needs to be updated; do you wish to stop the program?", processName );
		
		if ( MessageBox( NULL, ask, "SmartUpdate", MB_YESNO ) != IDYES )
			return false;
	}

	// firstly ask process to stop
	QuitProcess( pid );

	progressDialog.SetRange( 0, 20 );
	progressDialog.SetStep( 1 );
	progressDialog.SetPos( 0 );

	int seconds = 20;
	while( seconds > 0 )
	{
		CString status;
		status.Format( "Stopping %s...", processName );
		progressDialog.SetStatus( status );
		progressDialog.StepIt();

		if ( WaitProcess( pid, 1000 ) )
			return true;
		
		seconds--;

		if ( progressDialog.CheckCancelButton() )
			exit( -1 );
	}

	if ( theApp.m_AskBeforeKill )
	{
		CString ask;
		ask.Format( "%s has failed to stop, do you want to terminate the program?", processName );

		if ( MessageBox( NULL, ask, "SmartUpdate", MB_YESNO ) != IDYES )
			return false;
	}

	CString status;
	status.Format( "Terminating %s...", processName );
	progressDialog.SetStatus( status );

	if (! KillProcess( pid ) )
		return false;

	return true;
}

//----------------------------------------------------------------------------

#pragma warning( disable: 4800 )

static bool UpdateFile( const char * src, const char * dst )
{
	CString status;
	status.Format("Updating...%s", dst );
	progressDialog.SetStatus( status );

	if ( progressDialog.CheckCancelButton() )
		exit( -1 );

	DWORD fa = GetFileAttributes( dst );
	if ( fa == 0xffffffff )
	{
		// file doesn't exist, should move without a problem
		if ( MoveFile( src, dst ) )
			return false;			// file move succeeded
	}
	
	// check for the read-only flag
	if ( fa & FILE_ATTRIBUTE_READONLY )
		SetFileAttributes( dst, FILE_ATTRIBUTE_NORMAL );		// remove the read-only flag

	// file exists, it might be locked
	if ( DeleteFile( dst ) ) 
	{
		// deleted the file, rename the new file
		if ( MoveFile( src, dst ) )
			return false;
	}

	// file is locked, find the exe which has this file currently locked
	CSimpleProcessAPI papi;
	
	// get the PID of the locking process
	CMapStringToString map;
	papi.BuildProcessList( map );

	DWORD pid = papi.GetFirstProcessLockingModule( dst, map );
	while ( pid != 0 )
	{
		if (! StopProcess( pid ) )
			return true;

		// check for another process using the file
		pid = papi.GetFirstProcessLockingModule( dst, map );
	}

	// ok try to update the file once more
	if ( DeleteFile( dst ) ) 
	{
		// deleted the file, rename the new file
		if ( MoveFile( src, dst ) )
			return false;
	}

	// attempt to move the file during reboot, only works on NT/2000 machines
	MoveFileEx(src, dst, MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING );

	// all attempts failed, we will need to reboot now
	return true;
}

static bool UpdateFiles( const char * path )
{
	bool reboot = false;

	// look for a install.ini file, if found check the contents and run any installers that might need to be ran..
	char sInstallConfig[ MAX_PATH ];
	strcpy_s( sInstallConfig, sizeof(sInstallConfig), path );
	strcat_s( sInstallConfig, sizeof(sInstallConfig), "install.ini.upd" );

	if ( GetFileAttributes( sInstallConfig ) == 0xffffffff )
	{
		// no install.ini.upd found, try install.ini then..
		strcpy_s( sInstallConfig, sizeof(sInstallConfig), path );
		strcat_s( sInstallConfig, sizeof(sInstallConfig), "install.ini" );
	}

	int nInstallSteps = GetPrivateProfileInt( "SmartUpdate", "InstallSteps", 0, sInstallConfig );
	Log( "Found %d install steps in %s", nInstallSteps, sInstallConfig );

	for(int i=0;i<nInstallSteps;++i)
	{
		char sProduct[ 32 ];
		GetPrivateProfileString( "SmartUpdate", "Product", "", sProduct, sizeof(sProduct), sInstallConfig );
		char sBaseKey[ 64 ];
		_snprintf_s( sBaseKey, sizeof(sBaseKey ), "Install_%s_Step%d", sProduct, i );
		char sRegKey[ 128 ];
		_snprintf_s( sRegKey, sizeof(sRegKey ), "%s_Registry", sBaseKey );
		char sRegKeyValue[ 256 ];
		GetPrivateProfileString( "SmartUpdate", sRegKey, "", sRegKeyValue, sizeof(sRegKeyValue), sInstallConfig );
		if ( sRegKeyValue[0] == 0 )
			_snprintf_s( sRegKeyValue, sizeof(sRegKeyValue), "Software\\Palestar\\SmartUpdate\\%s", sRegKey );	// generate a registry key then..
		
		Log( "Looking for Registry Key %s", sRegKeyValue );

		HKEY hKey;
		if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, sRegKeyValue, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL ) != ERROR_SUCCESS )
		{
			MessageBox( NULL, "Error: Failed to open registry!", "SmartUpdate", MB_OK );
			exit( -1 );
		}

		DWORD nType = REG_DWORD;
		DWORD nValue = 0; 
		DWORD nValueSize = sizeof(nValue);
		if ( RegQueryValueEx( hKey, "STATE", NULL, &nType, (BYTE *)&nValue, &nValueSize ) != ERROR_SUCCESS || nValue == 0 )
		{
			Log( "%s key not found, Running custom build step.", sRegKeyValue );

			char sRunKey[ 128 ];
			_snprintf_s ( sRunKey, sizeof(sRunKey), "%s_Run", sBaseKey );

			char sRun[ MAX_PATH ];
			GetPrivateProfileString( "SmartUpdate", sRunKey, "", sRun, sizeof(sRun), sInstallConfig );

			// no registry key found, run the installer and once it completes, add the value to the registry..
			DWORD nProcessID = StartProcess( sRun );
			if ( nProcessID == 0 )
			{
				MessageBox( NULL, "Error: Failed to run install step!", sRun, MB_OK );
				exit( -1 );
			}

			// wait for the process to complete..
			DWORD nExitCode = 0;
			if (! WaitProcess( nProcessID, (60 * 60) * 1000, &nExitCode ) )		// wait an hour for it to run..
				nExitCode = 1;
			
			Log( "Process %s completed, exit code is %d", sRun, nExitCode );
			if ( nExitCode == 0 )
			{
				// set the registry so we don't run this install step again..
				DWORD nStepCompleted = 1;
				RegSetValueEx( hKey, "STATE", 0, REG_DWORD, (BYTE *)&nStepCompleted, sizeof(nStepCompleted) );
			}
			else
				MessageBox( NULL, "Error: Install step failed with an error.", sRun, MB_OK );
		}
		else
		{
			Log( "%s key found", sRegKeyValue );
		}
		RegCloseKey( hKey );
	}

	char mask[ MAX_PATH ];
	strcpy_s( mask, sizeof(mask),  path );
	strcat_s( mask, sizeof(mask), "*.upd" );

	int fileCount = 0;

	// count the number of files firstly, so we can setup the progress bar
	WIN32_FIND_DATA ff;
	HANDLE ffh = FindFirstFile( mask, &ff );
	if ( ffh != INVALID_HANDLE_VALUE )
	{
		bool found = true;
		while(found)
		{
			if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
				fileCount++;
			found = FindNextFile(ffh,&ff);
		}
		FindClose(ffh);
	}

	int file = 0;

	// check for files first, then check for directories to recurse into
	ffh = FindFirstFile( mask, &ff );
	if ( ffh != INVALID_HANDLE_VALUE )
	{
		bool found = true;
		while(found)
		{
			progressDialog.SetRange( 0, fileCount );
			progressDialog.SetPos( file );

			if ( progressDialog.CheckCancelButton() )
				exit( -1 );

			if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
			{
				char src[ MAX_PATH ];
				strcpy_s( src, sizeof(src), path );
				strcat_s( src, sizeof(src), ff.cFileName );
				char dst[ MAX_PATH ];
				strcpy_s( dst, sizeof(dst), src );
				dst[ strlen( dst ) - 4 ] = 0;	// remove the .upd extension

				reboot |= UpdateFile( src, dst );

				file++;
			}

			found = FindNextFile(ffh,&ff);
		}
		FindClose(ffh);
	}

	// recurse into the subdirectories
	strcpy_s( mask, sizeof(mask), path );
	strcat_s( mask, sizeof(mask), "*.*" );

	ffh = FindFirstFile( mask, &ff );
	if ( ffh != INVALID_HANDLE_VALUE )
	{
		bool found = true;
		while( found )
		{
			if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && ff.cFileName[0] != '.' )
			{
				char recurse[ MAX_PATH ];
				strcpy_s( recurse, sizeof(recurse), path );
				strcat_s( recurse, sizeof(recurse), ff.cFileName );
				strcat_s( recurse, sizeof(recurse), "\\" );

				reboot |= UpdateFiles( recurse );
			}
			found = FindNextFile(ffh, &ff );
		}
		FindClose( ffh );
	}

	return reboot;
}

static bool Shutdown()
{
	HANDLE hToken; 
	if (! OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ) 
	{
		// not NT or 2000, then do standard shutdown
		ExitWindowsEx(EWX_REBOOT, 0);
		return true;
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
		MessageBox( NULL, "Failed to adjust token privileges for system shutdown!", "SmartUpdate", MB_OK );
		return false;
	}

	// Shut down the system and force all applications to close. 
	if (! ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0) ) 
	{
		MessageBox( NULL, "Failed to shutdown system!", "SmartUpdate", MB_OK );
		return false;
	}

	// system shutting down
	return true;
}

//----------------------------------------------------------------------------


BOOL CSmartUpdateApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	m_AskBeforeKill = true;
	m_AskBeforeReboot = true;

	if ( strlen( m_lpCmdLine ) < 1 )
	{
		MessageBox( NULL, "Usage: SmartUpdate <[Path][File]>", "Invalid Command Line", MB_OK );
		return FALSE;
	}

	char path[ MAX_PATH ];
	char processId[ MAX_PATH ];
	char postExecute[ MAX_PATH ];

	if ( (GetFileAttributes( m_lpCmdLine ) & FILE_ATTRIBUTE_DIRECTORY) == 0 )
	{
		char iniFile[ MAX_PATH ];
		strcpy_s( iniFile, sizeof(iniFile), m_lpCmdLine );
	
		GetPrivateProfileString( "Update", "Path", "", path, MAX_PATH, iniFile );
		GetPrivateProfileString( "Update", "ProcessId", "0", processId, MAX_PATH, iniFile );
		GetPrivateProfileString( "Update", "PostExecute", "", postExecute, MAX_PATH, iniFile );
		m_AskBeforeKill = GetPrivateProfileInt( "Update", "AskKill", 1, iniFile ) != 0 ? true : false;
		m_AskBeforeReboot = GetPrivateProfileInt( "Update", "AskReboot", 1, iniFile ) != 0 ? true : false;
	}
	else
	{
		processId[ 0 ] = 0;
		postExecute[ 0 ] = 0;
		strcpy_s( path, sizeof(path), m_lpCmdLine );
	}

	if ( path[ strlen( path ) - 1 ] != '\\' )
		strcat_s( path, sizeof(path), "\\" );

	progressDialog.Create();
	progressDialog.SetStep( 1 );

	// wait for parent process to end
	DWORD pid = strtoul( processId, NULL, 10 );
	if ( pid != 0 )
	{
		progressDialog.SetStatus( "Waiting for parent process to exit..." );
		if (! WaitProcess( pid, 10 * 1000 ) )
			StopProcess( pid );
	}

	// update the files
	progressDialog.SetStatus( "Check for updates..." );
	if ( UpdateFiles( path ) )
	{
		// some files were NOT updated, ask the user to reboot
		if ( m_AskBeforeReboot )
		{
			if ( MessageBox( NULL, "SmartUpdate could not update some files; do you wish to reboot to update these files?", "SmartUpdate", MB_YESNO ) != IDYES )
				exit( -1 );
		}

		// some files were locked, return the error code indicating this. The calling application
		// should either prompt the user to reboot or reboot automatically (in the case of a service!)
		char program[ MAX_PATH ];
		GetModuleFileName( GetModuleHandle(NULL), program, MAX_PATH );

		// create copy of this exe to run during startup
		char program2[ MAX_PATH ];
		strcpy_s( program2, sizeof(program2), program );
		char * slash = strrchr( program2, '\\' );
		if ( slash != NULL )
		{
			*slash = 0;
			strcat_s( program2, sizeof(program2), "\\SmartUpdate2.exe" );

			// lets this copy file fail silently, if it does it's because we're already running MirrorUpdate2.exe
			CopyFile( program, program2, false );
		}
		else
		{
			MessageBox( NULL, "Error: Failed to parse program path!", "MirrorUpdate", MB_OK );
			exit( -1 );
		}

		// write an entry to the registry so that this application is ran again upon user login, 
		// so that the files get updated
		HKEY hKey;
		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
		{
			MessageBox( NULL, "Error: Failed to open registry!", "MirrorUpdate", MB_OK );
			exit( -1 );
		}

		char keyValue[ MAX_PATH ];
		_snprintf_s( keyValue, sizeof(keyValue), "\"%s\" %s", program2, path );

		// find an empty key slot
		if ( RegQueryValueEx( hKey, "SmartUpdate", 0, NULL, NULL, NULL ) != ERROR_SUCCESS )
			if ( RegSetValueEx( hKey, "SmartUpdate", 0,  REG_SZ, (unsigned char *)keyValue, strlen(keyValue) + 1 ) != ERROR_SUCCESS )
			{
				MessageBox( NULL, "Error: Failed to write to registry!", "MirrorUpdate", MB_OK );
				exit( -1 );
			}
		
		RegCloseKey( hKey );

		// start the system shutdown
		Shutdown();
	}

	// run the post update program
	if ( strlen( postExecute ) > 0 )
		StartProcess( postExecute );

	return FALSE;
}
