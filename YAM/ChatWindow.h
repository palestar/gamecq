#if !defined(AFX_CHATWINDOW_H__A50D4281_70FF_11D7_9412_00001CDB2E9A__INCLUDED_)
#define AFX_CHATWINDOW_H__A50D4281_70FF_11D7_9412_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatWindow.h : header file
//

#include "ChatView.h"
#include "ChatEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CChatWindow dialog

class CChatWindow : public CDialog
{
// Construction
public:
	void sendChat( CString text );
	CChatWindow(CWnd* pParent = NULL, bool bEcho = TRUE );   // standard constructor

	void putOldText( String oldText );
	void addNewMessage( const MetaClient::Chat chat );

// Dialog Data
	//{{AFX_DATA(CChatWindow)
	enum { IDD = IDD_CHATWINDOW_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnCancel();
	void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CChatWindow)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CChatView			*m_wndChatView;
	CChatEdit			m_wndChatEdit;
	bool				m_bEcho;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATWINDOW_H__A50D4281_70FF_11D7_9412_00001CDB2E9A__INCLUDED_)
