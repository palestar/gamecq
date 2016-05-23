// GCQL.h : main header file for the GCQL application
//

#if !defined(AFX_GCQL_H__5B4DD8E7_720C_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_GCQL_H__5B4DD8E7_720C_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// main symbols
#include "resource.h"       
// GameCQ
#include "GCQ/MetaClient.h"

#include "ChatLog.h"
#include "ChatFrame.h"
#include "WebFrame.h"
#include "ServerFrame.h"
#include "CacheFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CGCQLApp:
// See GCQL.cpp for the implementation of this class
//

class CGCQLApp : public CWinApp //, public CBCGWorkspace
{
public:
	CGCQLApp();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCQLApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CGCQLApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	// Helpers
	static int					selectBestServer( Array< MetaClient::Server > & servers );
	static bool					selectGame( dword gameId, bool a_bLeaveAllRooms );
	static bool					openURL( const char * pURL );

	static bool					doLogin();
	static void					gotoHomeDirectory();
	static CharString			sConfigName;

	// ChatLog
	static CChatLog				s_ChatLog;
	
	// Static
	static CChatFrame *			sm_pChatFrame;
	static CWebFrame *			sm_pWebFrame;
	static CServerFrame *		sm_pServerFrame;
	static CCacheFrame *		sm_pCacheFrame;

	static MetaClient			sm_MetaClient;				// our connection to the meta client
	static MetaClient::Game		sm_Game;						// our game information
	static dword				sm_SessionID;				// our session id

	static dword				sm_LoginTime;
	static BOOL					sm_bBtnDown;
	static POINT				sm_ptMouseDown;				

	static int					sm_ChatBufferSize;
	static COLORREF				sm_ChatColor;
	static int					sm_TextSize;					// size of the text in the chatwindow
	static bool					sm_bEnableWordFilter;
	static bool					sm_bChatSound;
	static bool					sm_bMessageSound;
	static bool					sm_bTaskBarMessages;
	static int					sm_RefreshGameListTime;
	static bool					sm_AlwaysLog;				// always save chatlog
	static dword				sm_AutoAwayTime;				// how long until user is automatically set away
	static bool					sm_RoomAnnounce;
	static dword				sm_Language;					// default language
	static bool					sm_AutoLogin;				// Automatically login
	static bool					sm_bMinimized;
	static bool					sm_bEnableAutoAway;
	static dword				sm_nUpdateCheckInterval;
	static COLORREF				sm_BackgroundColor1;
	static COLORREF				sm_BackgroundColor2;
	static COLORREF				sm_FrameColor;
	static COLORREF				sm_ButtonFrame1;
	static COLORREF				sm_ButtonFrame2;
	static COLORREF				sm_ButtonHighlight;
	static COLORREF				sm_GripperColor1;
	static COLORREF				sm_GripperColor2;
	static COLORREF				sm_TextColor;
	static COLORREF				sm_GrayedTextColor;
	static COLORREF				sm_StatusColor;

private:
	// Data
	CSingleDocTemplate *		m_pDocTemplate;
	HICON						m_hIcon;

	struct MultipleInstances
	{
		bool bAllowStartup;
		HWND hOther;
	};
	
	bool m_bSettingsLoaded;		// Remember if settings were loaded, so they won't be overwritten with default values
public:
	afx_msg void OnGamecqChangepassword();
	afx_msg void OnGamecqChangeusername();
	afx_msg void OnGamecqChangeemail();
	afx_msg void OnGamecqSavechat();
	afx_msg void OnUpdateGamecqSavechat(CCmdUI *pCmdUI);
	afx_msg void OnGamecqSavechatAs();
	afx_msg void OnUpdateGamecqSavechatAs(CCmdUI *pCmdUI);
	afx_msg void OnGamecqStopchatlog();
	afx_msg void OnUpdateGamecqStopchatlog(CCmdUI *pCmdUI);
	afx_msg void OnEditFindusers();
	afx_msg void OnViewOptions();
	afx_msg void OnViewIgnores();
	afx_msg void OnAppExit();
	afx_msg void OnUpdateAdminServers(CCmdUI *pCmdUI);
	afx_msg void OnViewHome();
	afx_msg void OnViewChat();
	afx_msg void OnViewGames();
	afx_msg void OnViewServers();
	afx_msg void OnViewManual();
	afx_msg void OnViewFAQ();
	afx_msg void OnViewSupport();
};

// custom control ID's
const int ID_TAB_CONTROL		= WM_USER + 255;
const int ID_TASKBAR_ICON		= WM_USER + 256;

extern CGCQLApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCQL_H__5B4DD8E7_720C_11D5_BA96_00C0DF22DE85__INCLUDED_)
