// ChatLog.h: interface for the ChatLog class.
//
//////////////////////////////////////////////////////////////////////

#include "gcql.h"

#if !defined(AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_)
#define AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define REL_LOGPATH "\\.ChatLogs"

class CChatLog  
{
public:
	CChatLog();
	virtual ~CChatLog();

	CString			getLogFileName();
	CString			getLogPath();
	void			initialize(const TCHAR * pNickName);
	void			write( const TCHAR * pLogText );
	bool			isLogging();

	bool			startLogging();
	bool			startLogging( const TCHAR * pFileName, 
						const TCHAR * pPathAndName );
	void			stopLogging();
	
private:
	CString			closeOpenTags( const TCHAR * pInput );
	bool			isGoodChar( TCHAR cChar );
	CString			filterNickname( const TCHAR * pName );
	BYTE			openLogFile();
	BYTE			openLogFile( const TCHAR * pFileName, 
						const TCHAR * pPathAndName );
	CString			getAppPath();
	void			checkDirectory();
	void			writeSeperator();
	void			writeHTMLheader();
	CString			generateFileName();
	void			finishHTMLfile();

	CString			sCurrFileName;
	CString			sNickName;
	CStdioFile		fileStream;
	bool			bIsActive;
	bool			bNoChangeName;

};

#endif // !defined(AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_)
