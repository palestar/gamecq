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

// BCGTasksPane.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "globals.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "BCGPopupMenu.h"
#include "BCGWorkspace.h"
#include "BCGTasksPane.h"
#include "BCGToolbarMenuButton.h"
#include "winable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CBCGWorkspace* g_pWorkspace;

#define ID_SCROLL_VERT	1

static const int iBorderSize = 1;
static const int iNavToolbarId = 1;
static const int iAnimTimerId = 4;
static const int iScrollTimerId = 5;

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

const UINT CBCGTasksPane::idOther	= 4;
const UINT CBCGTasksPane::idForward	= 5;
const UINT CBCGTasksPane::idBack	= 6;

clock_t CBCGTasksPane::m_nLastAnimTime = 0;
const int CBCGTasksPane::m_iAnimTimerDuration = 30;
const int CBCGTasksPane::m_iScrollTimerDuration = 80;

/////////////////////////////////////////////////////////////////////////////
// CBCGTasksPaneMenuButton

void CBCGTasksPaneMenuButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	CBCGVisualManager::GetInstance ()->OnEraseOutlookCaptionButton (pDC, rectClient, this);
}

void CBCGTasksPaneMenuButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	CBCGVisualManager::GetInstance ()->OnDrawOutlookCaptionButtonBorder (pDC, rectClient,
		this, uiState);
}

IMPLEMENT_DYNAMIC(CBCGTasksPaneMenuButton, CBCGMenuButton)


/////////////////////////////////////////////////////////////////////////////
// CTasksPaneHistoryButton

class CTasksPaneHistoryButton : public CBCGToolbarMenuButton
{
	friend class CBCGTasksPane;
	DECLARE_SERIAL(CTasksPaneHistoryButton)

public:
    CTasksPaneHistoryButton(int iImage = -1)
		: CBCGToolbarMenuButton ()
	{
		m_iImage = iImage;
		m_bLocked = TRUE;

		m_pParentBar = NULL;
	}
	//***********************************************************************
	CTasksPaneHistoryButton (UINT uiID, int iImage, 
							LPCTSTR lpszText = NULL, BOOL bUserButton = FALSE)
		: CBCGToolbarMenuButton ()
	{
		m_nID = uiID;
		m_bUserButton = bUserButton;

		SetImage (iImage);
		m_strText = (lpszText == NULL) ? _T("") : lpszText;

		CMenu menu;
		menu.CreatePopupMenu ();
		CreateFromMenu (menu.GetSafeHmenu ());

		m_pParentBar = NULL;
	}
	//***********************************************************************
	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGToolbarMenuButton::OnChangeParentWnd (pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST (CBCGTasksPane, pWndParent);
	}
	//***********************************************************************
	void UpdateMenu ()
	{
		if (m_pParentBar == NULL)
		{
			return;
		}

		if (m_nID == ID_BCGBARRES_TASKPANE_BACK)
		{
			m_pParentBar->GetPreviousPages (m_lstPages);
		}
		else if (m_nID == ID_BCGBARRES_TASKPANE_FORWARD)
		{
			m_pParentBar->GetNextPages (m_lstPages);
		}

		CMenu menu;
		menu.CreatePopupMenu ();

		for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL; )
		{
			CString& strPageName = m_lstPages.GetNext (pos);
			menu.AppendMenu (MF_STRING, m_nID, strPageName);
		}

		CreateFromMenu (menu.GetSafeHmenu ());
	}

// data:
	CBCGTasksPane*		m_pParentBar;
	CStringList			m_lstPages;	// pages history
};

IMPLEMENT_SERIAL(CTasksPaneHistoryButton, CBCGToolbarMenuButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CTasksPaneMenuButton

class CTasksPaneMenuButton : public CBCGToolbarMenuButton
{
	friend class CBCGTasksPane;
	DECLARE_SERIAL(CTasksPaneMenuButton)

public:
	CTasksPaneMenuButton(HMENU hMenu = NULL) :
		CBCGToolbarMenuButton ((UINT)-1, hMenu, -1)
	{
		m_pParentBar = NULL;
	}
	//***********************************************************************
	virtual HMENU CreateMenu () const
	{
		if (m_pParentBar == NULL)
		{
			return NULL;
		}

		ASSERT_VALID (m_pParentBar);

		return m_pParentBar->CreateMenu ();
	}
	//***********************************************************************
	virtual CBCGPopupMenu* CreatePopupMenu ()
	{
		CBCGPopupMenu* pMenu = CBCGToolbarMenuButton::CreatePopupMenu ();
		if (pMenu == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		pMenu->SetRightAlign (TRUE);
		pMenu->SetMessageWnd (m_pParentBar);

		return pMenu;
	}
	//***********************************************************************
	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGToolbarMenuButton::OnChangeParentWnd (pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST (CBCGTasksPane, pWndParent);
	}
	
// data:
	CBCGTasksPane*	m_pParentBar;
};

IMPLEMENT_SERIAL(CTasksPaneMenuButton, CBCGToolbarMenuButton, 1)


/////////////////////////////////////////////////////////////////////////////
// CTasksPaneToolBar

IMPLEMENT_SERIAL(CTasksPaneToolBar, CBCGToolBar, 1)

BEGIN_MESSAGE_MAP(CTasksPaneToolBar, CBCGToolBar)
	//{{AFX_MSG_MAP(CTasksPaneToolBar)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

//********************************************************************************
LRESULT CTasksPaneToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// handle delay hide/show
	BOOL bVis = GetStyle() & WS_VISIBLE;
	UINT swpFlags = 0;
	if ((m_nStateFlags & delayHide) && bVis)
		swpFlags = SWP_HIDEWINDOW;
	else if ((m_nStateFlags & delayShow) && !bVis)
		swpFlags = SWP_SHOWWINDOW;
	m_nStateFlags &= ~(delayShow|delayHide);
	if (swpFlags != 0)
	{
		SetWindowPos(NULL, 0, 0, 0, 0, swpFlags|
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}

	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if ((GetStyle() & WS_VISIBLE) &&
		(m_pDockBar == NULL || (m_pDockBar->GetStyle() & WS_VISIBLE)))
	{
		OnUpdateCmdUI((CFrameWnd*) GetOwner (), (BOOL)wParam);
	}

	return 0L;
}
//******************************************************************************************
void CTasksPaneToolBar::AdjustLocations ()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd () == NULL || !::IsWindow (m_hWnd))
	{
		return;
	}

	CBCGToolBar::AdjustLocations ();

	//----------------------------------
	// Get menu button and close button:
	//----------------------------------
	CBCGToolbarButton* pCloseBtn = NULL;
	CTasksPaneMenuButton* pMenuBtn = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);
		
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
		}
		else
		{
			if (pButton->IsKindOf (RUNTIME_CLASS (CTasksPaneMenuButton)))
			{
				pMenuBtn = DYNAMIC_DOWNCAST (CTasksPaneMenuButton, pButton);
				ASSERT_VALID (pMenuBtn);
			}
			else
			{
				if (pButton->GetImage () == 3)
				{
					pCloseBtn = pButton;
					ASSERT_VALID (pCloseBtn);
				}

			}
		}
	}	

	CRect rectClient;
	GetClientRect (&rectClient);
	
	BOOL bShowCloseButton = FALSE;
	BOOL bStrechMenuButton = TRUE;

	if (pMenuBtn != NULL)
	{
		CRect rectMenuBtn = pMenuBtn->Rect ();
		int nMin = rectMenuBtn.left + rectMenuBtn.Height () * 3;
		int nMax = rectClient.right - 1;
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			nMax = rectClient.right - 1 - rectMenuBtn.Height ();
		}

		// -------------------
		// Adjust menu button:
		// -------------------
		if (bStrechMenuButton)
		{
			rectMenuBtn.right = max (nMin, nMax);
			pMenuBtn->SetRect (rectMenuBtn);
		}

		// --------------------
		// Adjust close button:
		// --------------------
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			CRect rectCloseBtn = pMenuBtn->Rect ();
			rectCloseBtn.left = rectMenuBtn.right;
			rectCloseBtn.right = rectMenuBtn.right + rectCloseBtn.Height ();

			if (rectCloseBtn.right < rectClient.right - 1)
			{
				rectCloseBtn.OffsetRect (rectClient.right - 1 - rectCloseBtn.right, 0);
			}

			pCloseBtn->SetRect (rectCloseBtn);
			pCloseBtn->Show (TRUE);
		}
		else if (pCloseBtn != NULL)
		{
			pCloseBtn->Show (FALSE);
		}
	}
	//UpdateTooltips ();
}
//******************************************************************************************
void CTasksPaneToolBar::UpdateMenuButtonText (const CString& str)
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CTasksPaneMenuButton* pMenuBtn = DYNAMIC_DOWNCAST (CTasksPaneMenuButton, pButton);
		if (pMenuBtn != NULL)
		{
			ASSERT_VALID (pMenuBtn);

			pMenuBtn->m_strText = str;
		}
	}
}
//******************************************************************************************
void CTasksPaneToolBar::UpdateButtons ()
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CTasksPaneHistoryButton* pHistoryBtn = DYNAMIC_DOWNCAST (CTasksPaneHistoryButton, pButton);
		if (pHistoryBtn != NULL)
		{
			pHistoryBtn->UpdateMenu ();
		}
	}
}
//******************************************************************************************

BOOL CTasksPaneToolBar::OnUserToolTip (CBCGToolbarButton* pButton, CString& strTTText) const
{
	ASSERT_VALID (pButton);

	if (pButton->IsKindOf (RUNTIME_CLASS (CTasksPaneMenuButton)))
	{
		CBCGLocalResource locaRes;
		strTTText.LoadString (ID_BCGBARRES_TASKPANE_OTHER);
		return TRUE;
	}

	CBCGToolbarButton* pNavButton = pButton;
	if (pNavButton != NULL)
	{
		ASSERT_VALID (pNavButton);
		strTTText = pNavButton->m_strText;
		return TRUE;
	}
		
	return CBCGToolBar::OnUserToolTip (pButton, strTTText);
}


/////////////////////////////////////////////////////////////////////////////
// CBCGTasksPane

IMPLEMENT_DYNAMIC(CBCGTasksPane, CBCGOutlookBar)

CBCGTasksPane::CBCGTasksPane(): CBCGOutlookBar (), m_nMaxHistory (10)
{
	m_nSize = 200;	// Initial width
	m_bFlatBorder = TRUE;

	m_arrHistoryStack.Add (0);
	m_iActivePage = 0;
	m_pHotTask = NULL;
	m_pClickedTask = NULL;
	m_pHotGroupCaption = NULL;
	m_pClickedGroupCaption = NULL;
	m_bCanCollapse = TRUE;
	
	m_hFont = NULL;
	m_sizeIcon = CSize (0, 0);
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_nRowHeight = 0;

	m_nVertMargin = -1;	// default, use Visual Manager's settings
	m_nHorzMargin = -1;
	m_nGroupVertOffset = -1;
	m_nGroupCaptionHeight = -1;
	m_nGroupCaptionHorzOffset = -1;
	m_nGroupCaptionVertOffset = -1;
	m_nTasksHorzOffset = -1;
	m_nTasksIconHorzOffset = -1;
	m_nTasksIconVertOffset = -1;

	m_bOffsetCustomControls = TRUE;

	m_rectTasks.SetRectEmpty ();

	m_bUseNavigationToolbar = FALSE;
	m_bHistoryMenuButtons = FALSE;
	m_uiToolbarBmpRes = 0;
	m_sizeToolbarImage = CSize (0, 0);
	m_sizeToolbarButton = CSize (0, 0);
	m_rectToolbar.SetRectEmpty ();

	m_bUseScrollButtons = FALSE;
	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();
	m_iScrollBtnHeight = CMenuImages::Size ().cy + 2 * iBorderSize;
	m_iScrollMode = 0;
	
	m_bAnimationEnabled = !globalData.bIsRemoteSession;
	m_pAnimatedGroup = NULL;
	m_sizeAnim = CSize (0, 0);

	m_btnOther.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
	m_btnOther.m_bDrawFocus = FALSE;
	m_btnOther.m_bOSMenu = FALSE;
	m_btnOther.m_bRightAlign = TRUE;
	m_btnForward.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
	m_btnForward.m_bDrawFocus = FALSE;
	m_btnBack.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
	m_btnBack.m_bDrawFocus = FALSE;
	
	m_bWrapTasks = FALSE;
	m_bWrapLabels = FALSE;

	m_pAccessible = NULL;
	m_pAccTaskInfo = NULL;
}

