#if !defined(AFX_PROCESSSEARCHLOGS_H__EC5DAEC0_57A2_11D7_9411_00001CDB2E9A__INCLUDED_)
#define AFX_PROCESSSEARCHLOGS_H__EC5DAEC0_57A2_11D7_9411_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessSearchLogs.h : header file
//

#include "ProcessServer.h"

/////////////////////////////////////////////////////////////////////////////
// CProcessSearchLogs dialog

class CProcessSearchLogs : public CDialog
{
// Construction
public:
	CProcessSearchLogs( ProcessClient * pClient, CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CProcessSearchLogs)
	enum { IDD = IDD_PROCESSSEARCHLOGS };
	CComboBox	m_ResultSettings;
	CString	m_FileMask;
	CString	m_SearchString;
	BOOL	m_UseRegExp;
	CString	m_ResultText;
	BOOL	m_ResolveClientId;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessSearchLogs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CProcessSearchLogs)
	afx_msg void OnSearch();
	afx_msg void OnSave();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	ProcessClient *	m_pClient;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSSEARCHLOGS_H__EC5DAEC0_57A2_11D7_9411_00001CDB2E9A__INCLUDED_)
