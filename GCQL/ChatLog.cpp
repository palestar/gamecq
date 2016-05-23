// CChatLog.cpp: implementation of the CChatLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChatLog.h"
#include "Standard/String.h"
#include "Standard/RegExpM.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatLog::CChatLog()
{
	bIsActive = false;
	bNoChangeName = false;
}


CChatLog::~CChatLog()
{
	stopLogging();
}


bool CChatLog::startLogging()
{
	if ( bIsActive )							// ignore if already logging
		return false;
	
	assert( sNickName != _T("") );					// nickname must not be empty

	checkDirectory();							// make sure logfile directory exists
	
	switch( openLogFile() )						// check result
	{
		case 0:	return false;					// error opening the file
		case 1: writeSeperator(); break;		// logfile already existed, write seperator
		case 2: writeHTMLheader(); break;		// new file, write HTML header
	}
		
	// we are here, thus everything worked fine
	bIsActive = true;							// so mark as logging
	return true;	
}

bool CChatLog::startLogging( const TCHAR * sFileName, const TCHAR * sPathAndName )
{
	if ( bIsActive )							// ignore if already logging
		return false;
	
	bNoChangeName = true;						// don't autochange filename
	switch( openLogFile( sFileName, sPathAndName ) )			// check result
	{
		case 0:	bNoChangeName = false; return false;// error opening the file
		case 1: writeSeperator(); break;			// logfile already existed, write seperator
		case 2: writeHTMLheader(); break;			// new file, write HTML header
	}
		
	// we are here, thus everything worked fine
	bIsActive = true;							// so mark as logging
	return true;	
}


void CChatLog::stopLogging()
{
	if ( !bIsActive )							// ignore if not logging
		return;
	
	bIsActive = false;
	bNoChangeName = false;
	fileStream.Close();
}


void CChatLog::finishHTMLfile()
{
	if ( !bIsActive )							// ignore if not logging
		return;
	
	fileStream.WriteString( _T("<br>\n</body></html>") );
}


CString CChatLog::generateFileName()
{
	CString sFileName;
	sFileName.Format( _T("GCQL_%s_%s.html"), sNickName, CTime::GetCurrentTime().Format(_T("%Y.%m.%d")) );

	return sFileName;
}


bool CChatLog::isLogging()
{
	return bIsActive;
}


void CChatLog::writeHTMLheader()
{
	if ( !fileStream )
		return;
	
	CTime tCurrTime = CTime::GetCurrentTime();

	write(_T("<HTML>\n  <HEAD>\n   <TITLE>GameCQ Chatlog by ")+ sNickName 
			+ tCurrTime.Format(_T(" %m/%d/%Y")) + _T("</TITLE>\n  </HEAD>\n")
			+ _T("   <BODY bgcolor=\"#000000\" text=\"#FFFFFF\">") );

}


void CChatLog::writeSeperator()
{
	if ( !fileStream )
		return;

	CTime tCurrTime = CTime::GetCurrentTime();

	write(_T("<hr><h3><center><font color=\"#FFFFFF\">Logging resumed at ") 
			+ tCurrTime.Format(_T("%H:%M:%S ")) + _T("</font></center></h3><hr>"));
}


void CChatLog::write(const TCHAR * sLogText)
{
	if ( !bNoChangeName && generateFileName().Compare( sCurrFileName ) != 0 )	
	{													// passed the 24h barrier ?
		finishHTMLfile();								// so close the old file
		stopLogging();
		startLogging();									// and create a new one for the new date
	}

	fileStream.WriteString( closeOpenTags( sLogText ) + _T("<br>\n") );
	fileStream.Flush();	// this is needed to prevent buffering. Without this line up to a half hour of chat was buffered
}


