// ResponseControl.h: interface for the CResponseControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESPONSECONTROL_H__303096C0_D30B_11D6_9411_00001CDB2E9A__INCLUDED_)
#define AFX_RESPONSECONTROL_H__303096C0_D30B_11D6_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Standard\String.h"



class CResponseControl  
{
public:
	CResponseControl();
	virtual ~CResponseControl();

	enum Result{
		b_OK,
		b_Whisper,
		b_Ignore
	};


	Result	receivedMessage( String msg );					// to be called before the filtered message is put into the queue
	Result	answeredMessage( String msg, String answer );	// to be called after the bot responded to it

private:
	struct RespCtrl
	{
		String	sLastRecv;	// last received message
		dword	tLastRecv;	// receive time
		String	sLastResp;	// last message the bot responded to
		String	sLastAnswer;// last answer the bot gave
		dword	tLastResp;	// time of last response
		dword	tPrevResp;	// time of previous response
		dword	tKeepSilent;	// if set the bot keeps whispering responses until time expires
	};
	
	RespCtrl	m_RespCtrl;
};

#endif // !defined(AFX_RESPONSECONTROL_H__303096C0_D30B_11D6_9411_00001CDB2E9A__INCLUDED_)
