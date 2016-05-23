/*
	ChatFrame.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef CHATFRAME_H
#define CHATFRAME_H

//---------------------------------------------------------------------------------------------------

#include "SplitWnd.h"
#include "CaptionBar.h"

#define CFrameWnd	CBCGFrameWnd

class CChatEdit;

class CChatFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CChatFrame)
public:
	void			clearIdleTime();
	void			setChatLineText( const TCHAR * pNewText );
	void			onPrivateMessage( dword authorId, const char * pAuthor, const char * pMessage );
	CChatEdit *		getChatEdit();

	dword			m_nRoomId;
	bool			m_bLoginActive;
	bool			m_bSplitterInit;
	HICON			m_hIcon;
	HICON			m_hMessageIcon;
	bool			m_bPrivateMessage;
	CxSplitterWnd	m_wndSplitter;
	CBCGToolBar		m_wndTopChatBar;
	CBCGToolBar		m_wndChatBar;

	int				m_GameButton;
	int				m_RoomsButton;
	int				m_FriendsButton;
	int				m_ClanButton;
	int				m_StaffButton;

	Array< MetaClient::Game >
					m_Games;
	Array< MetaClient::Room >
					m_Rooms;
	Array< MetaClient::ShortProfile >
					m_Friends;
	Array< MetaClient::ShortProfile >
					m_Clan;
	Array< MetaClient::ShortProfile >
					m_Staff;

	bool			m_bAutoAway;
	bool			m_bAway;
	bool			m_bMinimized;
	dword			m_TimeIdle;
	dword			m_ReconnectTime;

protected:
	DECLARE_MESSAGE_MAP()

	CChatFrame();           // protected constructor used by dynamic creation
	virtual ~CChatFrame();

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnChatSendchat();
	afx_msg void OnChatEdit();
	afx_msg void OnUpdateChatEdit(CCmdUI *pCmdUI);
	afx_msg void OnChatColor();
	afx_msg void OnChatCreateRoom();

	afx_msg LRESULT OnToolbarReset(WPARAM,LPARAM);
	virtual BOOL OnShowPopupMenu (CBCGPopupMenu* pMenuPopup);

	afx_msg void OnJoinRoom( UINT nID );
	afx_msg void OnFriendMessage( UINT nID );
	afx_msg void OnFriendDelete( UINT nID );
	afx_msg void OnLobby( UINT nID );
	afx_msg void OnClanMessage( UINT nID );
	afx_msg void OnStaffMessage( UINT nID );
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
