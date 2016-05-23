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

// MousePage.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "CBCGToolbarCustomize.h"
#include "MousePage.h"
#include "BCGMouseManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGMousePage property page

IMPLEMENT_DYNCREATE(CBCGMousePage, CPropertyPage)

CBCGMousePage::CBCGMousePage() : CPropertyPage(CBCGMousePage::IDD)
{
	//{{AFX_DATA_INIT(CBCGMousePage)
	m_strCommandDescription = _T("");
	//}}AFX_DATA_INIT

	m_iCurrViewId = -1;
}

CBCGMousePage::~CBCGMousePage()
{
}

void CBCGMousePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGMousePage)
	DDX_Control(pDX, IDC_BCGBARRES_LIST_VIEWS, m_wndListOfViews);
	DDX_Control(pDX, IDC_BCGBARRES_LIST_OF_COMMANDS, m_wndListOfCommands);
	DDX_Control(pDX, IDC_BCGBARRES_COMMAND_DESCRIPTION, m_wndCommandDescription);
	DDX_Text(pDX, IDC_BCGBARRES_COMMAND_DESCRIPTION, m_strCommandDescription);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGMousePage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGMousePage)
	ON_BN_CLICKED(IDC_BCGBARRES_NO_DBLCLIICK, OnNoDblcliick)
	ON_BN_CLICKED(IDC_BCGBARRES_USE_DBLCLIICK, OnUseDblcliick)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_LIST_OF_COMMANDS, OnSelchangeListOfCommands)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_BCGBARRES_LIST_VIEWS, OnItemchangedListViews)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGMousePage message handlers