CBCGTasksPane::~CBCGTasksPane()
{
	while (!m_lstTasksPanes.IsEmpty ())
	{
		delete m_lstTasksPanes.RemoveHead ();
	}

	if (m_pAccTaskInfo != NULL)
	{
		delete m_pAccTaskInfo;
	}
}

BEGIN_MESSAGE_MAP(CBCGTasksPane, CBCGOutlookBar)
	//{{AFX_MSG_MAP(CBCGTasksPane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_BACK, OnBack)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_FORWARD, OnForward)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_TASKPANE_BACK, OnUpdateBack)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_TASKPANE_FORWARD, OnUpdateForward)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_HOME, OnHome)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_CLOSE, OnClose)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_OTHER, OnOther)
	ON_MESSAGE (WM_GETOBJECT, OnGetObject)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGTasksPane message handlers

int CBCGTasksPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGOutlookBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strOther;
	CString strForward;
	CString strBack;
	{
		CBCGLocalResource locaRes;

		strOther.LoadString (ID_BCGBARRES_TASKPANE_OTHER);
		strForward.LoadString (ID_BCGBARRES_TASKPANE_FORWARD);
		strBack.LoadString (ID_BCGBARRES_TASKPANE_BACK);
		m_strCaption.LoadString (IDS_BCGBARRES_TASKPANE);
	}

	//-------------------------------------
	// Add default page to m_lstTasksPanes:
	//-------------------------------------
	AddPage (m_strCaption);
	SetCaption (m_strCaption);
	
	// ---------------
	// Create toolbar:
	// ---------------
	if (!CreateNavigationToolbar ())
	{
		TRACE(_T("Can't create taskspane toolbar bar\n"));
		return FALSE;
	}

	CreateFonts ();

	if (globalData.m_hcurHand == NULL)
	{
		CBCGLocalResource locaRes;
		globalData.m_hcurHand = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_HAND);
	}

	CRect rectDummy;
	rectDummy.SetRectEmpty ();
	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, ID_SCROLL_VERT);

	m_btnOther.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,	this, idOther);
	m_btnForward.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,	this, idForward);
	m_btnBack.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,	this, idBack);

	m_btnOther.SetTooltip (strOther);

	m_btnForward.SetTooltip (strForward);
	m_btnForward.SetStdImage (CMenuImages::IdArowForward);

	m_btnBack.SetTooltip (strBack);
	m_btnBack.SetStdImage (CMenuImages::IdArowBack);

	return 0;
}
//******************************************************************************************
void CBCGTasksPane::OnSize(UINT nType, int cx, int cy) 
{
	CBCGOutlookBar::OnSize(nType, cx, cy);

	AdjustLocations ();
	ReposTasks ();
		
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
}
//******************************************************************************************
int CBCGTasksPane::ReposTasks (BOOL bCalcHeightOnly/* = FALSE*/)
{
	if (globalData.bIsRemoteSession)
	{
		m_bAnimationEnabled = FALSE;
	}

	if (GetSafeHwnd () == NULL || m_lstTaskGroups.IsEmpty ())
	{
		return 0;
	}

	if ((m_rectTasks.top < 0) || (m_rectTasks.bottom <= m_rectTasks.top) ||
		(m_rectTasks.left < 0) || (m_rectTasks.right <= m_rectTasks.left))
	{
		return 0; // m_rectTasks is not set yet
	}

	CRect rectTasks = m_rectTasks;
	rectTasks.DeflateRect (
		(GetHorzMargin() != -1 ? GetHorzMargin() :
		CBCGVisualManager::GetInstance ()->GetTasksPaneHorzMargin()), 
		(GetVertMargin() != -1 ? GetVertMargin() :
		CBCGVisualManager::GetInstance ()->GetTasksPaneVertMargin()));

	CClientDC dc (this);
	CFont* pFontOld = dc.SelectObject (&m_fontBold);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	m_nRowHeight = max (tm.tmHeight, m_sizeIcon.cy);

	int y = rectTasks.top - m_nVertScrollOffset * m_nRowHeight;

	// ---------------
	// Get active page
	// ---------------
	CBCGTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// -------------
	// Recalc groups
	// -------------
	POSITION pos = NULL;

	for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			dc.SelectObject (&m_fontBold);

			// -----------------
			// Calc caption size
			// -----------------
			int nCaptionHeight = 0;
			if (!pGroup->m_strName.IsEmpty())
			{
				CFont* pFontOld = dc.SelectObject (&globalData.fontBold);
				CSize sizeText = dc.GetTextExtent (pGroup->m_strName);
				dc.SelectObject (pFontOld);

				int nVOffset = (GetGroupCaptionVertOffset() != -1 ? GetGroupCaptionVertOffset() :
					CBCGVisualManager::GetInstance ()->GetTasksPaneGroupCaptionVertOffset());
				int nHeight = (GetGroupCaptionHeight() != -1 ? GetGroupCaptionHeight() : 
					CBCGVisualManager::GetInstance ()->GetTasksPaneGroupCaptionHeight());

				nCaptionHeight = max( sizeText.cy + nVOffset, nHeight );
			}
			else
			{
				nCaptionHeight = 0;
			}

			if (pGroup->m_hIcon != NULL && 
				(pGroup->m_sizeIcon.cx < rectTasks.Width () - nCaptionHeight))
			{
				if (nCaptionHeight < pGroup->m_sizeIcon.cy)
				{
					y += pGroup->m_sizeIcon.cy - nCaptionHeight;
				}
			}

			if (!bCalcHeightOnly)
			{
				pGroup->m_rect = CRect (rectTasks.left, y, 
					rectTasks.right, y + nCaptionHeight);
			}

			y += nCaptionHeight;
			int yGroup = y;

			SetFont (&dc);

			if (m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty() &&
				!(m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0 && !bCalcHeightOnly))
			{
				if (!bCalcHeightOnly)
				{
					// ---------------------
					// Recalc tasks in group
					// ---------------------
					pGroup->m_rectGroup = CRect (rectTasks.left, yGroup - 1, 
						rectTasks.right, yGroup - 1);

					for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
					{
						CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
						ASSERT_VALID (pTask);

						if (pTask->m_hwndTask == NULL)
						{
							pTask->m_rect.SetRectEmpty ();
						}
					}
				}
			}
			else // not collapsed
			{
				// ---------------------
				// Recalc tasks in group
				// ---------------------
				BOOL bNeedHeaderOffset = TRUE;
				BOOL bNeedFooterOffset = TRUE;
				CSize sizeGroupBorders = GetTasksGroupBorders ();

				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
				{
					CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_hwndTask == NULL)
					{
						if (pTask->m_bVisible)
						{
							if (bNeedHeaderOffset)
							{
								y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
							}

							int nTaskHOffset = (GetTasksHorzOffset() != -1 ? GetTasksHorzOffset() :
								CBCGVisualManager::GetInstance ()->GetTasksPaneTaskHorzOffset());
							int nIconHOffset = (GetTasksIconHorzOffset() != -1 ? GetTasksIconHorzOffset() :
								CBCGVisualManager::GetInstance ()->GetTasksPaneIconHorzOffset());

							// -----------------
							// if multiline text
							// -----------------
							if ((pTask->m_uiCommandID == 0) ? m_bWrapLabels : m_bWrapTasks)
							{
								CRect rectTask = rectTasks;
								rectTask.DeflateRect (nTaskHOffset, 0);
								rectTask.top = y;
								rectTask.bottom = y + m_sizeIcon.cy;

								// Determines the width of the text rectangle
								CRect rectText = rectTask;
								rectText.left += m_sizeIcon.cx + nIconHOffset;

								// Determines the height of the text rectangle
								CFont* pFontOld = dc.SelectObject (&globalData.fontUnderline);
								int cy = dc.DrawText (pTask->m_strName, rectText, DT_CALCRECT | DT_WORDBREAK);
								dc.SelectObject (pFontOld);
								
								if (pTask->m_bIsSeparator)
								{
									cy = max (cy, 10);
								}
								cy = max (cy, m_sizeIcon.cy);
								rectTask.bottom = rectTask.top + cy;

								if (!bCalcHeightOnly)
								{
									pTask->m_rect = rectTask;
								}
								
								y += cy + (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
							// ----------------
							// single-line text
							// ----------------
							else
							{
								CFont* pFontOld = dc.SelectObject (&globalData.fontUnderline);
								CSize sizeText = dc.GetTextExtent (pTask->m_strName);
								dc.SelectObject (pFontOld);

								int cy = max (sizeText.cy, m_sizeIcon.cy);

								if (!bCalcHeightOnly)
								{
									pTask->m_rect = CRect (
										rectTasks.left + nTaskHOffset, 
										y, 
										rectTasks.left + sizeText.cx + m_sizeIcon.cx + nTaskHOffset + nIconHOffset, 
										y + cy);
									pTask->m_rect.right = max(pTask->m_rect.left, rectTasks.right - nTaskHOffset);
								}
								
								y += cy + (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
						}
						else
						{
							if (!bCalcHeightOnly)
							{
								pTask->m_rect.SetRectEmpty();
							}
						}
					}
					
					else // Use child window
					{
						if (bNeedHeaderOffset && pTask->m_bVisible)
						{
							if (m_bOffsetCustomControls)
							{
								y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
							}
							else
							{
								y += sizeGroupBorders.cy;
							}
						}

						CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
						ASSERT_VALID(pChildWnd);

						if (!bCalcHeightOnly)
						{
							CRect rectChildWnd = rectTasks;
							rectChildWnd.bottom = y + (pTask->m_bVisible ? pTask->m_nWindowHeight : 0);
							rectChildWnd.top = max (m_rectTasks.top + 1, y);
							rectChildWnd.bottom = min (m_rectTasks.bottom - 1, rectChildWnd.bottom);

							if (m_bOffsetCustomControls)
							{
								rectChildWnd.DeflateRect ((GetTasksHorzOffset() != -1 ? GetTasksHorzOffset(): 
									CBCGVisualManager::GetInstance ()->GetTasksPaneTaskHorzOffset()), 0);
							}
							else
							{
								rectChildWnd.DeflateRect (sizeGroupBorders.cx, 0);
							}

							pTask->m_rect = rectChildWnd;
						}

						if (pTask->m_bVisible)
						{
							y += pTask->m_nWindowHeight;
							bNeedHeaderOffset = TRUE;
							bNeedFooterOffset = TRUE;
						}
					}

					if (bNeedFooterOffset && pTask->m_bVisible)
					{
						if (m_bOffsetCustomControls)
						{
							y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
								CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
						}
						else
						{
							y += sizeGroupBorders.cy;
						}
					}

					// constrain task's height during the animation:
					if (!bCalcHeightOnly)
					{
						if (m_bAnimationEnabled && pGroup == m_pAnimatedGroup)
						{
							if (y > yGroup + m_sizeAnim.cy)
							{
								y = yGroup + max (0, m_sizeAnim.cy);
								pTask->m_rect.bottom = max (pTask->m_rect.top, min (pTask->m_rect.bottom, y - 1));
							}
						}
					}
				}

				if (!bCalcHeightOnly)
				{
					pGroup->m_rectGroup = CRect (rectTasks.left, yGroup, rectTasks.right, y);
				}
			}

			y += (GetGroupVertOffset() != -1 ? GetGroupVertOffset() :
				CBCGVisualManager::GetInstance ()->GetTasksPaneGroupVertOffset());
		}
	}


    if (!bCalcHeightOnly)
    {
        // ---------------------------------------------
        // Find the last task group for the active page:
        // ---------------------------------------------
        CBCGTasksGroup* pLastGroup = NULL;
        for (POSITION posGroup = m_lstTaskGroups.GetTailPosition (); posGroup != NULL; )
        {
            CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetPrev (posGroup);
            ASSERT_VALID (pGroup);

            if (pGroup->m_pPage == pActivePage)
            {
                pLastGroup = pGroup;
                break;
            }
        }

        if (pLastGroup != NULL)
        {
			// ---------------------------------------------
			// Offset the last group if it's bottom aligned:
			// ---------------------------------------------
			if (pLastGroup->m_bIsBottom && !pLastGroup->m_lstTasks.IsEmpty () &&
				m_nVertScrollTotal == 0)
			{
				int nOffset = 0;
				for (POSITION posTask = pLastGroup->m_lstTasks.GetTailPosition (); posTask != NULL;)
				{
					CBCGTask* pTask = (CBCGTask*) pLastGroup->m_lstTasks.GetPrev (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_bVisible)
					{
						nOffset = rectTasks.bottom - pLastGroup->m_rectGroup.bottom;
						break;
					}
				}

				if (nOffset > 0)
				{
					for (POSITION posTask = pLastGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
					{
						CBCGTask* pTask = (CBCGTask*) pLastGroup->m_lstTasks.GetNext (posTask);
						ASSERT_VALID (pTask);

						if (pTask->m_bVisible)
						{
							pTask->m_rect.OffsetRect (0, nOffset);
						}
					}

					pLastGroup->m_rect.OffsetRect (0, nOffset);
					pLastGroup->m_rectGroup.OffsetRect (0, nOffset);
				}
			}
		}


		// --------------------------------------------
		// Repos or hide child windows for active page:
		// --------------------------------------------
		for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
		{
			CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
			ASSERT_VALID (pGroup);

			if (pGroup->m_pPage == pActivePage)
			{
				BOOL bCollapsed = m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty();
				BOOL bAnimating = m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0;
				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
				{
					CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_hwndTask != NULL) // Use child window
					{
						CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
						ASSERT_VALID(pChildWnd);
							
						if (bCollapsed && !bAnimating || !pTask->m_bVisible || pTask->m_rect.IsRectEmpty ())
						{
							pChildWnd->ShowWindow (SW_HIDE);
						}
						else
						{
							pChildWnd->SetWindowPos (NULL,
								pTask->m_rect.left, pTask->m_rect.top,
								pTask->m_rect.Width (), pTask->m_rect.Height (),
								SWP_NOZORDER | SWP_NOACTIVATE);
							pChildWnd->ShowWindow (SW_SHOWNOACTIVATE);
						}
					}

				}
			}
		}
    }

	dc.SelectObject (pFontOld);
	return y - (GetGroupVertOffset() != -1 ? GetGroupVertOffset() :
		CBCGVisualManager::GetInstance ()->GetTasksPaneGroupVertOffset()) +
		m_nVertScrollOffset * m_nRowHeight -
		(GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
		CBCGVisualManager::GetInstance ()->GetTasksPaneIconVertOffset()) +
		(GetVertMargin() != -1 ? GetVertMargin() :
		CBCGVisualManager::GetInstance ()->GetTasksPaneVertMargin());
}
//******************************************************************************************
void CBCGTasksPane::OnEraseWorkArea (CDC* pDC, CRect /*rectWorkArea*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CRect rectFill = m_rectTasks;
	rectFill.InflateRect (0, m_nVertScrollOffset * m_nRowHeight, 0, 0);
	CBCGVisualManager::GetInstance ()->OnFillTasksPaneBackground (pDC, rectFill);

	// ---------------
	// Get active page
	// ---------------
	CBCGTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);
	
	// ---------------------
	// Draw all tasks groups
	// ---------------------
	CRgn rgnClipTask;
	rgnClipTask.CreateRectRgnIndirect (CRect (0, 0, 0, 0));

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; )
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (!pGroup->m_bIsCollapsed || pGroup->m_strName.IsEmpty() ||
				(m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0))
			{
				if (!pGroup->m_rectGroup.IsRectEmpty ())
				{
					CBCGVisualManager::GetInstance ()->OnFillTasksGroupInterior (
						pDC, pGroup->m_rectGroup);
				}
				if (!pGroup->m_rect.IsRectEmpty ())
				{
					CBCGVisualManager::GetInstance ()->OnDrawTasksGroupCaption (
						pDC, pGroup, m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
				}
				if (!pGroup->m_rectGroup.IsRectEmpty ())
				{
					CBCGVisualManager::GetInstance ()->OnDrawTasksGroupAreaBorder (
						pDC, pGroup->m_rectGroup, pGroup->m_bIsSpecial, 
						pGroup->m_strName.IsEmpty());

					// --------------
					// Draw all tasks
					// --------------
					for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
					{
						CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
						ASSERT_VALID (pTask);

						if (pTask->m_bVisible && pTask->m_hwndTask == NULL) // the task is not child window
						{
							rgnClipTask.SetRectRgn (&pTask->m_rect);
							pDC->SelectClipRgn (&rgnClipTask);

							CBCGVisualManager::GetInstance ()->OnDrawTask(
								pDC, pTask, &m_lstIcons, (pTask == m_pHotTask));

							pDC->SelectClipRgn (NULL);
						}
					}
				}
			}
			
			else // group is collapsed
			{
				if (!pGroup->m_rect.IsRectEmpty ())
				{
					CBCGVisualManager::GetInstance ()->OnDrawTasksGroupCaption(pDC, pGroup, 
						m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
				}
			}

		}
	}
	
	rgnClipTask.DeleteObject ();

	// ------------------------
	// Draw navigation toolbar: 
	// ------------------------
	CRect rectToolbarOld = m_rectToolbar;
	if (m_bUseNavigationToolbar)
	{
		m_wndToolBar.Invalidate ();
		m_wndToolBar.UpdateWindow ();
	}

	// --------------------
	// Draw scroll buttons:
	// --------------------
	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable ())
		{	
			CBCGVisualManager::GetInstance ()->OnDrawScrollButtons(pDC, m_rectScrollUp,
				iBorderSize, CMenuImages::IdArowUp, m_iScrollMode < 0);
		}

		if (IsScrollDnAvailable ())
		{
			CBCGVisualManager::GetInstance ()->OnDrawScrollButtons(pDC, m_rectScrollDn,
				iBorderSize, CMenuImages::IdArowDown, m_iScrollMode > 0);
		}
	}
}
//******************************************************************************************
void CBCGTasksPane::SetIconsList (HIMAGELIST hIcons)
{
	ASSERT_VALID(this);

	if (m_lstIcons.GetSafeHandle () != NULL)
	{
		m_lstIcons.DeleteImageList ();
	}

	if (hIcons == NULL)
	{
		m_sizeIcon = CSize (0, 0);
	}
	else
	{
		m_lstIcons.Create (CImageList::FromHandle (hIcons));
		::ImageList_GetIconSize (hIcons, (int*) &m_sizeIcon.cx, (int*) &m_sizeIcon.cy);
	}

	AdjustLocations ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}
//******************************************************************************************
BOOL CBCGTasksPane::SetIconsList (UINT uiImageListResID, int cx,
							  COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiImageListResID))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	icons.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	icons.Add (&bmp, clrTransparent);

	SetIconsList (icons);
	return TRUE;
}
//******************************************************************************************
int CBCGTasksPane::AddPage (LPCTSTR lpszPageLabel)
{
	ASSERT(lpszPageLabel != NULL);

	CBCGTasksPanePage* pPage = new CBCGTasksPanePage(lpszPageLabel, this);
	ASSERT_VALID (pPage);

	m_lstTasksPanes.AddTail (pPage);

	RebuildMenu ();
	return (int) m_lstTasksPanes.GetCount() - 1;
}
//******************************************************************************************
void CBCGTasksPane::RemovePage (int nPageIdx)
{
	ASSERT(nPageIdx > 0);
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount ()-1);

	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(posPage != NULL);
	CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pPage);

	//----------------------
	// Reset an active page:
	//----------------------
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	int nOldActivePageIdx = m_arrHistoryStack[m_iActivePage];

	if (m_lstTasksPanes.GetCount () == 1)
	{
		int nOldActivePage = m_iActivePage;
		m_iActivePage = 0;
		ChangeActivePage (0, nOldActivePage);	// Default page
	}
	else if (nOldActivePageIdx >= nPageIdx)
	{
		int nOldActivePage = m_iActivePage;
		m_iActivePage--;
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
	else if (GetSafeHwnd () != NULL)
	{
		RebuildMenu ();

		AdjustLocations ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	//-----------------------------------------------
	// First, remove all tasks groups from this page:
	//-----------------------------------------------
	POSITION pos = NULL;

	for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	//-------------
	// Remove page:
	//-------------
	pos = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(pos != NULL);

	m_lstTasksPanes.RemoveAt (pos);
	delete pPage;

	// ----------------
	// Refresh history:
	// ----------------
	CArray <int, int> arrCopy;
	arrCopy.Copy (m_arrHistoryStack);
	m_arrHistoryStack.RemoveAll ();

	for (int i=0; i < arrCopy.GetSize (); i++)
	{
		if (arrCopy[i] < nPageIdx)
		{
			m_arrHistoryStack.Add (arrCopy[i]);
		}
		else if (arrCopy[i] > nPageIdx)
		{
			m_arrHistoryStack.Add (arrCopy[i]-1);
		}
	}
	RebuildMenu ();
}
//******************************************************************************************
void CBCGTasksPane::RemoveAllPages ()
{
	//----------------------
	// Reset an active page:
	//----------------------
	m_iActivePage = 0;
	ChangeActivePage (0, m_iActivePage);	// Default page
	m_arrHistoryStack.RemoveAll ();
	m_arrHistoryStack.Add (0);

	//--------------------------------------------------------
	// First, remove all tasks group except from default page:
	//--------------------------------------------------------
	for (POSITION pos = m_lstTaskGroups.FindIndex (1); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage != NULL) // except default page
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	//----------------------------------
	// Remove pages except default page:
	//----------------------------------
	while (m_lstTasksPanes.GetCount () > 1)
	{
		delete m_lstTasksPanes.RemoveTail ();
	}
}
//******************************************************************************************
int CBCGTasksPane::AddGroup (int nPageIdx, LPCTSTR lpszGroupName, BOOL bBottomLocation/* = FALSE*/, 
							 BOOL bSpecial/* = FALSE*/, HICON hIcon/* = NULL*/)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx <= m_lstTasksPanes.GetCount ()-1);

	// ---------------
	// Get active page
	// ---------------
	CBCGTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// -------------
	// Add new group
	// -------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (pGroup->m_bIsBottom)
			{
				pGroup->m_bIsBottom = FALSE;
			}
		}
	}

	m_lstTaskGroups.AddTail (new CBCGTasksGroup (lpszGroupName, bBottomLocation, 
		bSpecial, FALSE, pActivePage, hIcon));

	AdjustScroll ();
	ReposTasks ();

	return (int) m_lstTaskGroups.GetCount () - 1;
}
//******************************************************************************************
void CBCGTasksPane::RemoveGroup (int nGroup)
{
	ASSERT (nGroup >= 0);
	ASSERT (nGroup < m_lstTaskGroups.GetCount ());

	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	m_lstTaskGroups.RemoveAt (pos);
	delete pGroup;

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}
//******************************************************************************************
void CBCGTasksPane::RemoveAllGroups (int nPageIdx/* = 0*/)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	if (posPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID (pPage);

	//----------------------------------------
	// Remove all tasks groups from this page:
	//----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}
//******************************************************************************************
BOOL CBCGTasksPane::SetGroupName (int nGroup, LPCTSTR lpszGroupName)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	BOOL	bCaptionWasEmpty	= pGroup->m_strName.IsEmpty();

	pGroup->m_strName	= lpszGroupName;

	if ((!bCaptionWasEmpty && pGroup->m_strName.IsEmpty()) || 
		(bCaptionWasEmpty && !pGroup->m_strName.IsEmpty()))
	{
		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
	}
	else
	{
		InvalidateRect (&pGroup->m_rect);
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGTasksPane::SetGroupTextColor (int nGroup, COLORREF color, COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_clrText = (COLORREF) color;
	pGroup->m_clrTextHot = (COLORREF) colorHot;

	InvalidateRect (&pGroup->m_rect);
	UpdateWindow ();

	return TRUE;
}
//******************************************************************************************
BOOL CBCGTasksPane::CollapseGroup (CBCGTasksGroup* pGroup, BOOL bCollapse)
{
	ASSERT_VALID (pGroup);

	if ((!bCollapse && pGroup->m_bIsCollapsed) ||
		(bCollapse && !pGroup->m_bIsCollapsed))
	{
		pGroup->m_bIsCollapsed	= bCollapse;

		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	}

	return TRUE;
}
//******************************************************************************************
void CBCGTasksPane::CollapseAllGroups (BOOL bCollapse)
{
	// -------------------
	// Collapse all groups
	// -------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if ((!bCollapse && pGroup->m_bIsCollapsed) ||
			(bCollapse && !pGroup->m_bIsCollapsed))
		{
			pGroup->m_bIsCollapsed	= bCollapse;
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
void CBCGTasksPane::CollapseAllGroups (int nPageIdx, BOOL bCollapse)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	if (posPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID (pPage);

	// -----------------------------------------
	// Collapse all groups at the specified page
	// -----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			if ((!bCollapse && pGroup->m_bIsCollapsed) ||
				(bCollapse && !pGroup->m_bIsCollapsed))
			{
				pGroup->m_bIsCollapsed	= bCollapse;
			}
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
CBCGTasksGroup* CBCGTasksPane::GetTaskGroup (int nGroup) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	return pGroup;
}
//******************************************************************************************
BOOL CBCGTasksPane::GetGroupLocation (CBCGTasksGroup* pGroup, int &nGroup) const
{
	ASSERT_VALID (pGroup);

	int nGroupCount = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; nGroupCount++)
	{
		CBCGTasksGroup* pTaskGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pTaskGroup);

		if (pTaskGroup == pGroup)
		{
			nGroup = nGroupCount;
			return TRUE;
		}
	}

	return FALSE; // not found
}
//******************************************************************************************
int CBCGTasksPane::AddTask (int nGroup, LPCTSTR lpszTaskName, int nTaskIcon/* = -1*/,
						UINT uiCommandID/* = 0*/, DWORD dwUserData/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_lstTasks.AddTail (new CBCGTask (
		pGroup, lpszTaskName, nTaskIcon, uiCommandID, dwUserData));

	AdjustScroll ();
	ReposTasks ();

	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//******************************************************************************************
BOOL CBCGTasksPane::SetTaskName (int nGroup, int nTask, LPCTSTR lpszTaskName)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTask*	pTask	= (CBCGTask*) pGroup->m_lstTasks.GetAt (pos);
	pTask->m_strName	= lpszTaskName;

	if (pTask->m_bVisible)
		InvalidateRect (pTask->m_rect);

	return TRUE;
}
//*********************************************************************************
BOOL CBCGTasksPane::SetTaskTextColor (int nGroup, int nTask, COLORREF color,
									   COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTask*	pTask	= (CBCGTask*) pGroup->m_lstTasks.GetAt (pos);
	pTask->m_clrText	= color;
	pTask->m_clrTextHot	= colorHot;

	if (pTask->m_bVisible)
		InvalidateRect (pTask->m_rect);

	return TRUE;
}
//******************************************************************************************
BOOL CBCGTasksPane::ShowTask (int nGroup, int nTask, BOOL bShow)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTask*	pTask	= (CBCGTask*) pGroup->m_lstTasks.GetAt (pos);
	if ((!bShow && pTask->m_bVisible) ||
		(bShow && !pTask->m_bVisible))
	{
		pTask->m_bVisible	= bShow;

		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGTasksPane::ShowTaskByCmdId (UINT uiCommandID, BOOL bShow)
{
	int nGroup, nTask;

	if (!GetTaskLocation (uiCommandID, nGroup, nTask))
		return FALSE;

	return ShowTask (nGroup, nTask, bShow);
}
//******************************************************************************************
BOOL CBCGTasksPane::RemoveTask (int nGroup, int nTask)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());
	
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	delete pGroup->m_lstTasks.GetAt (pos);
	pGroup->m_lstTasks.RemoveAt (pos);

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

	return TRUE;
}
//******************************************************************************************
void CBCGTasksPane::RemoveAllTasks (int nGroup)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	CBCGTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	while (!pGroup->m_lstTasks.IsEmpty ())
	{
		delete pGroup->m_lstTasks.RemoveHead ();
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}
//******************************************************************************************
BOOL CBCGTasksPane::GetTaskLocation (UINT uiCommandID, int& nGroup, int& nTask) const
{
	nGroup	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; ++nGroup)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		nTask	= 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; ++nTask)
		{
			CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask->m_uiCommandID == uiCommandID)
			{
				return TRUE;
			}
		}
	}

	nGroup	= -1;
	nTask	= -1;

	return FALSE;
}
//******************************************************************************************
BOOL CBCGTasksPane::GetTaskLocation (HWND hwndTask, int& nGroup, int& nTask) const
{
	nGroup	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; ++nGroup)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		nTask	= 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; ++nTask)
		{
			CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask->m_hwndTask == hwndTask)
			{
				return TRUE;
			}
		}
	}

	nGroup	= -1;
	nTask	= -1;

	return FALSE;
}
//******************************************************************************************
CBCGTask* CBCGTasksPane::GetTask (int nGroup, int nTask) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	CBCGTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	POSITION pos = pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetAt (pos);
	ASSERT_VALID (pTask);

	return pTask;
}
//******************************************************************************************
BOOL CBCGTasksPane::GetTaskLocation (CBCGTask* pTask, int& nGroup, int& nTask) const
{
	ASSERT_VALID (pTask);
	ASSERT_VALID (pTask->m_pGroup);

	nGroup	= -1;
	nTask	= -1;

	CBCGTasksGroup* pGroupToFind = pTask->m_pGroup;

	int nGroupCount	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; nGroupCount++)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup == pGroupToFind)
		{
			int nTaskCount	= 0;
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; nTaskCount++)
			{
				CBCGTask* pCurTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pCurTask);

				if (pCurTask == pTask)
				{
					nGroup	= nGroupCount;
					nTask	= nTaskCount;
					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}
