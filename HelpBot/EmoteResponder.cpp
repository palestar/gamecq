// EmoteResponder.cpp: implementation of the CEmoteResponder class.
//
//////////////////////////////////////////////////////////////////////

#include "EmoteResponder.h"
#include "MessageLoop.h"

#include "Standard/Time.h"
#include "Standard/Settings.h"
#include "File/FileDisk.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEmoteResponder::CEmoteResponder()
{
	m_sBotName = "i903,Ü=§)(Mü0w,erfdf0=))),fdöLFDsdif)I=?§;öl";	// who cares...
}

CEmoteResponder::CEmoteResponder( CharString sBotName )
{
	Settings settings( "HelpBot", "./config.ini" );	// we assume the ini to be in the current directory for now <-- TODO

	if( settings.get( "enableEmoteResponse", (dword)0 ) == 0 )
		return;

	m_Debug = false;
	m_tolerance = settings.get( "emoteTolerance", (dword)1 );		
		
	int nWeightCount =	settings.get( "weightCount", (dword)0 );
	for( int i = 0 ; i < nWeightCount ; i++ )
	{
		CharString sLine = settings.get( CharString().format("w%d", i), "" );
		if( sLine.length() > 0 )
		{
			const char whitespace[] = {'@',0};
			CharString token;
			sLine.tokenize( token, whitespace );
			token.trim();
			CharString snippet = token;
			sLine.tokenize( token, whitespace );
			token.trim();
			int weight = strtol( token, NULL, 10 );

			WordWeight ww;
			snippet.lower();
			ww.snippet = snippet;
			ww.weight = weight;

			m_weights.push( ww );
		}
	}

	m_sBotName = sBotName;

	m_spamBlock = CSpamBlock(  settings.get( "respondMinTime", (dword)30 ), settings.get( "sameAuthorMinTime", (dword)60 ), settings.get( "silentTime", (dword)45 ) );

	
	loadEmotes( "./emotes.txt" );
}

CEmoteResponder::~CEmoteResponder()
{
}

CharString CEmoteResponder::checkEmote(CharString sUserName, CharString sText)
{
	if( m_Debug )
		LOG_STATUS( "DEBUG", "Entering checkEmote()" );

	if( sText.find("**") >= 0 )	// may have badwords, so don´t act on it
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote may have badwords, so don´t act on it." );
		return "";
	}

	if( ! sText.beginsWith( CharString("/") + sUserName + " " ) )	// emotes have to begin with the username
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emotes have to begin with the username." );
		return "";
	}

	if( sText.find( m_sBotName ) < 0 )	// is this emote about the bot ?
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote not about the bot." );
		return "";
	}
	
	sText.right( sText.length() - ( sUserName.length() + 2 ) ); // keep the "pure" emote-text

	if( sText.length() - m_sBotName.length() < 10 )	// too short to react on it
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote too short." );
		return "";
	}

	if( sText.find('@') >= 0 )	// bot doesn´t like emotes that include @ for various reasons (i.e. emails)
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote can't include @." );
		return "";
	}

	// url's aren't needed either
	if( sText.find("http://") >= 0 || sText.find("ftp://") >= 0 || sText.find("www.") >= 0 )
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote can't contain URL's." );
		return "";
	}

	// own and username may not be partially equal
	if( sUserName.find(m_sBotName) >= 0 || m_sBotName.find(sUserName) >= 0 )
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote contains name too similar." );
		return "";
	}
	
	// now each occurance of the sendername gets replaced with @senderName and each occurance
	// of the targetname (the bot) with @recvName.
	sText.replace( sUserName, "@senderName" );
	sText.replace( m_sBotName, "@recvName" );

	addEmote( sText );
	

	if( m_spamBlock.isSpam( sUserName, sText ) )
	{
		if( m_Debug )
			LOG_STATUS( "DEBUG", "Emote isSpam." );
		return "";
	}


	if( checkEmotesCorruption() )	// emotes corrupted ?
	{
		return buildEmote( sUserName, tryToRespondTo( sText ) );
	}
	else
		return "";
}

void CEmoteResponder::addEmote(CharString emote)
{
	for( int i = 0 ; i < m_emotes.size() ; i++ )
		if( ((CharString)m_emotes[i]).compareNoCase( emote ) == 0 )
			return;

	m_emotes.push( emote );
	stripEmote( emote );
	m_emotesStripped.push( emote );
	m_emotesWeighted.push( evaluateStripped( emote ) );
}

