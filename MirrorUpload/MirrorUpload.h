// MirrorUpload.h : main header file for the MIRRORUPLOAD application
//

#if !defined(AFX_MIRRORUPLOAD_H__7448A50F_0A92_4AC2_8033_5034F113F9E0__INCLUDED_)
#define AFX_MIRRORUPLOAD_H__7448A50F_0A92_4AC2_8033_5034F113F9E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadApp:
// See MirrorUpload.cpp for the implementation of this class
//

class CMirrorUploadApp : public CWinApp
{
public:
	CMirrorUploadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorUploadApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMirrorUploadApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIRRORUPLOAD_H__7448A50F_0A92_4AC2_8033_5034F113F9E0__INCLUDED_)
