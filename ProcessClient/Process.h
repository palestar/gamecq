#if !defined(AFX_PROCESS_H__9DD989FA_A78E_445A_8999_67061EB12B08__INCLUDED_)
#define AFX_PROCESS_H__9DD989FA_A78E_445A_8999_67061EB12B08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Process.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CProcess dialog

class CProcess : public CDialog
{
// Construction
public:
	CProcess(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcess)
	enum { IDD = IDD_PROCESS };
	CString	m_Name;
	BOOL	m_bRunOnce;
	BOOL	m_Disabled;
	CString	m_Exec;
	CString	m_Arguments;
	CString	m_Config;
	CString	m_Log;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcess)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcess)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESS_H__9DD989FA_A78E_445A_8999_67061EB12B08__INCLUDED_)
