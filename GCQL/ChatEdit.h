#if !defined(AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatEdit.h : header file
//

#include "SpamBlock.h"

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
	void		sendChat( const TCHAR * pSendText, bool addHistory );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void setLastUserId( dword userId );
	void setChatLine( const TCHAR * pNewText );
	bool acceptsInput();
	void enableInput();
	virtual ~CChatEdit();

	CSpamBlock					m_SpamBlock;

	// Generated message map functions
protected:
	//{{AFX_MSG(CChatEdit)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CList< CString, CString >	m_ChatHistory;
	POSITION					m_ChatPosition;
	dword						m_LastUserId;
	bool						m_AcceptInput;

};

/////////////////////////////////////////////////////////////////////////////

class CChatEditButton : public CBCGToolbarEditBoxButton 
{
	DECLARE_SERIAL(CChatEditButton)

// Construction
public:
	CChatEditButton() :
		CBCGToolbarEditBoxButton (ID_CHAT_EDIT, 
			CImageHash::GetImageOfCommand(ID_CHAT_EDIT, FALSE), ES_AUTOHSCROLL, 600 )
	{}

// Overrides
protected:
	virtual CEdit *		CreateEdit( CWnd* pWndParent, const CRect& rect);
	virtual BOOL		NotifyCommand (int iNotifyCode);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATEDIT_H__2379B381_74A2_11D5_BA96_00C0DF22DE85__INCLUDED_)
