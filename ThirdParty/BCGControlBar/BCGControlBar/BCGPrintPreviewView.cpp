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
//
//********************************************************************

#include "stdafx.h"
#include "BCGPrintPreviewView.h"
#include "bcgbarres.h"
#include "bcglocalres.h"

IMPLEMENT_DYNCREATE(CBCGPrintPreviewView, CPreviewView)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iSimplePaneIndex = 255;

/////////////////////////////////////////////////////////////////////////////
// CBCGPrintPreviewToolBar

BEGIN_MESSAGE_MAP(CBCGPrintPreviewToolBar, CBCGToolBar)
	//{{AFX_MSG_MAP(CBCGPrintPreviewToolBar)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPrintPreviewToolBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
	// Prevent print preview toolbar context menu appearing
}

INT_PTR CBCGPrintPreviewToolBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CBCGLocalResource locaRes;
	return CBCGToolBar::OnToolHitTest (point, pTI);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPrintPreviewView

CBCGPrintPreviewView::CBCGPrintPreviewView()
{
	m_iPagesBtnIndex = -1;
	m_iOnePageImageIndex = -1;
	m_iTwoPageImageIndex = -1;
	m_pWndStatusBar = NULL;
	m_bIsStatusBarSimple = FALSE;
	m_nSimpleType = 0;
	m_nCurrentPage = 1;
}
//*********************************************************************************
CBCGPrintPreviewView::~CBCGPrintPreviewView()
{
	if (m_pWndStatusBar != NULL)
	{
		CStatusBarCtrl& statusBar = m_pWndStatusBar->GetStatusBarCtrl ();
		
		//----------------------------------
		// Restore previous StatusBar state:
		//----------------------------------
		statusBar.SetText (NULL, iSimplePaneIndex, m_nSimpleType);
		statusBar.SetSimple (m_bIsStatusBarSimple);
	}
}


