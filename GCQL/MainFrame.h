/**
	@file MainFrame.h
	@brief TODO
	
	(c)2006 Palestar Inc
	@author Richard Lyle @date 4/20/2008 3:19:41 PM
*/

#ifndef MAINFRAME_H
#define MAINFRAME_H

//---------------------------------------------------------------------------------------------------

#define CFrameWnd	CBCGFrameWnd

// CMainFrame frame
class CMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)
	DECLARE_MESSAGE_MAP()

public:
	enum Tabs
	{
		TAB_CHAT	= 0,
		TAB_BROWSE	= 1,
		TAB_LAUNCH	= 2,
		TAB_SERVERS	= 3,

		TAB_LAST
	};

	CMainFrame();           // protected constructor used by dynamic creation
	virtual ~CMainFrame();

	afx_msg int OnCreateClient(LPCREATESTRUCT lpCreateStruct, CCreateContext* pContext);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnGCQLCommand(WPARAM, LPARAM);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL DestroyWindow();
	virtual BOOL OnShowPopupMenu (CBCGPopupMenu* pMenuPopup);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	bool			SetActiveTab( Tabs a_nTab );

protected:
	// Data
	HICON			m_hIcon;
	CBCGTabWnd		m_wndTabs;
	CImageList		m_TabImages;
	bool			m_bPlacementRestored;
	CCaptionBar		m_wndCaptionBar;
	CBitmap			m_bmpCaption;

	bool			m_bUpdateCheck;
	dword			m_nLastUpdateCheck;
	dword			m_nUpdateCheckInterval;		// how often to check for updates
};



#endif

//---------------------------------------------------------------------------------------------------
//EOF
