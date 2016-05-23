//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#if !defined(AFX_BCGTASKSPANE_H__B1A20707_683B_4112_8E91_F5F720821638__INCLUDED_)
#define AFX_BCGTASKSPANE_H__B1A20707_683B_4112_8E91_F5F720821638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGTasksPane.h : header file
//

#include "BCGOutlookBar.h"
#include "BCGMenuButton.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGTasksPane additional classes

class CBCGTasksPane;

class BCGCONTROLBARDLLEXPORT CBCGTasksPaneMenuButton : public CBCGMenuButton
{
	DECLARE_DYNAMIC(CBCGTasksPaneMenuButton)

	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);
};

class BCGCONTROLBARDLLEXPORT CBCGTasksPanePage : public CObject
{
public:
	CBCGTasksPanePage(LPCTSTR lpszName, CBCGTasksPane *pTaskPane)
	{
		m_strName = lpszName;
		m_pTaskPane = pTaskPane;
	}

	virtual ~CBCGTasksPanePage ()
	{
		m_pTaskPane = NULL;
	}

	CString			m_strName;
	CBCGTasksPane*	m_pTaskPane;
};

class BCGCONTROLBARDLLEXPORT CBCGTasksGroup : public CObject
{
public:
	CBCGTasksGroup(LPCTSTR lpszName, BOOL bIsBottom, BOOL bIsSpecial = FALSE, 
		BOOL bIsCollapsed = FALSE, CBCGTasksPanePage* pPage = NULL, HICON hIcon = NULL)
	{
		m_pPage = pPage;
		m_strName =  lpszName;
		m_bIsBottom = bIsBottom;
		m_bIsSpecial = bIsSpecial;
		m_rect.SetRectEmpty ();
		m_rectGroup.SetRectEmpty ();
		m_bIsCollapsed = bIsCollapsed;
		m_hIcon = hIcon;
		m_sizeIcon = CSize(0, 0);

		m_clrText		= (COLORREF)-1;
		m_clrTextHot	= (COLORREF)-1;

		ICONINFO iconInfo;
		::ZeroMemory(&iconInfo, sizeof(iconInfo));
		::GetIconInfo(m_hIcon, &iconInfo);
		
		BITMAP bm;
		::ZeroMemory(&bm, sizeof(bm));
		::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);

		m_sizeIcon = CSize(bm.bmWidth, bm.bmHeight);

		::DeleteObject (iconInfo.hbmColor);
		::DeleteObject (iconInfo.hbmMask);
	}

	virtual ~CBCGTasksGroup()
	{
		while (!m_lstTasks.IsEmpty ())
		{
			delete m_lstTasks.RemoveHead ();
		}
		m_pPage = NULL;
	}

	CBCGTasksPanePage*	m_pPage;
	CString				m_strName;
	CObList				m_lstTasks;
	BOOL				m_bIsBottom;
	BOOL				m_bIsSpecial;
	CRect				m_rect;
	CRect				m_rectGroup;
	BOOL				m_bIsCollapsed;
	HICON				m_hIcon;
	CSize				m_sizeIcon;
	COLORREF			m_clrText;
	COLORREF			m_clrTextHot;
};

class BCGCONTROLBARDLLEXPORT CBCGTask : public CObject
{
public:
	CBCGTask(CBCGTasksGroup* pGroup, LPCTSTR lpszName, int nIcon, 
			UINT uiCommandID, DWORD dwUserData = 0,
			HWND hwndTask = NULL, BOOL bAutoDestroyWindow = FALSE, 
			int nWindowHeight = 0)
	{
		m_pGroup		= pGroup;
		m_strName		= lpszName == NULL ? _T("") : lpszName;
		m_nIcon			= nIcon;
		m_uiCommandID	= uiCommandID;
		m_dwUserData	= dwUserData;
		m_hwndTask		= hwndTask;
		m_bAutoDestroyWindow = bAutoDestroyWindow;
		m_nWindowHeight	= nWindowHeight;
		m_bVisible		= TRUE;
		m_bEnabled		= TRUE;
		m_bIsSeparator	= lpszName == NULL;
		m_clrText		= (COLORREF)-1;
		m_clrTextHot	= (COLORREF)-1;

		m_rect.SetRectEmpty ();
	}

