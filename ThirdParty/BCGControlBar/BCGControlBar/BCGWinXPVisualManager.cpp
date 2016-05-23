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
//
// BCGWinXPVisualManager.cpp: implementation of the CBCGWinXPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "multimon.h"
#include "BCGWinXPVisualManager.h"
#include "BCGPopupMenuBar.h"
#include "BCGButton.h"
#include "BCGSizingControlBar.h"
#include "BCGDrawManager.h"
#include "BCGToolbarMenuButton.h"
#include "BCGTabWnd.h"
#include "BCGTasksPane.h"
#include "BCGStatusBar.h"
#include "BCGCaptionBar.h"
#include "BCGPopupWindow.h"
#include "BCGFrameImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if (defined(SCHEMA_STRINGS)) || (! defined(TMSCHEMA_H))

#define RP_GRIPPER			1
#define RP_GRIPPERVERT		2

#define TP_BUTTON			1
#define TP_SEPARATOR		5
#define TP_SEPARATORVERT	6

#define WP_MINCAPTION		3
#define WP_MDIMINBUTTON		16
#define	WP_SMALLCLOSEBUTTON	19
#define WP_MDICLOSEBUTTON	20
#define WP_MDIRESTOREBUTTON	22

#define SP_PANE				1

#define BP_PUSHBUTTON		1

#define CP_DROPDOWNBUTTON	1

#define HP_HEADERITEM		1
#define HP_HEADERSORTARROW	4

#define SBP_SIZEBOX			10

#define TS_NORMAL			1
#define TS_HOT				2
#define TS_PRESSED			3
#define TS_DISABLED			4
#define TS_CHECKED			5
#define TS_HOTCHECKED		6

#define PBS_NORMAL			1
#define PBS_HOT				2
#define PBS_PRESSED			3
#define PBS_DISABLED		4
#define PBS_DEFAULTED		5

#define CBS_NORMAL			1
#define CBS_HOT				2
#define CBS_PUSHED			3
#define	CBS_DISABLED		4

#define CBXS_NORMAL			1
#define CBXS_HOT			2
#define CBXS_PRESSED		3
#define CBXS_DISABLED		4

#define MNCS_ACTIVE			1
#define MNCS_INACTIVE		2

#define TIBES_NORMAL		1
#define TIBES_HOT			2
#define TIBES_SELECTED		3
#define TIBES_DISABLED		4
#define TIBES_FOCUSED		5

#define HIS_NORMAL			1
#define HIS_HOT				2
#define HIS_PRESSED			3

#define HILS_NORMAL			1
#define HILS_HOT			2
#define HILS_PRESSED		3

#define HIRS_NORMAL			1
#define HIRS_HOT			2
#define HIRS_PRESSED		3

#define HSAS_SORTEDUP		1
#define HSAS_SORTEDDOWN		2

#define SZB_RIGHTALIGN		1
#define SZB_LEFTALIGN		2

#define EBP_HEADERBACKGROUND		1
#define EBP_NORMALGROUPBACKGROUND	5
#define EBP_NORMALGROUPCOLLAPSE		6
#define EBP_NORMALGROUPEXPAND		7
#define EBP_NORMALGROUPHEAD			8
#define EBP_SPECIALGROUPBACKGROUND	9
#define EBP_SPECIALGROUPCOLLAPSE	10
#define EBP_SPECIALGROUPEXPAND		11
#define EBP_SPECIALGROUPHEAD		12

#define EBNGC_NORMAL	1
#define EBNGC_HOT		2
#define EBNGC_PRESSED	3

#define EBNGE_NORMAL	1
#define EBNGE_HOT		2
#define EBNGE_PRESSED	3

#define EBSGC_NORMAL	1
#define EBSGC_HOT		2
#define EBSGC_PRESSED	3

#define EBSGE_NORMAL	1
#define EBSGE_HOT		2
#define EBSGE_PRESSED	3

#define TVP_GLYPH		2
#define GLPS_CLOSED		1
#define GLPS_OPENED		2

#define SPNP_UP			1
#define SPNP_DOWN		2
#define SPNP_UPHORZ		3
#define SPNP_DOWNHORZ	4

#define UPS_NORMAL		1
#define UPS_HOT			2
#define UPS_PRESSED		3
#define UPS_DISABLED	4

#define TABP_TABITEM				1
#define TABP_TABITEMLEFTEDGE		2
#define TABP_TABITEMRIGHTEDGE		3
#define TABP_TABITEMBOTHEDGE		4
#define TABP_TOPTABITEM				5
#define TABP_TOPTABITEMLEFTEDGE		6
#define TABP_TOPTABITEMRIGHTEDGE	7
#define TABP_TOPTABITEMBOTHEDGE		8
#define TABP_PANE					9
#define TABP_BODY					10

#define TIS_NORMAL		1
#define TIS_HOT			2
#define TIS_SELECTED	3
#define TIS_DISABLED	4
#define TIS_FOCUSED		5

