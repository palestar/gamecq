//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#include "stdafx.h"
#include <process.h>    
#include <afxmt.h>

#include "bcgsound.h"
#include "BCGPopupMenu.h"

static int g_nBCGSoundState = BCGSOUND_NOT_STARTED;
static HANDLE g_hThreadSound = NULL;
static const int nThreadDelay = 5;

void BCGSoundThreadProc (LPVOID)
{
	const DWORD dwFlags = (SND_SYNC | SND_NODEFAULT | SND_ALIAS | SND_NOWAIT);

	while (g_nBCGSoundState != BCGSOUND_TERMINATE)
	{
		switch (g_nBCGSoundState)
		{
		case BCGSOUND_MENU_COMMAND:
			::PlaySound (_T("MenuCommand"), NULL, dwFlags);
			g_nBCGSoundState = BCGSOUND_IDLE;
			break;
			
		case BCGSOUND_MENU_POPUP:
			::PlaySound (_T("MenuPopup"), NULL, dwFlags);
			g_nBCGSoundState = BCGSOUND_IDLE;
		}

		::Sleep (nThreadDelay);
	}
	
	::PlaySound (NULL, NULL, SND_PURGE);

	_endthread();
}
//*****************************************************************************************
void BCGPlaySystemSound (int nSound)
{
	if (!CBCGPopupMenu::IsMenuSound ())
	{
		return;
	}

	if (g_nBCGSoundState == BCGSOUND_NOT_STARTED)
	{
		if (nSound == BCGSOUND_TERMINATE)
		{
			return;
		}
		
		static CCriticalSection cs;
		cs.Lock ();
		
		ASSERT (g_hThreadSound == NULL);
		
		//-------------------------
		// Initialize sound thread:
		//-------------------------
		g_hThreadSound = (HANDLE) ::_beginthread (BCGSoundThreadProc, 0, NULL);
		if (g_hThreadSound > 0 && g_hThreadSound != (HANDLE) -1)
		{
			::SetThreadPriority (g_hThreadSound, THREAD_PRIORITY_BELOW_NORMAL);
			g_nBCGSoundState = nSound;
		}
		else
		{
			g_hThreadSound = NULL;
		}
		
		cs.Unlock ();
	}
	else
	{
		g_nBCGSoundState = nSound;
		if (g_nBCGSoundState == BCGSOUND_TERMINATE)
		{
			//------------------------
			// Terminate sound thread:
			//------------------------
			g_hThreadSound = NULL;
		}
	}
}