	virtual ~CBCGTask()
	{
		if (m_hwndTask != NULL && m_bAutoDestroyWindow)
		{
			CWnd* pWnd = CWnd::FromHandlePermanent (m_hwndTask);
			if (pWnd != NULL)
			{
				pWnd->DestroyWindow ();
				delete pWnd;
			}
			else
			{
				::DestroyWindow (m_hwndTask);
			}
		}
		m_pGroup = NULL;
	}

	CBCGTasksGroup*	m_pGroup;
	CString			m_strName;
	int				m_nIcon;
	UINT			m_uiCommandID;
	DWORD			m_dwUserData;
	HWND			m_hwndTask;
	BOOL			m_bAutoDestroyWindow;
	CRect			m_rect;
	BOOL			m_bVisible;
	BOOL			m_bEnabled;
	int				m_nWindowHeight;
	BOOL			m_bIsSeparator;
	COLORREF		m_clrText;
	COLORREF		m_clrTextHot;
};

class CTasksPaneToolBar : public CBCGToolBar
{
	friend class CBCGTasksPane;
	DECLARE_SERIAL(CTasksPaneToolBar)

// Overrides
public:
	CTasksPaneToolBar()
	{
		m_pBtnBack = NULL;
		m_pBtnForward = NULL;
	}

	virtual BOOL AllowShowOnList () const	{	return FALSE;	}
	//virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

	void UpdateMenuButtonText (const CString& str);
	void UpdateButtons ();

protected:
	virtual void AdjustLocations ();
	virtual BOOL OnUserToolTip (CBCGToolbarButton* pButton, CString& strTTText) const;

	// Generated message map functions
	//{{AFX_MSG(CTasksPaneToolBar)
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CBCGToolbarMenuButton*	m_pBtnBack;
	CBCGToolbarMenuButton*	m_pBtnForward;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGTasksPane window

class BCGCONTROLBARDLLEXPORT CBCGTasksPane : public CBCGOutlookBar
{
	friend class CTasksPaneToolBar;
	DECLARE_DYNAMIC(CBCGTasksPane)

// Construction
public:
	CBCGTasksPane();

// Attributes
protected:
	CObList			m_lstTasksPanes;
	CArray<int, int> m_arrHistoryStack;
	int				m_iActivePage;
	const int		m_nMaxHistory;

	CObList			m_lstTaskGroups;
	CBCGTask*		m_pHotTask;
	CBCGTask*		m_pClickedTask;
	CBCGTasksGroup*	m_pHotGroupCaption;
	CBCGTasksGroup*	m_pClickedGroupCaption;
	BOOL			m_bCanCollapse;

	CString			m_strCaption;
	
	HFONT			m_hFont;
	CScrollBar		m_wndScrollVert;		// Vertical scroll bar
	CFont			m_fontBold;
	CFont			m_fontUnderline;
	CImageList		m_lstIcons;
	CSize			m_sizeIcon;

	CRect			m_rectTasks;

	BOOL				m_bUseNavigationToolbar;
	BOOL				m_bHistoryMenuButtons;
	CTasksPaneToolBar	m_wndToolBar;
	UINT				m_uiToolbarBmpRes;
	CSize				m_sizeToolbarImage;
	CSize				m_sizeToolbarButton;
	CRect				m_rectToolbar;

	BOOL			m_bUseScrollButtons;
	CRect			m_rectScrollUp;
	CRect			m_rectScrollDn;
	int				m_iScrollMode;	// -1 - Up, 0 - None, 1 - Down
	int				m_iScrollBtnHeight;
	
	int				m_nVertScrollOffset;
	int				m_nVertScrollTotal;
	int				m_nVertScrollPage;
	int				m_nRowHeight;
	
	BOOL			m_bAnimationEnabled;
	CBCGTasksGroup*	m_pAnimatedGroup;
	CSize			m_sizeAnim;

