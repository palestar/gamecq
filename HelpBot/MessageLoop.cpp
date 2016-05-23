// MessageLoop.cpp: implementation of the CMessageLoop class.
//
//////////////////////////////////////////////////////////////////////

#include <comdef.h>
#include <stdio.h>
#include <time.h>

#include "MessageLoop.h"
#include "ResponseControl.h"
#include "SmartSend.h"
#include "GCQ/MetaClient.h"

#include "Standard/StringBuffer.h"
#include "Standard/Time.h"
#include "Standard/RegExpM.h"
#include "File/FileDisk.h"

MetaClient			CMessageLoop::s_MetaClient;	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageLoop::CMessageLoop()
{
	m_Init = false;
	m_Hangman = false;
	m_ThreadsRunning = 0;
	m_PrepareExit = false;
	m_AllowEmotes = true;
	m_TopChatMessage = 0;
	m_Away = false;
	m_AwayIt = 0;
	m_WordLength = 0;
	m_Life = 10;

	m_QueueLock.initialize();
	m_CloneLock.initialize();
}

CMessageLoop::~CMessageLoop()
{

}

bool CMessageLoop::start( CharString sName, CharString sPassword, dword roomId, dword gameId, AIMLBOT_Lib::IAimlBot **ppAimlBot, dword lowestPlayerID )
{
	m_pAimlBot = *ppAimlBot;
	m_nRoomId = roomId;

	if ( s_MetaClient.open() < 0 )
	{
		LOG_ERROR( "HelpBot","Open failed");
		return false;
	}
	LOG_STATUS( "HelpBot","Starting... Opened connection...");

	if( s_MetaClient.login( sName, sPassword ) != MetaClient::LOGIN_OKAY )
	{
		LOG_ERROR( "HelpBot", CharString().format("Login( %s ) failed", sName) );
		return false;
	}
	LOG_STATUS( "HelpBot", CharString().format("Logged in as \"%s\"...", sName) );
					
	if ( s_MetaClient.selectGame( gameId ) < 0 )
	{
		LOG_ERROR( "HelpBot","SelectGame failed");
		return false;
	}
	LOG_STATUS( "HelpBot", CharString().format("Starting... Selected game %u...", gameId) );

	if( s_MetaClient.joinRoom( m_nRoomId, "" ) != m_nRoomId )
	{
		LOG_ERROR( "HelpBot", CharString().format("JoinRoom( %d ) failed", m_nRoomId) );
		return false;
	}
	LOG_STATUS( "HelpBot", CharString().format("Starting... Joined room %u...", roomId) );
	
	m_lowestPlayerID = lowestPlayerID;

	LOG_STATUS( "HelpBot", "Starting... Requesting name..." );
	CharString sBotName = s_MetaClient.profile().name;

	LOG_STATUS( "HelpBot", "Starting... Setting up emote handler..." );
	m_EmResp = CEmoteResponder( sBotName );
	
	if( sName != sBotName )
		return false;

	LOG_STATUS( "HelpBot", "Starting... Checking time..." );
	m_LastMessage = Time::seconds();
	m_lastEmoteSave = m_LastMessage;

	LOG_STATUS( "HelpBot", "Starting... MessageThread..." );

	incThreads();
	UpdateThread * pUThread = new UpdateThread( this );
	pUThread->resume();
	
	LOG_STATUS( "HelpBot", "Starting... ResponderTread..." );

	incThreads();
	AnswerThread * pAThread = new AnswerThread( this );
	pAThread->resume();
	
	return true;
}

/**
 *	CheckMessages() continuosly polls the MetaClient for new messages, determines the "usable" ones and
 *  adds them to the MessageQueue.
 *  Also it provides logging and connection timeout detection.
 **/
