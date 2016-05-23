#if !defined(AFX_PROCESSSERVER_H__A6D48E18_96F4_461C_B93A_C631B0F4BEB8__INCLUDED_)
#define AFX_PROCESSSERVER_H__A6D48E18_96F4_461C_B93A_C631B0F4BEB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessServer.h : header file
//

#include "GCQ/ProcessClient.h"
#include "Resource.h"


/////////////////////////////////////////////////////////////////////////////
// CProcessServer dialog

class CProcessServer : public CDialog
{
// Construction
public:
	CProcessServer( CWnd* pParent = NULL);   // standard constructor

	HICON			m_hIcon;
	CharString		m_sName;
	ProcessClient	m_Client;

	Array< ProcessClient::Process >
					m_Processes;			// cached process information
	bool			m_bUpdating;

	dword			getProcessId();			// get the selected process id
	ProcessClient::Process 
					getProcess();			// get the selected process

// Dialog Data
	//{{AFX_DATA(CProcessServer)
	enum { IDD = IDD_PROCESS_SERVER };
	CProgressCtrl	m_cMemory;
	CProgressCtrl	m_cCPU;
	CButton	m_TerminateProcess;
	CButton	m_StopProcess;
	CButton	m_RestartProcess;
	CButton	m_StartProcess;
	CButton	m_ProcessLog;
	CButton	m_ConfigureProcess;
	CButton	m_DeleteProcess;
	CButton	m_EditProcess;
	CListCtrl	m_ProcessList;
	CString	m_sCPU;
	CString	m_sMemory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnRestart();
	afx_msg void OnConfigure();
	afx_msg void OnLog();
	afx_msg void OnAddProcess();
	afx_msg void OnEditProcess();
	afx_msg void OnDeleteProcess();
	afx_msg void OnConfigureProcess();
	afx_msg void OnProcessLog();
	afx_msg void OnStartProcess();
	afx_msg void OnRestartProcess();
	afx_msg void OnStopProcess();
	afx_msg void OnStartAll();
	afx_msg void OnRestartAll();
	afx_msg void OnStopAll();
	afx_msg void OnUpdate();
	afx_msg void OnTerminateProcess();
	afx_msg void OnSelectProcess(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectProcess2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessLog2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSearchLogs();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSSERVER_H__A6D48E18_96F4_461C_B93A_C631B0F4BEB8__INCLUDED_)