	int		m_nVertMargin;
	int		m_nHorzMargin;
	int		m_nGroupVertOffset;
	int		m_nGroupCaptionHeight;
	int		m_nGroupCaptionHorzOffset;
	int		m_nGroupCaptionVertOffset;
	int		m_nTasksHorzOffset;
	int		m_nTasksIconHorzOffset;
	int		m_nTasksIconVertOffset;
	BOOL	m_bOffsetCustomControls;

	CMenu						m_menuOther;
	CBCGTasksPaneMenuButton		m_btnOther;
	CBCGOutlookBarCaptionButton	m_btnForward;
	CBCGOutlookBarCaptionButton	m_btnBack;

	BOOL			m_bWrapTasks;
	BOOL			m_bWrapLabels;

	static clock_t		m_nLastAnimTime;
	static const int	m_iAnimTimerDuration;
	static const int	m_iScrollTimerDuration;

	//Accessability Support
	BCGACC_TASKINFO*    m_pAccTaskInfo;
	IAccessible*	    m_pAccessible;

// Operations
public:
	BOOL SetIconsList (UINT uiImageListResID, int cx, COLORREF clrTransparent = RGB (255, 0, 255));
	void SetIconsList (HIMAGELIST hIcons);

	void RecalcLayout (BOOL bRedraw = TRUE);

	//---------------
	// Pages support:
	//---------------
	int AddPage (LPCTSTR lpszPageLabel);
	void RemovePage (int nPageIdx);
	void RemoveAllPages ();
	
	int GetPagesCount() const 
	{ 
		return (int) m_lstTasksPanes.GetCount();
	}

	void SetActivePage (int nPageIdx);

	int GetActivePage () const
	{
		return m_arrHistoryStack[m_iActivePage];
	}

	void GetPreviousPages (CStringList& lstPrevPages) const;
	void GetNextPages (CStringList&  lstNextPages) const;

	void SetCaption (LPCTSTR lpszName);
	void SetPageCaption (int nPageIdx, LPCTSTR lpszName);

	BOOL GetPageByGroup (int nGroup, int &nPage) const;

	virtual void OnPressBackButton ();
	virtual void OnPressForwardButton ();
	virtual void OnPressHomeButton ();
	virtual void OnPressCloseButton();
	BOOL IsBackButtonEnabled () const		{ return m_iActivePage > 0; }
	BOOL IsForwardButtonEnabled () const	{ return m_iActivePage < m_arrHistoryStack.GetUpperBound (); }

	// --------------
	// Group support:
	// --------------
	int AddGroup (int nPageIdx, LPCTSTR lpszGroupName, BOOL bBottomLocation = FALSE, 
		BOOL bSpecial = FALSE, HICON hIcon = NULL);
	int AddGroup (LPCTSTR lpszGroupName, BOOL bBottomLocation = FALSE, 
		BOOL bSpecial = FALSE, HICON hIcon = NULL)
	{
		return AddGroup (0, lpszGroupName, bBottomLocation, bSpecial, hIcon);
	}
	void RemoveGroup (int nGroup);
	void RemoveAllGroups (int nPageIdx = 0);

	BOOL SetGroupName (int nGroup, LPCTSTR lpszGroupName);
	BOOL SetGroupTextColor (int nGroup, COLORREF color, COLORREF colorHot = (COLORREF)-1);
	BOOL CollapseGroup (CBCGTasksGroup* pGroup, BOOL bCollapse = TRUE);

	BOOL CollapseGroup (int nGroup, BOOL bCollapse = TRUE)
	{
		return CollapseGroup (GetTaskGroup (nGroup), bCollapse);
	}

	void CollapseAllGroups (BOOL bCollapse = TRUE);
	void CollapseAllGroups (int nPageIdx, BOOL bCollapse);

	void EnableGroupCollapse (BOOL bEnable) 
	{ 
		if (!bEnable)
		{
			CollapseAllGroups (FALSE);
		}

		m_bCanCollapse = bEnable; 
	}

	CBCGTasksGroup* GetTaskGroup (int nGroup) const;
	BOOL GetGroupLocation (CBCGTasksGroup* pGroup, int &nGroup) const;

