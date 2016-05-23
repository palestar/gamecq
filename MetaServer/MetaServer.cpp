/*
	MetaServer.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/ExceptionHandler.h"
#include "Standard/CommandLine.h"
#include "Standard/Settings.h"
#include "Standard/Process.h"
#include "GCQS/MetaServer.h"

//----------------------------------------------------------------------------

int main(int argc, char ** argv )
{
	if ( argc < 2 )
	{
		printf( "Usage: %s <iniFile>\n", argv[0] );
		return 1;
	}

	Event serverStop( CharString().format("StopProcess%u", Process::getCurrentProcessId()) );
	Settings settings( "MetaServer", argv[1] );

	StandardOutReactor output;

	// initialize the logging first thing before we do anything else..
	std::string logFile( settings.get( "logFile", "MetaServer.log" ) );
	std::string logExclude( settings.get( "logExclude", "" ) );
	unsigned int nMinLogLevel = settings.get( "logLevel", LL_STATUS );
	new FileReactor( logFile, nMinLogLevel, logExclude );

	MetaServer::Context context;
	context.dbname = settings.get( "dbname", "" );
	context.dbaddress = settings.get( "dbaddress", "" );
	context.dbport = settings.get( "dbport", 3306 );
	context.dbuid = settings.get( "dbuid", "" );
	context.dbpw = settings.get( "dbpw", "" );
	context.maxConnections = settings.get( "maxConnections", 4 );
	context.motdFile = settings.get( "motdFile", "" );
	context.address = settings.get( "address", "" );
	context.port = settings.get( "port", 8000 );
	context.maxClients = settings.get( "maxClients", 1000 );
	context.gameId = settings.get( "gameId", 1 );
	context.eventNotifyTime = settings.get( "eventNotifyTime", 3600 );

	// start the server
	MetaServer theServer;
	if (! theServer.start( context ) )
		return -1;

	// run the server forever, unless it crashes
	while( theServer.running() )
	{
		if (! serverStop.wait( 10 ) )
			break;

		theServer.update();
	}

	int seconds = settings.get( "shutdownTime", 90 );
	dword nNow = Time::seconds();
	dword nShutdownTime = nNow + seconds;
	dword nWarningTime = nNow;

	while( nNow < nShutdownTime )
	{
		nNow = Time::seconds();
		if ( nNow >= nWarningTime )
		{
			theServer.sendGlobalChat( CharString().format("/METASERVER: shutting down in %d seconds...", seconds) );
			seconds -= 10;
			nWarningTime = nNow + 10;
		}

		theServer.update();
		Thread::sleep( 10 );
	}
	theServer.sendGlobalChat( "/METASERVER: shutting down NOW..." );
	theServer.update();

	theServer.stop();

	return 0;
}

//----------------------------------------------------------------------------
//EOF
