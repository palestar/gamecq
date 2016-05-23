#if !defined(AFX_FINDUSER_H__E73A3AA0_64CB_11D6_9410_00001CDB2E9A__INCLUDED_)
#define AFX_FINDUSER_H__E73A3AA0_64CB_11D6_9410_00001CDB2E9A__INCLUDED_

#include "PlayerList.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindUser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindUser dialog

class CFindUser : public CDialog
{
// Construction
public:
	CFindUser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindUser)
	enum { IDD = IDD_FINDUSER };
	CString	m_UserID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindUser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindUser)
	afx_msg void OnFind();
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlayerEdit();
	afx_msg void OnPlayerAddfriend();
	afx_msg void OnPlayerAddignore();
	afx_msg void OnPlayerWho();
	afx_msg void OnPlayerSend();
	afx_msg void OnUpdatePlayerEdit(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int selected;
	Array< MetaClient::ShortProfile > found;
	CImageList m_AvatarIcons;
public:
	afx_msg void OnPlayerBan();
	afx_msg void OnUpdatePlayerBan(CCmdUI *pCmdUI);
	afx_msg void OnPlayerKick();
	afx_msg void OnUpdatePlayerKick(CCmdUI *pCmdUI);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDUSER_H__E73A3AA0_64CB_11D6_9410_00001CDB2E9A__INCLUDED_)
