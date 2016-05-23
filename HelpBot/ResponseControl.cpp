// ResponseControl.cpp: implementation of the CResponseControl class.
//
//////////////////////////////////////////////////////////////////////

#include "ResponseControl.h"
#include "Standard\Time.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResponseControl::CResponseControl()
{
	m_RespCtrl.tKeepSilent = 0;
	m_RespCtrl.tLastRecv = 0;
	m_RespCtrl.tLastResp = 0;
	m_RespCtrl.tPrevResp = 0;
}

CResponseControl::~CResponseControl()
{

}

CResponseControl::Result CResponseControl::receivedMessage( String msg )
{
	Result result = b_OK;
	dword currTime = Time::seconds();

	if( msg.compareNoCase( m_RespCtrl.sLastRecv ) == 0 )	// is this message different from the previous one ?
		if( ( currTime - m_RespCtrl.tLastRecv ) <= 15 )		// same message, and received within 15 seconds ?
			result = b_Ignore;	// Ignore it, its probably spam. If not, the others can tell em the old answer (if any)

	m_RespCtrl.sLastRecv = msg;			// remember this message
	m_RespCtrl.tLastRecv = currTime;	// and time

	return result;
}

CResponseControl::Result CResponseControl::answeredMessage( String msg, String answer )
{
	Result result = b_OK;
	dword currTime = Time::seconds();

	if( currTime < m_RespCtrl.tKeepSilent )			// Keep silent ?
		result = b_Whisper;		// Ok, whisper the answer, probably change to ignore later
	
	if( msg.compareNoCase( m_RespCtrl.sLastResp ) == 0 || answer.compareNoCase( m_RespCtrl.sLastAnswer ) == 0 )	// is this message or answer different from the previous one ?
		if( ( currTime - m_RespCtrl.tLastResp ) <= 45 )		// same message, and anwered within 45 seconds ?
			result = b_Ignore;	// Ignore it, its probably spam. If not, the others can tell em the old answer.

	if( ( currTime - m_RespCtrl.tPrevResp ) <= 60 )	// 3 Answers within 60 seconds ?
		m_RespCtrl.tKeepSilent = currTime + 60;		// keep silent for one minutes after this


	m_RespCtrl.tPrevResp = m_RespCtrl.tLastResp;
	m_RespCtrl.tLastResp = currTime;
	m_RespCtrl.sLastResp = msg;
	m_RespCtrl.sLastAnswer = answer;

	return result;
}