CharString CEmoteResponder::buildEmote( CharString sTargetName, CharString sEmote )
{
	CharString result = sEmote;
	result.replace( "@recvName", sTargetName );
	result.replace( "@senderName", m_sBotName );
	result = CharString("/me ") + result;
	return result;
}

int CEmoteResponder::evaluate(CharString sEmote)
{
	int result = 0;
	
	sEmote.lower();
	for( int i = 0 ; i < m_weights.size() ; i++ )
		if( emoteContainsWord( sEmote, ((WordWeight)m_weights[i]).snippet ) )
			result += ((WordWeight)m_weights[i]).weight;

	return result;
}

int CEmoteResponder::evaluateStripped(CharString sEmote)
{
	int result = 0;
	
	sEmote.lower();
	for( int i = 0 ; i < m_weights.size() ; i++ )
		if( sEmote.find( ((WordWeight)m_weights[i]).snippet ) >= 0 )
			result += ((WordWeight)m_weights[i]).weight;

	return result;
}

bool CEmoteResponder::canNotEvaluateCorrect(CharString sEmote)
{
	sEmote.lower();
	for( int i = 0 ; i < m_weights.size() ; i++ )
		if( emoteContainsWord( sEmote, ((WordWeight)m_weights[i]).snippet ) )
			return false;

	return true;
}

bool CEmoteResponder::sameEmote(CharString sEmote1, CharString sEmote2)
{
	CharString em1 = CMessageLoop::filterTags( sEmote1 );
	CharString em2 = CMessageLoop::filterTags( sEmote2 );

	em1.replace( "  ", " " );	// reduce dupe blanks to one blank
	em2.replace( "  ", " " );

	if( em1.compareNoCase( em2 ) == 0 )
		return true;
	
	return false;
}

void CEmoteResponder::stripEmote( CharString & emote )
{
	emote = CMessageLoop::filterTags( emote );
	emote.replace( "  ", " " );
}

bool CEmoteResponder::emoteContainsWord(CharString sEmote, CharString sSnippet)
{
	CharString em = CMessageLoop::filterTags( sEmote );
	CharString sn = CMessageLoop::filterTags( sSnippet );

	em.replace( "  ", " " );	// reduce dupe blanks to one blank
	sn.replace( "  ", " " );
	
	em.lower();
	sn.lower();

	if( em.find( sn ) >= 0 )
		return true;
	
	return false;
}

Array<dword> CEmoteResponder::findPossibleAnswers(CharString sEmote, int tolerance)
{
	Array<dword> results;
	CharString sStrippedEmote = sEmote;
	stripEmote( sStrippedEmote );
	int emscore = evaluate( sEmote );

	for( int i = 0 ; i < m_emotes.size() ; i++ )
	{
		if( m_emotesWeighted[i] == 0 )
			continue;	// can't respond with emotes we don't know
		
		if( sStrippedEmote.compareNoCase( m_emotesStripped[i] ) == 0 )	// same emote ?
			continue;	// don't respond with same emote

		if( abs( m_emotesWeighted[i] - emscore ) <= tolerance )	// emote within tolerance ?
			results.push( i );
	}
	if ( m_Debug && results.size() == 0 )
		LOG_STATUS( "DEBUG", "Emote result size zero." );
	else if ( m_Debug && results.size() > 0 )
			LOG_STATUS( "DEBUG", CharString().format( "Emote (result size %d.", results.size() ) );

	return results;
}

CharString CEmoteResponder::tryToRespondTo(CharString sEmote)
{
	if( canNotEvaluateCorrect( sEmote ) )	// don't know how to evaluate this emote, so don´t respond
	{
		if ( m_Debug )
			LOG_STATUS( "DEBUG", CharString().format( "Emote (%s) can't be evaluated.", sEmote ) );
		return "";
	}
	
	Array<dword> matches = findPossibleAnswers( sEmote, m_tolerance );
	if( matches.size() == 0 )
		return "";

	if( matches.size() == 1 )
		return m_emotes[ matches[0] ];

	return m_emotes[ matches[ Time::seconds() % matches.size() ] ];
}

void CEmoteResponder::saveEmotes( CharString emFile )
{
	if( checkEmotesCorruption() )	// only save if not corrupted
	{
		try {
			FileDisk::Ref pFile = new FileDisk( emFile , FileDisk::WRITE );	// TODO: still hardcoded...
			
			OutStream output( pFile );
			for( int i = 0 ; i < m_emotes.size() ; i++ )
				output << CharString( m_emotes[i] + "\n" ).buffer();
			pFile->close();
		}
		catch( File::FileError )
		{}
	}
	else
		loadEmotes( emFile );	// if corrupted then load a fresh copy
}

