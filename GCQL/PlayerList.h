#if !defined(AFX_PLAYERLIST_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_PLAYERLIST_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayerList.h : header file
//

#include "GCQ/MetaClient.h"

/////////////////////////////////////////////////////////////////////////////
// CPlayerList window

class CPlayerList : public CListCtrl
{
// Construction
public:
	CPlayerList();

// Attributes
public:
	dword				m_nRoomId;
	CImageList			m_AvatarIcons;
	Array< CString >	m_EmotionNames;
	Array< CString >	m_EmotionText;

// Operations
public:
	bool			isSelected() const;
	CString			getSelected() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlayerList();

private:
	int		m_OldY;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPlayerWho();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPlayerWhisper();
	afx_msg void OnPlayerSend();
	afx_msg void OnPlayerAddfriend();
	afx_msg void OnPlayerAddignore();
	afx_msg void OnPlayerEdit();
	afx_msg void OnUpdatePlayerEdit(CCmdUI* pCmdUI);
	afx_msg void OnPlayerMute();
	afx_msg void OnUpdatePlayerMute(CCmdUI* pCmdUI);
	afx_msg void OnPlayerUnmute();
	afx_msg void OnUpdatePlayerUnmute(CCmdUI* pCmdUI);
	afx_msg void OnPlayerKick();
	afx_msg void OnUpdatePlayerKick(CCmdUI* pCmdUI);
	afx_msg void OnPlayerBan();
	afx_msg void OnUpdatePlayerBan(CCmdUI* pCmdUI);
	afx_msg void OnPlayerClones();
	afx_msg void OnPlayerCheck();
	afx_msg void OnPlayerSession();
	afx_msg void OnPlayerModsend();
	//}}AFX_MSG
	afx_msg void OnEmotion( UINT nID );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERLIST_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_)