//******************************************************************************************
int CBCGTasksPane::AddWindow (int nGroup, HWND hwndTask, int nWndHeight, 
							  BOOL bAutoDestroyWindow/* = FALSE*/,
							  DWORD dwUserData/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT (::IsWindow (hwndTask));

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_lstTasks.AddTail (new CBCGTask (
		pGroup, _T(""), -1, 0, dwUserData, hwndTask, bAutoDestroyWindow, nWndHeight));

	AdjustScroll ();
	ReposTasks ();

	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//******************************************************************************************
BOOL CBCGTasksPane::SetWindowHeight (int nGroup, HWND hwndTask, int nWndHeight)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (::IsWindow (hwndTask));

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	POSITION pos2 = pGroup->m_lstTasks.GetHeadPosition();
	while (pos2 != NULL)
	{
		CBCGTask*	pTask	= (CBCGTask*) pGroup->m_lstTasks.GetNext(pos2);

		if (pTask->m_hwndTask == hwndTask)
		{
			pTask->m_nWindowHeight	= nWndHeight;

			if (!pGroup->m_bIsCollapsed)
			{
				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
			}

			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGTasksPane::SetWindowHeight (HWND hwndTask, int nWndHeight)
{
	ASSERT (::IsWindow (hwndTask));

	int	nGroup, nTask;
	if (GetTaskLocation (hwndTask, nGroup, nTask))
	{
		return SetWindowHeight (nGroup, hwndTask, nWndHeight);
	}

	return FALSE;
}
//******************************************************************************************
LRESULT CBCGTasksPane::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	CreateFonts ();

	AdjustScroll ();
	ReposTasks ();
	return 0;
}
//******************************************************************************************
LRESULT CBCGTasksPane::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) (m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//******************************************************************************************
void CBCGTasksPane::CreateFonts ()
{
	if (m_fontBold.GetSafeHandle () != NULL)
	{
		m_fontBold.DeleteObject ();
	}
	if (m_fontUnderline.GetSafeHandle () != NULL)
	{
		m_fontUnderline.DeleteObject ();
	}

	CFont* pFont = CFont::FromHandle (
		m_hFont != NULL ? m_hFont : (HFONT) ::GetStockObject (DEFAULT_GUI_FONT));
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);

	lf.lfWeight = FW_NORMAL;
	lf.lfUnderline = TRUE;
	m_fontUnderline.CreateFontIndirect (&lf);
}
//******************************************************************************************
void CBCGTasksPane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);	

	AdjustLocations ();
	ReposTasks ();
}
//******************************************************************************************
HFONT CBCGTasksPane::SetFont (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), 
		m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//****************************************************************************************
