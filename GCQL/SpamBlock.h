// SpamBlock.h: interface for the CSpamBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPAMBLOCK_H__C2E48860_F4C5_11D6_9411_00001CDB2E9A__INCLUDED_)
#define AFX_SPAMBLOCK_H__C2E48860_F4C5_11D6_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CSpamBlock  
{
public:
	CSpamBlock();
	virtual ~CSpamBlock();

	bool check( String text );		
	bool heartbeat();			

private:
	
	struct SpamContext
	{
		Array<String>	text;		// last lines of text sent by the user
		Array<dword>	time;		// time of the last lines sent
		dword			muteUntil;	// time-point until the user is muted (if lower than current time, the user isn't muted)
		unsigned short	spamLevel;	// each time the user gets muted it is raised by one, then lowered over time
		dword			lastStatusChange;	// time of last spamLevel / mute change
		bool			muted;		// user currently muted ?
	};
	
	static class CSpamCheck
	{
	public:
		static bool check( SpamContext * sc, String text );	// checks each line sent by the user, returns false if the user appears to be spamming
		static bool heartbeat( SpamContext * sc );				// heartbeat, to be called every 1 to 10 seconds. (optionally only in case the user is muted), returns false if user should be still muted
	private:
		static dword calcMuteTimeForLevel( unsigned short lvl );
		static bool isSpamming( SpamContext * sc, String text );
	};
	
	SpamContext sc;
};

#endif // !defined(AFX_SPAMBLOCK_H__C2E48860_F4C5_11D6_9411_00001CDB2E9A__INCLUDED_)