#define	TMT_TEXTCOLOR	3803

#endif

IMPLEMENT_DYNCREATE(CBCGWinXPVisualManager, CBCGVisualManagerXP)

BOOL CBCGWinXPVisualManager::m_b3DTabsXPTheme = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGWinXPVisualManager::CBCGWinXPVisualManager(BOOL bIsTemporary) :
	CBCGVisualManagerXP (bIsTemporary)
{
	m_bShadowHighlightedImage = FALSE;

	m_bOfficeStyleMenus = FALSE;

	m_nVertMargin = 12;
	m_nHorzMargin = 12;
	m_nGroupVertOffset = 15;
	m_nGroupCaptionHeight = 25;
	m_nGroupCaptionHorzOffset = 13;
	m_nGroupCaptionVertOffset = 7;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;
}

CBCGWinXPVisualManager::~CBCGWinXPVisualManager()
{
}

BOOL CBCGWinXPVisualManager::IsWinXPThemeAvailible ()
{
	CBCGWinXPVisualManager* pWinXPManager =
		DYNAMIC_DOWNCAST (CBCGWinXPVisualManager, m_pVisManager);
	if (pWinXPManager != NULL)
	{
		return pWinXPManager->m_hThemeWindow != NULL;
	}

	// Create a temporary manager and check it:
	CBCGWinXPVisualManager winXPManager (TRUE /* Temporary */);
	return winXPManager.m_hThemeWindow != NULL;
}

void CBCGWinXPVisualManager::SetOfficeStyleMenus (BOOL bOn)
{
	m_bOfficeStyleMenus = bOn;
}

void CBCGWinXPVisualManager::OnUpdateSystemColors ()
{
	CBCGVisualManagerXP::OnUpdateSystemColors ();
	CBCGWinXPThemeManager::UpdateSystemColors ();

	m_bShadowHighlightedImage = TRUE;

	if (m_hThemeWindow != NULL)
	{
		m_bShadowHighlightedImage = FALSE;
	}

	if (m_pfGetThemeColor != NULL && m_hThemeToolBar != NULL)
	{
		(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_TEXTCOLOR, &globalData.clrBarText);
	}
}

