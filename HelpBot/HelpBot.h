/*
	HelpBot.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef HELPBOT_H
#define HELPBOT_H

//---------------------------------------------------------------------------------------------------

#include "GCQ\MetaClient.h"
#include "MessageLoop.h"

class CHelpBot  
{
public:
	bool running();
	static MetaClient s_MetaClient;

	
	CHelpBot();
	virtual ~CHelpBot();

	bool start( CharString sName, CharString sPassword, int nRoomId, int nGameId, 
		CharString sBotData, dword nLowestPlayerId );
	void stop();


private:
	CMessageLoop m_MLoop;

	HRESULT CHelpBot::GetBot( CharString sBotFileDir, AIMLBOT_Lib::IAimlBot **ppIAimlBot);
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF
