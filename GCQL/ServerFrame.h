/*
	ServerFrame.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef SERVERFRAME_H
#define SERVERFRAME_H

//---------------------------------------------------------------------------------------------------

#define CFrameWnd	CBCGFrameWnd

class CServerFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CServerFrame)
public:
	CServerFrame();           // protected constructor used by dynamic creation
	virtual ~CServerFrame();

	bool			m_bPlacementRestored;
	CBCGToolBar		m_wndServerBar;
	CStatusBar		m_wndStatusBar;
	int				m_FilterButton;

protected:
	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL DestroyWindow();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarReset(WPARAM,LPARAM);
	virtual BOOL OnShowPopupMenu (CBCGPopupMenu* pMenuPopup);
	afx_msg void OnFilter( UINT nID );
};



#endif

//---------------------------------------------------------------------------------------------------
//EOF
