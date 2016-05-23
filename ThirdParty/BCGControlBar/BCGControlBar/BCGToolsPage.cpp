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

// BCGToolsPage.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "bcgcontrolbar.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "BCGToolsPage.h"
#include "BCGUserToolsManager.h"
#include "CBCGToolbarCustomize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iBtnNew = 0;
static const int iBtnDelete = 1;
static const int iBtnMoveUp = 2;
static const int iBtnMoveDn = 3;

/////////////////////////////////////////////////////////////////////////////
// CBCGToolsPage property page

#pragma warning (disable : 4355)

CBCGToolsPage::CBCGToolsPage() : 
	CPropertyPage(CBCGToolsPage::IDD),
	m_wndToolsList (this)
{
	//{{AFX_DATA_INIT(CBCGToolsPage)
	m_strCommand = _T("");
	m_strArguments = _T("");
	m_strInitialDirectory = _T("");
	//}}AFX_DATA_INIT

	m_pSelTool = NULL;
	m_pParentSheet = NULL;
}

#pragma warning (default : 4355)

CBCGToolsPage::~CBCGToolsPage()
{
}

void CBCGToolsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGToolsPage)
	DDX_Control(pDX, IDD_BCGBARRES_MENU_INITIAL_DIRECTORY, m_wndInitialDirBtn);
	DDX_Control(pDX, IDD_BCGBARRES_MENU_ARGUMENTS, m_wndArgumentsBtn);
	DDX_Control(pDX, IDD_BCGBARRES_ARGUMENTS, m_wndArgumentsEdit);
	DDX_Control(pDX, IDD_BCGBARRES_INITIAL_DIRECTORY, m_wndInitialDirEdit);
	DDX_Control(pDX, IDD_BCGBARRES_COMMAND, m_wndCommandEdit);
	DDX_Control(pDX, IDD_BCGBARRES_BROWSE_COMMAND, m_wndBrowseBtn);
	DDX_Control(pDX, IDD_BCGBARRES_COMMANDS_LIST, m_wndToolsList);
	DDX_Text(pDX, IDD_BCGBARRES_COMMAND, m_strCommand);
	DDX_Text(pDX, IDD_BCGBARRES_ARGUMENTS, m_strArguments);
	DDX_Text(pDX, IDD_BCGBARRES_INITIAL_DIRECTORY, m_strInitialDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGToolsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGToolsPage)
	ON_BN_CLICKED(IDD_BCGBARRES_BROWSE_COMMAND, OnBcgbarresBrowseCommand)
	ON_EN_UPDATE(IDD_BCGBARRES_ARGUMENTS, OnUpdateTool)
	ON_BN_CLICKED(IDD_BCGBARRES_MENU_ARGUMENTS, OnArgumentsOptions)
	ON_BN_CLICKED(IDD_BCGBARRES_MENU_INITIAL_DIRECTORY, OnInitialDirectoryOptions)
	ON_EN_UPDATE(IDD_BCGBARRES_COMMAND, OnUpdateTool)
	ON_EN_UPDATE(IDD_BCGBARRES_INITIAL_DIRECTORY, OnUpdateTool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGToolsPage message handlers

BOOL CBCGToolsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	VERIFY (g_pUserToolsManager != NULL);

	m_pParentSheet = DYNAMIC_DOWNCAST (CBCGToolbarCustomize, GetParent ());
	ASSERT (m_pParentSheet != NULL);

	//-------------
	// Add buttons:
	//-------------
	m_wndToolsList.SetStandardButtons ();
	m_pParentSheet->OnInitToolsPage ();

	//------------
	// Fill tools:
	//------------
	const CObList& lstTools = g_pUserToolsManager->GetUserTools ();
	for (POSITION pos = lstTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGUserTool* pTool = (CBCGUserTool*) lstTools.GetNext (pos);
		ASSERT_VALID (pTool);

		m_wndToolsList.AddItem (pTool->m_strLabel, (DWORD_PTR) pTool);
	}

	UINT uMenuID = 0;
	uMenuID = g_pUserToolsManager->GetInitialDirMenuID ();
	if (uMenuID)
	{
		m_wndInitialDirBtn.ShowWindow(SW_SHOW);
		m_menuInitialDir.LoadMenu (uMenuID);
		m_wndInitialDirBtn.m_hMenu = m_menuInitialDir.GetSubMenu (0)->GetSafeHmenu ();
	}

	uMenuID = g_pUserToolsManager->GetArgumentsMenuID ();
	if (uMenuID)
	{
		m_wndArgumentsBtn.ShowWindow(SW_SHOW);
		m_menuArguments.LoadMenu (uMenuID);
		m_wndArgumentsBtn.m_hMenu = m_menuArguments.GetSubMenu (0)->GetSafeHmenu ();
	}

	m_wndInitialDirBtn.m_bRightArrow = TRUE;
	m_wndArgumentsBtn.m_bRightArrow  = TRUE;

	EnableControls ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//***************************************************************************
BOOL CToolsList::OnBeforeRemoveItem (int iItem)
{
	CBCGUserTool* pTool = (CBCGUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	g_pUserToolsManager->RemoveTool (pTool);
	m_pParent->m_pSelTool = NULL;

	return TRUE;
}
//***************************************************************************
void CToolsList::OnAfterAddItem (int iItem)
{
	CBCGUserTool* pTool = m_pParent->CreateNewTool ();
	if (pTool == NULL)
	{
		RemoveItem (iItem);
		return;
	}

	ASSERT_VALID (pTool);

	pTool->m_strLabel = GetItemText (iItem);
	SetItemData (iItem, (DWORD_PTR) pTool);

	OnSelectionChanged ();
}
//***************************************************************************
void CToolsList::OnAfterRenameItem (int iItem)
{
	CBCGUserTool* pTool = (CBCGUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	pTool->m_strLabel = GetItemText (iItem);
}
//***************************************************************************
void CToolsList::OnAfterMoveItemUp (int iItem)
{
	CBCGUserTool* pTool = (CBCGUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	g_pUserToolsManager->MoveToolUp (pTool);
}
//***************************************************************************
void CToolsList::OnAfterMoveItemDown (int iItem)
{
	CBCGUserTool* pTool = (CBCGUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	g_pUserToolsManager->MoveToolDown (pTool);
}
//**************************************************************************
void CToolsList::OnSelectionChanged ()
{
	int iSelItem = GetSelItem ();
	CBCGUserTool* pSelTool = (iSelItem < 0) ? 
		NULL : (CBCGUserTool*) GetItemData (iSelItem);

	if (pSelTool == NULL)
	{
		m_pParent->m_strCommand.Empty ();
		m_pParent->m_strArguments.Empty ();
		m_pParent->m_strInitialDirectory.Empty ();
	}
	else
	{
		ASSERT_VALID (pSelTool);

		m_pParent->m_strCommand          = pSelTool->GetCommand ();
		m_pParent->m_strArguments        = pSelTool->m_strArguments;
		m_pParent->m_strInitialDirectory = pSelTool->m_strInitialDirectory;
	}

	ASSERT_VALID (m_pParent->m_pParentSheet);
	m_pParent->m_pParentSheet->OnBeforeChangeTool (m_pParent->m_pSelTool);

	m_pParent->m_pSelTool = pSelTool;
	m_pParent->UpdateData (FALSE);

	m_pParent->EnableControls ();

	m_pParent->m_pParentSheet->OnAfterChangeTool (m_pParent->m_pSelTool);
}
//**************************************************************************
void CBCGToolsPage::OnBcgbarresBrowseCommand() 
{
	CFileDialog dlg (TRUE, g_pUserToolsManager->GetDefExt (), NULL, 0,
		g_pUserToolsManager->GetFilter (), this);
	if (dlg.DoModal () == IDOK)
	{
		m_strCommand = dlg.GetPathName ();
		UpdateData (FALSE);
		OnUpdateTool();
	}
}
//**************************************************************************
void CBCGToolsPage::OnUpdateTool() 
{
	UpdateData ();

	int iSelItem = m_wndToolsList.GetSelItem ();
	CBCGUserTool* pSelTool = (iSelItem >= 0) ?
		(CBCGUserTool*) m_wndToolsList.GetItemData (iSelItem) : NULL;

	if (pSelTool == NULL)
	{
		m_strCommand.Empty ();
		m_strArguments.Empty ();
		m_strInitialDirectory.Empty ();

		UpdateData (FALSE);
	}
	else
	{
		ASSERT_VALID (pSelTool);

		pSelTool->SetCommand (m_strCommand);
		pSelTool->m_strArguments = m_strArguments;
		pSelTool->m_strInitialDirectory = m_strInitialDirectory;
	}

	EnableControls ();
}
//******************************************************************************
CBCGUserTool* CBCGToolsPage::CreateNewTool ()
{
	ASSERT_VALID (m_pParentSheet);

	const int nMaxTools = g_pUserToolsManager->GetMaxTools ();

	if (g_pUserToolsManager->GetUserTools ().GetCount () == nMaxTools)
	{
		CBCGLocalResource locaRes;
		
		CString strError;
		strError.Format (IDS_BCGBARRES_TOO_MANY_TOOLS_FMT, nMaxTools);
		AfxMessageBox (strError);

		return NULL;
	}

	CBCGUserTool* pTool = g_pUserToolsManager->CreateNewTool ();
	ASSERT_VALID (pTool);

	return pTool;
}
//*******************************************************************************
void CBCGToolsPage::OnOK() 
{
	OnUpdateTool();
	CPropertyPage::OnOK();
}
//******************************************************************************
BOOL CBCGToolsPage::OnKillActive() 
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParentSheet);

	if (!m_pParentSheet->CheckToolsValidity (
		g_pUserToolsManager->GetUserTools ()))
	{
		return FALSE;
	}

	return CPropertyPage::OnKillActive();
}
//******************************************************************************
void CBCGToolsPage::EnableControls ()
{
	BOOL bEnableItemProps = (m_wndToolsList.GetSelItem () >= 0);

	m_wndCommandEdit.EnableWindow (bEnableItemProps);
	m_wndArgumentsEdit.EnableWindow (bEnableItemProps);
	m_wndInitialDirEdit.EnableWindow (bEnableItemProps);
	m_wndBrowseBtn.EnableWindow (bEnableItemProps);

	m_wndInitialDirBtn.EnableWindow (bEnableItemProps);
	m_wndArgumentsBtn.EnableWindow (bEnableItemProps);
}
//******************************************************************************
void CBCGToolsPage::OnArgumentsOptions () 
{
	if (m_wndArgumentsBtn.m_nMenuResult != 0)
	{
		CString strItem;
		strItem.LoadString (m_wndArgumentsBtn.m_nMenuResult);

		//-----------------------------------------------
		// Insert text to the current arguments position:
		//-----------------------------------------------
		for (int i = 0; i < strItem.GetLength (); i++)
		{
			m_wndArgumentsEdit.SendMessage (WM_CHAR, (TCHAR) strItem [i]);
		}
	}
}
//******************************************************************************
void CBCGToolsPage::OnInitialDirectoryOptions () 
{
	if (m_wndInitialDirBtn.m_nMenuResult != 0)
	{
		CString strItem;
		strItem.LoadString (m_wndInitialDirBtn.m_nMenuResult);

		//-----------------------------------------------
		// Insert text to the current directory position:
		//-----------------------------------------------
		for (int i = 0; i < strItem.GetLength (); i++)
		{
			m_wndInitialDirEdit.SendMessage (WM_CHAR, (TCHAR) strItem [i]);
		}
	}
}

#endif // BCG_NO_CUSTOMIZATION