void CBCGWinXPVisualManager::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CControlBar* pBar)
{
	if (m_hThemeRebar == NULL)
	{
		CBCGVisualManagerXP::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

	BOOL bSideBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGSizingControlBar));

	CRect rectFill = rectGripper;

	if (bSideBar)
	{
		bHorz = !bHorz;
	}

	COLORREF clrTextOld = pDC->SetTextColor (globalData.clrBarText);

	if (bSideBar)
	{
		//------------------
		// Draw bar caption:
		//------------------
		int nOldBkMode = pDC->SetBkMode (TRANSPARENT);

		const CFont& font = CBCGMenuBar::GetMenuFont (bHorz);

		CFont* pOldFont = pDC->SelectObject ((CFont*) &font);

		CString strCaption;
		pBar->GetWindowText (strCaption);
		strCaption = _T(" ") + strCaption + _T(" ");

		CRect rectText = rectGripper;
		UINT uiTextFormat = 0;

		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);

		CPoint ptTextOffset (0, 0);
		if (bHorz)
		{
			ptTextOffset.y = (rectGripper.Height () - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptTextOffset.x = (rectGripper.Width () - tm.tmHeight + 1) / 2;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;
			pDC->DrawText (strCaption, &rectText, uiTextFormat);

			rectGripper.left = rectText.left + pDC->GetTextExtent (strCaption).cx + 5;
		}
		else
		{
			rectText.left = rectText.right - ptTextOffset.x;
			rectText.top = rectGripper.top + ptTextOffset.y;
			rectText.bottom = rectGripper.top + 3 * ptTextOffset.y;

			uiTextFormat |= DT_NOCLIP;

			pDC->DrawText (strCaption, &rectText, uiTextFormat);

			rectGripper.top = rectText.top + pDC->GetTextExtent (strCaption).cx + 5;
		}

		pDC->SelectObject(pOldFont);
		pDC->SetBkMode(nOldBkMode);
	}

	pDC->SetTextColor (clrTextOld);

	if (rectGripper.right > rectGripper.left &&
		rectGripper.bottom > rectGripper.top)
	{
		CRect rectGripperTheme = rectGripper;
		const int nGripperOffset = 2;

		if (bHorz)
		{
			rectGripperTheme.DeflateRect (0, nGripperOffset);
			rectGripperTheme.OffsetRect (nGripperOffset, 0);

			if (!bSideBar)
			{
				rectGripperTheme.right = rectGripperTheme.left + 3 * nGripperOffset;
			}
		}
		else
		{
			rectGripperTheme.DeflateRect (nGripperOffset, 0);
			rectGripperTheme.OffsetRect (0, nGripperOffset);

			if (!bSideBar)
			{
				rectGripperTheme.bottom = rectGripperTheme.top + 3 * nGripperOffset;
			}
		}

		(*m_pfDrawThemeBackground) (m_hThemeRebar, pDC->GetSafeHdc(),
			bHorz ? RP_GRIPPER : RP_GRIPPERVERT, 0, &rectGripperTheme, 0);
	}
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea)
{
	ASSERT_VALID (pBar);

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGStatusBar)) &&
		m_hThemeStatusBar != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, 
			pDC->GetSafeHdc (),
			0, 0, &rectClient, 0);
		return;
	}

	if (m_pfDrawThemeBackground == NULL ||
		m_hThemeRebar == NULL ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGCaptionBar)))
	{
		CBCGVisualManagerXP::OnFillBarBackground (pDC, pBar,rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
	{
		if (m_bOfficeStyleMenus)
		{
			CBCGVisualManagerXP::OnFillBarBackground (pDC, pBar,rectClient, rectClip, bNCArea);
		}
		else
		{
			::FillRect (pDC->GetSafeHdc (), rectClient, ::GetSysColorBrush (COLOR_MENU));
		}

		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGSizingControlBar)))
	{
		pDC->FillRect (rectClient, &globalData.brBarFace);
		return;
	}

	FillRebarPane (pDC, pBar, rectClient);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect)
{
	CBCGVisualManager::OnDrawBarBorder (pDC, pBar, rect);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnFillButtonInterior (CDC* pDC,
				CBCGToolbarButton* pButton, CRect rect,
				CBCGVisualManager::BCGBUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	BOOL bIsMenuBar = FALSE;
	BOOL bIsPopupMenu = FALSE;

	CBCGToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
	if (pMenuButton != NULL)
	{
		bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));

		bIsPopupMenu = pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));
	}

	if (m_bOfficeStyleMenus && (bIsPopupMenu || bIsMenuBar))
	{
		CBCGVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	if (bIsPopupMenu && state != ButtonsIsHighlighted &&
		state != ButtonsIsPressed)
	{
		return;
	}

	int nState = TS_NORMAL;
	
	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		nState = TS_DISABLED;
	}
	else if ((pButton->m_nStyle & TBBS_PRESSED) && state == ButtonsIsHighlighted)
	{
		nState = TS_PRESSED;
	}
	else if (pButton->m_nStyle & TBBS_CHECKED)
	{
		nState = (state == ButtonsIsHighlighted) ? TS_HOTCHECKED : TS_CHECKED;
	}
	else if (state == ButtonsIsHighlighted)
	{
		nState = TS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
}
//**************************************************************************************
COLORREF CBCGWinXPVisualManager::GetToolbarButtonTextColor (CBCGToolbarButton* pButton, 
														CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	if (m_hThemeToolBar == NULL ||
		pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton)))
	{
		return CBCGVisualManagerXP::GetToolbarButtonTextColor (pButton, state);
	}

	return CBCGVisualManager::GetToolbarButtonTextColor (pButton, state);
}
//************************************************************************************
void CBCGWinXPVisualManager::OnHighlightMenuItem (CDC*pDC, CBCGToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CBCGVisualManagerXP::OnHighlightMenuItem (pDC, pButton,	rect, clrText);
	}
	else
	{
		CBCGVisualManager::OnHighlightMenuItem (pDC, pButton,	rect, clrText);
	}
}
//************************************************************************************
COLORREF CBCGWinXPVisualManager::GetHighlightedMenuItemTextColor (CBCGToolbarMenuButton* pButton)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CBCGVisualManagerXP::GetHighlightedMenuItemTextColor (pButton);
	}
	else
	{
		return CBCGVisualManager::GetHighlightedMenuItemTextColor (pButton);
	}
}
//************************************************************************************
void CBCGWinXPVisualManager::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CBCGVisualManagerXP::OnHighlightRarelyUsedMenuItems  (pDC, rectRarelyUsed);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGDrawManager dm (*pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;
	dm.HighlightRect (rectRarelyUsed, 94);
}
//************************************************************************************
void CBCGWinXPVisualManager::OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	if (m_bOfficeStyleMenus)
	{
		CBCGToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
		if (pMenuButton != NULL)
		{
			BOOL bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
				pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));

			BOOL bIsPopupMenu = pMenuButton->GetParentWnd () != NULL &&
				pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));

			if (bIsPopupMenu || bIsMenuBar)
			{
				CBCGVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
				return;
			}
		}
	}
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawButtonSeparator (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawButtonSeparator (pDC, pButton, rect, state, bHorz);
		return;
	}

	rect.InflateRect (2, 2);

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), 
		bHorz ? TP_SEPARATOR : TP_SEPARATORVERT,
		0, &rect, 0);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawSeparator (CDC* pDC, CControlBar* pBar,
										 CRect rect, BOOL bHorz)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	if (m_bOfficeStyleMenus && pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
	{
		CBCGVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), 
		bHorz ? TP_SEPARATOR : TP_SEPARATORVERT, 
		0, &rect, 0);
}
//***************************************************************************************
void CBCGWinXPVisualManager::OnDrawCaptionButton (
						CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz,
						BOOL bMaximized, BOOL bDisabled)
{
	if (m_hThemeButton == NULL || m_hThemeWindow == NULL)
	{
		CBCGVisualManagerXP::OnDrawCaptionButton (pDC, pButton, bHorz, bMaximized, bDisabled);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT (pButton != NULL);

	BOOL bPushed = pButton->m_bPushed;
	BOOL bFocused = pButton->m_bFocused;

	int nState = PBS_NORMAL;
	if (bDisabled)
	{
		nState = PBS_DISABLED;
	}
	else if (bPushed && bFocused)
	{
		nState = PBS_PRESSED;
	}
	else if (bFocused)
	{
		nState = PBS_HOT;
	}

	CRect rect = pButton->GetRect ();

	if (pButton->m_nHit == HTCLOSE_BCG)
	{
		(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), 
			WP_SMALLCLOSEBUTTON, nState, &rect, 0);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, 
		nState, &rect, 0);

	pDC->SetTextColor (RGB (0, 0, 0));
	pButton->DrawIcon (pDC, rect, bHorz, bMaximized, bDisabled);
}
//***********************************************************************************
COLORREF CBCGWinXPVisualManager::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CBCGVisualManagerXP::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	if (bIsSelected)
	{
		pDC->FillRect (rect, &globalData.brHilite);
		pDC->Draw3dRect (rect, globalData.clrMenuText, globalData.clrMenuText);

		return globalData.clrTextHilite;
	}

	::FillRect (pDC->GetSafeHdc (), rect, ::GetSysColorBrush (COLOR_MENU));
	return ::GetSysColor (COLOR_MENUTEXT);
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	CBCGVisualManagerXP::OnDrawTearOffCaption (pDC, rect, bIsActive);
}
//***********************************************************************************
void CBCGWinXPVisualManager::OnDrawMenuSystemButton (CDC* pDC, CRect rect, 
												UINT uiSystemCommand, 
												UINT nStyle, BOOL bHighlight)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGVisualManagerXP::OnDrawMenuSystemButton (pDC, rect, uiSystemCommand, nStyle, bHighlight);
		return;
	}

	int nPart;
	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		nPart = WP_MDICLOSEBUTTON;
		break;

	case SC_RESTORE:
		nPart = WP_MDIRESTOREBUTTON;
		break;

	case SC_MINIMIZE:
		nPart = WP_MDIMINBUTTON;
		break;

	default:
		return;
	}

	BOOL bIsDisabled = (nStyle & TBBS_DISABLED);
	BOOL bIsPressed = (nStyle & TBBS_PRESSED);

	int nState = CBS_NORMAL;
	if (bIsDisabled)
	{
		nState = CBS_DISABLED;
	}
	else if (bIsPressed && bHighlight)
	{
		nState = CBS_PUSHED;
	}
	else if (bHighlight)
	{
		nState = CBS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), nPart,
		nState, &rect, 0);
}
//********************************************************************************
void CBCGWinXPVisualManager::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle)
{
	if (m_hThemeStatusBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawStatusBarPaneBorder (pDC, pBar, rectPane, uiID, nStyle);
		return;
	}

	if (!(nStyle & SBPS_NOBORDERS))
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), SP_PANE,
			0, &rectPane, 0);
	}
}
//**************************************************************************************
void CBCGWinXPVisualManager::OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		ASSERT_VALID (pMenu);

		BOOL bConnectMenuToParent = m_bConnectMenuToParent;
		m_bConnectMenuToParent = FALSE;

		if (m_hThemeWindow == NULL)
		{
			m_bConnectMenuToParent = TRUE;
		}
		else if (!CBCGToolBar::IsCustomizeMode ())
		{
			CBCGToolbarMenuButton* pMenuButton = pMenu->GetParentButton ();

			if (pMenuButton != NULL)
			{
				BOOL bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
					pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));
				if (bIsMenuBar)
				{
					m_bConnectMenuToParent = TRUE;
				}
			}
		}

		CBCGVisualManagerXP::OnDrawMenuBorder (pDC, pMenu, rect);

		m_bConnectMenuToParent = bConnectMenuToParent;
	}
	else
	{
		CBCGVisualManager::OnDrawMenuBorder (pDC, pMenu, rect);
	}
}
//****************************************************************************************
void CBCGWinXPVisualManager::OnDrawComboDropButton (CDC* pDC, CRect rect,
											    BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* pButton)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGVisualManagerXP::OnDrawComboDropButton (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	int nState = bDisabled ? CBXS_DISABLED : bIsDropped ? CBXS_PRESSED : bIsHighlighted ? CBXS_HOT : CBXS_NORMAL;

	(*m_pfDrawThemeBackground) (m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, 
		nState, &rect, 0);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* pButton)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGVisualManagerXP::OnDrawComboBorder (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (&rect,  ::GetSysColor (COLOR_HIGHLIGHT), ::GetSysColor (COLOR_HIGHLIGHT));
	}
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsHighlighted,
												CBCGToolbarEditBoxButton* pButton)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGVisualManagerXP::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted)
	{
		pDC->Draw3dRect (&rect,  ::GetSysColor (COLOR_HIGHLIGHT), ::GetSysColor (COLOR_HIGHLIGHT));
	}
}
//**************************************************************************************
BOOL CBCGWinXPVisualManager::OnFillOutlookPageButton (CBCGButton* pButton,
												CDC* pDC, const CRect& rectClient,
												COLORREF& clrText)
{
	if (m_hThemeButton == NULL)
	{
		return CBCGVisualManagerXP::OnFillOutlookPageButton (pButton, pDC, rectClient, clrText);
	}

	int nState = PBS_NORMAL;

	if (pButton->IsPressed () ||pButton->IsChecked ())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = PBS_HOT;
	}

	CRect rect = rectClient;
	rect.InflateRect (1, 1);

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, 
		nState, &rect, 0);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGWinXPVisualManager::OnDrawOutlookPageButtonBorder (CBCGButton* pButton,
												CDC* pDC, CRect& rectClient,
												UINT uiState)
{
	if (m_hThemeButton == NULL)
	{
		return CBCGVisualManagerXP::OnDrawOutlookPageButtonBorder (pButton, pDC, rectClient, uiState);
	}

	return TRUE;
}
//****************************************************************************************
CSize CBCGWinXPVisualManager::GetButtonExtraBorder () const
{
	if (m_hThemeWindow == NULL)
	{
		return CBCGVisualManagerXP::GetButtonExtraBorder ();
	}

	return CSize (2, 2);
}
//****************************************************************************************
void CBCGWinXPVisualManager::OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (m_hThemeHeader == NULL)
	{
		CBCGVisualManagerXP::OnDrawHeaderCtrlBorder (pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	int nState = HIS_NORMAL;

	if (bIsPressed)
	{
		nState = HIS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = HIS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERITEM, nState, &rect, 0);
}
//*****************************************************************************************
void CBCGWinXPVisualManager::OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl,
												   CDC* pDC,
												   CRect& rect, BOOL bIsUp)
{
	if (m_hThemeHeader == NULL)
	{
		CBCGVisualManagerXP::OnDrawHeaderCtrlSortArrow (pCtrl, pDC, rect, bIsUp);
		return;
	}

/* TODO
	int nState = bIsUp ? HSAS_SORTEDUP : HSAS_SORTEDDOWN;

	(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERSORTARROW, nState, &rect, 0);
*/
	#define POINTS_NUM	3
	POINT pts [POINTS_NUM];

	if (bIsUp)
	{
		pts [0].x = rect.left;
		pts [0].y = rect.bottom;

		pts [1].x = rect.CenterPoint ().x;
		pts [1].y = rect.top;

		pts [2].x = rect.right;
		pts [2].y = rect.bottom;
	}
	else
	{
		pts [0].x = rect.left;
		pts [0].y = rect.top;

		pts [1].x = rect.CenterPoint ().x;
		pts [1].y = rect.bottom;

		pts [2].x = rect.right;
		pts [2].y = rect.top;
	}

	CBrush br (globalData.clrBarShadow);
	CBrush* pOldBrush = pDC->SelectObject (&br);

	CPen* pOldPen = (CPen*) pDC->SelectStockObject (NULL_PEN);

	pDC->Polygon (pts, POINTS_NUM);

	pDC->SelectObject (pOldBrush);
	pDC->SelectObject (pOldPen);
}
//*********************************************************************************
void CBCGWinXPVisualManager::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGButton* pButton,
											  CBCGTabWnd* pWndTab)
{
	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pWndTab->IsFlatTab () || 
		pWndTab->IsOneNoteStyle () || pWndTab->IsVS2005Style ())
	{
		CBCGVisualManagerXP::OnEraseTabsButton (pDC, rect, pButton, pWndTab);
		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CRect rectTabs;
	pWndTab->GetClientRect (&rectTabs);

	CRect rectTabArea;
	pWndTab->GetTabsRect (rectTabArea);

	if (pWndTab->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		rectTabs.top = rectTabArea.top;
		rectTabs.bottom += 2;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints (pButton, rectTabs);

	OnEraseTabsArea (pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn (NULL);
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT uiState,
												 CBCGTabWnd* pWndTab)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawTabsButtonBorder (pDC, rect, pButton, uiState, pWndTab);
		return;
	}

	int nState = TS_NORMAL;

	if (!pButton->IsWindowEnabled ())
	{
		nState = TS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = TS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = TS_HOT;
	}

	globalData.DrawParentBackground (pButton, pDC, rect);

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGVisualManagerXP::OnFillTasksPaneBackground (pDC, rectWorkArea);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_HEADERBACKGROUND,
		0, &rectWorkArea, 0);
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup,
								BOOL bIsHighlighted, BOOL bIsSelected, BOOL bCanCollapse)
{
	ASSERT_VALID(pDC);
	ASSERT(pGroup != NULL);

	const int nIconMargin = 10;

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawTasksGroupCaption (pDC, pGroup, bIsHighlighted, bIsSelected);
		return;
	}

	if (pGroup->m_strName.IsEmpty())
	{
		return;
	}

	// -------------------------------
	// Draw group caption (Windows XP)
	// -------------------------------
	if (pGroup->m_bIsSpecial)
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD,
			0, &pGroup->m_rect, 0);
	}
	else
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
			0, &pGroup->m_rect, 0);
	}

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL 
		&& pGroup->m_sizeIcon.cx < pGroup->m_rect.Width () - pGroup->m_rect.Height());
	if (bShowIcon)
	{
		CPoint pointIcon(pGroup->m_rect.left+1, pGroup->m_rect.bottom - pGroup->m_sizeIcon.cy);
		pDC->DrawIcon(pointIcon, pGroup->m_hIcon);
	}
	
	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject (&globalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bCanCollapse && bIsHighlighted)
	{
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ? 
				::GetSysColor(COLOR_WINDOW) : pGroup->m_clrTextHot);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ? 
				globalData.clrHilite : pGroup->m_clrTextHot);
		}
	}
	else
	{
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ? 
				::GetSysColor(COLOR_WINDOW) : pGroup->m_clrText);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ? 
				globalData.clrHilite : pGroup->m_clrText);
		}
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);
	
	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	
	CRect rectText = pGroup->m_rect;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	+ nIconMargin: 
		(nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset));
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right - pGroup->m_rect.Height());

	pDC->DrawText (pGroup->m_strName, rectText, DT_SINGLELINE);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse)
	{
		CRect rectButton = pGroup->m_rect;
		rectButton.left = max(rectButton.left, rectButton.right - rectButton.Height());
		
		if (pGroup->m_bIsSpecial)
		{
			if (!pGroup->m_bIsCollapsed)
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPCOLLAPSE, EBSGC_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPCOLLAPSE, EBSGC_NORMAL, &rectButton, 0);
				}
			}
			else
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPEXPAND, EBSGE_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPEXPAND, EBSGE_NORMAL, &rectButton, 0);
				}
			}
		}
		else
		{
			if (!pGroup->m_bIsCollapsed)
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPCOLLAPSE, EBNGC_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPCOLLAPSE, EBNGC_NORMAL, &rectButton, 0);
				}
			}
			else
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPEXPAND, EBNGE_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPEXPAND, EBNGE_NORMAL, &rectButton, 0);
				}
			}
		}
	}
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial)
{
	ASSERT_VALID(pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGVisualManagerXP::OnFillTasksGroupInterior (pDC, rect);
		return;
	}

	if (!bSpecial)
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
			EBP_NORMALGROUPBACKGROUND, 0, &rect, 0);
	}
	else
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
			EBP_SPECIALGROUPBACKGROUND, 0, &rect, 0);
	}
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial, 
														BOOL bNoTitle)
{
	if (m_hThemeExplorerBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawTasksGroupAreaBorder(pDC, rect, bSpecial, bNoTitle);
		return;
	}

	ASSERT_VALID(pDC);

	// Draw underline
	if (bNoTitle)
	{
		CRect rectDraw = rect;
		rectDraw.bottom = rectDraw.top + 1;
		
		if (bSpecial)
		{
			(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD,
				0, &rectDraw, 0);
		}
		else
		{
			(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
				0, &rectDraw, 0);
		}
	}
	
	return;
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
										BOOL bIsHighlighted, BOOL bIsSelected)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT(pTask != NULL);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawTask (pDC, pTask, pIcons, bIsHighlighted, bIsSelected);
		return;
	}

	if (pTask->m_bIsSeparator)
	{
		// --------------
		// Draw separator
		// --------------
		CRect rectDraw = pTask->m_rect;
		rectDraw.top = pTask->m_rect.CenterPoint ().y;
		rectDraw.bottom = rectDraw.top + 1;

		// draw same as group caption
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
			0, &rectDraw, 0);
		return;
	}
	
	// ---------
	// Draw icon
	// ---------
	CRect rectText = pTask->m_rect;
	CSize sizeIcon(0, 0);
	::ImageList_GetIconSize (pIcons->m_hImageList, (int*) &sizeIcon.cx, (int*) &sizeIcon.cy);
	if (pTask->m_nIcon >= 0 && sizeIcon.cx > 0)
	{
		pIcons->Draw (pDC, pTask->m_nIcon, rectText.TopLeft (), ILD_TRANSPARENT);
	}
	int nTaskPaneOffset = pTask->m_pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	rectText.left += sizeIcon.cx + (nTaskPaneOffset != -1 ? nTaskPaneOffset : m_nTasksIconHorzOffset);

	// ---------
	// Draw text
	// ---------
	BOOL bIsLabel = (pTask->m_uiCommandID == 0);

	CFont* pFontOld = NULL;
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bIsLabel)
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			::GetSysColor(COLOR_WINDOWTEXT) : pTask->m_clrText);
	}
	else if (!pTask->m_bEnabled)
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (globalData.clrGrayedText);
	}
	else if (bIsHighlighted)
	{
		pFontOld = pDC->SelectObject (&globalData.fontUnderline);
		pDC->SetTextColor (pTask->m_clrTextHot == (COLORREF)-1 ?
			globalData.clrHilite : pTask->m_clrTextHot);
	}
	else
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			globalData.clrHilite : pTask->m_clrText);
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	CBCGTasksPane* pTaskPane = pTask->m_pGroup->m_pPage->m_pTaskPane;
	ASSERT_VALID (pTaskPane);

	BOOL bMultiline = bIsLabel ? 
		pTaskPane->IsWrapLabelsEnabled () : pTaskPane->IsWrapTasksEnabled ();

	if (bMultiline)
	{
		pDC->DrawText (pTask->m_strName, rectText, DT_WORDBREAK);
	}
	else
	{
		pDC->DrawText (pTask->m_strName, rectText, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);
}
//************************************************************************************
void CBCGWinXPVisualManager::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brBarFace);

	if (bHilited)
	{
		CBCGDrawManager dm (*pDC);
		dm.HighlightRect (rect);

		pDC->Draw3dRect (rect,
			globalData.clrBarHilite,
			globalData.clrBarDkShadow);
	}

	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}