CBCGTask* CBCGTasksPane::TaskHitTest (CPoint pt) const
{
	if (!m_rectTasks.PtInRect (pt))
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CBCGTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// -----------------------------
	// Test all tasks in active page
	// -----------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
			{
				CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pTask);

				if (pTask->m_bVisible && pTask->m_rect.PtInRect (pt))
				{
					if (pTask->m_uiCommandID != 0) // ignore labels
					{
						return pTask;
					}
				}
			}
		}
	}

	return NULL;
}
//***************************************************************************************
CBCGTasksGroup* CBCGTasksPane::GroupCaptionHitTest (CPoint pt) const
{
	if (!m_rectTasks.PtInRect (pt))
	{
		return NULL;
	}
	
	if (!m_bCanCollapse)
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CBCGTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);
	
	// ------------------------------
	// Test all groups in active page
	// ------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (pGroup->m_rect.PtInRect (pt))
			{
				return pGroup;
			}
		}
	}

	return NULL;
}
//***************************************************************************************
BOOL CBCGTasksPane::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	CBCGTask* pTaskHit = TaskHitTest (ptCursor);
	if (m_pClickedTask != NULL && m_pClickedTask->m_bEnabled || 
		pTaskHit != NULL && pTaskHit->m_bEnabled)
	{
		::SetCursor (globalData.m_hcurHand);
		return TRUE;
	}

	if (m_bCanCollapse && (m_pClickedGroupCaption != NULL || 
		GroupCaptionHitTest (ptCursor) != NULL))
	{
		::SetCursor (globalData.m_hcurHand);
		return TRUE;
	}
	
	return CBCGOutlookBar::OnSetCursor(pWnd, nHitTest, message);
}
//******************************************************************************************
void CBCGTasksPane::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGOutlookBar::OnMouseMove(nFlags, point);

	if (m_bUseScrollButtons)
	{
		if (m_rectScrollUp.PtInRect (point) && IsScrollUpAvailable ())
		{
			m_iScrollMode = -1;
			InvalidateRect (m_rectScrollUp);
		}
		else if (m_rectScrollDn.PtInRect (point) && IsScrollDnAvailable ())
		{
			m_iScrollMode = 1;
			InvalidateRect (m_rectScrollDn);
		}
		else
		{
			m_iScrollMode = 0;
		}

		if (m_iScrollMode != 0)
		{
			SetTimer (iScrollTimerId, m_iScrollTimerDuration, NULL);
			return;
		}
	}

	CBCGTasksGroup* pHotGroup = GroupCaptionHitTest (point);
	CBCGTask* pHotTask = TaskHitTest (point);

	// ----------
	// No changes
	// ----------
	if (m_pHotTask == pHotTask && m_pHotGroupCaption == pHotGroup)
	{
		return;
	}

	// ----------------
	// No new hot areas
	// ----------------
	if (pHotTask == NULL && pHotGroup == NULL)
	{
		if (m_pHotGroupCaption != NULL)
		{
			// remove old group caption hotlight
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			if (m_pClickedGroupCaption == NULL)
			{
				ReleaseCapture ();
			}
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		}

		if (m_pHotTask != NULL)
		{
			// remove old task hotlight
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			if (m_pClickedTask == NULL)
			{
				ReleaseCapture ();
			}
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		}

		GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}

	// ---------------------
	// New hot group caption
	// ---------------------
	if (pHotGroup != NULL)
	{
		if (m_pHotGroupCaption == NULL)
		{
			if (GetCapture () != NULL)
			{
				return;
			}
			SetCapture ();
		}
		else
		{	
			// remove old group caption hotlight
			CRect rectTask = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);
		}
		
		// remove old task hotlight
		if (m_pHotTask != NULL)
		{
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		}

		// add new group caption hotlight
		m_pHotGroupCaption = pHotGroup;
		RedrawWindow (pHotGroup->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);
	}
	
	// ------------
	// New hot task
	// ------------
	else if (pHotTask != NULL)
	{
		if (!pHotTask->m_bEnabled)
		{
			GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
			return;
		}

		if (m_pHotTask == NULL)
		{
			if (GetCapture () != NULL)
			{
				return;
			}
			SetCapture ();
		}
		else
		{
			// remove old task hotlight
			CRect rectTask = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);
		}
		
		// remove old group caption hotlight
		if (m_pHotGroupCaption != NULL)
		{
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		}

		// add new task hotlight
		m_pHotTask = pHotTask;
		RedrawWindow (pHotTask->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);

		if (pHotTask->m_uiCommandID != 0)
		{
			ShowCommandMessageString (pHotTask->m_uiCommandID);
		}
	}
}
//******************************************************************************************
void CBCGTasksPane::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBCGOutlookBar::OnLButtonUp(nFlags, point);

	if (m_pHotTask == NULL && m_pClickedTask == NULL && 
		m_pHotGroupCaption == NULL && m_pClickedGroupCaption == NULL)
	{
		return;
	}

	ReleaseCapture ();

	// --------------------------
	// Handle group caption click
	// --------------------------
	CBCGTasksGroup* pHotGroupCaption = m_pHotGroupCaption;
	BOOL bIsGroupCaptionClick = (m_pHotGroupCaption != NULL && 
		m_pHotGroupCaption == GroupCaptionHitTest (point) && 
		m_pClickedGroupCaption == m_pHotGroupCaption);

	m_pHotGroupCaption = NULL;
	m_pClickedGroupCaption = NULL;

	if (bIsGroupCaptionClick)
	{
		pHotGroupCaption->m_bIsCollapsed = !pHotGroupCaption->m_bIsCollapsed;

		if (m_bAnimationEnabled)
		{
			m_pAnimatedGroup = pHotGroupCaption;
			m_sizeAnim = m_pAnimatedGroup->m_rectGroup.Size ();
			
			SetTimer (iAnimTimerId, m_iAnimTimerDuration, NULL);
			m_nLastAnimTime = clock ();
		}

		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
		
		// Trigger mouse move event (to change selection notification):
		SendMessage (WM_MOUSEMOVE, nFlags, MAKELPARAM (point.x, point.y));
		return;
	}
	else
	{
		CRect rectGroupCaption = (m_pHotGroupCaption != NULL) ? 
			m_pHotGroupCaption->m_rect : CRect (0, 0, 0, 0);
		RedrawWindow (rectGroupCaption, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	CRect rectTask = (m_pHotTask != NULL) ? m_pHotTask->m_rect : CRect (0, 0, 0, 0);

	// -----------------
	// Handle task click
	// -----------------
	CBCGTask* pHotTask = m_pHotTask;
	BOOL bIsTaskClick = (m_pHotTask != NULL && m_pHotTask == TaskHitTest (point) &&
		m_pClickedTask == m_pHotTask);

	m_pHotTask = NULL;
	m_pClickedTask = NULL;

	RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);

	if (bIsTaskClick)
	{
		// Find task number and group number:
		ASSERT_VALID (pHotTask->m_pGroup);

		int nTaskNumber = -1;
		int i = 0;

		for (POSITION posTask = pHotTask->m_pGroup->m_lstTasks.GetHeadPosition (); 
			posTask != NULL; i++)
		{
			CBCGTask* pTask = (CBCGTask*) pHotTask->m_pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask == pHotTask)
			{
				nTaskNumber = i;
				break;
			}
		}

		int nGroupNumber = -1;
		i = 0;

		for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; i++)
		{
			CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
			ASSERT_VALID (pGroup);

			if (pHotTask->m_pGroup == pGroup)
			{
				nGroupNumber = i;
				break;
			}
		}

		OnClickTask (nGroupNumber, nTaskNumber, pHotTask->m_uiCommandID, pHotTask->m_dwUserData);
	}
}
//*********************************************************************************
void CBCGTasksPane::Serialize(CArchive& ar)
{
	CBCGOutlookBar::Serialize (ar);

	if (ar.IsLoading ())
	{
		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60000)
		{
			// Load margin settings:
			ar >> m_nVertMargin;
			ar >> m_nHorzMargin;
			ar >> m_nGroupVertOffset;
			ar >> m_nGroupCaptionHeight;
			ar >> m_nGroupCaptionHorzOffset;
			ar >> m_nGroupCaptionVertOffset;
			ar >> m_nTasksHorzOffset;
			ar >> m_nTasksIconHorzOffset;
			ar >> m_nTasksIconVertOffset;

			// Load active page index:
			int nActivePage = 0;
			ar >> nActivePage;
			ASSERT (nActivePage >= 0);
			ASSERT (nActivePage < m_lstTasksPanes.GetCount ());
				
			// Load the titles of pages:
			CStringArray arrPagesNames;
			arrPagesNames.Serialize (ar);
			if (arrPagesNames.GetSize () == m_lstTasksPanes.GetCount ())
			{
				int i = 0;
				POSITION pos = m_lstTasksPanes.GetHeadPosition ();
				while (pos != NULL && i < arrPagesNames.GetSize ())
				{
					CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetNext (pos);
					ASSERT_VALID (pPage);

					pPage->m_strName = arrPagesNames[i++];
				}
			}

			// Change active page:
			SetActivePage (nActivePage);
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60300)
		{
			// Load taskpane's caption:
			ar >> m_strCaption;
			UpdateCaption ();
		}
	}
	else
	{
		// Save margin settings:
		ar << m_nVertMargin;
		ar << m_nHorzMargin;
		ar << m_nGroupVertOffset;
		ar << m_nGroupCaptionHeight;
		ar << m_nGroupCaptionHorzOffset;
		ar << m_nGroupCaptionVertOffset;
		ar << m_nTasksHorzOffset;
		ar << m_nTasksIconHorzOffset;
		ar << m_nTasksIconVertOffset;

		// Save active page index:
		ar << GetActivePage ();

		// Save the titles of pages:
		CStringArray arrPagesNames;
		for (POSITION pos = m_lstTasksPanes.GetHeadPosition (); pos != NULL;)
		{
			CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetNext (pos);
			ASSERT_VALID (pPage);

			arrPagesNames.Add (pPage->m_strName);
		}

		arrPagesNames.Serialize (ar);

		// Save taskpane's caption:
		ar << m_strCaption;
	}
}
//******************************************************************************************
void CBCGTasksPane::OnCancelMode() 
{
	CBCGOutlookBar::OnCancelMode();
	
	if (m_pHotTask != NULL || m_pClickedTask != NULL)
	{
		CRect rectTask = m_pHotTask != NULL ? m_pHotTask->m_rect : CRect (0, 0, 0, 0);
		m_pHotTask = NULL;
		m_pClickedTask = NULL;
		ReleaseCapture ();
		RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	if (m_pHotGroupCaption != NULL || m_pClickedGroupCaption != NULL)
	{
		CRect rectTask = m_pHotGroupCaption != NULL ? 
			m_pHotGroupCaption->m_rect : CRect (0, 0, 0, 0);
		m_pHotGroupCaption = NULL;
		m_pClickedGroupCaption = NULL;
		ReleaseCapture ();
		RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	m_pClickedTask = NULL;
	m_pClickedGroupCaption = NULL;
}
//******************************************************************************************
void CBCGTasksPane::OnClickTask (int nGroupNumber, int nTaskNumber, 
							 UINT uiCommandID, DWORD /*dwUserData*/)
{
	if (uiCommandID != 0)
	{
		GetOwner ()->PostMessage (WM_COMMAND, uiCommandID);

		NotifyAccessibility (nGroupNumber, nTaskNumber);
	}
}
//****************************************************************************************
void CBCGTasksPane::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_pClickedGroupCaption = GroupCaptionHitTest (point);
	m_pClickedTask = TaskHitTest (point);

	CBCGOutlookBar::OnLButtonDown(nFlags, point);
}
//****************************************************************************************

//-----------------------------------------------------
// My "classic " trick - how I can access to protected
// member m_pRecentFileList?
//-----------------------------------------------------
class CBCGApp : public CWinApp
{
	friend class CBCGTasksPane;
};

int CBCGTasksPane::AddMRUFilesList(int nGroup, int nMaxFiles /* = 4 */)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	POSITION posFirstMRUFile = NULL;

	// Clean up old MRU files from the group:
	for (pos = pGroup->m_lstTasks.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (pos);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID >= ID_FILE_MRU_FILE1 &&
			pTask->m_uiCommandID <= ID_FILE_MRU_FILE16)
		{
			posFirstMRUFile = posSave;
			pGroup->m_lstTasks.GetNext (posFirstMRUFile);

			delete pGroup->m_lstTasks.GetAt (posSave);
			pGroup->m_lstTasks.RemoveAt (posSave);
		}
	}

	CRecentFileList* pRecentFileList = 
		((CBCGApp*) AfxGetApp ())->m_pRecentFileList;

	if (pRecentFileList == NULL)
	{
		return (int) pGroup->m_lstTasks.GetCount () - 1;
	}

	int nNum = min (pRecentFileList->GetSize(), nMaxFiles);

	for (int i = 0; i < nNum; i++)
	{
		if ((*pRecentFileList)[i].GetLength() != 0)
		{
			const int MAX_NAME_LEN = 512;

			TCHAR lpcszBuffer [MAX_NAME_LEN + 1];
			memset(lpcszBuffer, 0, MAX_NAME_LEN * sizeof(TCHAR));

			if (GetFileTitle((*pRecentFileList)[i], lpcszBuffer, MAX_NAME_LEN) != 0)
			{
				ASSERT(FALSE);
			}

			CBCGTask* pTask = new CBCGTask (pGroup, lpcszBuffer, -1, ID_FILE_MRU_FILE1 + i);
			ASSERT_VALID (pTask);

			if (posFirstMRUFile == NULL)
			{
				pGroup->m_lstTasks.AddTail (pTask);
			}
			else
			{
				pGroup->m_lstTasks.InsertBefore (posFirstMRUFile, pTask);
			}
		}
	}

	AdjustScroll ();
	ReposTasks ();
	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//*******************************************************************************************
