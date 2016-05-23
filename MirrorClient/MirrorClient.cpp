// MirrorClient2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Standard/CommandLine.h"
#include "Network/MirrorClient.h"



//----------------------------------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	CharString sCmdLine = lpCmdLine;
	CommandLine cmdLine( sCmdLine );

	if ( cmdLine.argumentCount() != 3 && cmdLine.argumentCount() != 5 )
	{
		MessageBox( NULL, TEXT("Usage: MirrorClient <Path> <Mirror Address> <Mirror Port> [UID] [PW]"), TEXT("Invalid Command Line"), MB_OK );
		return 0;
	}

	// check for updated files
	MirrorClient mirrorClient;
	
	CharString	path = cmdLine.argument( 0 );
	CharString	address = cmdLine.argument( 1 );
	int			port = strtol( cmdLine.argument( 2 ), NULL, 10 );

	if (! mirrorClient.open( address, port, path, NULL, false ) )
		return -1;

	bool bLogin = cmdLine.argumentCount() == 5;
	if ( bLogin && !mirrorClient.login( cmdLine.argument( 3 ), cmdLine.argument( 4 ) ) )
			return -1;

	dword nJob = mirrorClient.syncronize( bLogin );
	if ( nJob != 0 )
	{
		mirrorClient.waitJob( nJob, 86400 * 1000 );
		mirrorClient.close();
		return 1;
	}
	else
	{
		// no files to get
		mirrorClient.close();
		return 0;
	}
}



