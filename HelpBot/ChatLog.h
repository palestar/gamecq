// ChatLog.h: interface for the CChatLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATLOG_H__D8C1AF40_D7B6_11D6_9411_00001CDB2E9A__INCLUDED_)
#define AFX_CHATLOG_H__D8C1AF40_D7B6_11D6_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Standard\String.h"



class CChatLog  
{
public:
	static void write( String text );
	CChatLog();
	virtual ~CChatLog();

};

#endif // !defined(AFX_CHATLOG_H__D8C1AF40_D7B6_11D6_9411_00001CDB2E9A__INCLUDED_)