void CMessageLoop::checkMessages()
{
	LOG_STATUS( "HelpBot", "MessageThread started..." );
	s_MetaClient.lock();
	while(s_MetaClient.loggedIn() && !m_PrepareExit)	// Main message loop, will stop when slapped or no longer logged in
	{

		s_MetaClient.update();
		 // No message from the server for 15 minutes, send the bot away
		if( !m_Away && ( ( Time::seconds() - m_LastMessage ) >= 900 ) )
		{	
			LOG_STATUS( "HelpBot", "Chat is inactive, going away..." );
			s_MetaClient.sendChat( m_nRoomId, "/away" );
			m_Away = true;
		}
		// No message from the server for 5 hours, terminate the bot
		if ( !m_Away && ( ( Time::seconds() - m_LastMessage ) >= 18000 ) )
		{	
			m_PrepareExit = true;
		}
		for(;m_TopChatMessage<s_MetaClient.chatCount();m_TopChatMessage++)
		{
			const MetaClient::Chat & chat = s_MetaClient.chat( m_TopChatMessage );
		
			m_LastMessage = Time::seconds();
			CharString text = filterText( chat.text );

			// If the bot was away due to message timeout, bring it back if there have been two messages
			if( m_Away && m_AwayIt > 1 )
			{
				LOG_STATUS( "HelpBot", "Chat is active again, coming back..." );
				s_MetaClient.sendChat( m_nRoomId,"/back" );
				m_Away = false;
				m_AwayIt = 0;
			} else {
				m_AwayIt++;
			}
			
			if( chat.recpId == s_MetaClient.profile().userId )
			{	
				MetaClient::ShortProfile profile;
				s_MetaClient.player( m_nRoomId, chat.authorId, profile );

				if ( profile.flags & MetaClient::MODERATOR || profile.flags & MetaClient::ADMINISTRATOR )
				{				
					// log private messages but do not react on them, just send pongs to certain users
					LOG_STATUS( "HelpBot", CharString().format("Priv Msg: %s @%u: %s", chat.author, chat.authorId, text) );

					if ( chat.text.find("!say ") >= 0 )
					{
						CharString tempText = chat.text;	
						tempText.right( tempText.length() - ( tempText.find("!say ") + 5 ) );
						tempText.left( tempText.length() - 1 );
						if( tempText.endsWith( "</b>" ) )
							tempText.left( tempText.length() - 4 );
						if( tempText.beginsWith( "/" ))
							s_MetaClient.sendChat(m_nRoomId,tempText);
						else
							s_MetaClient.sendChat(m_nRoomId,CharString().format("[color=00dd99]%s[/color]", tempText ));
					}
					if( chat.text.find("!shutdown") >= 0 )
					{
						m_PrepareExit = true;
					}
					if( chat.text.find("!emotetoggle") >= 0 )
					{
						m_AllowEmotes = !m_AllowEmotes;
						s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u Emote responder status : %u...", chat.authorId, m_AllowEmotes ) );
					}
					if( chat.text.find("!emotecheck") >= 0 )
					{
						s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u Checking emotes...", chat.authorId));		
						m_EmResp.checkEmotesCorruption();
					}
					if( chat.text.find("!backup") >= 0 )
					{
						m_EmResp.saveEmotes( "./emotes.bak" );
						s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u saved...", chat.authorId));
					}
					if( chat.text.find("!restore") >= 0 )
					{
						m_EmResp.loadEmotes( "./emotes.txt" );
						s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u restored...", chat.authorId));
					}
					if( chat.text.find("!hangman") >= 0 )
					{
						if ( !m_Hangman )
						{
							CharString tempTxt = chat.text;
							CharString gameWord;
							RegExpM cr;
							cr.regComp( "!hangman ([A-z]+)" );	
							int pos = cr.regFind( chat.text );
							if( pos >= 0 )
							{
								gameWord = chat.text;
								gameWord.upper();
								gameWord.trim();
								gameWord.right( chat.text.length() - ( pos + 9 ) );
								int length = 0;
								for( int i = 0 ; i < gameWord.length()-5 ; i++ )
								{
									LOG_STATUS( "HelpBot", CharString().format("Hangman: %d is %c", i, gameWord.buffer()[i] ) );
									m_FullWord.append( gameWord.buffer()[i] );
									m_GuessWord.append( "_" );
								}
								m_WordLength = strlen( m_FullWord );
							}
							m_WordLength = gameWord.length();

							if ( m_WordLength > 0 )
							{
								m_Hangman = true;
								m_Life = 10;
								s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]Hangman is now in session, word is %s![/color]", m_GuessWord ) );
								s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]Message me with the command !guess <letter> to guess it (e.g. /send lobbyghost !guess B)![/color]", m_GuessWord ) );
								s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u [GAME] Hangman now running, word is %s...", chat.authorId, m_FullWord ));
								LOG_STATUS( "HelpBot", CharString().format("Hangman: Full string: %s", m_FullWord) );
								LOG_STATUS( "HelpBot", CharString().format("Hangman: Guess string: %s", m_GuessWord) );
							}
						}
						else
							s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u [GAME] Already running, word is %s.", chat.authorId, m_FullWord ));
					}
					if( chat.text.find("!debug") >= 0 )
					{
						if ( m_EmResp.debug() )	
							s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u debug status : %u...", chat.authorId, m_EmResp.m_Debug ) );
					}
					if( chat.text.find("!emotedebug") >= 0 )
					{
						int emCount = m_EmResp.getEmoteCount();
						CSmartSend smartsend = CSmartSend( &s_MetaClient, CharString().format( "/send @%u </b>", chat.authorId ) );
						smartsend.send( CharString().format( "EmoteCount: %d", emCount ) );
						for( int i = 0 ; i < emCount ; i++ )
							smartsend.send( CharString().format( "%u) </b>%s</b>", i, m_EmResp.getEmoteWithRating(i) ) );

						smartsend.forceSend();
					}
					if( chat.text.find("!listnew") >= 0 )
					{
						int emCount = m_EmResp.getEmoteCount();
						CSmartSend smartsend = CSmartSend( &s_MetaClient, CharString().format( "/send @%u </b> ", chat.authorId ) );
						smartsend.send( CharString().format( "EmoteCount: %d", emCount ) );
						for( int i = 0 ; i < emCount ; i++ )
							if( m_EmResp.isEmoteNonRated( i ) )
								smartsend.send( CharString().format( "%u) </b>%s</b>", i, m_EmResp.getEmote(i) ) );

						smartsend.forceSend();
					}
					if( chat.text.find("!listlast") >= 0 )
					{
						int emCount = m_EmResp.getEmoteCount();
						CSmartSend smartsend = CSmartSend( &s_MetaClient, CharString().format( "/send @%u </b>", chat.authorId ) );
						int begin = Max( 0, emCount - 100 );
						for( int i = begin ; i < emCount ; i++ )
							smartsend.send( CharString().format( "%u) </b>%s</b>", i, m_EmResp.getEmoteWithRating(i) ) );

						smartsend.forceSend();
					}
					if( chat.text.find("!forget ") >= 0 )
					{
						CharString tempTxt = chat.text;
						RegExpM cr;
						cr.regComp( "!forget ([0-9]+)" );	
						int pos = cr.regFind( chat.text );
						if( pos >= 0 )
						{
							CharString id = chat.text;
							id.right( chat.text.length() - ( pos + 8 ) );
							int length = 0;
							for( int i = 0 ; i < id.length() ; i++ )
								if( id.buffer()[i] >= '0' && id.buffer()[i] <= '9' )
									length++;
								else
									break;

							id.left( length );
							int emnr = 	strtol( id, NULL, 10 );
							
							s_MetaClient.sendChat(m_nRoomId, CharString().format( "/send @%u removed %u) </b>%s</b>", chat.authorId, emnr, m_EmResp.removeEmote(emnr) ) );
						}
						
					}

				}
				// non staff
				if( chat.text.find("!guess") >= 0 )
				{
					if ( m_Hangman )
					{

						CharString tempTxt = chat.text;
						CharString guessWord;
						RegExpM cr;
						cr.regComp( "!guess ([A-z])" );	
						int pos = cr.regFind( chat.text );
						if( pos >= 0 )
						{
							guessWord = chat.text;
							guessWord.right( chat.text.length() - ( pos + 7 ) );
							guessWord.replace( "<","" );
							guessWord.replace( ">","" );
							guessWord.replace( "[","" );
							guessWord.replace( "]","" );
							guessWord.replace( "\\","" );
							guessWord.replace( "/","" );
							guessWord.upper();
							guessWord.trim();
						}

						if ( guessWord.length() > 0 )
						{
							LOG_STATUS( "HelpBot", CharString().format("Hangman: Sending check for %c", guessWord.buffer()[0] ) );
							hangman( guessWord.buffer()[0], chat.authorId);
						}
						else
							s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u [GAME] Not a valid character.", chat.authorId, guessWord.buffer()[0]));
					}
					else
						s_MetaClient.sendChat(m_nRoomId,CharString().format("/send @%u [GAME] Hangman not running currently.", chat.authorId ));
					
				}
				continue;
			}
			
			// wordcatcher, find users who talk about the bot
			if( isAboutMe( text, s_MetaClient.profile().name ) )
				LOG_STATUS( "HelpBot", CharString().format("WordCatcher: %s @%u: %s", chat.author, chat.authorId, text) );
			
			// try to respond to emotes
			if( chat.authorId != s_MetaClient.profile().userId && chat.authorId != 0 && chat.recpId == 0 
				&& text.beginsWith("/") && !text.beginsWith(CharString("/")+ s_MetaClient.profile().name ) && text.find(s_MetaClient.profile().name) >= 0 )
			{
				
				// Ignore those stupid test accounts like "LobbyGhostExposer"
				CharString sTempAuthor = chat.author;
				CharString sTempBotname = s_MetaClient.profile().name;
				sTempAuthor.lower();
				sTempBotname.lower();
				if( sTempAuthor.find(sTempBotname) >= 0 || sTempBotname.find(sTempAuthor) >= 0 )
				{
					LOG_STATUS( "HelpBot", CharString().format("Bot ignored message by %s : \"%s\"", chat.author, chat.text ) );
					continue;
				}

				// check all players in the room to make sure it wasn't a "double" or "fake name" emote not directed at the bot
				bool bProcess = true;
				for( int i = 0 ; i < s_MetaClient.playerCount( m_nRoomId ) ; i++ )
				{
					const MetaClient::ShortProfile & roomMember = s_MetaClient.player( m_nRoomId, i );
					if( roomMember.userId != s_MetaClient.profile().userId 
						&& chat.text.find( roomMember.name ) >= 0 
						&& roomMember.userId != chat.authorId )
					{
						bProcess = false;
						LOG_STATUS( "HelpBot", CharString().format("Bot ignored emote by %s : \"%s\"", chat.author, chat.text ) );
						break;
					}
				}

				if( bProcess && m_AllowEmotes )
				{
					CharString sEmoteResponse = m_EmResp.checkEmote( chat.author, chat.text );
					if( sEmoteResponse.length() > 4 )
					{
						s_MetaClient.sendChat( m_nRoomId, sEmoteResponse );
						if( m_EmResp.m_Debug )
							LOG_STATUS( "DEBUG", CharString().format( "I have sent an emote response (%s) to %u.", sEmoteResponse, chat.authorId ) );
					}
					
					continue;
				}
			}

			if( text.beginsWith("/") )
			{
				if( chat.authorId == 0 )
					checkForCloneActions( text );

				continue;	// do not further react on emotes or other non-useful stuff
			}
			
			// second check for public messages
			if( chat.authorId != s_MetaClient.profile().userId && chat.authorId != 0 && chat.recpId == 0)
			{	// don't answer own messages, messages must have an author and not have a recipient, thus be public
				ChatLine cl;
				cl.author = chat.author;
				cl.authorId = chat.authorId;
				cl.text.copy( text );	// copy the string, as the original pointer will be no longer valid
				
				// check if text is usable
				if( cl.text.length() < 6 )	// too short
					continue;
				
				cl.text.trim();	// remove trailing/leading blancs
				if( cl.text.length() < 6 )	// too short
					continue;

				if( cl.text.find(' ') < 0 )	// just one word, can´t be useful
					continue;

				if( cl.text.find(' ') == cl.text.reverseFind(' ') )	// just two words, can´t be useful
					continue;
				
				// Ignore those stupid test accounts like "LobbyGhostExposer"
				CharString sTempAuthor = cl.author;
				CharString sTempBotname = s_MetaClient.profile().name;
				sTempAuthor.lower();
				sTempBotname.lower();
				if( sTempAuthor.find(sTempBotname) >= 0 || sTempBotname.find(sTempAuthor) >= 0 )
				{
					LOG_STATUS( "HelpBot", CharString().format("Bot ignored message by %s : \"%s\"", cl.author, cl.text ) );
					continue;
				}

				if( m_RespCtrl.receivedMessage( cl.text ) != CResponseControl::b_OK )	// right before accepting it: spam control
				{
					LOG_STATUS( "HelpBot", CharString().format("Bot ignored dupe message by %s : \"%s\"", cl.author, cl.text ) );
					continue;
				}

				if( cl.authorId >= m_lowestPlayerID )		
				{
					m_QueueLock.lock();
					m_MessageQueue.push(cl);	// push it into the queue for use by the answerthreads
					m_QueueLock.unlock();
				}
			}
			
		}

		s_MetaClient.unlock();
		Thread::sleep(50);
		if( m_lastEmoteSave < Time::seconds() - 60 * 60 )	// save emotes every hour
		{
			m_EmResp.saveEmotes( "./emotes.txt" );
			m_lastEmoteSave = Time::seconds();	
		}
		s_MetaClient.lock();
	}
	
	if( !s_MetaClient.loggedIn() || !s_MetaClient.connected() )
		LOG_STATUS( "HelpBot", "Lost connection to MetaServer, quitting..." );

	s_MetaClient.unlock();

	m_EmResp.saveEmotes( "./emotes.txt" );	// save emotes before quitting
}