CScrollBar* CBCGTasksPane::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	return (CScrollBar* ) &m_wndScrollVert;
}
//******************************************************************************************
void CBCGTasksPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	int nPrevOffset = m_nVertScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset--;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset++;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= m_nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += m_nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = min (max (0, m_nVertScrollOffset), 
		m_nVertScrollTotal - m_nVertScrollPage + 1);

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	SetScrollPos (SB_VERT, m_nVertScrollOffset);
	
	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}
//*****************************************************************************************
void CBCGTasksPane::SetScrollSizes ()
{
	ASSERT_VALID (this);

	if (m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
		m_nVertScrollOffset = 0;
	}
	else
	{
		int nPageHeight = m_rectTasks.Height ();
		if (m_bUseScrollButtons)
		{
			nPageHeight -= m_iScrollBtnHeight + iBorderSize;
		}
		BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
		if ((m_bUseNavigationToolbar || ForceShowNavToolbar ()) && bMultiPage)
		{
			nPageHeight += m_rectToolbar.Height ();
		}

		m_nVertScrollPage = nPageHeight / m_nRowHeight - 1;

		int nTotalHeight = ReposTasks (TRUE);
		if (nTotalHeight == 0 || nTotalHeight <= nPageHeight)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
			m_nVertScrollOffset = 0;
		}
		else
		{
			m_nVertScrollTotal = nTotalHeight / m_nRowHeight - 1;
		}

		m_nVertScrollOffset = min (max (0, m_nVertScrollOffset), 
			m_nVertScrollTotal - m_nVertScrollPage + 1);
	}


	if (!m_bUseScrollButtons)
	{
		SCROLLINFO si;

		ZeroMemory (&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);

		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_nVertScrollTotal;
		si.nPage = m_nVertScrollPage;
		si.nPos = m_nVertScrollOffset;

		SetScrollInfo (SB_VERT, &si, TRUE);
	}
	m_wndScrollVert.EnableScrollBar (!m_bUseScrollButtons && m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
}
//****************************************************************************************
void CBCGTasksPane::AdjustScroll ()
{
	ASSERT_VALID (this);
	
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rectClient = m_rectWorkArea;

	// --------------------------
	// Adjust navigation toolbar: 
	// --------------------------
	CRect rectToolbarOld = m_rectToolbar;
	BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
	if ((m_bUseNavigationToolbar || ForceShowNavToolbar ()) && bMultiPage)
	{
		int nToolbarHeight = m_wndToolBar.CalcFixedLayout (FALSE, TRUE).cy;

		m_rectToolbar = rectClient;
		m_rectToolbar.bottom = m_rectToolbar.top + nToolbarHeight;

		rectClient.top += m_rectToolbar.Height ();

		m_wndToolBar.SetWindowPos (NULL, m_rectToolbar.left, m_rectToolbar.top, 
									m_rectToolbar.Width (), nToolbarHeight,
									SWP_NOACTIVATE | SWP_NOZORDER);
		m_wndToolBar.ShowWindow (TRUE);
	}
	else
	{
		m_rectToolbar.SetRectEmpty ();
		m_wndToolBar.ShowWindow (FALSE);
	}

	// --------------------
	// Calculate work area:
	// --------------------
	m_rectTasks = rectClient;

	// ------------------
	// Adjust scroll bar:
	// ------------------
	SetScrollSizes ();

	m_wndScrollVert.EnableWindow (!m_bUseScrollButtons);
	if (!m_bUseScrollButtons && m_nVertScrollTotal > 0)
	{
		int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);

		m_rectTasks.right -= cxScroll;

		m_wndScrollVert.SetWindowPos (NULL, rectClient.right - cxScroll, rectClient.top,
			cxScroll, rectClient.Height (), SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
		rectClient.right -= cxScroll;
	}
	else
	{
		m_wndScrollVert.SetWindowPos (NULL, 0, 0,
			0, 0, SWP_HIDEWINDOW);
	}

	// ----------------------
	// Adjust scroll buttons:
	// ----------------------
	CRect rectScrollUpOld = m_rectScrollUp;
	CRect rectScrollDnOld = m_rectScrollDn;

	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();

	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable ())
		{
			m_rectScrollUp = rectClient;
			m_rectScrollUp.top += iBorderSize;
			m_rectScrollUp.bottom = m_rectScrollUp.top + m_iScrollBtnHeight;

			rectClient.top += m_iScrollBtnHeight + iBorderSize;
		}

		if (IsScrollDnAvailable ())
		{
			m_rectScrollDn = rectClient;
			m_rectScrollDn.top = m_rectScrollDn.bottom - m_iScrollBtnHeight;

			rectClient.bottom -= m_iScrollBtnHeight + iBorderSize;
		}

		m_rectTasks = rectClient;
	}
	else if (m_pAnimatedGroup != NULL/* animation is in progress */)
	{
		KillTimer (iScrollTimerId);
		m_iScrollMode = 0;
	}

	// ------------------------------
	// Invalidate navigation toolbar:
	// ------------------------------
	if (rectToolbarOld != m_rectToolbar)
	{
		InvalidateRect (m_rectToolbar);
		InvalidateRect (rectToolbarOld);
		UpdateWindow ();
	}

	// --------------------------
	// Invalidate scroll buttons:
	// --------------------------
	BOOL bScrollButtonsChanged = FALSE;

	if (rectScrollUpOld != m_rectScrollUp)
	{
		InvalidateRect (rectScrollUpOld);
		InvalidateRect (m_rectScrollUp);

		bScrollButtonsChanged = TRUE;
	}

	if (rectScrollDnOld != m_rectScrollDn)
	{
		InvalidateRect (rectScrollDnOld);
		InvalidateRect (m_rectScrollDn);

		bScrollButtonsChanged = TRUE;
	}

	if (bScrollButtonsChanged)
	{
		UpdateWindow ();
	}
}
//****************************************************************************************
void CBCGTasksPane::OnDestroy() 
{
	while (!m_lstTaskGroups.IsEmpty ())
	{
		delete m_lstTaskGroups.RemoveHead ();
	}

	CBCGOutlookBar::OnDestroy();
}
//****************************************************************************************
void CBCGTasksPane::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID(this);

	m_btnOther.SendMessage (WM_CANCELMODE);
	m_btnForward.SendMessage (WM_CANCELMODE);
	m_btnBack.SendMessage (WM_CANCELMODE);

	CBCGOutlookBar::AdjustLocations ();

	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;

	if (bVert && !m_rectCaption.IsRectEmpty () && m_lstTasksPanes.GetCount () > 1 &&
		!m_bUseNavigationToolbar)
	{
		// -----------------
		// Adjust m_btnOther
		// -----------------
		int nOtherBtnX = m_rectCaption.right - 2*m_rectCaption.Height () + 3*BORDER_SIZE;
		int nY = m_rectCaption.top + BORDER_SIZE;
		int nSize = m_rectCaption.Height () - 2*BORDER_SIZE;

		if (nOtherBtnX > m_rectCaption.left)
		{
			CRect rectOther;
			m_btnOther.GetClientRect (&rectOther);
			m_btnOther.MapWindowPoints (this, &rectOther);

			if (rectOther.left != nOtherBtnX || rectOther.top != nY)
			{
				m_btnOther.SetWindowPos (&wndTop, nOtherBtnX, nY, nSize, nSize, SWP_NOACTIVATE);
			}
			m_btnOther.ShowWindow (SW_SHOWNOACTIVATE);
		}
		else
		{
			m_btnOther.ShowWindow (SW_HIDE);
		}

		// ---------------------------------
		// Adjust m_btnBack and m_btnForward
		// ---------------------------------
		int nBackBtnX = m_rectCaption.left + BORDER_SIZE;
		int nForwardBtnX = m_rectCaption.left + nSize + BORDER_SIZE;

		if (nForwardBtnX + nSize + BORDER_SIZE < nOtherBtnX)
		{
			// m_btnBack
			CRect rectBack;
			m_btnBack.GetClientRect (&rectBack);
			m_btnBack.MapWindowPoints (this, &rectBack);

			if (rectBack.left != nBackBtnX || rectBack.top != nY || rectBack.Width () != nSize)
			{
				m_btnBack.SetWindowPos (&wndTop, nBackBtnX, nY, nSize, nSize, SWP_NOACTIVATE);
			}
			m_btnBack.ShowWindow (SW_SHOWNOACTIVATE);
			m_btnBack.EnableWindow(m_iActivePage > 0);
			m_btnBack.SetStdImage (m_iActivePage > 0 ? 
									CMenuImages::IdArowBack : CMenuImages::IdArowBackDsbl);

			// m_btnForward
			CRect rectForward;
			m_btnForward.GetClientRect (&rectForward);
			m_btnForward.MapWindowPoints (this, &rectForward);

			if (rectForward.left != nForwardBtnX || rectForward.top != nY || rectForward.Width () != nSize)
			{
				m_btnForward.SetWindowPos (&wndTop, nForwardBtnX, nY, nSize, nSize, SWP_NOACTIVATE);
			}
			m_btnForward.ShowWindow (SW_SHOWNOACTIVATE);
			m_btnForward.EnableWindow(m_iActivePage < m_arrHistoryStack.GetUpperBound ());
			m_btnForward.SetStdImage (m_iActivePage < m_arrHistoryStack.GetUpperBound () ?
									CMenuImages::IdArowForward : CMenuImages::IdArowForwardDsbl);
		}
		else
		{
			m_btnBack.ShowWindow (SW_HIDE);
			m_btnForward.ShowWindow (SW_HIDE);
		}

	}
	else
	{
		m_btnOther.ShowWindow (SW_HIDE);
		m_btnOther.SetWindowPos (&wndTop, 0, 0, 0, 0, SWP_NOACTIVATE);
		m_btnForward.ShowWindow (SW_HIDE);
		m_btnForward.SetWindowPos (&wndTop, 0, 0, 0, 0, SWP_NOACTIVATE);
		m_btnBack.ShowWindow (SW_HIDE);
		m_btnBack.SetWindowPos (&wndTop, 0, 0, 0, 0, SWP_NOACTIVATE);
	}

	AdjustScroll ();
}
//****************************************************************************************
void CBCGTasksPane::OnDrawCaption (CDC* pDC, CRect rectCaption, CString strTitle, BOOL bHorz)
{
	ASSERT_VALID(pDC);

	CBCGOutlookBar::OnDrawCaption (pDC, rectCaption, strTitle, bHorz);

	if (bHorz)
	{
		CBCGOutlookBar::OnDrawCaption (pDC, rectCaption, strTitle, bHorz);
	}
	else
	{
		pDC->FillRect (rectCaption, &globalData.brBarFace);

		if (!m_bFlatBorder)
		{
			pDC->Draw3dRect (rectCaption, globalData.clrBtnShadow, globalData.clrBtnDkShadow);
			rectCaption.DeflateRect(1, 1);
		}

		pDC->Draw3dRect (rectCaption, globalData.clrBtnHilite, globalData.clrBtnShadow);
		rectCaption.DeflateRect(2, 2);

		// Deflate the rect for the caption
		CRect rectDeflated = rectCaption;
		if (m_lstTasksPanes.GetCount () > 1 && !m_bUseNavigationToolbar)
		{
			rectDeflated.DeflateRect(2*CAPTION_HEIGHT - 2*BORDER_SIZE, 0);
		}

		CPoint ptOrg = CPoint(rectDeflated.left + 3, rectCaption.top);

		pDC->ExtTextOut (ptOrg.x, ptOrg.y,
			ETO_CLIPPED, rectDeflated, strTitle, NULL);
		
		// --------------
		// Redraw buttons
		// --------------
		if (pDC->RectVisible (&m_rectCaption))
		{
			m_btnOther.RedrawWindow();
			m_btnForward.RedrawWindow();
			m_btnBack.RedrawWindow();
		}
	}
}
//****************************************************************************************
BOOL CBCGTasksPane::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (m_bUseNavigationToolbar)
	{
		if (CControlBar::OnCommand (wParam, lParam))
		{
			return TRUE;
		}
	}

	// Find the control send the message:
	HWND hWndCtrl = (HWND)lParam;

	// ---------------------------
	// Handle Other caption button
	// ---------------------------
	if (m_btnOther.GetSafeHwnd () == hWndCtrl)
	{
		if (m_btnOther.m_nMenuResult != 0)
		{
			int nMenuIndex = CBCGPopupMenuBar::GetLastCommandIndex ();
			if (nMenuIndex >= 0)
			{
				SetActivePage (nMenuIndex);
			}
		}
		
		return TRUE;
	}

	// -----------------------------
	// Handle Forward caption button
	// -----------------------------
	if (m_btnForward.GetSafeHwnd () == hWndCtrl)
	{
		ASSERT(LOWORD (wParam) == idForward);

		OnPressForwardButton ();
		return TRUE;
	}
	
	// --------------------------
	// Handle Back caption button
	// --------------------------
	if (m_btnBack.GetSafeHwnd () == hWndCtrl)
	{
		ASSERT(LOWORD (wParam) == idBack);
		
		OnPressBackButton ();
		return TRUE;
	}

	return CBCGOutlookBar::OnCommand (wParam, lParam);
}
//****************************************************************************************
void CBCGTasksPane::RebuildMenu ()
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount ()-1);

	if (m_menuOther.m_hMenu != NULL)
	{
		m_menuOther.DestroyMenu ();
	}

	HMENU hMenu = CreateMenu ();
	m_menuOther.Attach (hMenu);
	m_btnOther.m_hMenu = hMenu;

	m_wndToolBar.UpdateButtons ();
}
//****************************************************************************************
void CBCGTasksPane::SaveHistory (int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount ()-1);

	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount ()-1);
	
	if (nPageIdx == m_arrHistoryStack[m_iActivePage])
	{
		return;
	}

	if (m_iActivePage < m_arrHistoryStack.GetUpperBound ())
	{
		int nStackTailCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
		m_arrHistoryStack.RemoveAt (m_iActivePage+1, nStackTailCount);
	}
	if (m_arrHistoryStack.GetSize () == m_nMaxHistory)
	{
		m_arrHistoryStack.RemoveAt (0);
		if (m_iActivePage > 0)
		{
			m_iActivePage--;
		}
	}
	m_arrHistoryStack.Add (nPageIdx);
}
//****************************************************************************************
void CBCGTasksPane::ChangeActivePage (int nNewPageHistoryIdx, int nOldPageHistoryIdx)
{
	ASSERT(nNewPageHistoryIdx >= 0);
	ASSERT(nNewPageHistoryIdx <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(nOldPageHistoryIdx >= 0);
	ASSERT(nOldPageHistoryIdx <= m_arrHistoryStack.GetUpperBound ());

	int nNewPageIdx = m_arrHistoryStack[nNewPageHistoryIdx];
	int nOldPageIdx = m_arrHistoryStack[nOldPageHistoryIdx];

	ASSERT(nNewPageIdx >= 0);
	ASSERT(nNewPageIdx <= m_lstTasksPanes.GetCount ()-1);
	ASSERT(nOldPageIdx >= 0);
	ASSERT(nOldPageIdx <= m_lstTasksPanes.GetCount ()-1);

	if (nNewPageIdx == nOldPageIdx)
	{
		// Already active, do nothing
		return;
	}

	if (GetSafeHwnd () == NULL)
	{
		OnActivateTasksPanePage ();

		RebuildMenu ();	
		return;
	}

	// ------------------------------------------
	// Hide all windows for previous active page:
	// ------------------------------------------
	CBCGTasksPanePage* pOldPage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (nOldPageIdx);
	ASSERT(posPage != NULL);
	
	pOldPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pOldPage);

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; )
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pOldPage)
		{
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
			{
				CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pTask);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->ShowWindow (SW_HIDE);
				}
			}
		}
	}

	// ------------------
	// Update page title:
	// ------------------
	UpdateCaption ();

	// ------------------
	// Change active page
	// ------------------
	OnActivateTasksPanePage ();

	RebuildMenu ();
	
	m_nVertScrollOffset = 0;
	AdjustLocations ();
	ReposTasks ();
	
	Invalidate ();
	UpdateWindow ();
}
//****************************************************************************************
void CBCGTasksPane::SetActivePage (int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount ());
	

	// ------------------------------------------
	// Activate the page specified by index
	// saving the current one in the history list
	// ------------------------------------------
	if (GetActivePage () != nPageIdx)
	{
		SaveHistory (nPageIdx);
		int nOldActivePage = m_iActivePage;
		m_iActivePage = (int) m_arrHistoryStack.GetUpperBound ();
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
BOOL CBCGTasksPane::GetPageByGroup (int nGroup, int &nPage) const
{
	ASSERT (nGroup >= 0);
	ASSERT (nGroup < m_lstTaskGroups.GetCount ());

	CBCGTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	int nPageCount = 0;
	for (POSITION posPage = m_lstTasksPanes.GetHeadPosition (); posPage != NULL; nPageCount++)
	{
		CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetNext (posPage);
		ASSERT_VALID (pPage);

		if (pPage == pGroup->m_pPage)
		{
			nPage = nPageCount;
			return TRUE;
		}
	}

	ASSERT (FALSE);
	return FALSE;
}
//****************************************************************************************
void CBCGTasksPane::SetCaption (LPCTSTR lpszName)
{
	ASSERT (lpszName != NULL);

	m_strCaption = lpszName;
	SetWindowText (lpszName);

	UpdateCaption ();
}
//****************************************************************************************
void CBCGTasksPane::SetPageCaption (int nPageIdx, LPCTSTR lpszName)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	ASSERT (lpszName != NULL);

	POSITION pos = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT (pos != NULL);
	CBCGTasksPanePage* pPage = (CBCGTasksPanePage*)m_lstTasksPanes.GetAt (pos);
	ASSERT_VALID (pPage);

	pPage->m_strName = lpszName;

	UpdateCaption ();
}
//****************************************************************************************
BOOL CBCGTasksPane::CreateNavigationToolbar ()
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	// ---------------
	// Create toolbar:
	// ---------------
	if (!m_wndToolBar.Create (this, dwDefaultToolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, iNavToolbarId))
	{
		return FALSE;
	}

	m_wndToolBar.SetBarStyle (m_wndToolBar.GetBarStyle () & ~CBRS_GRIPPER);
	m_wndToolBar.SetOwner (this);

	// All commands will be routed via this bar, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame (FALSE);

	CSize sizeNavImage = globalData.Is32BitIcons () ? CSize (16, 16) : CSize (12, 12);
	const int nImageMargin = 4;

	CSize sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);

	// -----------------------
	// Load navigation images:
	// -----------------------
	if (m_uiToolbarBmpRes == 0)
	{
		//----------------------
		// Use default resource:
		//----------------------
		m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

		CBCGLocalResource	lr;
		BOOL bIsLoaded = m_wndToolBar.LoadBitmap (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_TASKPANE32 : IDB_BCGBARRES_TASKPANE, 
			0, 0, TRUE);
		ASSERT (bIsLoaded);
	}
	else
	{
		if (m_sizeToolbarImage != CSize (0, 0))
		{
			sizeNavImage = m_sizeToolbarImage;

			if (m_sizeToolbarButton != CSize (0, 0))
			{
				sizeNavButton = m_sizeToolbarButton;
			}
			else
			{
				sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);
			}
		}

		m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

		BOOL bIsLoaded = m_wndToolBar.LoadBitmap (m_uiToolbarBmpRes, 0, 0, TRUE);
		ASSERT (bIsLoaded);
	}

	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strBack;
	CString strForward;
	CString strHome;
	CString strClose;
	CString strTaskpane;

	{
		CBCGLocalResource locaRes;
		strBack.LoadString (ID_BCGBARRES_TASKPANE_BACK);
		strForward.LoadString (ID_BCGBARRES_TASKPANE_FORWARD);
		strHome.LoadString (ID_BCGBARRES_TASKPANE_HOME);
		strClose.LoadString (ID_BCGBARRES_TASKPANE_CLOSE);
		strTaskpane.LoadString (IDS_BCGBARRES_TASKPANE);
	}

	// --------------------
	// Add toolbar buttons:
	// --------------------
	m_wndToolBar.RemoveAllButtons ();

	if (m_bHistoryMenuButtons)
	{
		// Create drop-down menubutton for the "Back" button:
		CTasksPaneHistoryButton* pBtnBack = new CTasksPaneHistoryButton (
												ID_BCGBARRES_TASKPANE_BACK, 0, strBack);
		m_wndToolBar.m_pBtnBack = pBtnBack;

		if (pBtnBack != NULL)
		{
			m_wndToolBar.InsertButton (pBtnBack);
			pBtnBack->SetMessageWnd (this);
			pBtnBack->OnChangeParentWnd (this);
			pBtnBack->m_bDrawDownArrow = TRUE;
		}

		// Create drop-down menubutton for the "Forward" button:
		CTasksPaneHistoryButton* pBtnForward = new CTasksPaneHistoryButton (
												ID_BCGBARRES_TASKPANE_FORWARD, 1, strForward);
		m_wndToolBar.m_pBtnForward = pBtnForward;

		if (pBtnForward != NULL)
		{
			m_wndToolBar.InsertButton (pBtnForward);
			pBtnForward->SetMessageWnd (this);
			pBtnForward->OnChangeParentWnd (this);
			pBtnForward->m_bDrawDownArrow = TRUE;
		}
	}
	else
	{
		m_wndToolBar.InsertButton (new CBCGToolbarButton (ID_BCGBARRES_TASKPANE_BACK, 0, strBack));
		m_wndToolBar.InsertButton (new CBCGToolbarButton (ID_BCGBARRES_TASKPANE_FORWARD, 1, strForward));
	}

	m_wndToolBar.InsertButton (new CBCGToolbarButton (ID_BCGBARRES_TASKPANE_HOME, 2, strHome));

	m_wndToolBar.InsertSeparator ();

	CTasksPaneMenuButton* pButton = new CTasksPaneMenuButton (m_menuOther.GetSafeHmenu ());

	if (pButton != NULL)
	{
		m_wndToolBar.InsertButton (pButton);
		pButton->m_bText = TRUE;
		pButton->m_bImage = FALSE;
		pButton->m_bLocked = TRUE;
		pButton->m_strText = strTaskpane;
		pButton->OnChangeParentWnd (this);
	}
	
	m_wndToolBar.InsertButton (new CBCGToolbarButton (ID_BCGBARRES_TASKPANE_CLOSE, 3, strClose));

	return TRUE;
}
//****************************************************************************************
void CBCGTasksPane::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case iAnimTimerId:
		if (m_pAnimatedGroup != NULL && m_nRowHeight != 0)
		{
			ASSERT_VALID (m_pAnimatedGroup);

			clock_t nCurrAnimTime = clock ();

			int nDuration = nCurrAnimTime - m_nLastAnimTime;
			int nSteps = (int) (.5 + (float) nDuration / m_iAnimTimerDuration);

			if (m_pAnimatedGroup->m_bIsCollapsed) // collapsing
			{
				m_sizeAnim.cy -= nSteps * m_nRowHeight;
			}
			else // expanding
			{
				m_sizeAnim.cy += nSteps * m_nRowHeight;
			}

			CRect rectUpdate = m_rectTasks;
			rectUpdate.top = m_pAnimatedGroup->m_rect.top - 1;
			InvalidateRect (rectUpdate);

			ReposTasks ();

			rectUpdate = m_rectTasks;
			rectUpdate.top = m_pAnimatedGroup->m_rect.top - 1;
			InvalidateRect (rectUpdate);

			RedrawWindow (NULL, NULL, RDW_ERASE);

			// stop rule:
			if (m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy < 0 ||
				!m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy > m_pAnimatedGroup->m_rectGroup.Height ())
			{
				m_pAnimatedGroup = NULL;
				m_sizeAnim = CSize (0, 0);
			}

			m_nLastAnimTime = nCurrAnimTime;
		}
		else
		{
			KillTimer (iAnimTimerId);
			m_pAnimatedGroup = NULL;
		}
		break;

	case iScrollTimerId:
		{
			CPoint point;
			::GetCursorPos (&point);
			ScreenToClient (&point);

			if (m_rectScrollUp.PtInRect (point) && m_iScrollMode < 0)	// Scroll Up
			{
				m_nVertScrollOffset--;

				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else if (m_rectScrollDn.PtInRect (point) && m_iScrollMode > 0)	// Scroll Down
			{
				m_nVertScrollOffset++;

				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else
			{
				KillTimer (iScrollTimerId);
				m_iScrollMode = 0;
				InvalidateRect (m_rectScrollDn);
				InvalidateRect (m_rectScrollUp);
				UpdateWindow ();
			}
		}
		break;
	}
	
	CBCGOutlookBar::OnTimer(nIDEvent);
}
//****************************************************************************************
void CBCGTasksPane::RecalcLayout (BOOL bRedraw/* = TRUE*/)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	AdjustLocations ();
	ReposTasks ();

	if (bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane idle update through CTaskCmdUI class

class CTaskCmdUI : public CCmdUI        // class private to this file !
{
	// m_nIndex - taskgroup index
	// m_pOther - taskspane pointer

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int /*nCheck*/) {}		// ignore
	virtual void SetRadio(BOOL bOn = TRUE) {bOn;}	// ignore
	virtual void SetText(LPCTSTR lpszText);
};

void CTaskCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGTasksPane* pTasksPane = (CBCGTasksPane*)m_pOther;
	ASSERT(pTasksPane != NULL);
	ASSERT_KINDOF(CBCGTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	// Enable all tasks with uiCommandID in the taskgroup:
	CBCGTasksGroup* pGroup = pTasksPane->GetTaskGroup (m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
	{
		CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_bEnabled != bOn)
			{
				pTask->m_bEnabled = bOn;
				pTasksPane->InvalidateRect (pTask->m_rect);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->EnableWindow (bOn);
				}
			}
		}
	}
}
//*************************************************************************************
void CTaskCmdUI::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	CBCGTasksPane* pTasksPane = (CBCGTasksPane*)m_pOther;
	ASSERT(pTasksPane != NULL);
	ASSERT_KINDOF(CBCGTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	//Remove any amperstands and trailing label (ex.:"\tCtrl+S")
	CString strNewText(lpszText);

	int iOffset = strNewText.Find (_T('\t'));
	if (iOffset != -1)
	{
		strNewText = strNewText.Left (iOffset);
	}

	// Set name for all tasks with uiCommandID in the taskgroup:
	CBCGTasksGroup* pGroup = pTasksPane->GetTaskGroup (m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
	{
		CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_strName != strNewText)		
			{
				pTask->m_strName = strNewText;

				pTasksPane->InvalidateRect (pTask->m_rect);
			}
		}
	}
}
//*************************************************************************************
void CBCGTasksPane::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CTaskCmdUI state;
	state.m_pOther = this;

	// update all tasks:
	state.m_nIndexMax = (UINT)GetGroupCount ();
	state.m_nIndex = 0;
	for (POSITION posGroup = m_lstTaskGroups.GetHeadPosition (); posGroup != NULL; state.m_nIndex++)
	{
		CBCGTasksGroup* pGroup = (CBCGTasksGroup*) m_lstTaskGroups.GetNext (posGroup);
		ASSERT_VALID (pGroup);

		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
		{
			CBCGTask* pTask = (CBCGTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (g_pUserToolsManager != NULL &&
				g_pUserToolsManager->IsUserToolCmd (pTask->m_uiCommandID))
			{
				bDisableIfNoHndler = FALSE;
			}

			//state.m_nIndex == taskgroup index
			state.m_nID = pTask->m_uiCommandID;

			// ignore separators and system commands
			if (pTask->m_uiCommandID != 0 &&
				!IsSystemCommand (pTask->m_uiCommandID) &&
				pTask->m_uiCommandID < AFX_IDM_FIRST_MDICHILD)
			{
				// check for handlers in the target (owner)
				state.DoUpdate(pTarget, bDisableIfNoHndler);
			}
		}
	}

	CBCGOutlookBar::OnUpdateCmdUI (pTarget, bDisableIfNoHndler);
}
//****************************************************************************************
HMENU CBCGTasksPane::CreateMenu () const
{
	// ------------------------------------------------
	// Create popup menu with a list of taskpane pages:
	// ------------------------------------------------
	CMenu menu;
	menu.CreatePopupMenu ();

	for (POSITION pos = m_lstTasksPanes.GetHeadPosition (); pos != NULL;)
	{
		CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetNext (pos);
		ASSERT_VALID (pPage);
		
		menu.AppendMenu (MF_STRING, ID_BCGBARRES_TASKPANE_OTHER, pPage->m_strName);
	}

	HMENU hMenu = menu.Detach ();

	// ------------------------------------
	// Check menu item for the active page:
	// ------------------------------------
	if (hMenu != NULL)
	{
		int iPage = GetActivePage ();
		::CheckMenuItem (hMenu, iPage, MF_BYPOSITION | MF_CHECKED);
	}

	return hMenu;
}
//********************************************************************************
void CBCGTasksPane::EnableNavigationToolbar (BOOL bEnable,
		UINT uiToolbarBmpRes, CSize sizeToolbarImage, CSize sizeToolbarButton)
{
	BOOL bReloadImages = m_wndToolBar.GetSafeHwnd () != NULL &&
		(m_uiToolbarBmpRes != uiToolbarBmpRes);

	m_bUseNavigationToolbar = bEnable;
	m_uiToolbarBmpRes = uiToolbarBmpRes;
	m_sizeToolbarImage = sizeToolbarImage;
	m_sizeToolbarButton = sizeToolbarButton;
	
	m_wndToolBar.m_bLargeIconsAreEnbaled = FALSE;

	if (bReloadImages)
	{
		CSize sizeNavImage = globalData.Is32BitIcons () ? CSize (16, 16) : CSize (12, 12);
		const int nImageMargin = 4;

		CSize sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);

		m_wndToolBar.m_ImagesLocked.Clear ();

		if (m_uiToolbarBmpRes == 0)
		{
			//----------------------
			// Use default resource:
			//----------------------
			m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

			CBCGLocalResource	lr;
			BOOL bIsLoaded = m_wndToolBar.LoadBitmap (
				globalData.Is32BitIcons () ? IDB_BCGBARRES_TASKPANE32 : IDB_BCGBARRES_TASKPANE,
				0, 0, TRUE);
			ASSERT (bIsLoaded);
		}
		else
		{
			if (m_sizeToolbarImage != CSize (0, 0))
			{
				sizeNavImage = m_sizeToolbarImage;

				if (m_sizeToolbarButton != CSize (0, 0))
				{
					sizeNavButton = m_sizeToolbarButton;
				}
				else
				{
					sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);
				}
			}

			m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

			BOOL bIsLoaded = m_wndToolBar.LoadBitmap (m_uiToolbarBmpRes, 0, 0, TRUE);
			ASSERT (bIsLoaded);
		}
	}

	UpdateCaption ();
}
//****************************************************************************************
void CBCGTasksPane::UpdateCaption ()
{
	POSITION pos = m_lstTasksPanes.FindIndex (GetActivePage ());
	ASSERT(pos != NULL);
	
	CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (pos);
	ASSERT_VALID(pPage);

	BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
	if ((m_bUseNavigationToolbar || ForceShowNavToolbar ()) && bMultiPage)
	{
		SetWindowText (m_strCaption);
	}
	else
	{
		SetWindowText(pPage->m_strName);
	}

	m_wndToolBar.UpdateMenuButtonText (pPage->m_strName);
}
//*************************************************************************************
void CBCGTasksPane::OnBack ()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = -1;

		if (m_wndToolBar.m_pBtnBack != NULL)
		{
			ASSERT_VALID (m_wndToolBar.m_pBtnBack);
			if (m_wndToolBar.m_pBtnBack->IsClickedOnMenu ())
			{
				iPage = CBCGPopupMenuBar::GetLastCommandIndex ();
			}
		}

		// Go back
		int nPrevPagesCount = m_iActivePage;
		if (iPage >= 0 && iPage < nPrevPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage -= iPage + 1;
			ChangeActivePage (m_iActivePage, nOldActivePage);
			
			return;
		}
	}

	OnPressBackButton ();
}
//*************************************************************************************
void CBCGTasksPane::OnForward ()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = -1;

		if (m_wndToolBar.m_pBtnForward != NULL)
		{
			ASSERT_VALID (m_wndToolBar.m_pBtnForward);
			if (m_wndToolBar.m_pBtnForward->IsClickedOnMenu ())
			{
				iPage = CBCGPopupMenuBar::GetLastCommandIndex ();
			}
		}

		// Go forward
		int nNextPagesCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
		if (iPage >= 0 && iPage < nNextPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage += iPage + 1;
			ChangeActivePage (m_iActivePage, nOldActivePage);

			return;
		}
	}

	OnPressForwardButton ();
}
//*************************************************************************************
void CBCGTasksPane::OnUpdateBack (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsBackButtonEnabled ());
}
//*************************************************************************************
void CBCGTasksPane::OnUpdateForward (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsForwardButtonEnabled ());
}
//*************************************************************************************
void CBCGTasksPane::OnHome ()
{
	OnPressHomeButton ();
}
//*************************************************************************************
void CBCGTasksPane::OnClose ()
{
	OnPressCloseButton ();
}
//*************************************************************************************
void CBCGTasksPane::OnOther()
{
	// ------------------------------------
	// Handle "Other Task Pane" menubutton:
	// ------------------------------------
	int iPage = CBCGPopupMenuBar::GetLastCommandIndex ();

	ASSERT (iPage >= 0);
	ASSERT (iPage < GetPagesCount());

	SetActivePage (iPage);
}
//****************************************************************************************
void CBCGTasksPane::OnPressBackButton ()
{
	// --------------------------
	// Handle Back caption button
	// --------------------------
	if (m_iActivePage > 0)
	{
		ASSERT (m_iActivePage >= 0);
		ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

		int nOldActivePage = m_iActivePage;
		m_iActivePage--;
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
void CBCGTasksPane::OnPressForwardButton ()
{
	// -----------------------------
	// Handle Forward caption button
	// -----------------------------
	if (m_iActivePage < m_arrHistoryStack.GetUpperBound ())
	{
		ASSERT (m_iActivePage >= 0);
		ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

		int nOldActivePage = m_iActivePage;
		m_iActivePage++;
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
void CBCGTasksPane::OnPressHomeButton ()
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	if (GetActivePage () != 0)
	{
		SetActivePage (0);
	}
}
//****************************************************************************************
void CBCGTasksPane::OnPressCloseButton ()
{
	if (m_bUseNavigationToolbar)
	{
		if (m_pDockSite != NULL)
		{
			m_pDockSite->ShowControlBar(this, FALSE, FALSE); // hide
		}
	}
	else
	{
		// Emulate m_btnClose click:
		SendMessage (WM_COMMAND, MAKEWPARAM (m_nIDClose, BN_CLICKED), 
								(LPARAM) m_btnClose.GetSafeHwnd ());
	}
}
//********************************************************************************
void CBCGTasksPane::GetPreviousPages (CStringList& lstPrevPages) const
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	// -----------------------------------------
	// Collect names list of the previous pages:
	// -----------------------------------------
	lstPrevPages.RemoveAll ();
	const int nCount = m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage - 1 - i];

		POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
		ASSERT(posPage != NULL);

		CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
		ASSERT_VALID(pPage);

		lstPrevPages.AddTail (pPage->m_strName);
	}
}
//********************************************************************************
void CBCGTasksPane::GetNextPages (CStringList& lstNextPages) const
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	// -------------------------------------
	// Collect names list of the next pages:
	// -------------------------------------
	lstNextPages.RemoveAll ();
	const int nCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage + 1 + i];

		POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
		ASSERT(posPage != NULL);

		CBCGTasksPanePage* pPage = (CBCGTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
		ASSERT_VALID(pPage);

		lstNextPages.AddTail (pPage->m_strName);
	}
}
//********************************************************************************
void CBCGTasksPane::EnableHistoryMenuButtons (BOOL bEnable)
{
	if (m_bHistoryMenuButtons == bEnable)
	{
		return;
	}

	BOOL bRecreateToolBar = FALSE;

	if (m_wndToolBar.GetSafeHwnd () != NULL)
	{
		bRecreateToolBar = TRUE;
		m_wndToolBar.DestroyWindow ();
	}

	m_bHistoryMenuButtons = bEnable;

	if (bRecreateToolBar)
	{
		CreateNavigationToolbar ();
		m_wndToolBar.UpdateButtons ();
	}
}

