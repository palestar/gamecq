/**
	@file ProcessClientCLI.cpp
	Command Line interface based Process Client.

	(c)2013 Palestar Inc
	@author Richard Lyle @date 8/11/2013 12:05:52 PM
*/

#include "GCQ/ProcessClient.h"
#include "Standard/Settings.h"
#include "Standard/Thread.h"

#include <stdio.h>

//---------------------------------------------------------------------------------------------------

int main( int argc, char ** argv )
{
	if ( argc < 3 )
	{
		printf( "Usage: %s <Config> <Command> [Command Arguments]\n"
			"COMMANDS:\n"
			"START <Process Name> [BLOCK] - Start the given process by name.\n"
			"STOP <Process Name> - Stop a process by name.\n"
			"LIST - List all running processes.\n"
			"RESTART - Restart the ProcessServer.\n", argv[0] );
		return 1;
	}

	Settings config( "ProcessClientCLI", argv[1] );

	ProcessClient client;
	if (! client.open( config.get( "address", "localhost" ), config.get( "port", 9005 ) ) )
	{
		printf( "ERROR: Failed to connect to process server.\n" );
		return 2;
	}

	if (! client.login( config.get( "user", "" ), config.get( "password", "" ) ) )
	{
		printf( "ERROR: Failed to login to process server.\n" );
		return 3;
	}

	if ( stricmp( argv[2], "START" ) == 0 )
	{
		if( argc < 4 )
		{
			printf( "ERROR: Argument required for START command.\n" );
			return 1;
		}

		Array< ProcessClient::Process > processList;
		if (! client.getProcessList( processList ) )
		{
			printf( "ERROR: Failed to get process list.\n" );
			return 1;
		}

		for(int i=0;i<processList.size();++i)
		{
			if ( stricmp( processList[i].name.cstr(), argv[3] ) == 0 )
			{
				if ( client.startProcess( processList[i].processId ) )
				{
					printf( "Process %s started...\n", processList[i].name.cstr() );

					// are we waiting for the process to exit as well?
					if ( argc > 4 && atoi( argv[4] ) != 0 )
					{
						dword nProcessId = processList[i].processId;

						// wait for process to stop
						bool bProcessRunning = true;
						while( bProcessRunning )
						{
							Thread::sleep( 5000 );		

							if (! client.getProcessList( processList ) )
							{
								printf( "ERROR: Failed to get process list.\n" );
								return 1;
							}

							for(int i=0;i<processList.size();++i)
							{
								if ( processList[i].processId == nProcessId )
								{
									if ( (processList[i].flags & ProcessClient::PF_RUNNING) == 0 )
										bProcessRunning = false;
									break;
								}
							}
						}
					}

					return 0;
				}
				else
				{
					printf( "ERROR: Failed to start process %s.\n", processList[i].name.cstr() );
					return 1;
				}
			}
		}

		printf( "ERROR: Failed to find process by name (%s).", argv[3] );
		return 1;
	}
	else if ( stricmp( argv[2], "STOP" ) == 0 )
	{
		if( argc < 4 )
		{
			printf( "ERROR: Argument required for STOP command.\n" );
			return 1;
		}

		Array< ProcessClient::Process > processList;
		if (! client.getProcessList( processList ) )
		{
			printf( "ERROR: Failed to get process list.\n" );
			return 1;
		}

		for(int i=0;i<processList.size();++i)
		{
			if ( stricmp( processList[i].name.cstr(), argv[3] ) == 0 )
			{
				if ( client.stopProcess( processList[i].processId ) )
				{
					printf( "Process %s stopping...\n", processList[i].name.cstr() );
					return 0;
				}
				else
				{
					printf( "ERROR: Failed to stop process %s.\n", processList[i].name.cstr() );
					return 1;
				}
			}
		}

		printf( "ERROR: Failed to find process by name (%s).", argv[3] );
		return 1;
	}
	else if ( stricmp( argv[2], "LIST" ) == 0 )
	{
		Array< ProcessClient::Process > processList;
		if (! client.getProcessList( processList ) )
		{
			printf( "ERROR: Failed to get process list.\n" );
			return 1;
		}

		for(int i=0;i<processList.size();++i)
		{
			ProcessClient::Process & proc = processList[i];

			printf( "Process: %s, ID: %u, Exe: %s, Args: %s, Config: %s, Log: %s\n",
				proc.name.cstr(), proc.processId, proc.executable.cstr(),
				proc.arguments.cstr(), proc.config.cstr(), proc.log.cstr() );
		}

		return 0;
	}
	else if ( stricmp( argv[2], "RESTART" ) == 0 )
	{
		if ( client.restartAll() )
		{
			printf( "Process server restarted...\n" );
			return 0;
		}
		
		printf( "ERROR: Failed to restart process server.\n" );
		return 1;
	}

	printf( "ERROR: Unknown command (%s).\n", argv[2] );
	return 4;
}

//---------------------------------------------------------------------------------------------------
//EOF
