// SpamBlock.cpp: implementation of the CSpamBlock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gcql.h"
#include "SpamBlock.h"
#include "Standard/Time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpamBlock::CSpamBlock()
{
	sc.spamLevel = 0;
	sc.muteUntil = 0;
	sc.lastStatusChange = 0;
	sc.muted = false;
}

CSpamBlock::~CSpamBlock()
{

}

bool CSpamBlock::heartbeat()
{
	return CSpamCheck::heartbeat( &sc );
}

bool CSpamBlock::check( String text )
{
	return CSpamCheck::check( &sc, text );
}




bool CSpamBlock::CSpamCheck::heartbeat( SpamContext * sc )
{
	if( !sc->muted )
		return true;

	if( sc->muteUntil <= Time::seconds() )
	{
		sc->muted = false;
		sc->lastStatusChange = Time::seconds();
		return true;
	}
	else
		return false;
}

bool CSpamBlock::CSpamCheck::check( SpamContext *sc, String text )
{
	dword currtime = Time::seconds();

	if( isSpamming( sc, text ) )
	{
		int muteTime;
		sc->muted = true;
		sc->lastStatusChange = currtime;
		sc->spamLevel++;
		muteTime = calcMuteTimeForLevel( sc->spamLevel );
		sc->muteUntil = currtime + muteTime;
		MetaClient & client = CGCQLApp::sm_MetaClient;
		client.sendLocalChat("\n---------------------------------------------\nYour chat has been disabled due to spam\n---------------------------------------------\n"); // send a message to the client
		// Due to the mute time the user would get a "fresh start" after unmute.
		// By adding the mutetime to the time of each line in which the
		// user was silent by force the mute-time isn´t considered in further checks.
		for( int i = 0 ; i < 5 ; i++ )
			sc->time[i] += muteTime;

		return false;
	}

	if( sc->spamLevel > 0 && !sc->muted )	// check if we can try to reduce the spamlevel
		if( sc->lastStatusChange <= currtime - ( sc->spamLevel * 90 ) )
		{
			sc->lastStatusChange = currtime;
			sc->spamLevel--;
		}

	return true;
}

bool CSpamBlock::CSpamCheck::isSpamming(SpamContext * sc, String text)
{
	bool bFlag;
	dword currtime = Time::seconds();
	assert( sc->text.size() == sc->time.size() );
	if( sc->text.size() > 4 )
	{
		sc->text.remove( 0 );
		sc->time.remove( 0 );
	}
	
	sc->text.push( text );
	sc->time.push( currtime );
	
	if( sc->text.size() < 5 )	// to keep it simple we need at least 5 messages from the user for the spamcheck to work
		return false;	// not enough messages yet, assume he isn´t spamming

	// check for flooding by messagetime
	if( sc->time[2] > currtime - 2 )		// 3 messages within 2 seconds
		return true;

	if( sc->time[0] > currtime - 4 )	// 5 messages within 6 seconds
		return true;
	
	
	// check for same text, thus repeats
	if( sc->time[2] > currtime - 6 )		// 3 repeats within 6 seconds
		if( ((String)sc->text[2]).compareNoCase( sc->text[3] ) == 0 && ((String)sc->text[3]).compareNoCase( sc->text[4] ) )
			return true;
		
	if( sc->time[0] > currtime - 20 )		// 5 repeats within 20 seconds
	{
		bFlag = true;
		for( int i = 0 ; i < 4 ; i++ )
			if( ((String)sc->text[i]).compareNoCase( sc->text[i+1] ) != 0 )
			{
				bFlag = false;	// delete flag if there is a different message
				break;
			}

		if( bFlag )		// all equal -> spam
			return true;
	}
	
	
	// bytecheck
	int bytes = 0;
	for( int i = 0 ; i < 5 ; i++ )		// 600 bytes in 4 seconds
		if( sc->time[i] > currtime - 3 )
			bytes += ((String)sc->text[i]).length();
	if( bytes >= 1000 )
		return true;
	
	bytes = 0;
	for( int i = 0 ; i < 5 ; i++ )		// 1500 bytes in 15 seconds
		if( sc->time[i] > currtime - 10 )
			bytes += ((String)sc->text[i]).length();
	if( bytes >= 2000 )
		return true;
	
	// TODO: partial repeat check still needs to be added

	return false;
}

dword CSpamBlock::CSpamCheck::calcMuteTimeForLevel( unsigned short lvl )
{
	assert( lvl > 0 );
	switch( lvl )
	{
		case 1: return 10;
		case 2: return 20;
		case 3: return 40;
		case 4: return 60;
		case 5: return 90;
		case 6: return 120;
		default: return 180;
	}
}
