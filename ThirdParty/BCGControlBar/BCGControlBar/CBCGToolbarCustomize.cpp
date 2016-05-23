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

// CBCGToolbarCustomize.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "afxpriv.h"
#include "bcgbarres.h"
#include "CBCGToolbarCustomize.h"
#include "BCGToolBar.h"
#include "BCGMenuBar.h"
#include "bcglocalres.h"
#include "BCGMouseManager.h"
#include "BCGContextMenuManager.h"
#include "BCGTearOffManager.h"
#include "ButtonsTextList.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGKeyboardManager.h"
#include "BCGUserToolsManager.h"
#include "BCGToolbarMenuButton.h"
#include "BCGDockBar.h"
#include "BCGUserTool.h"
#include "BCGWorkspace.h"
#include "BCGHelpIds.h"
#include "BCGVisualManager.h"
#include "BCGSkinManager.h"
#include "ImageEditDlg.h"
#include "BCGTasksPane.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CBCGToolbarCustomize*	g_pWndCustomize = NULL;
extern CBCGWorkspace*	g_pWorkspace;

static const int iButtonMargin = 8;

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarCustomize

IMPLEMENT_DYNAMIC(CBCGToolbarCustomize, CPropertySheet)

CBCGToolbarCustomize::CBCGToolbarCustomize(
	CFrameWnd* pWndParentFrame,
	BOOL bAutoSetFromMenus /* = FALSE */,
	UINT uiFlags /* = 0xFFFF */,
	CList<CRuntimeClass*,CRuntimeClass*>*	plistCustomPages /* = NULL */)
	 : CPropertySheet(_T(""), pWndParentFrame),
	 m_bAutoSetFromMenus (bAutoSetFromMenus),
	 m_uiFlags (uiFlags)
{

	if((m_uiFlags & BCGCUSTOMIZE_MENUAMPERS) == 0)
	{
		m_bSaveMenuAmps = FALSE;
		
	}else
	{
		m_bSaveMenuAmps = TRUE;
	}

	//-------------------------------
	// ET: Add optional custom pages:
	//-------------------------------
	if (plistCustomPages != NULL) 
	{
		// does not use lib local resources, so moved to front
		ASSERT_VALID(plistCustomPages);
		for(POSITION pos=plistCustomPages->GetHeadPosition(); pos; )
		{
			CRuntimeClass* pClass = plistCustomPages->GetNext(pos);
			m_listCustomPages.AddTail( (CPropertyPage*) pClass->CreateObject() );
		}
	}

	if (!CBCGVisualManager::GetInstance ()->IsLook2000Allowed ())
	{
		m_uiFlags &= ~BCGCUSTOMIZE_LOOK_2000;
	}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager == NULL)
	{
		ASSERT ((m_uiFlags & BCGCUSTOMIZE_SELECT_SKINS) == 0);
		m_uiFlags &= ~BCGCUSTOMIZE_SELECT_SKINS;
	}
#else
	m_uiFlags &= ~BCGCUSTOMIZE_SELECT_SKINS;