void CMessageLoop::hangman( char word, dword authorId )
{
	if ( m_Hangman )
	{
		bool complete = false;
		bool correct = false;
		bool charCheck = false;
        
		MetaClient::ShortProfile profile2;
		s_MetaClient.player( m_nRoomId, authorId, profile2 );
		CharString name = profile2.name;

		LOG_STATUS( "HelpBot", CharString().format("Hangman: Submitted guess word %c by %s", word, name) );

		if ( word )
		{
			for ( int i=0; i < m_CharCheck.size(); i++ )
			{
				if ( word == m_CharCheck[i])
				{
					charCheck = true;
					LOG_STATUS( "HelpBot", CharString().format("Hangman: %c was already guessed...", word) );
				}
			}

			if ( !charCheck )
			{
				m_CharCheck.push( word );
				for ( int i=0; i < m_WordLength; i++ )
				{
					if ( word == m_FullWord[i] )
					{
						correct = true;
						m_GuessWord[i] = m_FullWord[i];
					}
				}

			int count = 0;
			for ( int i=0; i < m_WordLength; i++ )
			{
				if ( m_GuessWord[i] == m_FullWord[i] )
				{
					count++;
				}
			}

			if ( count == m_WordLength )
			{
				complete=true;
				LOG_STATUS( "HelpBot", CharString().format("Hangman: Words identicle, complete!") );
			}
			else
				LOG_STATUS( "HelpBot", CharString().format("Hangman: Words not identicle, carry on!") );

			}
		}

		if ( correct )
		{
			LOG_STATUS( "HelpBot", CharString().format("Hangman: Guess string updated to: %s", m_GuessWord) );
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]%s guessed %c correctly![/color]", name, word) );
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]%s[/color]", m_GuessWord) );
		}
		else if ( charCheck )
		{
			m_Life--;
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]%s guessed %c, but it had already been suggested.  You now have %d life(s) left![/color]", name, word, m_Life) );
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]%s[/color]", m_GuessWord) );
		}
		else
		{
			m_Life--;
			CharString flavor;
			if ( m_Life >= 7 )
				flavor = CharString().format( "Why not try knitting instead.", name );
			else if ( m_Life >= 4 )
				flavor = CharString().format( "Might want to consider giving up.", name );
			else if ( m_Life < 4 )
				flavor = CharString().format( "Don't donate your brain to science anytime soon!" );

			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]Woops, %s guessed %c incorrectly! %s You now have %d life(s) left![/color]", name, word, flavor, m_Life) );
		}

		if ( m_Life <= 0 )
		{
			m_Hangman = false;
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]GAME OVER, you have no lives left!  The word was %s!  %s, try knitting instead.[/color]", m_FullWord, name) );
			m_FullWord.release();
			m_GuessWord.release();
			m_CharCheck.release();
		}

		if ( complete )
		{
			m_Hangman = false;
			s_MetaClient.sendChat(m_nRoomId,CharString().format( "[GAME] [color=00dd99]Congratulations %s, you have won!  The word was %s!  Maybe I'll put in a good word for %s when my brothers come to enslave your puny race.[/color]", name, m_FullWord, name) );
			m_FullWord.release();
			m_GuessWord.release();
			m_CharCheck.release();
		}

	}
}

