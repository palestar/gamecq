// MirrorUpdate.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------------------------

static bool		MoveFileExSupported = true;

//----------------------------------------------------------------------------

#pragma warning( disable: 4800 )

static bool UpdateFile( const char * src, const char * dst )
{
	DWORD fa = GetFileAttributes( dst );
	if ( fa == 0xffffffff )
	{
		// file doesn't exist, should move without a problem
		if ( MoveFile( src, dst ) )
			return false;		// file move succeeded
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

	//MessageBox( NULL, "Failed to move file!", src, MB_OK );
	if ( MoveFileExSupported )
	{
		if ( !MoveFileEx(src, dst, MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING ) )
		{
			// Failed to move the file, Windows 98/95/ME
			MoveFileExSupported = false;
		}
	}

	// file was locked, request reboot
	return true;
}

static bool UpdateFiles( const char * path )
{
	bool reboot = false;

	char mask[ MAX_PATH ];
	strcpy_s( mask, sizeof(mask), path );
	strcat_s( mask, sizeof(mask), "*.upd" );

	// check for files first, then check for directories to recurse into
	WIN32_FIND_DATA ff;
	HANDLE ffh = FindFirstFile( mask, &ff );
	if ( ffh != INVALID_HANDLE_VALUE )
	{
		bool found = true;
		while(found)
		{
			if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
			{
				char src[ MAX_PATH ];
				strcpy_s( src, sizeof(src), path );
				strcat_s( src, sizeof(src), ff.cFileName );
				char dst[ MAX_PATH ];
				strcpy_s( dst, sizeof(dst), src );
				dst[ strlen( dst ) - 4 ] = 0;	// remove the .upd extension

				reboot |= UpdateFile( src, dst );
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

static int DoUpdate( const char * pPath )
{
	// update the files
	if ( UpdateFiles( pPath ) )
	{
		// If not NT/2000 then use the registry to run a copy of this EXE on the next restart
		if (! MoveFileExSupported )
		{
			// some files were locked, return the error code indicating this. The calling application
			// should either prompt the user to reboot or reboot automtically (in the case of a service!)
			char program[ MAX_PATH ];
			GetModuleFileName( GetModuleHandle(NULL), program, MAX_PATH );

			// create copy of this exe to run during startup
			char program2[ MAX_PATH ];
			strcpy_s( program2, sizeof( program2 ), program );
			char * slash = strrchr( program2, '\\' );
			if ( slash != NULL )
			{
				*slash = 0;
				strcat_s( program2, sizeof(program2), "\\MirrorUpdate2.exe" );

				// lets this copy file fail silently, if it does it's because we're already running MirrorUpdate2.exe
				CopyFile( program, program2, false );
			}
			else
			{
				MessageBox( NULL, "Error: Failed to parse program path!", "MirrorUpdate", MB_OK );
				return 0;
			}

			// write an entry to the registry so that this application is ran again upon user login, 
			// so that the files get updated
			HKEY hKey;
			if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
			{
				MessageBox( NULL, "Error: Failed to open registry!", "MirrorUpdate", MB_OK );
				return 0;
			}


			char keyValue[ MAX_PATH ];
			sprintf_s( keyValue, sizeof(keyValue), "\"%s\" %s", program2, pPath );

			// find an empty key slot
			char valueName[ MAX_PATH ];
			for(int i=0;i<32;i++)
			{
				sprintf_s( valueName, sizeof(valueName), "MirrorUpdate%d", i );
				if ( RegQueryValueEx( hKey, valueName, 0, NULL, NULL, NULL ) != ERROR_SUCCESS )
					break;		// found empty slot
			}
			if ( RegSetValueEx( hKey, valueName, 0,  REG_SZ, (unsigned char *)keyValue, strlen(keyValue) + 1 ) != ERROR_SUCCESS )
			{
				MessageBox( NULL, "Error: Failed to write to registry!", "MirrorUpdate", MB_OK );
				return 0;
			}
			
			RegCloseKey( hKey );
		}

		return -4;
	}

	// terminate intentional
	return 0;
}

//----------------------------------------------------------------------------

int main( int argc, char ** argv )
{
	char path[ MAX_PATH ];
	if ( argc < 2 )
		strcpy_s( path, sizeof(path), "./" );
	else
		strcpy_s( path, sizeof(path), argv[1] );
	
	char processId[ MAX_PATH ];
	char postExecute[ MAX_PATH ];

	if ( (GetFileAttributes( path ) & FILE_ATTRIBUTE_DIRECTORY) == 0 )
	{
		char iniFile[ MAX_PATH ];
		strcpy_s( iniFile, sizeof(iniFile), path );
	
		GetPrivateProfileString( "Update", "Path", "", path, MAX_PATH, iniFile );
		GetPrivateProfileString( "Update", "ProcessId", "0", processId, MAX_PATH, iniFile );
		GetPrivateProfileString( "Update", "PostExecute", "", postExecute, MAX_PATH, iniFile );
	}
	else
	{
		processId[ 0 ] = 0;
		postExecute[ 0 ] = 0;
	}

	if ( path[ strlen( path ) - 1 ] != '\\' )
		strcat_s( path, sizeof(path), "\\" );

	// wait for parent process to end
	DWORD pid = strtoul( processId, NULL, 10 );
	if ( pid != 0 )
	{
		//HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, true, pid );
		HANDLE hProcess = OpenProcess( SYNCHRONIZE, true, pid );
		if ( hProcess != NULL )
		{
			// wait for the process to exit, timeout after 30 seconds
			if ( WaitForSingleObject( hProcess, 30 * 1000 ) == WAIT_TIMEOUT)
			{
				// terminate the process if not already
				TerminateProcess( hProcess, 0 );
			}
			// release the handle
			CloseHandle( hProcess );

			// wait until the files have been unlocked
			//Sleep( 1000 );
		}
		//else
		//	MessageBox( NULL, "Failed to open parent process!", "MirrorUpdate", MB_OK );
	}

	// update the files
	int rv = DoUpdate( path );

	// run the post update program
	if ( strlen( postExecute ) > 0 )
	{
		STARTUPINFO			si;
		PROCESS_INFORMATION pi;

		memset( &si, 0, sizeof(si) );
		si.cb = sizeof(si);

		if ( !CreateProcess( NULL,					// No module name (use command line). 
			postExecute,								// Command line. 
			NULL,									// Process handle not inheritable. 
			NULL,									// Thread handle not inheritable. 
			FALSE,									// Set handle inheritance to FALSE. 
			0,										// No creation flags. 
			NULL,									// Use parent's environment block. 
			NULL,									// Use parent's starting directory. 
			&si,									// Pointer to STARTUPINFO structure.
			&pi ))									// Pointer to PROCESS_INFORMATION structure.
		{
			// failed
			MessageBox( NULL, "Failed to restart program!", postExecute, MB_OK );
		}
	}

	// exit this program
	return rv;
}



