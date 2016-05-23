/*
	WebFrame.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef WEBFRAME_H
#define WEBFRAME_H

//---------------------------------------------------------------------------------------------------

#define CFrameWnd	CBCGFrameWnd

class CWebFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CWebFrame)
protected:
	CWebFrame();           // protected constructor used by dynamic creation
	virtual ~CWebFrame();

	bool			m_bPlacementRestored;

public:
	CBCGToolBar		m_wndNavBar;
	CStatusBar		m_wndStatusBar;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNavbarHome();
	afx_msg void OnUpdateNavbarHome(CCmdUI* pCmdUI);
	afx_msg void OnNavbarNews();
	afx_msg void OnUpdateNavbarNews(CCmdUI* pCmdUI);
	afx_msg void OnNavbarForum();
	afx_msg void OnUpdateNavbarForum(CCmdUI* pCmdUI);
	afx_msg void OnNavbarDownloads();
	afx_msg void OnUpdateNavbarDownloads(CCmdUI* pCmdUI);
	afx_msg void OnNavbarProfile();
	afx_msg void OnUpdateNavbarProfile(CCmdUI* pCmdUI);

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