void CEmoteResponder::loadEmotes( CharString emfile )
{
	m_emotes.release();
	m_emotesStripped.release();
	m_emotesWeighted.release();

	char * pEM = FileDisk::loadTextFile( emfile );
	if ( pEM != NULL )
	{
		CharString sEM = pEM;
		CharString token;
		const char whitespace[] = {'\n',0};	// whitespace char needs to be 0-terminated !

		sEM.tokenize( token, whitespace );
		while( token.length() != 0 )
		{
			addEmote( token );
			if( sEM.length() == 0 )
				break;
			sEM.tokenize( token, whitespace );
		}

		FileDisk::unloadTextFile( pEM );
	}
}



int CEmoteResponder::getEmoteCount()
{
	return m_emotes.size();
}

CharString CEmoteResponder::getEmote( int n )
{
	if( n > m_emotes.size() - 1 || n < 0 )
		return "Error";

	return m_emotes[n];
}

CharString CEmoteResponder::getEmoteWithRating( int n )
{
	if( n > m_emotes.size() - 1 || n < 0 )
		return "Error";

	return CharString().format("%d : ", evaluate( m_emotes[n] ) ) + m_emotes[n];
}

CharString CEmoteResponder::removeEmote( int n )
{
	if( n > m_emotes.size() - 1 || n < 0 )
		return "Error";
	
	CharString removedEmote = m_emotes[n];
	m_emotes.remove( n );
	m_emotesWeighted.remove( n );
	m_emotesStripped.remove( n );
	return removedEmote;
}

bool CEmoteResponder::debug()
{
	m_Debug = !m_Debug;
	return true;
}

bool CEmoteResponder::isEmoteNonRated( int n )
{
	if( n > m_emotes.size() - 1 || n < 0 )
		return true;
	
	return canNotEvaluateCorrect( m_emotes[n] );
}

bool CEmoteResponder::checkEmotesCorruption()
{
	for( int i = 0 ; i < m_emotes.size() ; i++ )
	{
		if( ((CharString)m_emotes[i]).find("@recvName") < 0 || m_emotes[i].length() <= 0 )
		{
			if ( m_Debug )
				LOG_STATUS( "DEBUG", CharString().format( "Emote %d corrupt.", i ) );
			return false;
		}
	}
	
	if( m_emotes.size() != m_emotesStripped.size() || m_emotes.size() != m_emotesWeighted.size() )
	{
		if ( m_Debug )
			LOG_STATUS( "DEBUG", "Emote size wrong." );
		return false;
	}

	if ( m_Debug )
		LOG_STATUS( "DEBUG", "Checking complete. No corruption found." );

	return true;
}




///////////////////////////////////////////////////////////////////
/////////////////// Private Class CSpamBlock //////////////////////
///////////////////////////////////////////////////////////////////




CEmoteResponder::CSpamBlock::CSpamBlock()
{
	m_respondMinTime = 30;
	m_sameAuthorMinTime = 60;
	m_silentTime = 45;

	m_keepQuiet = 0;
	m_lastResponse = 0;
	m_lastAuthor = "";
}

CEmoteResponder::CSpamBlock::CSpamBlock( int respondMinTime, int sameAuthorMinTime, int silentTime )
{
	m_respondMinTime = respondMinTime;
	m_sameAuthorMinTime = sameAuthorMinTime;
	m_silentTime = silentTime;

	m_keepQuiet = 0;
	m_lastResponse = 0;
	m_lastAuthor = "";
}

CEmoteResponder::CSpamBlock::~CSpamBlock()
{
}

bool CEmoteResponder::CSpamBlock::isSpam( CharString author, CharString text )
{
	bool result = false;
	dword currTime = Time::seconds();
	assert( currTime >= m_lastResponse );

	if( m_keepQuiet > currTime )
		result = true;

	if( ( currTime - m_lastResponse ) < m_respondMinTime )
		result = true;		
		
	if( ( currTime - m_lastResponse ) < m_sameAuthorMinTime && author == m_lastAuthor )
		result = true;

	m_lastAuthor = author;
	m_lastResponse = currTime;
	if( result )
	{
		m_keepQuiet = m_keepQuiet <= currTime ? currTime : m_keepQuiet;
		m_keepQuiet += m_silentTime;
	}
	
	if( m_keepQuiet > ( m_lastResponse + 5 * 60) )	// don´t add up more than 5 minutes silence time
		m_keepQuiet = m_lastResponse + 5 * 60;

	return result;
}