	int GetGroupCount () const
	{
		return (int) m_lstTaskGroups.GetCount ();
	}

	// -------------
	// Task support:
	// -------------
	int AddTask (int nGroup, LPCTSTR lpszTaskName, int nTaskIcon = -1,
		UINT uiCommandID = 0, DWORD dwUserData = 0);
	int AddSeparator (int nGroup)
	{
		return AddTask (nGroup, NULL); 
	}
	BOOL SetTaskName (int nGroup, int nTask, LPCTSTR lpszTaskName);
	BOOL SetTaskTextColor (int nGroup, int nTask, COLORREF color, COLORREF colorHot = (COLORREF)-1);
	BOOL ShowTask (int nGroup, int nTask, BOOL bShow = TRUE);
	BOOL ShowTaskByCmdId (UINT uiCommandID, BOOL bShow = TRUE);
	BOOL RemoveTask (int nGroup, int nTask);
	void RemoveAllTasks (int nGroup);
	BOOL GetTaskLocation (UINT uiCommandID, int& nGroup, int& nTask) const;
	BOOL GetTaskLocation (HWND hwndTask, int& nGroup, int& nTask) const;
	CBCGTask* GetTask (int nGroup, int nTask) const;
	BOOL GetTaskLocation (CBCGTask* pTask, int &nGroup, int& nTask) const;

	int GetTaskCount (int nGroup) const
	{
		ASSERT(nGroup >= 0);
		ASSERT(nGroup < m_lstTaskGroups.GetCount ());

		CBCGTasksGroup* pGroup = GetTaskGroup (nGroup);
		ASSERT_VALID (pGroup);

		return (int) pGroup->m_lstTasks.GetCount ();
	}

	int AddWindow (int nGroup, HWND hwndTask, int nWndHeight, 
		BOOL bAutoDestroyWindow = FALSE, DWORD dwUserData = 0);
	BOOL SetWindowHeight (int nGroup, HWND hwndTask, int nWndHeight);
	BOOL SetWindowHeight (HWND hwndTask, int nWndHeight);

	int AddLabel (int nGroup, LPCTSTR lpszLabelName, int nTaskIcon = -1) 
	{ 
		return AddTask(nGroup, lpszLabelName, nTaskIcon); 
	}

	int AddMRUFilesList (int nGroup, int nMaxFiles = 4);

	// --------
	// Margins:
	// --------
	int GetVertMargin () const
	{
		return m_nVertMargin;
	}

	void SetVertMargin (int n = -1)
	{
		ASSERT(n >= -1);
		m_nVertMargin = n;
	}

	int GetHorzMargin () const
	{
		return m_nHorzMargin;
	}

	void SetHorzMargin (int n = -1)
	{
		ASSERT(n >= -1);
		m_nHorzMargin = n;
	}

	int GetGroupVertOffset () const
	{
		return m_nGroupVertOffset;
	}

	void SetGroupVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupVertOffset = n;
	}

	int GetGroupCaptionHeight () const
	{
		return m_nGroupCaptionHeight;
	}

	void SetGroupCaptionHeight (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionHeight = n;
	}

	int GetGroupCaptionHorzOffset () const
	{
		return m_nGroupCaptionHorzOffset;
	}

