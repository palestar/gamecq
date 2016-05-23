#if !defined(AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatEdit.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CChatEdit window

class CChatEdit : public CEdit
{
// Construction
public:
	CChatEdit();

// Attributes
public:

// Operations
public:
	void		sendChat();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChatEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChatEdit)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_)
