#if !defined(AFX_MESSAGE_H__6CBD4D73_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_MESSAGE_H__6CBD4D73_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Message.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessage dialog

class CMessage : public CDialog
{
// Construction
public:
	CMessage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessage)
	enum { IDD = IDD_MESSAGE };
	CButton	m_CancelControl;
	CEdit	m_MessageControl;
	CButton	m_SendControl;
	CString	m_To;
	CString	m_Message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMessage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGE_H__6CBD4D73_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
