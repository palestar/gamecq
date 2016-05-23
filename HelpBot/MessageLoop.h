// MessageLoop.h: interface for the CMessageLoop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGELOOP_H__D6889E00_CC17_11D6_9411_00001CDB2E9A__INCLUDED_)
#define AFX_MESSAGELOOP_H__D6889E00_CC17_11D6_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GCQ\MetaClient.h"
#include "ResponseControl.h"
#include "EmoteResponder.h"

#import "Bot\AimlBot.dll"



class CMessageLoop  
{
public:
	static MetaClient			s_MetaClient;

	
	bool start( CharString sNick, CharString sPassword, dword roomId, dword gameId, 
		AIMLBOT_Lib::IAimlBot **ppAimlBot, dword lowestPlayerID );
	
	CMessageLoop();
	virtual ~CMessageLoop();
	
	void stop();
	bool running();
	bool waitQuit();

	static CharString filterTags( CharString sText );			// strips commonly used BBCode and HTML tags
private:
	bool player( dword userId, MetaClient::ShortProfile & player ) const; // get player information by uId
	CharString filterText( CharString sText );					// reduces text to alphanumeric + dots
	void checkMessages();
	void checkQueue();
	void hangman( char word, dword authorId );
	bool isAboutMe( CharString sText, CharString sBotName );	// is there a chance this textline addresses the bot ?
	void doCloneAction( dword userId,
		CharString sNoCloneSendChat, CharString sNoCloneLog,
		CharString sCloneLog );								// if userId has no clones sendChat() first string and
														// log() the second, else log() third string

	void checkForCloneActions( CharString sText );			// process text to see if its the result of a /clone command

	struct ChatLine
	{
		CharString				author;
		dword				authorId;
		CharString				text;
	};

	struct CloneAction
	{
		dword userId;
		CharString noCloneSendChat;
		CharString noCloneLog;
		CharString cloneLog;

		CloneAction & operator=( const CloneAction & copy );
	};

	dword					m_nRoomId;
	int						m_TopChatMessage;		// index of the last line pulled from the metaclient
	Queue<struct ChatLine>	m_MessageQueue;			// newly received chatdata is stored here until another thread grabs it
	CriticalSection			m_QueueLock;			// for exclusive access to the queue, also internally used for the threadcount
	CResponseControl		m_RespCtrl;				// Response control object, used to determine if a message should be ignored or the response whispered
	int						m_ThreadsRunning;		// Number of threads running. 0 usually means ready to exit (+ a few ms delay)
	bool					m_PrepareExit;			// If set all threads will exit asap (within half a second)
	bool					m_AllowEmotes;			// Enable/Disable emotes during runtime
	dword					m_LastMessage;			// Holds the time of the last message received, used to check for connection timeouts
	bool					m_Away;					// If no message arrives for a period of time send the bot away
	int						m_AwayIt;				// Away itterator, so we can count how many replies we want until the bot is back
	AIMLBOT_Lib::IAimlBot*	m_pAimlBot;				// The bot... externally supplied reference to parse the messages
	CEmoteResponder			m_EmResp;				// Emote responder, saves all emotes on the bot, i.e. slaps for later usage
	dword					m_lowestPlayerID;		// Lowest playerID the bot gives newbie-help to.
	dword					m_lastEmoteSave;		// Time the emotes got saved
	Array<CloneAction>		m_CloneActions;			// Things to do after a clone check was done
	CriticalSection			m_CloneLock;			// for exclusive access to m_CloneActions
	Array< char >			m_gameWord;
	Array< char >			m_guessWord;
	bool					m_Hangman;
	bool					m_Init;
	CharString				m_FullWord;
	CharString				m_GuessWord;
	int						m_WordLength;
	int						m_Life;
	Array< char >			m_CharCheck;

class UpdateThread : public SafeThread		// MetaClient Chatdata -> MessageQueue
	{
	public:
		// Construction
		UpdateThread( CMessageLoop * pLoop );
		// Thread interface
		int				run();
	private:
		CMessageLoop *	m_pLoop;
	};

	friend UpdateThread;

class AnswerThread : public SafeThread		// MessageQueue -> AimlBot
	{
	public:
		// Construction
		AnswerThread( CMessageLoop * pLoop );
		// Thread interface
		int				run();
	private:
		CMessageLoop *	m_pLoop;
	};

	friend AnswerThread;

	inline bool incThreads()				// call directly before starting a thread
	{
		m_QueueLock.lock();
		if( !m_PrepareExit )
			m_ThreadsRunning++;
		m_QueueLock.unlock();
		return m_PrepareExit;
	}

	inline void decThreads()						// call before "delete this" of a thread
	{
		m_QueueLock.lock();
		m_ThreadsRunning--;
		m_QueueLock.unlock();
	}

};

inline CMessageLoop::CloneAction & CMessageLoop::CloneAction::operator=( const CloneAction & copy )
{
	userId			= copy.userId;
	noCloneSendChat	= copy.noCloneSendChat;
	noCloneLog		= copy.noCloneLog;
	cloneLog		= copy.cloneLog;
	return *this;
}




#endif // !defined(AFX_MESSAGELOOP_H__D6889E00_CC17_11D6_9411_00001CDB2E9A__INCLUDED_)