void CBCGMousePage::OnNoDblcliick() 
{
	g_pMouseManager->SetCommandForDblClick (m_iCurrViewId, 0);
	EnableDblClickControls (FALSE);
}
//*************************************************************************************************
void CBCGMousePage::OnUseDblcliick() 
{
	EnableDblClickControls ();

	if (m_iCurrViewId < 0)
	{
		MessageBeep ((UINT) -1);
		return;
	}

	g_pMouseManager->SetCommandForDblClick (m_iCurrViewId, 0);
}
//*************************************************************************************************
void CBCGMousePage::OnSelchangeListOfCommands() 
{
	ASSERT (g_pMouseManager != NULL);

	if (m_iCurrViewId < 0)
	{
		MessageBeep ((UINT) -1);
		return;
	}

	int iIndex = m_wndListOfCommands.GetCurSel ();
	UINT uiCmdId = (UINT) m_wndListOfCommands.GetItemData (iIndex);

	//-------------------------
	// Get command description:
	//-------------------------
	CFrameWnd* pParent = GetParentFrame ();
	if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
	{
		pParent->GetMessageString (uiCmdId, m_strCommandDescription);
	}
	else
	{
		m_strCommandDescription.Empty ();
	}

	//----------------------
	// Update mouse manager:
	//----------------------
	g_pMouseManager->SetCommandForDblClick (m_iCurrViewId, uiCmdId);

	UpdateData (FALSE);
}
//*************************************************************************************************
void CBCGMousePage::OnItemchangedListViews(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT (g_pMouseManager != NULL);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	*pResult = 0;

	if (pNMListView->uChanged != LVIF_STATE)
	{
		return;
	}

	for (int i = 0; i < m_wndListOfViews.GetItemCount (); i ++)
	{
		UINT uState = m_wndListOfViews.GetItemState (i, LVIF_STATE | LVIS_SELECTED);
		if (uState & LVIS_SELECTED)
		{
			m_iCurrViewId = (int) m_wndListOfViews.GetItemData (i);
			ASSERT (m_iCurrViewId >= 0);

			UINT uiCmd = g_pMouseManager->GetViewDblClickCommand (m_iCurrViewId);
			if (uiCmd == 0)
			{
				CheckDlgButton (IDC_BCGBARRES_USE_DBLCLIICK, 0);
				CheckDlgButton (IDC_BCGBARRES_NO_DBLCLIICK, 1);
				EnableDblClickControls (FALSE);
			}
			else
			{
				CheckDlgButton (IDC_BCGBARRES_USE_DBLCLIICK, 1);
				CheckDlgButton (IDC_BCGBARRES_NO_DBLCLIICK, 0);
				EnableDblClickControls ();
				SelectCommand (uiCmd);
			}

			break;
		}
	}
}
//*************************************************************************************************
BOOL CBCGMousePage::OnInitDialog() 
{
	ASSERT (g_pMouseManager != NULL);

	CPropertyPage::OnInitDialog();

	CStringList listOfViewNames;
	g_pMouseManager->GetViewNames (listOfViewNames);

	//-------------------
	// Create image list:
	//-------------------
	if (!m_ViewsImages.Create (	::GetSystemMetrics (SM_CXSMICON),
								::GetSystemMetrics (SM_CYSMICON),
								ILC_COLOR | ILC_MASK, 
								(int) listOfViewNames.GetCount (), 1))
	{
		ASSERT (FALSE);
	}

	m_wndListOfViews.SetImageList (&m_ViewsImages, LVSIL_SMALL);

	POSITION pos;
	
	//-----------------
	// Fill views list:
	//-----------------
	CRect rect;
	m_wndListOfViews.GetClientRect (&rect);
	m_wndListOfViews.InsertColumn (0, _T(""), LVCFMT_LEFT, rect.Width () - 1);

	ASSERT (!listOfViewNames.IsEmpty ());

	int iMaxWidth = 0;

	for (pos = listOfViewNames.GetHeadPosition (); pos != NULL;)
	{
		CString strViewName = listOfViewNames.GetNext (pos);
		
		int iImageIndex = -1;

		//---------------
		// Add view icon:
		//---------------
		UINT uiViewIconId = g_pMouseManager->GetViewIconId (
			g_pMouseManager->GetViewIdByName (strViewName));

		HICON hViewIcon;
		if (uiViewIconId != 0 &&
			(hViewIcon = AfxGetApp ()->LoadIcon (uiViewIconId)) != NULL)
		{
			iImageIndex = m_ViewsImages.Add (hViewIcon);
			::DestroyIcon (hViewIcon);
		}

		int iIndex = m_wndListOfViews.GetItemCount ();
		for (int i = 0; i < m_wndListOfViews.GetItemCount (); i ++)
		{
			CString strText = m_wndListOfViews.GetItemText (i, 0);
			if (strText > strViewName)
			{
				iIndex = i;
				break;
			}
		}

		m_wndListOfViews.InsertItem (iIndex, strViewName, iImageIndex);
		m_wndListOfViews.SetItemData (iIndex, 
			(DWORD) g_pMouseManager->GetViewIdByName (strViewName));

		int iStrWidth = m_wndListOfViews.GetStringWidth (strViewName);
		iMaxWidth = max (iStrWidth, iMaxWidth);
	}

	//----------------------------------
	// Add icon width pluse some pixels:
	//----------------------------------
	IMAGEINFO info;
	m_ViewsImages.GetImageInfo (0, &info);
	CRect rectImage = info.rcImage;

	iMaxWidth += rectImage.Width () + 10;
	m_wndListOfViews.SetColumnWidth (0, iMaxWidth);

	//--------------------
	// Fill commands list:
	//--------------------
	CBCGToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	pWndParent->FillAllCommandsList (m_wndListOfCommands);

	//-----------------------	
	// Select the first view:
	//-----------------------
	m_wndListOfViews.SetItemState  (0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndListOfViews.EnsureVisible (0, FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*************************************************************************************************
void CBCGMousePage::EnableDblClickControls (BOOL bEnable)
{
	m_wndListOfCommands.EnableWindow (bEnable);
	m_wndCommandDescription.EnableWindow (bEnable);

	if (!bEnable)
	{
		m_wndListOfCommands.SetCurSel (-1);

		m_strCommandDescription.Empty ();
		UpdateData (FALSE);
	}
}
//*************************************************************************************************
BOOL CBCGMousePage::SelectCommand (UINT uiCmd)
{
	//-------------------------------
	// Get selected item description:
	//-------------------------------
	CFrameWnd* pParent = GetParentFrame ();
	if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
	{
		pParent->GetMessageString (uiCmd, m_strCommandDescription);
	}
	else
	{
		m_strCommandDescription.Empty ();
	}

	UpdateData (FALSE);

	//----------------------------------------
	// Select command in the commands listbox:
	//----------------------------------------
	for (int iCmdIndex = 0; iCmdIndex < m_wndListOfCommands.GetCount (); iCmdIndex ++)
	{
		if (uiCmd == (UINT) m_wndListOfCommands.GetItemData (iCmdIndex))
		{
			m_wndListOfCommands.SetCurSel (iCmdIndex);
			m_wndListOfCommands.SetTopIndex (iCmdIndex);

			return TRUE;
		}
	}

	return FALSE;
}

#endif // BCG_NO_CUSTOMIZATION