void CChatLog::checkDirectory()
{
	HANDLE			fFile;						// File Handle
	WIN32_FIND_DATA	fileinfo;					// File Information Structure
	CString			logFilePath = getLogPath();
	fFile = FindFirstFile( logFilePath ,&fileinfo);

	// if the file exists and it is a directory
	if( fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
		FindClose( fFile );						//  Directory Exists close file
	else
		if( CreateDirectory( logFilePath ,NULL ) )
			SetFileAttributes( logFilePath ,FILE_ATTRIBUTE_NORMAL );
		
}


CString CChatLog::getAppPath()
{
	TCHAR szBuffer[_MAX_PATH]; 

	VERIFY(::GetModuleFileName( AfxGetInstanceHandle(), szBuffer, _MAX_PATH) );

	CString sPath = (CString)szBuffer;
	sPath = sPath.Left( sPath.ReverseFind('\\') );

	return sPath;
}


BYTE CChatLog::openLogFile()
{
	sCurrFileName = generateFileName();
	CString	logFilePath = getLogPath() + "\\" + sCurrFileName;
	return openLogFile( sCurrFileName, logFilePath );
}

BYTE CChatLog::openLogFile( const TCHAR * sFileName, const TCHAR * sPathAndName )
{
	BYTE			result = 2;
	HANDLE			fFile;					// File Handle
	WIN32_FIND_DATA	fileinfo;				// File Information Structure
	
	sCurrFileName = sFileName;
	CString			logFilePath = sPathAndName;
	fFile = FindFirstFile( logFilePath ,&fileinfo);

	if ( fFile != INVALID_HANDLE_VALUE )
	{
		FindClose( fFile );					//  File exists
		result = 1;
	}
	
	if( ! fileStream.Open( logFilePath , CFile::modeNoTruncate | CFile::modeCreate
						| CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText ) )
		return 0;							// failed

	fileStream.SeekToEnd();
	return result;
}

void CChatLog::initialize(const TCHAR * nickName)
{
	sNickName = filterNickname( nickName );
}

CString CChatLog::getLogFileName()
{
	return sCurrFileName;
}

CString CChatLog::getLogPath()
{
	return getAppPath() + REL_LOGPATH;
}

CString CChatLog::filterNickname(const TCHAR * pName)
{
	CString sName = pName;
	for ( int i = 0 ; i < sName.GetLength() ; i++ )
		if( !isGoodChar( sName.GetAt( i ) ) )
			sName.SetAt( i, '_' );

	return sName;
}

bool CChatLog::isGoodChar(TCHAR cChar)
{
	// ISO 9660 Filename specification:
	if( cChar >= '0' && cChar <= '9' )
		return true;
	if( cChar >= 'A' && cChar <= 'Z' )
		return true;
	if( cChar >= 'a' && cChar <= 'z' )
		return true;
	if( cChar == '_' )
		return true;
	
	// Windows FAT extended filenames
	if(    cChar == '$' || cChar == '%' || cChar == 0x27 || cChar == '`' || cChar == '-' 
		|| cChar == '@' || cChar == '^' || cChar == '!'  || cChar == '&' || cChar == '['
		|| cChar == ']' || cChar == '(' || cChar == ')'  || cChar == '#' || cChar == '~' 
	  )
		return true;

	return false;
}

CString CChatLog::closeOpenTags(const TCHAR * pInput)
{
	CString sInput = pInput;
	// count open tags
	WideString sTemp = pInput;
	sTemp.lower();

	int nBoldOpen = RegExpM::regSearchReplace( sTemp, STR("<b>"), STR(""));
	nBoldOpen -= RegExpM::regSearchReplace( sTemp, STR("</b>"), STR(""));
	int nItalicsOpen = RegExpM::regSearchReplace( sTemp, STR("<i>"), STR(""));
	nItalicsOpen -= RegExpM::regSearchReplace( sTemp, STR("</i>"), STR(""));
	int nFontOpen = RegExpM::regSearchReplace( sTemp, STR("<font color="), STR(""));
	nFontOpen -= RegExpM::regSearchReplace( sTemp, STR("</font>"), STR(""));

	// and close them (if any)
	for ( int i = 0 ; i < nBoldOpen ; i++ )
		sInput += _T("</b>");
	for ( int i = 0 ; i < nItalicsOpen ; i++ )
		sInput += _T("</i>");
	for ( int i = 0 ; i < nFontOpen ; i++ )
		sInput += _T("</font>");

	return sInput;
}
