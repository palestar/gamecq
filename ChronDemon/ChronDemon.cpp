// ChronDemon.cpp : Defines the entry point for the application.
//

#include "Standard/CommandLine.h"
#include "Standard/Process.h"
#include "Standard/Settings.h"
#include "Standard/Time.h"
#include "GCQ/MetaClient.h"

#include <stdio.h>
#include <time.h>

//----------------------------------------------------------------------------

int main(int argc, char ** argv )
{
	if ( argc < 2 )
	{
		printf( "Usage: %s <iniFile>\n", argv[0] );
		return 1;
	}

	Settings settings( "ChronDemon", argv[1] );

	// initialize the logging first thing before we do anything else..
	std::string logFile( settings.get( "logFile", "ChronDemon.log" ) );
	std::string logExclude( settings.get( "logExclude", "" ) );
	unsigned int nMinLogLevel = settings.get( "logLevel", LL_STATUS );
	new FileReactor( logFile, nMinLogLevel, logExclude );

	CharString	metaServer = settings.get( "metaServer", "meta-server.palestar.com" );
	int		metaServerPort = settings.get( "metaServerPort", 8000 );
	int		chatRoom = settings.get( "chatRoom", (dword)0 );
	CharString	user = settings.get( "user", "" );
	CharString	password = settings.get( "password", "" );

	CharString	mday = settings.get( "mday", "*" );
	CharString	month = settings.get( "month", "*" );
	CharString	wday = settings.get(  "wday", "*" );
	CharString	hour = settings.get( "hour", "*" );
	CharString	minute = settings.get( "minute", "*" );
	
	CharString sTimeMask = month + "/"  + mday + "/* " + hour + ":" + minute + ":" + "*";

	dword	nWarningTime = settings.get( "warningTime", 1800 );
	CharString	sMessage = settings.get( "warningMessage", "/notice System Maintenance" );
	CharString	sExecute = settings.get( "execute", "" );
	bool	bExit = settings.get( "exit", 1 ) != 0;
	
	LOG_STATUS( "ChronDemon", "ChronDemon started, metaServer = %s : %d, mday = %s, month = %s, wday = %s, hour = %s, minute = %s, execute = %s",
		metaServer.cstr(), metaServerPort, mday.cstr(), month.cstr(), wday.cstr(), hour.cstr(), minute.cstr(), sExecute.cstr() );

	Event serverStop( CharString().format("StopProcess%u", Process::getCurrentProcessId()) );

	void * pProcess = NULL;

	dword nLastTrigger = Time::seconds();
	while( true )
	{
		Thread::sleep( 100 );
		
		if ( serverStop.signaled() )
			return 0;

		dword nCurrentTime = Time::seconds();
		if ( (nCurrentTime - nLastTrigger) <= 60 )
			continue;		// skip for now, don't let the trigger happen more than once per minute

		if ( Time::isTime( nCurrentTime, sTimeMask ) )
		{
			nLastTrigger = nCurrentTime;

			LOG_STATUS( "ChronDemon", "Executing %s...", sExecute.cstr() );
			pProcess = Process::start( sExecute );

			// wait for process to end
			while( pProcess != NULL )
			{
				Thread::sleep( 100 );

				if ( serverStop.signaled() )
				{
					// kill the process if we are told to stop, don't leave it zombied..
					Process::stop( pProcess );
					return 0;
				}

				// check for our process to end..
				if (! Process::active( pProcess ) )
				{
					// process has ended, get the exit code...
					int nExitCode = Process::exitCode( pProcess );

					LOG_STATUS( "ChronDemon", CharString().format("Child process has ended, exit code = %d", nExitCode) );

					Process::close( pProcess );
					pProcess = NULL;

					// if exit flag is true we are going to exit, returning the child process exit code..
					if ( bExit )
					{
						if ( nWarningTime > 0 )
						{
							// optionally, connect to the metaserver and broadcast a message 
							MetaClient metaClient;
							if (! metaClient.open( metaServer, metaServerPort ) && metaClient.login( user, password ) )
							{
								LOG_STATUS( "ChronDemon", "Connected to meta server!" );

								dword nCurrentTime = Time::seconds();
								dword nEndTime = nCurrentTime + nWarningTime;

								while( nCurrentTime < nEndTime )
								{
									dword nTimeLeft = nEndTime - nCurrentTime;

									CharString sTimeLeft;
									sTimeLeft.format("%d %s", 
										nTimeLeft > 60 ? nTimeLeft / 60 : nTimeLeft,
										nTimeLeft > 60 ? "minute(s)" : "second(s)");

									// replace the "$T" token with the time remaining...
									CharString sChat( sMessage );
									sChat.replace( "$T", sTimeLeft );

									metaClient.sendChat( 0, sChat );

									LOG_STATUS( "ChronDemon", CharString().format("Exiting in %d seconds...", nTimeLeft) );

									int nSleepTime = 0;
									if ( nTimeLeft > (60 * 5 ) )
										nSleepTime = 60 * 5;			// sleep for 5 minutes
									else if ( nTimeLeft > 60 )
										nSleepTime = 60;				// sleep for 1 minute
									else
										nSleepTime = 10;				// sleep for 10 seconds

									nCurrentTime += nSleepTime;
									while( nSleepTime > 0 )
									{
										Thread::sleep( 1000 );
										if ( serverStop.signaled() )
											return 0;							// we've been stopped, so just exit quitely...

										nSleepTime--;
									}
								}
							}
							else
								LOG_STATUS( "ChronDemon", "Failed to connect to meta server!" );

							metaClient.close();
						}

						LOG_STATUS( "ChronDemon", CharString().format("Exiting %d...", nExitCode) );
						return nExitCode;
					}
				}
			} // while( pProcess != NULL)
		}

	}

	return 0;
}



