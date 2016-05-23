// yamDlg.h : Declare the yam dialog

#if !defined(AFX_yamDLG_H__435A175B_127F_11D4_9830_00105AA5E657__INCLUDED_)
#define AFX_yamDLG_H__435A175B_127F_11D4_9830_00105AA5E657__INCLUDED_

#pragma once

#include "yam.h"
#include "FriendList.h"
#include "ResizingDialog.h"
#include "ChatWindow.h"

#include "Standard/Semaphore.h"

class CSystemTray;

class CYamDlg : public CResizingDialog
{
public:
	void chatWindowClosed( CChatWindow * which, String oldText );
	void chatWindowSendText( CChatWindow * which, String text );
	void openChatWindow( dword userId );
	//// Public interface
	// Construction
	CYamDlg(CWnd* pParent = 0);


	//// Dialog Data
	//{{AFX_DATA(CYamDlg)
	enum { IDD = IDD_yam_DIALOG };
	CString	m_Status;
	//}}AFX_DATA
	static Array<MetaClient::ShortProfile>	s_friends;
	static Semaphore						s_lockFriends;
protected:
	String getFriendName( dword userId );
	String getFriendStatus( dword userId );
	int getChatWindow( CChatWindow * wnd );
	void OnCancel();
	void OnOK();
	void newChatWindow( dword userId );
	//// Overrides
	//{{AFX_VIRTUAL(CYamDlg)
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

	//// Message map
	//{{AFX_MSG(CYamDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnOpenConsole();
	//}}AFX_MSG
	void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSTRestore();
	afx_msg void OnSTExit();
	DECLARE_MESSAGE_MAP();
	
	typedef struct MsgWnd
	{
		bool			closed;
		dword			userId;
		String			oldText;
		CChatWindow		*pWnd;
	};

	//// Internal support functions
	void		SetupTrayIcon( bool bShow );
	void		SetupTaskBarButton( bool bShow );
	int			getChatWindow( dword userId );

	//// Internal data
	HICON						m_hIcon;
	HICON						m_hIconNormal;
	HICON						m_hIconNewMsg;
	bool						m_bMinimized;
	CSystemTray*				m_pTrayIcon;
	int							m_nTrayNotificationMsg;
	CFriendList					m_wndFriendList;
	int							m_TopChatMessage;
	Array<MsgWnd>				m_msgWnds;
	bool						m_bMsgWaiting;
private:
	void setMsgStatusNormal();
	void setMsgWaiting();
	void processNewChat( const MetaClient::Chat & chat );
	void updateChatWindowTitle( dword userId, bool quick );
	void reopenChatWindow( dword userId );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_yamDLG_H__435A175B_127F_11D4_9830_00105AA5E657__INCLUDED_)
