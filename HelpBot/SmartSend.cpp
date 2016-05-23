// SmartSend.cpp: implementation of the CSmartSend class.
//
//////////////////////////////////////////////////////////////////////

#include "SmartSend.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSmartSend::CSmartSend( MetaClient * client, CharString newSendPrefix, dword roomId /*= 0*/ ) :
	m_client( client ),
	m_prefix( newSendPrefix ),
	m_concat( newSendPrefix ),
	m_roomId( roomId )
{}

CSmartSend::~CSmartSend()
{}

void CSmartSend::forceSend()
{
	if( m_concat != m_prefix )
	{
		m_client->sendChat( m_roomId, m_concat );
		m_concat = m_prefix;
	}
}

void CSmartSend::send(CharString line)
{
	if( line.length() + m_concat.length() > 500 )
		forceSend();

	m_concat += CharString("\n") + line;
}
