// LogServer.cpp : Defines the entry point for the application.
//

#include "Standard/CommandLine.h"
#include "Standard/Time.h"
#include "Standard/Settings.h"
#include "Standard/Event.h"
#include "Standard/Process.h"
#include "Network/LogServer.h"

//---------------------------------------------------------------------------------------------------

int main(int argc, char ** argv )
{
	if ( argc < 2 )
	{
		printf( "Usage: %s <iniFile>\n", argv[0] );
		return 0;
	}

	Event serverStop( CharString().format("StopProcess%u", Process::getCurrentProcessId()) );

	// read the settings
	Settings config( "LogServer", argv[1] );

	// initialize the logging first thing before we do anything else..
	std::string logFile( config.get( "logFile", "LogServer.log" ) );
	std::string logExclude( config.get( "logExclude", "" ) );
	unsigned int nLogLevel = config.get( "logLevel", LL_STATUS );
	new FileReactor( logFile, nLogLevel, logExclude );

	int nPort = config.get( "Port", 10000 );
	int nMaxClients = config.get( "MaxClients", 128 );
	int nCPT = config.get( "CPT", 8 );

	// start the server
	LogServer theServer;
	if (! theServer.start( nLogLevel, nPort, nMaxClients ) )
		return -1;

	// run the server forever, unless it crashes
	while( theServer.running() )
	{
		if ( serverStop.signaled() )
			break;
		Thread::sleep( 1000 );
	}

	theServer.stop();
	return 0;
}