//********************************************************************************
LRESULT CBCGTasksPane::OnGetObject (WPARAM wParam, LPARAM lParam)
{ 	
	if (lParam != OBJID_CLIENT || !globalData.IsAccessibilitySupport ())
	{
		return (LRESULT)0L;
	}

	if (m_pAccTaskInfo == NULL)
	{
		return (LRESULT)0L;
	}

	if (m_pAccTaskInfo->m_nTask == -1 || m_pAccTaskInfo->m_nTaskGroup == -1)
	{
		return (LRESULT)0L;
	}

	CWnd* pWndMain = BCGGetTopLevelFrame (this);
	ASSERT_VALID (pWndMain);
	if (pWndMain != NULL)
	{
		BCGACCDATA accData;
		accData.m_wParam = wParam;
		accData.m_lParam = lParam;
		accData.m_nObjType = ACC_TASKPANE;
		accData.m_objData = (void*)m_pAccTaskInfo;
		accData.m_pWnd = this;
		pWndMain->SendMessage (BCGM_ACCGETOBGECT, (WPARAM)&accData);

		return globalData.GetAccObjectRes (&accData, m_pAccessible);
	}

	return (LRESULT)0L;
}
//********************************************************************************
void CBCGTasksPane::NotifyAccessibility (int nGroupNumber, int nTaskNumber)
{
	if (!globalData.IsAccessibilitySupport ())
	{
		return;
	}

	if (nGroupNumber == -1 || nGroupNumber == -1)
	{
		return;
	}

	if (m_pAccTaskInfo == NULL)
	{
		m_pAccTaskInfo = new BCGACC_TASKINFO;
	}
	
	m_pAccTaskInfo->m_nTask =  nTaskNumber;
	m_pAccTaskInfo->m_nTaskGroup = nGroupNumber;
	
	CBCGTask* pTask = GetTask (nGroupNumber, nTaskNumber);
	ASSERT_VALID (pTask);
	m_pAccTaskInfo->m_strText = pTask->m_strName;
	
	::NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd (), 
		OBJID_CLIENT , CHILDID_SELF);
}