	void SetGroupCaptionHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionHorzOffset = n;
	}

	int GetGroupCaptionVertOffset () const
	{
		return m_nGroupCaptionVertOffset;
	}

	void SetGroupCaptionVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionVertOffset = n;
	}

	int GetTasksHorzOffset () const
	{
		return m_nTasksHorzOffset;
	}

	void SetTasksHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksHorzOffset = n;
	}

	int GetTasksIconHorzOffset () const
	{
		return m_nTasksIconHorzOffset;
	}

	void SetTasksIconHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksIconHorzOffset = n;
	}

	int GetTasksIconVertOffset () const
	{
		return m_nTasksIconVertOffset;
	}

	void SetTasksIconVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksIconVertOffset = n;
	}

	void EnableOffsetCustomControls (BOOL bEnable)
	{
		m_bOffsetCustomControls = bEnable;
	}

	// ---------
	// Behavior:
	// ---------
	void EnableScrollButtons (BOOL bEnable = TRUE)
	{
		m_bUseScrollButtons = bEnable;
	}

	void EnableNavigationToolbar (BOOL bEnable = TRUE,
		UINT uiToolbarBmpRes = 0, CSize sizeToolbarImage = CSize (0, 0),
		CSize sizeToolbarButton = CSize (0, 0));

	BOOL IsNavigationToolbarEnabled () const
	{
		return m_bUseNavigationToolbar;
	}
	
	const CBCGToolBar* GetNavigationToolbar() const
	{
		return (CBCGToolBar*) &m_wndToolBar;
	}

	void EnableAnimation (BOOL bEnable = TRUE)
	{
		m_bAnimationEnabled = bEnable;
	}

	BOOL IsAnimationEnabled () const
	{
		return m_bAnimationEnabled;
	}

	void EnableHistoryMenuButtons (BOOL bEnable = TRUE);

	BOOL IsHistoryMenuButtonsEnabled () const
	{
		return m_bHistoryMenuButtons;
	}

	void EnableWrapTasks  (BOOL bEnable = TRUE)
	{
		m_bWrapTasks = bEnable;
	}

	void EnableWrapLabels  (BOOL bEnable = TRUE)
	{
		m_bWrapLabels = bEnable;
	}

	BOOL IsWrapTasksEnabled () const
	{
		return m_bWrapTasks;
	}

	BOOL IsWrapLabelsEnabled () const
	{
		return m_bWrapLabels;
	}

// Overrides
public:
	virtual void OnClickTask (int nGroupNumber, int nTaskNumber, 
							UINT uiCommandID, DWORD dwUserData);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGTasksPane)
	public:
	//}}AFX_VIRTUAL
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	virtual void OnActivateTasksPanePage () {}

	virtual void OnEraseWorkArea (CDC* pDC, CRect rectWorkArea);
	virtual void AdjustLocations ();
	virtual void OnDrawCaption (CDC* pDC, CRect rectCaption, CString strTitle, BOOL bHorz);
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	virtual CSize GetTasksGroupBorders () const { return CSize (1, 1);}

	virtual void NotifyAccessibility (int nGroupNumber, int nTaskNumber);

// Implementation
public:
	virtual ~CBCGTasksPane();

	HMENU CreateMenu () const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGTasksPane)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg void OnBack ();
	afx_msg void OnForward ();
	afx_msg void OnHome ();
	afx_msg void OnClose ();
	afx_msg void OnOther ();
	afx_msg void OnUpdateBack (CCmdUI* pCmdUI);
	afx_msg void OnUpdateForward (CCmdUI* pCmdUI);
	afx_msg LRESULT OnGetObject (WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	int ReposTasks (BOOL bCalcHeightOnly = FALSE);
	void CreateFonts ();
	HFONT SetFont (CDC* pDC);
	void SetScrollSizes ();
	void AdjustScroll ();
	void RebuildMenu ();
	void ChangeActivePage (int nNewPageHistoryIdx, int nOldPageHistoryIdx);
	void SaveHistory (int nPageIdx);
	BOOL CreateNavigationToolbar ();
	void UpdateCaption ();

	BOOL IsScrollUpAvailable ()
	{
		return m_nVertScrollOffset > 0;
	}

	BOOL IsScrollDnAvailable ()
	{
		return m_nVertScrollOffset <= m_nVertScrollTotal - m_nVertScrollPage && m_nVertScrollTotal > 0;
	}

	BOOL ForceShowNavToolbar () const
	{
		return m_rectCaption.IsRectEmpty ();
	}

	CBCGTask* TaskHitTest (CPoint pt) const;
	CBCGTasksGroup* GroupCaptionHitTest (CPoint pt) const;

// Constants:
	static const UINT idOther;
	static const UINT idForward;
	static const UINT idBack;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGTASKSPANE_H__B1A20707_683B_4112_8E91_F5F720821638__INCLUDED_)
