#if !defined(AFX_PROCESSLOG_H__76D0BCF1_42F5_45AD_9190_AC9CE906CDB4__INCLUDED_)
#define AFX_PROCESSLOG_H__76D0BCF1_42F5_45AD_9190_AC9CE906CDB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessLog.h : header file
//

#include "ProcessServer.h"
#include "File/FileDisk.h"



/////////////////////////////////////////////////////////////////////////////
// CProcessLog dialog

class CProcessLog : public CDialog
{
// Construction
public:
	CProcessLog( ProcessClient * pClient, dword processId, CWnd* pParent = NULL);   // standard constructor
	~CProcessLog();

	ProcessClient *	m_pClient;
	dword			m_ProcessId;
	dword			m_LogId;

	bool			m_Paused;
	bool			m_Saving;
	FileDisk		m_SaveFile;

// Dialog Data
	//{{AFX_DATA(CProcessLog)
	enum { IDD = IDD_PROCESS_LOG };
	CButton	m_CloseButton;
	CButton	m_SaveButton;
	CButton	m_ContinueButton;
	CButton	m_PauseButton;
	CEdit	m_TextControl;
	CString	m_Text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessLog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessLog)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnCloseFile();
	afx_msg void OnSaveFile();
	afx_msg void OnContinue();
	afx_msg void OnPause();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSLOG_H__76D0BCF1_42F5_45AD_9190_AC9CE906CDB4__INCLUDED_)
