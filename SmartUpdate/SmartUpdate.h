// SmartUpdate.h : main header file for the SMARTUPDATE application
//

#if !defined(AFX_SMARTUPDATE_H__EF983D06_A47F_4FEF_A657_F177D4EF7282__INCLUDED_)
#define AFX_SMARTUPDATE_H__EF983D06_A47F_4FEF_A657_F177D4EF7282__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSmartUpdateApp:
// See SmartUpdate.cpp for the implementation of this class
//

class CSmartUpdateApp : public CWinApp
{
public:
	CSmartUpdateApp();

	bool			m_AskBeforeKill;
	bool			m_AskBeforeReboot;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmartUpdateApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSmartUpdateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMARTUPDATE_H__EF983D06_A47F_4FEF_A657_F177D4EF7282__INCLUDED_)