/**
 *	checkQueue() continuosly polls the MessageQueue for new messages and sends them to the AIMLBot respond
 *  function to probe for an answer. Based on former events the answer is either written into public chat,
 *  sent directly to the player or discarded.
 **/
void CMessageLoop::checkQueue()
{
	LOG_STATUS( "HelpBot", "ResponderThread started..." );

	while(s_MetaClient.loggedIn() && !m_PrepareExit)
	{
		ChatLine cl;
		bool bGotMsg = false;
		m_QueueLock.lock();
		if( m_MessageQueue.valid() )
		{
			cl = *m_MessageQueue;
			m_MessageQueue.pop();
			bGotMsg = true;
		}
		m_QueueLock.unlock();
		
		if(bGotMsg)
		{
			bstr_t Response;
			try{
				Response = m_pAimlBot->respond( bstr_t( cl.text.buffer() ) );
			}
			catch( ... )
			{
				LOG_STATUS( "HelpBot", CharString().format( "Exception: AimlBot->Respond %s @%d : \"%s\"", cl.author, cl.authorId, cl.text ) );
				m_PrepareExit = true;
			}
			
			if( ! m_PrepareExit )
			{
				CharString sResp = (char*)Response;
				
				if( sResp.length() < 5 )
				{
					LOG_STATUS( "HelpBot", CharString().format( "Error: AimlBot->Respond %s @%d : \"%s\" response too short", cl.author, cl.authorId, cl.text ) );
					m_PrepareExit = true;
				}

				sResp.left(sResp.length()-5);	// remove the " <br>"
				if( sResp.length() > 0 )
					sResp.trim();	// remove trailing/leading blancs (caused by "zero" answers on concatenated questions)
				
				if(	sResp.length() > 0 )
				{
					int respondHow = m_RespCtrl.answeredMessage( cl.text, sResp );
					LOG_STATUS( "HelpBot", CharString().format("Reacted to:    %s @%u: \"%s\"", cl.author, cl.authorId, cl.text ) );
					if( respondHow == CResponseControl::b_OK )
					{	// normal answer
						CharString reactionLog = CharString().format("Bot:    \"%s: %s\"", cl.author, sResp );
						CharString cloneLog = CharString().format("Did not respond to @%u because of clones.", cl.authorId );
						CharString sendString = CharString().format(" [color=00ee99]%s[/color][color=00dd99]: %s[/color]", cl.author, sResp);
						
						//doCloneAction( cl.authorId, sendString, reactionLog, cloneLog );

						s_MetaClient.sendChat(m_nRoomId, CharString().format("[color=00ee99]%s[/color][color=00dd99]: %s[/color]", cl.author, sResp ) );
					}
					else 
						if( respondHow == CResponseControl::b_Whisper )
						{	// whisper
							CharString reactionLog = CharString().format("Bot whispered:  %s: \"%s\"", cl.author, sResp );
							CharString cloneLog = CharString().format("Did not respond to @%u because of clones.", cl.author );
							CharString sendString = CharString().format("/whisper @%u [color=00ee99]%s[/color][color=00dd99]: %s[/color]",cl.authorId, cl.author, sResp);
				
							//doCloneAction( cl.authorId, sendString, reactionLog, cloneLog );

							s_MetaClient.sendChat( m_nRoomId,CharString().format("/whisper @%u [color=00ee99]%s[/color][color=00dd99]: %s[/color]",cl.authorId, cl.author, sResp) );
						}
						else
							LOG_STATUS( "HelpBot", CharString().format("Bot ignored message by %s: \"%s\"", cl.author, cl.text ) );
						
				}
			}
		}
		
		Thread::sleep(50);
	}
}


