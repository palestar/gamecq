#if !defined(AFX_EDITEMOTION_H__6CBD4D77_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_EDITEMOTION_H__6CBD4D77_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditEmotion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditEmotion dialog

class CEditEmotion : public CDialog
{
// Construction
public:
	CEditEmotion(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditEmotion)
	enum { IDD = IDD_EMOTION_EDIT };
	CString	m_Name;
	CString	m_Text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditEmotion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditEmotion)
	virtual BOOL OnInitDialog();
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnMaxtextEdit1();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITEMOTION_H__6CBD4D77_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