BEGIN_MESSAGE_MAP(CBCGPrintPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CBCGPrintPreviewView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_NUMPAGE, OnUpdatePreviewNumPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPrintPreviewView message handlers

int CBCGPrintPreviewView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPreviewView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGLocalResource locaRes;

	ASSERT_VALID (m_pToolBar);

	const UINT uiToolbarHotID = globalData.Is32BitIcons () ? IDR_BCGRES_PRINT_PREVIEW32 : 0;

	if (!m_wndToolBar.Create (m_pToolBar,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_BORDER_3D) ||
		!m_wndToolBar.LoadToolBar(	IDR_BCGRES_PRINT_PREVIEW, 0, 0, TRUE /* Locked */, 
									0, 0, uiToolbarHotID))
	{
		TRACE0("Failed to create print preview toolbar\n");
		return FALSE;      // fail to create
	}

	//-------------------------------------------
	// Remember One Page/Two pages image indexes:
	//-------------------------------------------
	m_iPagesBtnIndex = m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_NUMPAGE);
	ASSERT (m_iPagesBtnIndex >= 0);
	
	CBCGToolbarButton* pButton= m_wndToolBar.GetButton (m_iPagesBtnIndex);
	ASSERT_VALID (pButton);

	m_iOnePageImageIndex = pButton->GetImage ();

	int iIndex = m_wndToolBar.CommandToIndex (ID_BCGRES_TWO_PAGES_DUMMY);
	ASSERT (iIndex >= 0);
	
	pButton= m_wndToolBar.GetButton (iIndex);
	ASSERT_VALID (pButton);

	m_iTwoPageImageIndex = pButton->GetImage ();

	//---------------------------------
	// Remove dummy "Two pages" button:
	//---------------------------------
	m_wndToolBar.RemoveButton (iIndex);

	//------------------------------------
	// Set "Print" button to image + text:
	//------------------------------------
	m_wndToolBar.SetToolBarBtnText (m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_PRINT));

	//---------------------------------
	// Set "Close" button to text only:
	//---------------------------------
	m_wndToolBar.SetToolBarBtnText (m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_CLOSE),
		NULL, TRUE, FALSE);

	//-------------------------
	// Change the Toolbar size:
	//-------------------------
	SetToolbarSize ();

	//-------------------------------------------
	// Set Application Status Bar to Simple Text:
	//-------------------------------------------
	CFrameWnd* pParentFrame = GetParentFrame ();
	ASSERT_VALID (pParentFrame);

	m_pWndStatusBar = DYNAMIC_DOWNCAST (CStatusBar,
		pParentFrame->GetDlgItem (AFX_IDW_STATUS_BAR));

	if (m_pWndStatusBar != NULL)
	{
		CStatusBarCtrl& wndStatusBar = m_pWndStatusBar->GetStatusBarCtrl ();
		
		//------------------------------
		// Save current StatusBar state:
		//------------------------------
		m_bIsStatusBarSimple = wndStatusBar.IsSimple ();
		wndStatusBar.GetTextLength (iSimplePaneIndex, &m_nSimpleType);

		//-------------------------------------
		// Set Simple Pane Style to No Borders:
		//-------------------------------------
		wndStatusBar.SetText (NULL, iSimplePaneIndex, SBT_NOBORDERS);

		//-------------------------------
		// Set status bar to simple mode:
		//-------------------------------
		wndStatusBar.SetSimple ();
	}

	return 0;
}
//*********************************************************************************
void CBCGPrintPreviewView::OnUpdatePreviewNumPage(CCmdUI *pCmdUI) 
{
	CPreviewView::OnUpdateNumPageChange(pCmdUI);

	//--------------------------------------------------
	// Change the Icon of AFX_ID_PREVIEW_NUMPAGE button:
	//--------------------------------------------------
	CBCGToolbarButton* pButton = m_wndToolBar.GetButton (m_iPagesBtnIndex);
	ASSERT_VALID (pButton);

	UINT nPages = m_nZoomState == ZOOM_OUT ? m_nPages : m_nZoomOutPages;
	pButton->SetImage (nPages == 1 ? m_iTwoPageImageIndex : m_iOnePageImageIndex);

	m_wndToolBar.InvalidateRect (pButton->Rect ());
}
//*********************************************************************************
void CBCGPrintPreviewView::OnDisplayPageNumber (UINT nPage, UINT nPagesDisplayed)
{
	ASSERT (m_pPreviewInfo != NULL);

	CFrameWnd* pParentFrame = GetParentFrame ();
	ASSERT_VALID (pParentFrame);

	int nSubString = (nPagesDisplayed == 1) ? 0 : 1;

	CString s;
	if (AfxExtractSubString (s, m_pPreviewInfo->m_strPageDesc, nSubString))
	{
		CString strPage;

		if (nSubString == 0)
		{
			strPage.Format (s, nPage);
		}
		else
		{
			UINT nEndPage = nPage + nPagesDisplayed - 1;
			strPage.Format (s, nPage, nEndPage);
		}

		if (m_pWndStatusBar != NULL)
		{
			m_pWndStatusBar->GetStatusBarCtrl().SetText (
				strPage, iSimplePaneIndex, SBT_NOBORDERS);
		}
		else
		{
			pParentFrame->SendMessage (WM_SETMESSAGESTRING, 0, 
										(LPARAM)(LPCTSTR) strPage);
		}
	}
	else
	{
		TRACE1("Malformed Page Description string. Could not get string %d.\n",
			nSubString);
	}
}
//*********************************************************************************
BCGCONTROLBARDLLEXPORT void BCGPrintPreview (CView* pView)
{
	ASSERT_VALID (pView);

	CPrintPreviewState *pState= new CPrintPreviewState;

	CBCGLocalResource locaRes;

	if (!pView->DoPrintPreview (IDD_BCGBAR_RES_PRINT_PREVIEW, pView, 
		RUNTIME_CLASS (CBCGPrintPreviewView), pState))
	{
		TRACE0("Error: OnFilePrintPreview failed.\n");
		AfxMessageBox (AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
	}
}
//*******************************************************************************
void CBCGPrintPreviewView::OnSize(UINT nType, int cx, int cy) 
{
	CPreviewView::OnSize(nType, cx, cy);
	
	//-------------------------
	// Change the Toolbar size:
	//-------------------------
	SetToolbarSize ();
}
//******************************************************************************
void CBCGPrintPreviewView::SetToolbarSize ()
{
	ASSERT_VALID (m_pToolBar);

	CSize szSize = m_wndToolBar.CalcFixedLayout (TRUE, TRUE);

	//----------------------------------------------------------------------
	// Print toolbar should occupy the whole width of the mainframe (Win9x):
	//----------------------------------------------------------------------
	CFrameWnd* pParent = GetParentFrame ();
	ASSERT_VALID (pParent);

	CRect rectParent;
	pParent->GetClientRect (rectParent);
	szSize.cx = rectParent.Width ();

	m_wndToolBar.SetWindowPos (NULL, 0, 0, szSize.cx, szSize.cy, 
				SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOZORDER);

	//----------------------------------------------------
	// Adjust parent toolbar (actually - dialog bar) size:
	//----------------------------------------------------
	m_pToolBar->m_sizeDefault.cy = szSize.cy;

	pParent->RecalcLayout();            // position and size everything
	pParent->UpdateWindow();
}