//*********************************************************************************
void CBCGWinXPVisualManager::OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, 
											CBCGButton* pButton)
{
	if (m_hThemeButton == NULL)
	{
		CBCGVisualManagerXP::OnEraseOutlookCaptionButton (pDC, rect, pButton);
	}
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT uiState)
{
	if (m_hThemeButton == NULL)
	{
		CBCGVisualManagerXP::OnDrawOutlookCaptionButtonBorder (pDC, rect, 
												 pButton, uiState);
		return;
	}

	int nState = PBS_HOT;

	if (!pButton->IsWindowEnabled ())
	{
		nState = PBS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = PBS_NORMAL;
	}

	CRect rect1 = rect;
	rect1.InflateRect (1, 1);

	pDC->FillRect (rect1, &globalData.brBarFace);

	if (nState == PBS_HOT)
	{
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON,
		nState, &rect1, 0);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawControlBorder (CWnd* pWndCtrl)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGVisualManagerXP::OnDrawControlBorder (pWndCtrl);
		return;
	}

	ASSERT_VALID (pWndCtrl);

	CWindowDC dc (pWndCtrl);

	CRect rect;
	pWndCtrl->GetWindowRect (rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;
	
	COLORREF clrBorder = (COLORREF)-1;

	if ((*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, 3801, &clrBorder) != S_OK)
	{
		CBCGVisualManagerXP::OnDrawControlBorder (pWndCtrl);
		return;
	}

	dc.Draw3dRect (&rect, clrBorder, clrBorder);
	rect.DeflateRect (1, 1);
	dc.Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
}
//**********************************************************************************
BOOL CBCGWinXPThemeManager::DrawPushButton (CDC* pDC, CRect rect, 
											 CBCGButton* pButton, UINT /*uiState*/)
{
	#define BP_PUSHBUTTON		1

	#define PBS_NORMAL			1
	#define PBS_HOT				2
	#define PBS_PRESSED			3
	#define PBS_DISABLED		4
	#define PBS_DEFAULTED		5

	if (m_hThemeButton == NULL)
	{
		return FALSE;
	}

	int nState = PBS_NORMAL;

	if (!pButton->IsWindowEnabled ())
	{
		nState = PBS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = PBS_HOT;
	}

	globalData.DrawParentBackground (pButton, pDC, rect);

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, 
		nState, &rect, 0);

	return TRUE;
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawSpinButtons (CDC* pDC, CRect rect, 
	int nState, BOOL bOrientation, CBCGSpinButtonCtrl* pSpinCtrl)
{
	if (m_hThemeSpin == NULL)
	{
		CBCGVisualManagerXP::OnDrawSpinButtons (pDC, rect, 
			nState, bOrientation, pSpinCtrl);
		return;
	}

	// Draw up part:
	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint ().y - 1;

	int nDrawState = UPS_NORMAL;

	if (nState & SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & SPIN_PRESSEDUP)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & SPIN_HIGHLIGHTEDUP)
	{
		nDrawState = UPS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeSpin, pDC->GetSafeHdc(), 
		bOrientation ? SPNP_UPHORZ : SPNP_UP, nDrawState, &rectUp, 0);

	// Draw up part:
	CRect rectDown = rect;
	rectDown.top = rect.CenterPoint ().y;

	nDrawState = UPS_NORMAL;

	if (nState & SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & SPIN_PRESSEDDOWN)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & SPIN_HIGHLIGHTEDDOWN)
	{
		nDrawState = UPS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeSpin, pDC->GetSafeHdc(), 
		bOrientation ? SPNP_DOWNHORZ : SPNP_DOWN, nDrawState, &rectDown, 0);
}
//*************************************************************************************
void CBCGWinXPVisualManager::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsLeftRightRounded ())
	{
		CBCGVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	int nState = TIS_NORMAL;
	if (bIsActive)
	{
		nState = TIS_SELECTED;
	}
	else if (iTab == pTabWnd->GetHighlightedTab ())
	{
		nState = TIS_HOT;
	}

	rectTab.right++;

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_TOP && !bIsActive)
	{
		rectTab.bottom--;
	}

	(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_TABITEM, nState, &rectTab, 0);

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		CBCGDrawManager dm (*pDC);
		dm.MirrorRect (rectTab, FALSE);
	}

	COLORREF clrTabText = globalData.clrWindowText;
	if (m_pfGetThemeColor != NULL)
	{
		(*m_pfGetThemeColor) (m_hThemeTab, TABP_TABITEM, nState, TMT_TEXTCOLOR, &clrTabText);
	}
	
	COLORREF cltTextOld = pDC->SetTextColor (clrTabText);

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	pDC->SetTextColor (cltTextOld);
}
//***********************************************************************************
void CBCGWinXPVisualManager::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsDialogControl ())
	{
		CBCGVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	rect.right += 10;

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		rect.top -= 3;

		CBCGMemDC memDC (*pDC, (CWnd*) pTabWnd);

		(*m_pfDrawThemeBackground) (m_hThemeTab, memDC.GetDC ().GetSafeHdc (), TABP_PANE, 0, &rect, 
			NULL);

		CBCGDrawManager dm (memDC.GetDC ());
		dm.MirrorRect (rect, FALSE);
	}
	else
	{
		rect.bottom += 2;

		(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_PANE, 0, &rect, 0);
	}
}
//***********************************************************************************
BOOL CBCGWinXPVisualManager::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		return CBCGVisualManagerXP::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGWinXPVisualManager::OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
			CRect rectSizeBox)
{
	if (m_hThemeScrollBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawStatusBarSizeBox (pDC, pStatBar, rectSizeBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX,
		SZB_RIGHTALIGN, &rectSizeBox, 0);
}
//************************************************************************************
void CBCGWinXPVisualManager::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTree == NULL)
	{
		CBCGVisualManagerXP::OnDrawExpandingBox (pDC, rect, bIsOpened, colorBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeTree, pDC->GetSafeHdc(), TVP_GLYPH,
		bIsOpened ? GLPS_OPENED : GLPS_CLOSED, &rect, 0);
}
//**************************************************************************************
void CBCGWinXPVisualManager::OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
			BOOL bProgressText)
{
	if (!DrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText))
	{
		CBCGVisualManagerXP::OnDrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText);
	}
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnErasePopupWindowButton (CDC* pDC, CRect rect, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (m_hThemeButton == NULL || pButton->IsCaptionButton ())
	{
		CBCGVisualManagerXP::OnErasePopupWindowButton (pDC, rect, pButton);
		return;
	}

	CRect rectParent;
	pButton->GetParent ()->GetClientRect (rectParent);

	pButton->GetParent ()->MapWindowPoints (pButton, rectParent);
	OnFillPopupWindowBackground (pDC, rectParent);
}
//**********************************************************************************
void CBCGWinXPVisualManager::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rect, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pButton);

	int nState = PBS_NORMAL;

	if (!pButton->IsWindowEnabled ())
	{
		nState = PBS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = PBS_HOT;
	}
	else
	{
		nState = PBS_NORMAL;
	}

	if (m_hThemeWindow != NULL && pButton->IsCloseButton () && pButton->IsCaptionButton ())
	{
		(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), 
			WP_SMALLCLOSEBUTTON, nState, &rect, 0);
		return;
	}

	if (m_hThemeButton == NULL)
	{
		CBCGVisualManagerXP::OnDrawPopupWindowButtonBorder (pDC, rect, pButton);
		return;
	}

	globalData.DrawParentBackground (pButton, pDC, rect);
	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
}
//**********************************************************************************
BOOL CBCGWinXPVisualManager::IsDefaultWinXPPopupButton (CBCGPopupWndButton* pButton) const
{
	ASSERT_VALID (pButton);
	return m_hThemeWindow != NULL && pButton->IsCloseButton () && pButton->IsCaptionButton ();
}
