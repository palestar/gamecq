// EmoteResponder.h: interface for the CEmoteResponder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMOTERESPONDER_H__CFCE0CC0_1DC1_11D7_9411_00001CDB2E9A__INCLUDED_)
#define AFX_EMOTERESPONDER_H__CFCE0CC0_1DC1_11D7_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Standard/CharString.h"
#include "Standard/Array.h"



class CEmoteResponder  
{
public:
	void saveEmotes( CharString emFile );
	void loadEmotes( CharString emfile );			// load learned emotes from file
	CharString checkEmote( CharString sUserName, CharString sText );
	CEmoteResponder();
	CEmoteResponder( CharString sBotName );
	virtual ~CEmoteResponder();
	
	// online administrating
	int getEmoteCount();
	CharString getEmote( int n );
	CharString getEmoteWithRating( int n );
	CharString removeEmote( int n );
	bool m_Debug;
	bool debug();
	bool isEmoteNonRated( int n );
	bool checkEmotesCorruption();		// false if the emotes somehow got corrupted

private:
	CharString tryToRespondTo( CharString sEmote );		// reduces results from findPossibleAnswers() to 1
	Array<dword> findPossibleAnswers( CharString sEmote, int tolerance );	// try to find a remote with a score within the tolerance
	int evaluate( CharString sEmote );				// return a score for the emote, < 0 means bad, 0: neutral, > 0: nice
	int evaluateStripped( CharString sEmote );		// same as evaluate(), but assuming emote and weights are already stripped
	bool canNotEvaluateCorrect( CharString sEmote );// is there insufficient information to get a correct (thus usable) evaluation result ?
	void addEmote( CharString emote );				// add an emote if its not already known
	CharString buildEmote( CharString sTargetName, CharString sEmote );	// Translate an emote from the saved-format to a sendable one
	bool sameEmote(CharString sEmote1, CharString sEmote2);	// are the text equal (after stripping tags, blanks etc)
	bool emoteContainsWord(CharString sEmote, CharString sSnippet);
	void stripEmote( CharString & emote );

class CSpamBlock
{
	public:
		CSpamBlock();
		CSpamBlock( int respondMinTime, int sameAuthorMinTime, int silentTime );
		bool isSpam( CharString author, CharString text );
		virtual ~CSpamBlock();

	private:
		dword m_respondMinTime;		// minium time between two responses
		dword m_sameAuthorMinTime;	// minium time between two responses to the same author
		int m_silentTime;			// how much time to add up for each limit violation

		dword m_keepQuiet;			// keep quiet until currenttime is larger than this number
		dword m_lastResponse;			// time of last response
		CharString m_lastAuthor;		// last player who used an emote on the bot
	};

	struct WordWeight
	{
		CharString snippet;
		int weight;
	};

	CharString m_sBotName;
	int m_tolerance;				// tolerance, the smaller the more accurate responses will be, however it also makes responses less likely
	Array<CharString> m_emotes;			// learned emotes
	Array<int>  m_emotesWeighted;	// scores for each emote
	Array<CharString> m_emotesStripped;	// filterText( m_emotes )
	Array<WordWeight> m_weights;	// word weights to evaluate emotes
	CSpamBlock m_spamBlock;

};

#endif // !defined(AFX_EMOTERESPONDER_H__CFCE0CC0_1DC1_11D7_9411_00001CDB2E9A__INCLUDED_)