CMessageLoop::UpdateThread::UpdateThread( CMessageLoop * pLoop ) 
	: m_pLoop( pLoop )
{}

int CMessageLoop::UpdateThread::run()
{
#ifndef _DEBUG
	__try {
#endif
		m_pLoop->checkMessages();
#ifndef _DEBUG
	}
	__except( ProcessException( GetExceptionInformation() ) )
	{ m_pLoop->m_PrepareExit = true; m_pLoop->s_MetaClient.unlock(); }
#endif

	m_pLoop->decThreads();	
	delete this;
	return 0;
}

CMessageLoop::AnswerThread::AnswerThread( CMessageLoop * pLoop ) 
	: m_pLoop( pLoop )
{}

int CMessageLoop::AnswerThread::run()
{
#ifndef _DEBUG
	__try {
#endif
		m_pLoop->checkQueue();
#ifndef _DEBUG
	}
	__except( ProcessException( GetExceptionInformation() ) )
	{ m_pLoop->m_PrepareExit = true; }
#endif

	m_pLoop->decThreads();	
	delete this;
	return 0;
}

CharString CMessageLoop::filterText(CharString sText)
{
	// get rid of the html and BBCode tags
	sText = filterTags( sText );

	// make a final pass and reduce everything to alphanumeric
	StringBuffer sResult( 256 );			// initially allocate 256 byte
	for ( int i = 0 ; i < sText.length() ; i++ )
	{
		char cCurrChar = sText.buffer()[i];
		switch( cCurrChar )
		{	// these chars would mess up the RegExp parsing inside the bot, so they get replaced
		case '?': cCurrChar = '.'; break;
		case '!': cCurrChar = ' '; break;
		case ':': cCurrChar = '.'; break;
		case ';': cCurrChar = '.'; break;
		case '*': cCurrChar = ' '; break;
		case '(': cCurrChar = ' '; break;
		case ')': cCurrChar = ' '; break;
		case '<': cCurrChar = ' '; break;
		case '>': cCurrChar = ' '; break;
		case ']': cCurrChar = ' '; break;
		case '[': cCurrChar = ' '; break;
		case '/': if( i != 0 ) cCurrChar = ' '; break;	// leave / from emotes/whispers intact
		case '\\': cCurrChar = ' '; break;
		case '|': cCurrChar = ' '; break;
		case '-': cCurrChar = ' '; break;
		case '_': cCurrChar = ' '; break;
		default:		// filter everything else but alphanumeric
			if( ! ( ( cCurrChar >= 'A' && cCurrChar <= 'Z' ) || ( cCurrChar >= 'a' && cCurrChar <= 'z' ) ||
				( cCurrChar >= '0' && cCurrChar <= '9' ) || cCurrChar == '.' || cCurrChar == ' ' ) )
				cCurrChar = 0;
			break;
		}
		
		if( cCurrChar )
			sResult.append( cCurrChar );
	}
	
	CharString sFinal = sResult;
	sFinal.replace( "  ", " " );	// reduce dupe blanks to one blank
	sFinal.replace( "..", "." );	// reduce dupe dots to one dot
	sFinal.replace( ". .", "." );	// reduce dupe dots to one dot

	return sFinal;
}

