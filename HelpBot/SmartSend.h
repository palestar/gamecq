// SmartSend.h: interface for the CSmartSend class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMARTSEND_H__DAF64D40_4348_11D7_9411_00001CDB2E9A__INCLUDED_)
#define AFX_SMARTSEND_H__DAF64D40_4348_11D7_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GCQ/MetaClient.h"
#include "Standard/CharString.h"


class CSmartSend  
{
public:
	CSmartSend( MetaClient * client, CharString newSendPrefix, dword roomId = 0 );
	virtual ~CSmartSend();

	void send( CharString line );
	void forceSend();
private:
	MetaClient *	m_client;
	CharString		m_prefix;
	CharString		m_concat;
	dword			m_roomId;
};

#endif // !defined(AFX_SMARTSEND_H__DAF64D40_4348_11D7_9411_00001CDB2E9A__INCLUDED_)
