//////////////////////////////////////////////////////////////////////
// CHelpBot Class
//////////////////////////////////////////////////////////////////////

#include <atlbase.h>
#include <time.h>
#include <stdio.h>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include "Standard/Queue.h"

#include "HelpBot.h"

#import "bot\AimlBot.dll"

const IID IID_IAimlBot = {0x50394c12,0xd895,0x11d5,{0xa5,0xda,0x00,0x50,0x04,0x1b,0x91,0xff}};
// Prototypes
HRESULT GetBot (AIMLBOT_Lib::IAimlBot **ppAimlBot);


CHelpBot::CHelpBot()
{

}

CHelpBot::~CHelpBot()
{

}



HRESULT CHelpBot::GetBot(CharString sBotFileDir, AIMLBOT_Lib::IAimlBot **ppIAimlBot)
{
	if ( !ppIAimlBot )
		return E_INVALIDARG;

	*ppIAimlBot = 0;

	HRESULT hr = E_INVALIDARG;
	AIMLBOT_Lib::IAimlBot *pIAimlBot = NULL;

	CLSID clsid;
	_bstr_t progid;

	_bstr_t InitFile;
	_bstr_t UserFile;

	InitFile = "bot.ini";
	UserFile = "localuser.txt";

	if ( !SetCurrentDirectory( CharString( sBotFileDir ) ) )
	{
		LOG_STATUS( "HelpBot", CharString().format("Could not move to Home Directory : %s", sBotFileDir) );
		return hr;
	}

	progid = "Aiml.Bot";
	hr = CLSIDFromProgID ( progid, &clsid );
	if ( hr != S_OK )
		return hr;

	hr = CoCreateInstance( clsid, NULL, CLSCTX_SERVER, IID_IAimlBot, (void **)&pIAimlBot );

	if (hr == S_OK && pIAimlBot)
	{
		hr = pIAimlBot->initialize2( InitFile, UserFile );

		if (hr != S_OK)
		{
			while ( 0 < pIAimlBot->Release() ); // crappy COM release...
			return hr;
		}

		*ppIAimlBot = pIAimlBot;
	}

	return hr;
}


// returns true if succeeded, if it returns false a second try some time later may succeeed
bool CHelpBot::start( CharString sName, CharString sPassword, int nRoomId, int nGameId, CharString sBotData, dword nLowestPlayerId )
{
	// Startup
	LOG_STATUS( "HelpBot","Starting...");
	AIMLBOT_Lib::IAimlBot* pIAimlBot;

	LOG_STATUS( "HelpBot","Starting... COM...");
	if FAILED( CoInitialize(NULL) )
	{
		LOG_ERROR( "HelpBot","COM failed");
		return false;
	}

	LOG_STATUS( "HelpBot","Starting... BotEngine");

	TCHAR cCurrDir[MAX_PATH];
	if( !GetCurrentDirectory( MAX_PATH, cCurrDir ) )
	{
		LOG_ERROR( "HelpBot", "Error, could not retrieve current directory" );
		return false;
	}

	if ( FAILED( GetBot( sBotData, &pIAimlBot ) ) )
	{
		SetCurrentDirectory( cCurrDir );
		LOG_ERROR( "HelpBot","BotEngine failed");
		return false;
	}	
	if( !SetCurrentDirectory( cCurrDir ) )
	{
		LOG_ERROR( "HelpBot", "Error, could not switch back to original directory" );
		return false;
	}
	
	LOG_STATUS( "HelpBot","Starting... Connecting to lobby...");
	if( !m_MLoop.start(sName, sPassword, nRoomId, nGameId, &pIAimlBot, nLowestPlayerId ) )
		return false;
	
	LOG_STATUS( "HelpBot","Startup successfull...");

	return true;
}

bool CHelpBot::running()
{
	return m_MLoop.running();
}

void CHelpBot::stop()
{
	LOG_STATUS( "HelpBot","Preparing for shutdown...");
	
	m_MLoop.stop();

	while( !m_MLoop.waitQuit() )
		Thread::sleep(500);

	LOG_STATUS( "HelpBot","Quitting.. logging off...");
	m_MLoop.s_MetaClient.logoff();

	LOG_STATUS( "HelpBot","Quitting.. closing connection...");
	m_MLoop.s_MetaClient.close();

	LOG_STATUS( "HelpBot","Quitting.. dropping COM...");
	CoUninitialize();

	LOG_STATUS( "HelpBot","Quitting.. bye.");
}