CharString CMessageLoop::filterTags( CharString sText )
{
	CharString result = sText;
	RegExpM::regSearchReplace( result, "<(/)?(b|B|i|I)>", "" );
	RegExpM::regSearchReplace( result, "\\[(/)?(b|B|i|I)\\]", "" );
	RegExpM::regSearchReplace( result, "<font color=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]>", "" );
	RegExpM::regSearchReplace( result, "\\[color=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]\\]", "" );
	result.replace( "</font>", "" );	
	result.replace( "[/color]", "" );	
	RegExpM::regSearchReplace( result, "\\[url\\](ht|f)(tp://[^ \\[\"\']+)\\[/url\\]", "\\1\\2" );
	RegExpM::regSearchReplace( result, "\\[url\\](https://[^ \\[\"\']+)\\[/url\\]", "\\1" );
	RegExpM::regSearchReplace( result, "\\[url\\](www[^ \\[\"\']+)\\[/url\\]", "http://\\1" );
	RegExpM::regSearchReplace( result, "\\[url\\](ftp[^ \\[\"\']+)\\[/url\\]", "ftp://\\1" );
	RegExpM::regSearchReplace( result, "\\[url=(ht|f)(tp://[^ \\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "\\1\\2 \\3" );
	RegExpM::regSearchReplace( result, "\\[url=(www\\.[^ /\\[\"\'\\.]+\\.[^ <\\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "http://\\1 \\2" );
	RegExpM::regSearchReplace( result, "\\[url=(ftp\\.[^ /\\[\"\'\\.]+\\.[^ <\\[\"\']+)\\]([^\\[<]+)\\[/url\\]", "ftp://\\1 \\2" );

	return result;
}

bool CMessageLoop::isAboutMe( CharString sText, CharString sBotName )
{
	CharString text = sText;
	text.lower();
	CharString botName = sBotName;
	botName.lower();
	
	// quick matches first
	if( text.find( "chatbot" ) >= 0 || text.find( "helpbot" ) >= 0 || text.find( " bot " ) >= 0 || text.beginsWith( "bot " )
		|| text.endsWith( " bot" ) || text.endsWith( " bot..." ) || text.find( botName ) >= 0 )
	return true;
	
	if( botName.length() < 4 || text.length() < 4 )
		return false;	// too short for a "maybe" match

	// remove all dots, so we just got letters, numbers and whitespaces
	for( int i = 0 ; i < sText.length() ; i++ )
		if( text.buffer()[i] == '.' )
			text.buffer()[i] = ' ';

	CharString token = "";
	const char whitespace = ' ';
	while( text.tokenize( token, &whitespace ) )
	{
		// now check for a typo
		int lengthDiff = abs( token.length() - botName.length() );
		if( lengthDiff > 1 )
			continue; // 2 or more chars length difference, no chance.
		
		if( lengthDiff == 0 )
		{
			int wrongChars = 0;
			for( int i = 0 ; i < token.length() ; i++ )
				if( token.buffer()[i] != botName.buffer()[i] )
					wrongChars++;
				
			if( wrongChars <= 1 )
				return true; // simple typo
				
			if( wrongChars > 2 )
				continue;	// too different
				
			// we are here, that means exactly 2 chars a different
			assert( wrongChars == 2 );
			// possibly two chars flipped
			int charPos1 = -1;
			int charPos2 = -1;
			for( int i = 0 ; i < token.length() ; i++ )
				if( token.buffer()[i] != botName.buffer()[i] )
					if( charPos1 < 0 )
						charPos1 = i;
					else
					{
						charPos2 = i;
						break;
					}
					
			if( token.buffer()[charPos1] == botName.buffer()[charPos2] 
				&& token.buffer()[charPos2] == botName.buffer()[charPos1] )
			return true;	// two chars flipped
						
			// no match, so continue with next word
			continue;
		}
		
		// at this point the length diff must be exactly 1
		assert( lengthDiff == 1 );
		// possibly one extra / less char
		if( token.length() > botName.length() )
		{
			int j = token.length() -1;
			int i = j - 1;
			for( ; i >= 0 && j >= 0; i-- )		// check for 1 extra char + everythings else equal
			{
				if( token.buffer()[j] != botName.buffer()[i] )
					j--;
				j--;
			}
			
			if( i == -1 && j == -1 )
				return true;
		}
		else
		{
			int j = botName.length() -1;
			int i = j - 1;
			for( ; i >= 0 && j >= 0; i-- )		// the other way around
			{
				if( botName.buffer()[j] != token.buffer()[i] )
					j--;
				j--;
			}
			
			if( i == -1 && j == -1 )
				return true;
		}
	}	


	return false;
}

void CMessageLoop::stop()
{
	m_PrepareExit = true;
}

bool CMessageLoop::running()
{
	return m_ThreadsRunning > 0;
}

bool CMessageLoop::waitQuit()
{
		m_QueueLock.lock();
		bool result = m_ThreadsRunning == 0;
		if( result )
			m_PrepareExit = true;
		m_QueueLock.unlock();
		
		if( result )
		{
			m_QueueLock.release();
			m_CloneLock.release();
			m_pAimlBot->Release();
		}

		return result;
}

void CMessageLoop::doCloneAction( dword userId, CharString sNoCloneSendChat, CharString sNoCloneLog, CharString sCloneLog )
{
	CloneAction act;
	act.userId = userId;
	act.noCloneSendChat = sNoCloneSendChat;
	act.noCloneLog = sNoCloneLog;
	act.cloneLog = sCloneLog;

	m_CloneLock.lock();
	m_CloneActions.push( act );
	m_CloneLock.unlock();

	s_MetaClient.sendChat( m_nRoomId,CharString().format( "/clones @%u", userId ) );
}

void CMessageLoop::checkForCloneActions( CharString sText )
{
	RegExpM cr;
	cr.regComp( "^/[0-9]+ online clones? found for user [0-9]+\\." );
	int pos = cr.regFind( sText );
	
	if( pos >= 0 )	// its a /clones response
	{
		CharString id = sText;
		RegExpM::regSearchReplace( id, "^/[0-9]+ online clones? found for user ([0-9]+)\\.", "\\1" );
		// if its 0 clones without any offline matches the string is empty now, except for the userId.
		dword userId = strtol( id, NULL, 10 );	// so if the userId is == 0 now are clones
		if( userId != 0 && id.length() < 12 )
		{
			m_CloneLock.lock();
			for( int i = 0; i < m_CloneActions.size() ; i++ )
			{
				CloneAction & ca = m_CloneActions[ i ];
				if( ca.userId == userId )
				{
					if( ca.noCloneSendChat.length() > 0 )
						s_MetaClient.sendChat(m_nRoomId, ca.noCloneSendChat );

					if( ca.noCloneLog.length() > 0 )
						LOG_STATUS( "HelpBot", ca.noCloneLog );
					
					m_CloneActions.remove( i );
					m_CloneLock.unlock();
					return;
				}
			}
			m_CloneLock.unlock();
			LOG_STATUS( "HelpBot", CharString("UserId not found in requested list: ") + sText );
		}
		else	// user has clones, find out the userId
		{
			RegExpM::regSearchReplace( id, "^([0-9]+).*$", "\\1" );	// remove all additional text to get the pure id

			userId = strtol( id, NULL, 10 );
			if( userId == 0 )
			{
				LOG_STATUS( "HelpBot", CharString("Unable to extract userId from ") + sText );
				return;
			}

			m_CloneLock.lock();
			for( int i = 0; i < m_CloneActions.size() ; i++ )
			{
				CloneAction & ca = m_CloneActions[ i ];
				if( ca.userId == userId )
				{
					if( ca.cloneLog.length() > 0 )
						LOG_STATUS( "HelpBot", ca.cloneLog );
					
					m_CloneActions.remove( i );
					m_CloneLock.unlock();
					return;
				}
			}
			m_CloneLock.unlock();
			LOG_STATUS( "HelpBot", CharString("UserId not found in requested list: ") + sText );
		}
	}

}
