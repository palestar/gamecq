// StopServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Standard/Event.h"
#include "Standard/CommandLine.h"



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	CharString sCmdLine = lpCmdLine;

	CommandLine cmdLine( sCmdLine );
	if ( cmdLine.argumentCount() != 1 )
	{
		MessageBox( NULL, TEXT("Usage: StopServer.exe <PID>"), TEXT("Invalid Command Line"), MB_OK );
		return 0;
	}

	int pid = strtol( cmdLine.argument( 0 ), NULL, 10 );
	if ( pid != 0 )
	{
		Event serverStop( CharString().format("StopProcess%u", pid) );
		serverStop.signal();

		return 1;
	}

	return 0;
}