#endif

	CBCGLocalResource locaRes;

	ASSERT (pWndParentFrame != NULL);
	m_pParentFrame = pWndParentFrame;

	m_pCustomizePage = new CBCGCustomizePage;
	m_pToolbarsPage = new CBCGToolbarsPage (m_pParentFrame);
	m_pToolsPage = new CBCGToolsPage ();
	m_pKeyboardPage = new CBCGKeyboardPage (m_pParentFrame, m_bAutoSetFromMenus);
	m_pMenuPage = new CBCGMenuPage (m_pParentFrame, m_bAutoSetFromMenus);
	m_pMousePage = new CBCGMousePage;

	//---------------------------------------
	// Add two main pages (available always):
	//---------------------------------------
	AddPage (m_pCustomizePage);
	AddPage (m_pToolbarsPage);

	//----------------
	// Add tools page:
	//----------------
	if (g_pUserToolsManager != NULL)
	{
		AddPage (m_pToolsPage);
	}

	//---------------------------------------------------------
	// Add keyboard customization page (available only if
	// the main application windows accelerator table is exist):
	//---------------------------------------------------------
	if (g_pKeyboardManager != NULL && pWndParentFrame->m_hAccelTable != NULL)
	{
		AddPage (m_pKeyboardPage);
	}

	//---------------------------------------------------------------
	// Add menu customization page (available only if the menu bar or
	// context menu manager are available):
	//---------------------------------------------------------------
	BOOL bMenuBarIsAvailable = FALSE;

	CBCGMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, m_pParentFrame);
	if (pMainMDIFrame != NULL)
	{
		bMenuBarIsAvailable = (pMainMDIFrame->IsMenuBarAvailable ());
	}
	else
	{
		CBCGFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, m_pParentFrame);
		if (pMainFrame != NULL)
		{
			bMenuBarIsAvailable = (pMainFrame->IsMenuBarAvailable ());
		}
	}

	if (g_pContextMenuManager != NULL || bMenuBarIsAvailable)
	{	
		AddPage (m_pMenuPage);
	}

	if (g_pMouseManager != NULL)
	{
		AddPage (m_pMousePage);
	}

	//-------------------------------
	// ET: Add optional custom pages:
	//-------------------------------
	for(POSITION pos=m_listCustomPages.GetHeadPosition(); pos; )
	{
		AddPage( m_listCustomPages.GetNext(pos) );
	}

	m_pOptionsPage = new CBCGOptionsPage (bMenuBarIsAvailable);
	AddPage (m_pOptionsPage);

	//----------------------------
	// Set property sheet caption:
	//----------------------------
	CString strCaption;
	strCaption.LoadString (IDS_BCGBARRES_PROPSHT_CAPTION);

	m_strAllCommands.LoadString (IDS_BCGBARRES_ALL_COMMANDS);
	m_pCustomizePage->SetAllCategory (m_strAllCommands);

	if (m_pKeyboardPage != NULL)
	{
		m_pKeyboardPage->SetAllCategory (m_strAllCommands);
	}

	SetTitle (strCaption);

	if (m_bAutoSetFromMenus)
	{
		SetupFromMenus ();
	}

	//-------------------------
	// Add a "New menu" button:
	//-------------------------
	CString strNewMenu;
	strNewMenu.LoadString (IDS_BCGBARRES_NEW_MENU);

	AddButton (strNewMenu, CBCGToolbarMenuButton (0, NULL, -1, strNewMenu));
}
//**************************************************************************************
CBCGToolbarCustomize::~CBCGToolbarCustomize()
{
	POSITION pos = m_ButtonsByCategory.GetStartPosition();
	while (pos != NULL)
	{
		CObList* pCategoryButtonsList;
		CString string;
		
		m_ButtonsByCategory.GetNextAssoc (pos, string, pCategoryButtonsList);
		ASSERT_VALID(pCategoryButtonsList);

		while (!pCategoryButtonsList->IsEmpty ())
		{
			delete pCategoryButtonsList->RemoveHead ();
		}

		delete pCategoryButtonsList;
	}

	m_ButtonsByCategory.RemoveAll();

	delete m_pCustomizePage;
	delete m_pToolbarsPage;
	delete m_pKeyboardPage;
	delete m_pMenuPage;
	delete m_pMousePage;
	delete m_pOptionsPage;

	if (m_pToolsPage != NULL)
	{
		delete m_pToolsPage;
	}

	//--------------------------------------
	// ET: delete all optional custom pages:
	//--------------------------------------
	while(!m_listCustomPages.IsEmpty())
	{
		delete m_listCustomPages.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(CBCGToolbarCustomize, CPropertySheet)
	//{{AFX_MSG_MAP(CBCGToolbarCustomize)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarCustomize message handlers
int CBCGToolbarCustomize::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	if (m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP)
	{
		ModifyStyleEx (0, WS_EX_CONTEXTHELP);
	}

	g_pWndCustomize = this;
	return 0;
}
//**************************************************************************************
void CBCGToolbarCustomize::PostNcDestroy()
{
	g_pWndCustomize = NULL;
	SetFrameCustMode (FALSE);

	CPropertySheet::PostNcDestroy();
	delete this;
}
//**************************************************************************************
void CBCGToolbarCustomize::AddButton (UINT uiCategoryId, const CBCGToolbarButton& button,
				int iInsertAfter)
{
	CString strCategory;
	strCategory.LoadString (uiCategoryId);

	AddButton (strCategory, button, iInsertAfter);
}
//**************************************************************************************
void CBCGToolbarCustomize::AddButton (LPCTSTR lpszCategory, const CBCGToolbarButton& button,
										int iInsertAfter)
{
	int iId = (int) button.m_nID;

	if (IsStandardCommand (iId))
	{
		//-------------------------------------------
		// Don't add MRU, system and Window commands:
		//-------------------------------------------
		return;
	}

	if (!CBCGToolBar::IsCommandPermitted (button.m_nID))
	{
		return;
	}

	CString strText = button.m_strText;
	strText.TrimLeft ();
	strText.TrimRight ();

	BOOL bToolBtn = FALSE;

	if (g_pUserToolsManager != NULL &&
		g_pUserToolsManager->IsUserToolCmd (iId))
	{
		CBCGUserTool* pTool = g_pUserToolsManager->FindTool (iId);

		if (pTool == NULL)
		{
			//------------------------------
			// Undefined user tool, skip it
			//------------------------------
			return;
		}

		ASSERT_VALID (pTool);

		//------------------------
		// Use tool name as label:
		//------------------------
		strText = pTool->m_strLabel;
		bToolBtn = TRUE;
	}

	if (strText.IsEmpty ())
	{
		//-------------------------------------------
		// Try to find the command name in resources:
		//-------------------------------------------
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString (button.m_nID) &&
			(iOffset = strMessage.Find (_T('\n'))) != -1)
		{
			strText = strMessage.Mid (iOffset + 1);
		}

		if (strText.IsEmpty () && lpszCategory == m_strAllCommands)
		{
			return;
		}
	}
	else
	{
		if(!m_bSaveMenuAmps)
		{
			strText.Remove (_T('&'));
		}

		//----------------------------------------
		// Remove trailing label (ex.:"\tCtrl+S"):
		//----------------------------------------
		int iOffset = strText.Find (_T('\t'));
		if (iOffset != -1)
		{
			strText = strText.Left (iOffset);
		}
	}

	//---------------------------------------------------------------
	// If text is still empty, assume dummy command and don't add it:
	//---------------------------------------------------------------
	if (strText.IsEmpty ())
	{
		return;
	}
	
	//--------------------------------------------------
	// Find a category entry or create new if not exist:
	//--------------------------------------------------
	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		//--------------------------------
		// Category not found! Create new:
		//--------------------------------
		pCategoryButtonsList = new CObList;
		m_ButtonsByCategory.SetAt (lpszCategory, pCategoryButtonsList);

		if (lpszCategory != m_strAllCommands)
		{
			m_strCategoriesList.AddTail (lpszCategory);
		}
	}
	else
	{
		//--------------------------------------------------------
		// Category is not a new. Maybe the button is exist also?
		//--------------------------------------------------------
		ASSERT (pCategoryButtonsList != NULL);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
		{
			CBCGToolbarButton* pButton = (CBCGToolbarButton*) pCategoryButtonsList->GetNext (pos);
			ASSERT (pButton != NULL);

			if ((pButton->m_nID == button.m_nID && pButton->m_nID != (UINT) -1) 
				|| (pButton->m_nID == (UINT) -1 && pButton->m_strText == button.m_strText))
				// The same exist... 
			{
				if (pButton->m_strText.IsEmpty ())
				{
					pButton->m_strText = button.m_strText;
				}

				return;
			}
		}
	}

	//-------------------------------------------------------------------
	// Create a new CBCGToolbarButton object (MFC class factory is used):
	//-------------------------------------------------------------------
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	CBCGToolbarButton* pButton = (CBCGToolbarButton*) pClass->CreateObject ();
	ASSERT_VALID (pButton);

	pButton->CopyFrom (button);
	pButton->m_strText = strText;

	if (bToolBtn)
	{
		pButton->SetImage (0);
	}

	//-------------------------------------------
	// Add a new button to the specific category:
	//-------------------------------------------
	BOOL bInserted = FALSE;
	if (iInsertAfter != -1)
	{
		POSITION pos = pCategoryButtonsList->FindIndex (iInsertAfter);
		if (pos != NULL)
		{
			pCategoryButtonsList->InsertBefore (pos, pButton);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		pCategoryButtonsList->AddTail (pButton);
	}

	if (lpszCategory != m_strAllCommands)
	{
		AddButton (m_strAllCommands, button);
	}

	pButton->OnAddToCustomizePage ();
}
//**************************************************************************************
int CBCGToolbarCustomize::RemoveButton (UINT uiCategoryId, UINT uiCmdId)
{
	if (uiCategoryId == (UINT) -1)	// Remove from ALL caregories
	{
		BOOL bFinish = FALSE;
		for (POSITION posCategory = m_strCategoriesList.GetHeadPosition();
			!bFinish;)
		{
			CString strCategory;
			if (posCategory == NULL)
			{
				strCategory = m_strAllCommands;
				bFinish = TRUE;
			}
			else
			{
				strCategory = m_strCategoriesList.GetNext (posCategory);
			}

			RemoveButton (strCategory, uiCmdId);
		}

		return 0;
	}

	CString strCategory;
	strCategory.LoadString (uiCategoryId);

	return RemoveButton (strCategory, uiCmdId);
}
//**************************************************************************************
int CBCGToolbarCustomize::RemoveButton (LPCTSTR lpszCategory, UINT uiCmdId)
{
	ASSERT (lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		// Category not found!
		return -1;
	}

	int i = 0;
	for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL; i ++)
	{
		POSITION posSave = pos;

		CBCGToolbarButton* pButton = (CBCGToolbarButton*) pCategoryButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == uiCmdId)
		{
			pCategoryButtonsList->RemoveAt (posSave);
			delete pButton;
			return i;
		}
	}

	return -1;
}
//**************************************************************************************
BOOL CBCGToolbarCustomize::AddToolBar (UINT uiCategory, UINT uiToolbarResId)
{
	CString strCategory;
	strCategory.LoadString (uiCategory);

	return AddToolBar (strCategory, uiToolbarResId);
}
//**************************************************************************************
BOOL CBCGToolbarCustomize::AddToolBar (LPCTSTR lpszCategory, UINT uiToolbarResId)
{
	struct CToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;

		WORD* items()
			{ return (WORD*)(this+1); }
	};

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiToolbarResId);
	ASSERT(lpszResourceName != NULL);

	//---------------------------------------------------
	// determine location of the bitmap in resource fork:
	//---------------------------------------------------
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
	{
		TRACE(_T("CBCGToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
	{
		TRACE(_T("CBCGToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
	{
		TRACE(_T("CBCGToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		::FreeResource (hGlobal);
		return FALSE;
	}

	ASSERT (pData->wVersion == 1);

	for (int i = 0; i < pData->wItemCount; i++)
	{
		UINT uiCmd = pData->items() [i];
		if (uiCmd > 0 && uiCmd != (UINT) -1)
		{
			AddButton (lpszCategory, CBCGToolbarButton (uiCmd, -1));
		}
	}

	::UnlockResource (hGlobal);
	::FreeResource (hGlobal);

	return TRUE;
}
//**************************************************************************************
BOOL CBCGToolbarCustomize::AddMenu (UINT uiMenuResId)
{
	CMenu menu;
	if (!menu.LoadMenu (uiMenuResId))
	{
		TRACE(_T("CBCGToolbarCustomize::AddMenu: Can't load menu %x\n"), uiMenuResId);
		return FALSE;
	}

	AddMenuCommands (&menu, FALSE);
	return TRUE;
}
//**************************************************************************************
//ET: Rename automatically imported categories (e.g. "?"->"Help")
BOOL CBCGToolbarCustomize::RenameCategory(LPCTSTR lpszCategoryOld, LPCTSTR lpszCategoryNew)
{
	// New Name must not be present
	POSITION pos = m_strCategoriesList.Find(lpszCategoryNew);
	if(pos)
		return FALSE;

	// ...but the old one must be
	pos = m_strCategoriesList.Find(lpszCategoryOld);
	if(!pos)
		return FALSE;

	// Change Name in Button-map too:
	CObList* pCategoryButtonsList;

	// new Category must not be present yet
	if (m_ButtonsByCategory.Lookup (lpszCategoryNew, pCategoryButtonsList))
		return FALSE;

	// ...but the old one must be
	if (!m_ButtonsByCategory.Lookup (lpszCategoryOld, pCategoryButtonsList))
		return FALSE;

	// change both or nothing
	m_strCategoriesList.SetAt(pos, lpszCategoryNew);
	m_ButtonsByCategory.RemoveKey(lpszCategoryOld);
	m_ButtonsByCategory.SetAt(lpszCategoryNew, pCategoryButtonsList);

	return TRUE;
}
//**************************************************************************************
void CBCGToolbarCustomize::ReplaceButton (UINT uiCmd, const CBCGToolbarButton& button)
{
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	BOOL bFinish = FALSE;
	for (POSITION posCategory = m_strCategoriesList.GetHeadPosition();
		!bFinish;)
	{
		CString strCategory;
		if (posCategory == NULL)
		{
			strCategory = m_strAllCommands;
			bFinish = TRUE;
		}
		else
		{
			strCategory = m_strCategoriesList.GetNext (posCategory);
		}

		CObList* pCategoryButtonsList;
		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
		{
			POSITION posSave = pos;
			CBCGToolbarButton* pButton = (CBCGToolbarButton*) pCategoryButtonsList->GetNext (pos);
			ASSERT (pButton != NULL);

			if (pButton->m_nID == uiCmd)	// Found!
			{
				CBCGToolbarButton* pNewButton = (CBCGToolbarButton*) pClass->CreateObject ();
				ASSERT_VALID (pNewButton);

				pNewButton->CopyFrom (button);
				if (pNewButton->m_strText.IsEmpty ())
				{
					pNewButton->m_strText = pButton->m_strText;
				}

				pCategoryButtonsList->SetAt (posSave, pNewButton);
				delete pButton;
			}
		}
	}
}
//**************************************************************************************
BOOL CBCGToolbarCustomize::SetUserCategory (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		TRACE(_T("CBCGToolbarCustomize::SetUserCategory: Can't find category '%s'\n"), 
			lpszCategory);
		return FALSE;
	}

	m_pCustomizePage->SetUserCategory (lpszCategory);
	return TRUE;
}
//**************************************************************************************
void CBCGToolbarCustomize::SetFrameCustMode (BOOL bCustMode)
{
	ASSERT_VALID (m_pParentFrame);

	//-------------------------------------------------------------------
	// Enable/disable all parent frame child windows (except docking bars
	// and our toolbars):
	//-------------------------------------------------------------------
	CWnd* pWndChild = m_pParentFrame->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		CRuntimeClass* pChildClass = pWndChild->GetRuntimeClass ();
		if (pChildClass == NULL ||
			pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGTasksPane)) ||
			(!pChildClass->IsDerivedFrom (RUNTIME_CLASS (CDockBar)) &&

		#ifndef BCG_NO_REBAR
			!pChildClass->IsDerivedFrom (RUNTIME_CLASS (CReBar)) &&
		#endif	// BCG_NO_REBAR
			 !pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGToolBar))))
		{
			pWndChild->EnableWindow (!bCustMode);
		}

		CBCGDockBar* pWndDockBar = (CBCGDockBar*) DYNAMIC_DOWNCAST (CDockBar, pWndChild);
		if (pWndDockBar != NULL)
		{
			pWndDockBar->SetCustomizationMode (bCustMode);
		}

		pWndChild = pWndChild->GetNextWindow ();
	}

	//-----------------------------------------------
	// Set/reset costumize mode for ALL our toolbars:
	//-----------------------------------------------
	CBCGToolBar::SetCustomizeMode (bCustMode);

	//-------------------------------------------------------------
	// Inform the parent frame about mode (for additional actions):
	//-------------------------------------------------------------
	m_pParentFrame->SendMessage (BCGM_CUSTOMIZETOOLBAR, (WPARAM) bCustMode);

	if (!bCustMode && m_pParentFrame->GetActiveFrame () != NULL)
	{
		//---------------------
		// Restore active view:
		//---------------------
		m_pParentFrame->GetActiveFrame ()->PostMessage (WM_SETFOCUS);
	}
}
//**************************************************************************************
BOOL CBCGToolbarCustomize::Create () 
{
	{
		CBCGLocalResource locaRes;

		DWORD dwExStyle = 0;

		if ((m_pParentFrame != NULL) && (m_pParentFrame->GetExStyle() & WS_EX_LAYOUTRTL))
		{
			dwExStyle |= WS_EX_LAYOUTRTL;
		}

		if (!CPropertySheet::Create (m_pParentFrame, (DWORD)-1, dwExStyle))
		{
			return FALSE;
		}
	}

	SetFrameCustMode (TRUE);
	return TRUE;
}
//*************************************************************************************
void CBCGToolbarCustomize::ShowToolBar (CBCGToolBar* pToolBar, BOOL bShow)
{
	m_pToolbarsPage->ShowToolBar (pToolBar, bShow);
}
//*************************************************************************************
BOOL CBCGToolbarCustomize::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	//----------------------
	// Show "Cancel" button:
	//----------------------
	CWnd *pWndCancel = GetDlgItem (IDCANCEL);
	if (pWndCancel == NULL)
	{
		return bResult;
	}

	pWndCancel->ShowWindow (SW_SHOW);
	pWndCancel->EnableWindow ();

	CRect rectClientCancel;
	pWndCancel->GetClientRect (&rectClientCancel);
	pWndCancel->MapWindowPoints (this, &rectClientCancel);

	//-------------------------------
	// Enlarge property sheet window:
	//-------------------------------
	CRect rectWnd;
	GetWindowRect(rectWnd);	

	SetWindowPos(NULL, 0, 0,
		rectWnd.Width (),
		rectWnd.Height () + rectClientCancel.Height () + 2 * iButtonMargin,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	//-------------------------------------------------
	// Move "Cancel" button to the right bottom corner:
	//-------------------------------------------------
	pWndCancel->SetWindowPos (NULL, 
		rectClient.right - rectClientCancel.Width () - iButtonMargin,
		rectClientCancel.top + iButtonMargin / 2,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	//---------------------------------------------------
	// Change "Cancel" button's style to "DEFPUSHBUTTON":
	//---------------------------------------------------
	CWnd *pWndOk = GetDlgItem (IDOK);
	if (pWndOk != NULL)
	{
		pWndOk->ModifyStyle (BS_DEFPUSHBUTTON, 0);
	}

	pWndCancel->ModifyStyle (0, BS_DEFPUSHBUTTON);

	//--------------------------------------------------------
	// Replace "Cancel" text to "Close" 
	// (CPropertyPage::CancelToClose method does nothing in a 
	// modeless property sheet):
	//--------------------------------------------------------
	CString strCloseText;
	
	{
		CBCGLocalResource locaRes;
		strCloseText.LoadString (IDS_BCGBARRES_CLOSE);
	}

	pWndCancel->SetWindowText (strCloseText);

	//------------------------
	// Adjust the Help button:
	//------------------------
	CButton *pWndHelp = (CButton*) GetDlgItem (IDHELP);
	if (pWndHelp == NULL)
	{
		return bResult;
	}

	if (m_uiFlags & BCGCUSTOMIZE_NOHELP)
	{
		pWndHelp->ShowWindow (SW_HIDE);
		pWndHelp->EnableWindow(FALSE);
	}
	else
	{
		m_btnHelp.SubclassWindow (pWndHelp->GetSafeHwnd ());
		m_btnHelp.ShowWindow (SW_SHOW);
		m_btnHelp.EnableWindow ();

		//-----------------------
		// Set Help button image:
		//-----------------------
		CBCGLocalResource locaRes;
		m_btnHelp.SetImage (globalData.Is32BitIcons () ? 
			IDB_BCGBARRES_HELP32 : IDB_BCGBARRES_HELP);

		m_btnHelp.SetWindowText (_T(""));

		//-------------------------------------------------
		// Move "Help" button to the left bottom corner and
		// adjust its size by the bitmap size:
		//-------------------------------------------------
		CSize sizeHelp = m_btnHelp.SizeToContent (TRUE);

		m_btnHelp.SetWindowPos (NULL, 
			rectClient.left + iButtonMargin,
			rectClientCancel.top,
			sizeHelp.cx, sizeHelp.cy,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return bResult;
}
//************************************************************************************
BOOL CBCGToolbarCustomize::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD (wParam))
	{
	case IDCANCEL:
		if (g_pUserToolsManager != NULL)
		{
			if (!CheckToolsValidity (g_pUserToolsManager->GetUserTools ()))
			{
				// Continue customization....
				if (GetActivePage () != m_pToolsPage)
				{
					SetActivePage (m_pToolsPage);
				}

				return TRUE;
			}
		}

		DestroyWindow ();
		return TRUE;

	case IDHELP:
		ASSERT_VALID (m_pParentFrame);
		m_pParentFrame->SendMessage (BCGM_CUSTOMIZEHELP, GetActiveIndex (), (LPARAM) this);
		return TRUE;
	}
		
	return CPropertySheet::OnCommand(wParam, lParam);
}
//************************************************************************************
void CBCGToolbarCustomize::EnableUserDefinedToolbars (BOOL bEnable)
{
	m_pToolbarsPage->EnableUserDefinedToolbars (bEnable);
}
//******************************************************************************************
void CBCGToolbarCustomize::AddMenuCommands (const CMenu* pMenu, BOOL bPopup, 
											LPCTSTR lpszCategory, LPCTSTR lpszMenuPath)
{
	ASSERT (pMenu != NULL);

	BOOL bIsWindowsMenu = FALSE;
	int iCount = (int) pMenu->GetMenuItemCount ();

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID (i);

		CString strText;
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);

		if(!m_bSaveMenuAmps)
		{
			strText.Remove (_T('&'));
		}

		switch (uiCmd)
		{
		case 0:		// Separator, ignore it.
			break;

		case -1:	// Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu (i);

				UINT uiTearOffId = 0;
				if (g_pTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pTearOffMenuManager->Parse (strText);
				}

				CString strCategory = strText;
				if (lpszCategory != NULL)
				{
					strCategory = lpszCategory;
				}

				if (m_bAutoSetFromMenus)
				{
					if (bPopup)
					{
						CBCGToolbarMenuButton menuButton ((UINT) -1, 
							pSubMenu->GetSafeHmenu (),
							-1,
							strText);

						menuButton.SetTearOff (uiTearOffId);
						AddButton (strCategory, menuButton);
					}

					CString strPath;
					if (lpszMenuPath != NULL)
					{
						strPath = lpszMenuPath;
					}

					strPath += strText;
					AddMenuCommands (pSubMenu, TRUE, strCategory, strPath);
				}
				else
				{
					AddMenuCommands (pSubMenu, TRUE);
				}

			}
			break;

		default:
			if (bPopup && uiCmd >= AFX_IDM_WINDOW_FIRST && uiCmd <= AFX_IDM_WINDOW_LAST)
			{
				bIsWindowsMenu = TRUE;
			}

			if (lpszCategory != NULL &&
				g_pUserToolsManager != NULL && 
				g_pUserToolsManager->GetToolsEntryCmd () == uiCmd)
			{
				//----------------------------------------------
				// Replace tools entry by the actual tools list:
				//----------------------------------------------
				AddUserTools (lpszCategory);
			}
			else
			{
				CBCGToolbarButton button (uiCmd, -1, strText);
				
				if (lpszMenuPath != NULL)
				{
					CString strCustom = CString (lpszMenuPath) + button.m_strText;

					LPTSTR pszCustom = strCustom.GetBuffer (strCustom.GetLength () + 1);

					for (int iCount = 0; iCount < lstrlen (pszCustom) - 1; iCount++)
					{
						if (pszCustom [iCount] == _TCHAR(' '))
						{
							CharUpperBuff (&pszCustom [iCount + 1], 1);
						}
					}

					strCustom.ReleaseBuffer();

					strCustom.Remove (_T(' '));
					button.m_strTextCustom = strCustom.SpanExcluding (_T("\t"));
				}

				AddButton (lpszCategory == NULL ?
							m_strAllCommands : lpszCategory, 
							button);
			}
		}
	}

	//--------------------------
	// Add windows manager item:
	//--------------------------
	if (bIsWindowsMenu && lpszCategory != NULL)
	{
		CBCGMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, m_pParentFrame);
		if (pMainMDIFrame != NULL && 
			pMainMDIFrame->m_uiWindowsDlgMenuId != 0 &&
			pMainMDIFrame->m_bShowWindowsDlgAlways)
		{
			AddButton (lpszCategory, 
				CBCGToolbarButton (pMainMDIFrame->m_uiWindowsDlgMenuId, -1,
									pMainMDIFrame->m_strWindowsDlgMenuText));
		}
	}
}
//******************************************************************************************
void CBCGToolbarCustomize::FillCategoriesComboBox (CComboBox& wndCategory,
												   BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext (pos);

		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition (); posCat != NULL;)
			{
				CBCGToolbarButton* pButton = 
					(CBCGToolbarButton*) pCategoryButtonsList->GetNext (posCat);
				ASSERT_VALID (pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString (strCategory);
			wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT (FALSE);
	}

	ASSERT_VALID (pCategoryButtonsList);

	int iIndex = wndCategory.AddString (m_strAllCommands);
	wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
}
//******************************************************************************************
void CBCGToolbarCustomize::FillCategoriesListBox (CListBox& wndCategory,
												  BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext (pos);

		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition (); posCat != NULL;)
			{
				CBCGToolbarButton* pButton = 
					(CBCGToolbarButton*) pCategoryButtonsList->GetNext (posCat);
				ASSERT_VALID (pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString (strCategory);
			wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT (FALSE);
	}

	ASSERT_VALID (pCategoryButtonsList);

	int iIndex = wndCategory.AddString (m_strAllCommands);
	wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
}
//*******************************************************************************************
void CBCGToolbarCustomize::FillAllCommandsList (CListBox& wndListOfCommands) const
{
	wndListOfCommands.ResetContent ();

	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pAllButtonsList))
	{
		return;
	}

	ASSERT_VALID (pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) pAllButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		int iIndex = wndListOfCommands.AddString (
			pButton->m_strTextCustom.IsEmpty () ? pButton->m_strText : pButton->m_strTextCustom);
		wndListOfCommands.SetItemData (iIndex, (DWORD) pButton->m_nID);
	}
}
//***************************************************************************************
BOOL CBCGToolbarCustomize::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if ((m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP) &&
		pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		pHelpInfo->dwContextId = BCGCBHELP_OFFSET + pHelpInfo->iCtrlId;
		AfxGetApp()->WinHelp(pHelpInfo->dwContextId, HELP_CONTEXTPOPUP);
	}
	else
	{
		ASSERT_VALID (m_pParentFrame);
		m_pParentFrame->SendMessage (BCGM_CUSTOMIZEHELP, GetActiveIndex (), 
									(LPARAM) this);
	}

	return TRUE;
}
//***************************************************************************************
LPCTSTR CBCGToolbarCustomize::GetCommandName (UINT uiCmd) const
{
	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pAllButtonsList))
	{
		return NULL;
	}

	ASSERT_VALID (pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) pAllButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == uiCmd)
		{
			return pButton->m_strText;
		}
	}

	return NULL;
}
//***************************************************************************************
void CBCGToolbarCustomize::SetupFromMenus ()
{
	//-------------------------------------------------------------------
	// Find all application document templates and add menue items to the
	// "All commands" category:
	//------------------------------------------------------------------
	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CMultiDocTemplate* pTemplate = 
				DYNAMIC_DOWNCAST (	CMultiDocTemplate, 
									pDocManager->GetNextDocTemplate (pos));
			if (pTemplate != NULL)
			{
				CMenu* pDocMenu = CMenu::FromHandle (pTemplate->m_hMenuShared);
				if (pDocMenu != NULL)
				{
					AddMenuCommands (pDocMenu, FALSE);
				}
			}
		}
	}

	//------------------------------------
	// Add commands from the default menu:
	//------------------------------------
	CMenu* pFrameMenu = CMenu::FromHandle (m_pParentFrame->m_hMenuDefault);
	if (pFrameMenu == NULL)
	{
		CBCGMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, m_pParentFrame);
		const CBCGMenuBar* pMenuBar = NULL;

		if (pMainMDIFrame != NULL)
		{
			pMenuBar = pMainMDIFrame->GetMenuBar ();
		}
		else
		{
			CBCGFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, m_pParentFrame);
			if (pMainFrame != NULL)
			{
				pMenuBar = pMainFrame->GetMenuBar ();
			}
		}

		if (pMenuBar != NULL)
		{
			pFrameMenu = CMenu::FromHandle (pMenuBar->GetDefaultMenu ());
		}
	}

	if (pFrameMenu != NULL)
	{
		AddMenuCommands (pFrameMenu, FALSE);
	}
}
//*****************************************************************************************
void CBCGToolbarCustomize::AddUserTools (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);
	ASSERT_VALID (g_pUserToolsManager);

	CBCGLocalResource locaRes;

	const CObList& lstTools = g_pUserToolsManager->GetUserTools ();
	for (POSITION pos = lstTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGUserTool* pTool = (CBCGUserTool*) lstTools.GetNext (pos);
		ASSERT_VALID (pTool);

		AddButton (lpszCategory,
			CBCGToolbarButton (pTool->GetCommandId (), 0, pTool->m_strLabel));
	}
}
//*******************************************************************************
void CBCGToolbarCustomize::OnContextMenu(CWnd* pWnd, CPoint /*point*/) 
{
	if (g_pWorkspace != NULL && (m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP))
	{
		g_pWorkspace->OnAppContextHelp (pWnd, dwBCGResHelpIDs);
	}
}
//*******************************************************************************
int CBCGToolbarCustomize::GetCountInCategory (LPCTSTR lpszItemName,
											  const CObList& lstCommands) const
{
	int nCount = 0;

	for (POSITION pos = lstCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) lstCommands.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_strText == lpszItemName)
		{
			nCount++;
		}
	}

	return nCount;
}
//********************************************************************************
BOOL CBCGToolbarCustomize::OnEditToolbarMenuImage (CWnd* pWndParent, CBitmap& bitmap, int nBitsPerPixel)
{
	CBCGImageEditDlg dlg (&bitmap, pWndParent, nBitsPerPixel);
	return (dlg.DoModal () == IDOK);
}

#endif // BCG_NO_CUSTOMIZATION
