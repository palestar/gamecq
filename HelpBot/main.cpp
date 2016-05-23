//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//#include <windows.h>


#include "Standard/CommandLine.h"
#include "Standard/Process.h"
#include "Standard/Settings.h"

#include "HelpBot.h"
#include <direct.h>

//----------------------------------------------------------------------------

static int StartBot( CHelpBot & bot, const char * pBotName, const char * pPassword, 
			  dword roomId, dword gameId, const char * pData, dword lowestPlayerId )
{
	if (! bot.start( pBotName, pPassword, roomId, gameId, pData, lowestPlayerId ) )
		return -1;
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	CharString sCmdLine = lpCmdLine;
	CommandLine cmdLine( sCmdLine );
	CharString path;

	if ( cmdLine.argumentCount() > 0 )
		path = cmdLine.argument( 0 );
	else
	{
		// We have no command line, get the current working directory and the default config.ini
		path = _getcwd( NULL, 0);
		path += "\\config.ini";
	}
	Settings settings( "HelpBot", path );

	// initialize the logging first thing before we do anything else..
	std::string logFile( settings.get( "logFile", "HelpBot.log" ) );
	std::string logExclude( settings.get( "logExclude", "" ) );
	unsigned int nMinLogLevel = settings.get( "logLevel", LL_STATUS );
	FileReactor fileReactor( logFile, nMinLogLevel, logExclude );

	CharString	sBotName =	settings.get( "name", "" );
	CharString	sBotPw	=	settings.get( "password", "" );
	dword	nRoomId =	settings.get( "roomId", (dword)0 );
	dword	nGameId =	settings.get( "gameId", (dword)0 );
	CharString	sDataDir =  settings.get( "dataDir", "" );
	dword	nLowestPlayerId =  settings.get( "lowestPlayerId", (dword)999999999 );
	
	if( sBotName.length() == 0 || sBotPw.length() == 0 || sDataDir.length() == 0 ||
		logFile.length() == 0 || nRoomId == 0 || nGameId == 0 )
		return 0;	// return 0 so we won´t restart endlessly in case of an incorrect inifile

	//TraceOutput().initialize( DebugOutput::LOG_FILE | DebugOutput::RESET_LOG_FILE, "HelpBot" );
	
	Event botStop( CharString().format("StopProcess%u", Process::getCurrentProcessId()) );

	// create the bot object
	CHelpBot theBot;
	// start the bot
	if ( StartBot( theBot, sBotName, sBotPw, nRoomId, nGameId, sDataDir, nLowestPlayerId ) < 0 )
		return -1;

	// run the server forever, unless it crashes
	int retstatus = -1;
	while( theBot.running() )
	{
		if (! botStop.wait( 30 * 1000 ) )
		{
			retstatus = 0;
			break;
		}
	}

	theBot.stop();

	return retstatus;
}


