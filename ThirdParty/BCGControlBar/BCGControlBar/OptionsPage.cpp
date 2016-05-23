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

// OptionsPage.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "bcgbarres.h"
#include "bcgcontrolbar.h"
#include "OptionsPage.h"
#include "BCGToolBar.h"
#include "BCGMenuBar.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "bcglocalres.h"
#include "CBCGToolbarCustomize.h"
#include "BCGVisualManager.h"
#include "BCGSkinManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGOptionsPage property page

IMPLEMENT_DYNCREATE(CBCGOptionsPage, CPropertyPage)

CBCGOptionsPage::CBCGOptionsPage(BOOL bIsMenuBarExist) : 
	CPropertyPage(CBCGOptionsPage::IDD),
	m_bIsMenuBarExist (bIsMenuBarExist)
{
	//{{AFX_DATA_INIT(CBCGOptionsPage)
	m_bShowTooltips = CBCGToolBar::m_bShowTooltips;
	m_bShowShortcutKeys = CBCGToolBar::m_bShowShortcutKeys;
	m_bRecentlyUsedMenus = CBCGMenuBar::m_bRecentlyUsedMenus;
	m_bShowAllMenusDelay = CBCGMenuBar::m_bShowAllMenusDelay;
	m_bLargeIcons = CBCGToolBar::m_bLargeIcons;
	m_bLook2000 = CBCGVisualManager::GetInstance ()->IsLook2000 ();
	//}}AFX_DATA_INIT

}

CBCGOptionsPage::~CBCGOptionsPage()
{
}

void CBCGOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGOptionsPage)
	DDX_Control(pDX, IDC_BCGBARRES_SKINS, m_wndSkinsBtn);
	DDX_Control(pDX, IDC_BCGBARRES_LOOK2000, m_wndLook2000);
	DDX_Control(pDX, IDC_BCGBARRES_LARGE_ICONS, m_wndLargeIcons);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, m_wndRUMenus);
	DDX_Control(pDX, IDC_BCGBARRES_RESET_USAGE_DATA, m_wndResetUsageBtn);
	DDX_Control(pDX, IDC_RU_MENUS_TITLE, m_wndRuMenusLine);
	DDX_Control(pDX, IDC_RU_MENUS_LINE, m_wndRuMenusTitle);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_MENUS_DELAY, m_wndShowAllMenusDelay);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_wndShowShortcutKeys);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS, m_bShowTooltips);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_bShowShortcutKeys);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, m_bRecentlyUsedMenus);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_MENUS_DELAY, m_bShowAllMenusDelay);
	DDX_Check(pDX, IDC_BCGBARRES_LARGE_ICONS, m_bLargeIcons);
	DDX_Check(pDX, IDC_BCGBARRES_LOOK2000, m_bLook2000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGOptionsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGOptionsPage)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, OShowTooltipsWithKeys)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_TOOLTIPS, OnShowTooltips)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET_USAGE_DATA, OnResetUsageData)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, OnShowRecentlyUsedMenus)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_MENUS_DELAY, OnShowMenusDelay)
	ON_BN_CLICKED(IDC_BCGBARRES_LARGE_ICONS, OnLargeIcons)
	ON_BN_CLICKED(IDC_BCGBARRES_LOOK2000, OnBcgbarresLook2000)
	ON_BN_CLICKED(IDC_BCGBARRES_SKINS, OnBcgbarresSkins)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGOptionsPage message handlers

BOOL CBCGOptionsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_wndShowShortcutKeys.EnableWindow (m_bShowTooltips);
	m_wndShowAllMenusDelay.EnableWindow (m_bRecentlyUsedMenus);

	if (CBCGToolBar::m_lstBasicCommands.IsEmpty () || !m_bIsMenuBarExist)
	{
		m_wndRUMenus.ShowWindow (SW_HIDE);
		m_wndRUMenus.EnableWindow (FALSE);

		m_wndResetUsageBtn.ShowWindow (SW_HIDE);
		m_wndResetUsageBtn.EnableWindow (FALSE);

		m_wndRuMenusLine.ShowWindow (SW_HIDE);
		m_wndRuMenusLine.EnableWindow (FALSE);

		m_wndRuMenusTitle.ShowWindow (SW_HIDE);
		m_wndRuMenusTitle.EnableWindow (FALSE);

		m_wndShowAllMenusDelay.ShowWindow (SW_HIDE);
		m_wndShowAllMenusDelay.EnableWindow (FALSE);
	}

	CBCGToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	if ((pWndParent->GetFlags () & BCGCUSTOMIZE_LOOK_2000) == 0)
	{
		m_wndLook2000.ShowWindow (SW_HIDE);
		m_wndLook2000.EnableWindow (FALSE);
	}

	if (pWndParent->GetFlags () & BCGCUSTOMIZE_NO_LARGE_ICONS)
	{
		m_wndLargeIcons.ShowWindow (SW_HIDE);
		m_wndLargeIcons.EnableWindow (FALSE);
		m_bLargeIcons = FALSE;
	}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	if ((pWndParent->GetFlags () & BCGCUSTOMIZE_SELECT_SKINS) == 0)
	{
		m_wndSkinsBtn.ShowWindow (SW_HIDE);
		m_wndSkinsBtn.EnableWindow (FALSE);
	}
	else
	{
		ASSERT (g_pSkinManager != NULL);
	}
#else
	m_wndSkinsBtn.ShowWindow (SW_HIDE);
	m_wndSkinsBtn.EnableWindow (FALSE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*******************************************************************************
void CBCGOptionsPage::OShowTooltipsWithKeys() 
{
	UpdateData ();
	CBCGToolBar::m_bShowShortcutKeys = m_bShowShortcutKeys;
}
//*******************************************************************************
void CBCGOptionsPage::OnShowTooltips() 
{
	UpdateData ();

	CBCGToolBar::m_bShowTooltips = m_bShowTooltips;
	m_wndShowShortcutKeys.EnableWindow (m_bShowTooltips);
}
//******************************************************************************
void CBCGOptionsPage::OnResetUsageData() 
{
	CBCGLocalResource locaRes;
	if (AfxMessageBox (IDS_BCGBARRES_RESET_USAGE_WARNING, MB_YESNO) == IDYES)
	{
		CBCGToolBar::m_UsageCount.Reset ();
	}
}
//*******************************************************************************
void CBCGOptionsPage::OnShowRecentlyUsedMenus() 
{
	UpdateData ();
	m_wndShowAllMenusDelay.EnableWindow (m_bRecentlyUsedMenus);

	CBCGMenuBar::m_bRecentlyUsedMenus = m_bRecentlyUsedMenus;
}
//*******************************************************************************
void CBCGOptionsPage::OnShowMenusDelay() 
{
	UpdateData ();
	CBCGMenuBar::m_bShowAllMenusDelay = m_bShowAllMenusDelay;
}
//*******************************************************************************
void CBCGOptionsPage::OnLargeIcons() 
{
	UpdateData ();
	CBCGToolBar::SetLargeIcons (m_bLargeIcons);
}
//*******************************************************************************
void CBCGOptionsPage::OnBcgbarresLook2000() 
{
	UpdateData ();
	CBCGVisualManager::GetInstance ()->SetLook2000 (m_bLook2000);
	AfxGetMainWnd()->Invalidate();
}
//********************************************************************************
void CBCGOptionsPage::OnBcgbarresSkins() 
{
#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	ASSERT (g_pSkinManager != NULL);
	g_pSkinManager->ShowSelectSkinDlg ();
#else
	ASSERT (FALSE);
#endif
}

#endif // BCG_NO_CUSTOMIZATION

