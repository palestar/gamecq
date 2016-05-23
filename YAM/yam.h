// yam.h : main header file for the yam application
//

#if !defined(AFX_yam_H__435A1759_127F_11D4_9830_00105AA5E657__INCLUDED_)
#define AFX_yam_H__435A1759_127F_11D4_9830_00105AA5E657__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "GCQ/MetaClient.h"

/////////////////////////////////////////////////////////////////////////////
// CYamApp:
// See yam.cpp for the implementation of this class
//

class CYamApp : public CWinApp
{
public:
	static bool openCon( bool bWarnings );
	CYamApp();

	static bool					doLogin();
	static void					gotoHomeDirectory();
	static MetaClient::Game		s_Game;						// our game information
	static dword				s_SessionID;				// our session id

	// Static
	static MetaClient			s_MetaClient;				// our connection to the meta client

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYamApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CYamApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_yam_H__435A1759_127F_11D4_9830_00105AA5E657__INCLUDED_)
