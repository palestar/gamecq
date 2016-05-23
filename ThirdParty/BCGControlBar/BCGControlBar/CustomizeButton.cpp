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

// CustomizeButton.cpp: implementation of the CCustomizeButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomizeButton.h"
#include "Globals.h"
#include "BCGToolbar.h"
#include "MenuImages.h"
#include "BCGToolbarComboBoxButton.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CCustomizeButton, CBCGToolbarMenuButton, VERSIONABLE_SCHEMA | 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomizeButton::CCustomizeButton()
{
	CommonInit ();
}
//****************************************************************************************
CCustomizeButton::CCustomizeButton(UINT uiCustomizeCmdId, const CString& strCustomizeText)
{
	CommonInit ();

	m_uiCustomizeCmdId = uiCustomizeCmdId;
	m_strCustomizeText = strCustomizeText;
}
//****************************************************************************************
void CCustomizeButton::CommonInit ()
{
	m_uiCustomizeCmdId = 0;
	m_bIsEmpty = FALSE;
	m_bDefaultDraw = TRUE;
	m_sizeExtra = CSize (0, 0);
	m_pWndParentToolbar = NULL;

	m_bIsPipeStyle = TRUE;
	m_bOnRebar = FALSE;
	m_bMenuRightAlign = TRUE;
}
//****************************************************************************************
CCustomizeButton::~CCustomizeButton()
{
}
//****************************************************************************************
void CCustomizeButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	m_pWndParentToolbar = DYNAMIC_DOWNCAST (CBCGToolBar, pWndParent);
	m_pWndParent = pWndParent;
	m_bText = FALSE;
	m_bIsEmpty = FALSE;
	m_bOnRebar = DYNAMIC_DOWNCAST (CReBar, pWndParent->GetParent ()) != NULL;
}
//****************************************************************************************
void CCustomizeButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* /*pImages*/,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (m_bMenuMode)
	{
		ASSERT (FALSE);	// Customize button is available for 
						// the "pure" toolbars only!
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CRect rectBorder = rect;

	//----------------------
	// Fill button interior:
	//----------------------
	m_bDefaultDraw = TRUE;

	FillInterior (pDC, rectBorder, bHighlight || IsDroppedDown ());

	int nMargin = CBCGVisualManager::GetInstance ()->GetToolBarCustomizeButtonMargin ();

	if (m_bDefaultDraw)
	{
		CSize sizeImage = CMenuImages::Size ();

		if (CBCGToolBar::IsLargeIcons ())
		{
			sizeImage.cx *= 2;
			sizeImage.cy *= 2;
		}

		if ((int) m_uiCustomizeCmdId > 0)
		{
			//-----------------
			// Draw menu image:
			//-----------------
			CRect rectMenu = rect;
			if (bHorz)
			{
				rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
			}
			else
			{
				rectMenu.right = rectMenu.left + sizeImage.cx + 2 * nMargin;
			}

			rectMenu.DeflateRect (
				(rectMenu.Width () - sizeImage.cx) / 2,
				(rectMenu.Height () - sizeImage.cy) / 2);

			if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)


			{
				if (!CBCGVisualManager::GetInstance ()->IsMenuFlatLook ())
				{
					rectMenu.OffsetRect (1, 1);
				}
			}

			CMenuImages::Draw (	pDC, 
				bHorz ? CMenuImages::IdArowDown : CMenuImages::IdArowLeft,
								rectMenu.TopLeft (), sizeImage);
		}

		if (!m_lstInvisibleButtons.IsEmpty ())
		{
			//-------------------
			// Draw "more" image:
			//-------------------
			CRect rectMore = rect;
			if (bHorz)
			{
				rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
			}
			else
			{
				rectMore.left = rectMore.right - sizeImage.cx - 2 * nMargin;
			}

			rectMore.DeflateRect (
				(rectMore.Width () - sizeImage.cx) / 2,
				(rectMore.Height () - sizeImage.cy) / 2);

			if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
			{
				if (!CBCGVisualManager::GetInstance ()->IsMenuFlatLook ())
				{
					rectMore.OffsetRect (1, 1);
				}
			}

			CMenuImages::Draw (	pDC, 
								bHorz ? CMenuImages::IdMoreButtons : CMenuImages::IdArowShowAll, 
								rectMore.TopLeft (),
								sizeImage);
		}
	}
	//--------------------
	// Draw button border:
	//--------------------
	if (!bCustomizeMode)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ||
			m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGVisualManager::ButtonsIsPressed);
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGVisualManager::ButtonsIsHighlighted);
		}
	}
}
//*****************************************************************************************
CBCGPopupMenu* CCustomizeButton::CreatePopupMenu ()
{
	if (CBCGToolBar::m_bAltCustomizeMode ||	CBCGToolBar::IsCustomizeMode ())
	{
		return NULL;
	}

	CBCGPopupMenu* pMenu = CBCGToolbarMenuButton::CreatePopupMenu ();
	if (pMenu == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	if (m_pWndParentToolbar->IsLocked ())
	{
		pMenu->GetMenuBar ()->m_pRelatedToolbar = m_pWndParentToolbar;
	}

	pMenu->m_bRightAlign = m_bMenuRightAlign && 
		(m_pWndParentToolbar->GetExStyle() & WS_EX_LAYOUTRTL) == 0;

	BOOL bIsLocked = (m_pWndParentToolbar == NULL || 
					m_pWndParentToolbar->IsLocked ());

	BOOL bIsFirst = TRUE;

	for (POSITION pos = m_lstInvisibleButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_lstInvisibleButtons.GetNext (pos);
		ASSERT_VALID (pButton);

		//--------------------------------------
		// Don't insert first or last separator:
		//--------------------------------------
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (bIsFirst)
			{
				continue;
			}

			if (pos == NULL)	// Last
			{
				break;
			}
		}

		int iIndex = -1;

		bIsFirst = FALSE;

		if (pButton->IsKindOf (RUNTIME_CLASS (CBCGToolbarMenuButton)))
		{
			iIndex = pMenu->InsertItem (*((CBCGToolbarMenuButton*) pButton));
		}
		else
		{
			if (pButton->m_nID == 0)
			{
				iIndex = pMenu->InsertSeparator ();
			}
			else
			{
				iIndex = pMenu->InsertItem (
					CBCGToolbarMenuButton (pButton->m_nID, NULL, 
						bIsLocked ? - 1 : pButton->GetImage (),
						pButton->m_strText,
						pButton->m_bUserButton));
			}
		}

		if (iIndex < 0)
		{
			ASSERT (FALSE);
			continue;
		}

		CBCGToolbarMenuButton* pMenuButton = pMenu->GetMenuItem (iIndex);
		if (pMenuButton == NULL)
		{
			ASSERT (pMenuButton != NULL);
			continue;
		}

		//-----------------------------------------------------
		// Text may be undefined, bring it from the tooltip :-(
		//-----------------------------------------------------
		if ((pMenuButton->m_strText.IsEmpty () || 
			pButton->IsKindOf (RUNTIME_CLASS (CBCGToolbarComboBoxButton)))
				&& pMenuButton->m_nID != 0)
		{
			CString strMessage;
			int iOffset;
			if (strMessage.LoadString (pMenuButton->m_nID) &&
				(iOffset = strMessage.Find (_T('\n'))) != -1)
			{
				pMenuButton->m_strText = strMessage.Mid (iOffset + 1);
			}
		}

        pMenuButton->m_bText = TRUE;
	}

	if ((int) m_uiCustomizeCmdId > 0)
	{
		if (!m_lstInvisibleButtons.IsEmpty ())
		{
			pMenu->InsertSeparator ();
		}

		if (m_pWndParentToolbar->IsAddRemoveQuickCustomize ())
		{
			CControlBarInfo pBarInfoMain;
			m_pWndParentToolbar->GetBarInfo(&pBarInfoMain);

			CBCGPopupMenu* pMenuCustomize = new CBCGPopupMenu ();

			CDockBar* pDockBar = m_pWndParentToolbar->m_pDockBar;
			if (pDockBar != NULL)
			{
				ASSERT_VALID (pDockBar);

				for (int i=0; i< pDockBar->m_arrBars.GetSize (); i++)
				{
					CControlBar* pControlBar = (CControlBar*)pDockBar->m_arrBars[i];
					if (HIWORD (pControlBar) == 0)
					{
						continue;
					}

					CBCGToolBar* pBar = DYNAMIC_DOWNCAST (CBCGToolBar, pControlBar);
					if (pBar != NULL )
					{
						ASSERT_VALID (pBar);
						if (!pBar->IsAddRemoveQuickCustomize ())
						{
						 	continue;
						}

						CControlBarInfo pBarInfo;
						pBar->GetBarInfo(&pBarInfo);

						if (pBarInfo.m_pointPos.y == pBarInfoMain.m_pointPos.y)
						{
							CString strCaption;
							pBar->GetWindowText (strCaption);

							strCaption.TrimLeft();
							strCaption.TrimRight();

							if (!strCaption.GetLength ())
							{
								CBCGLocalResource locaRes;
								strCaption.LoadString (IDS_BCGBARRES_UNTITLED_TOOLBAR);
							}

							CString strToolId;
							strToolId.Format(_T("%d"), pBar->GetDlgCtrlID ());	
								
							//------------------------
							// Insert Dummy Menu Item
							//------------------------
							CBCGPopupMenu* pMenuDummy = new CBCGPopupMenu ();
							pMenuDummy->InsertItem (CBCGToolbarMenuButton (1, NULL, -1, strToolId)); 

							CBCGToolbarMenuButton btnToolCaption ((UINT)-1, 
								pMenuDummy->GetMenuBar()->ExportToMenu (), -1, strCaption); 

							pMenuCustomize->InsertItem(btnToolCaption);
							delete pMenuDummy;
							
						}
					}
				}

				CBCGToolbarMenuButton btnStandard (m_uiCustomizeCmdId, NULL, -1,
					m_strCustomizeText);

				pMenuCustomize->InsertItem (btnStandard);

				CString strLabel;

				{
					CBCGLocalResource locaRes;
					strLabel.LoadString (IDS_BCGBARRES_ADD_REMOVE_BTNS);
				}

				CBCGToolbarMenuButton	btnAddRemove ((UINT)-1,
					pMenuCustomize->GetMenuBar ()->ExportToMenu (),	-1,	strLabel);

				btnAddRemove.EnableQuickCustomize ();

				delete pMenuCustomize;

				pMenu->InsertItem (btnAddRemove);
				pMenu->SetQuickMode ();
				pMenu->SetQuickCustomizeType (CBCGPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE);

			}
			else
			{
				CString strCaption;
				m_pWndParentToolbar->GetWindowText (strCaption);

				strCaption.TrimLeft();
				strCaption.TrimRight();

				if (!strCaption.GetLength ())
				{
					CBCGLocalResource locaRes;
					strCaption.LoadString (IDS_BCGBARRES_UNTITLED_TOOLBAR);
				}

				CString strToolId;
				strToolId.Format(_T("%d"), m_pWndParentToolbar->GetDlgCtrlID ());	
					
				//------------------------
				// Insert Dummy Menu Item
				//------------------------
				CBCGPopupMenu* pMenuDummy = new CBCGPopupMenu ();
				pMenuDummy->InsertItem (CBCGToolbarMenuButton (1, NULL, -1, strToolId)); 

				CBCGToolbarMenuButton btnToolCaption ((UINT)-1, 
					pMenuDummy->GetMenuBar()->ExportToMenu (), -1, strCaption); 

				pMenuCustomize->InsertItem(btnToolCaption);
				delete pMenuDummy;

					CBCGToolbarMenuButton btnStandard (m_uiCustomizeCmdId, NULL, -1,
					m_strCustomizeText);

				pMenuCustomize->InsertItem (btnStandard);

				CString strLabel;

				{
					CBCGLocalResource locaRes;
					strLabel.LoadString (IDS_BCGBARRES_ADD_REMOVE_BTNS);
				}

				CBCGToolbarMenuButton	btnAddRemove ((UINT)-1,
					pMenuCustomize->GetMenuBar ()->ExportToMenu (),	-1,	strLabel);

				btnAddRemove.EnableQuickCustomize ();

				delete pMenuCustomize;

				pMenu->InsertItem (btnAddRemove);
				pMenu->SetQuickMode ();
				pMenu->SetQuickCustomizeType (CBCGPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE);
			}
		}
		else // for old versions (< 6.4) compatibility.
		{
			pMenu->InsertItem (CBCGToolbarMenuButton (m_uiCustomizeCmdId, NULL, -1,
				m_strCustomizeText));
		}
	}

	//-----------------------------------------------------------
	// All menu commands should be routed via the same window as
	// parent toolbar commands:
	//-----------------------------------------------------------
	if (m_pWndParentToolbar != NULL)
	{
		pMenu->m_pMessageWnd = m_pWndParentToolbar->GetOwner ();
	}

	return pMenu;
}
//*****************************************************************************************
SIZE CCustomizeButton::OnCalculateSize (CDC* /*pDC*/, const CSize& sizeDefault, BOOL bHorz)
{
	if (m_bIsEmpty)
	{
		return CSize (0, 0);
	}

	if (m_strText.IsEmpty ())
	{
		CBCGLocalResource locaRes;
		m_strText.LoadString (IDS_BCGBARRES_TOOLBAR_OPTIONS);

		ASSERT (!m_strText.IsEmpty ());
	}

	if (m_pWndParentToolbar != NULL && m_pWndParentToolbar->IsFloating ())
	{
		return CSize (0, 0);
	}

	int nMargin = CBCGVisualManager::GetInstance ()->GetToolBarCustomizeButtonMargin ();
	const int xLargeIcons = CBCGToolBar::IsLargeIcons () ? 2 : 1;

	if (bHorz)
	{
		return CSize (	CMenuImages::Size ().cx * xLargeIcons + 2 * nMargin, 
						sizeDefault.cy);
	}
	else
	{
		return CSize (	sizeDefault.cx, 
						CMenuImages::Size ().cy * xLargeIcons + 2 * nMargin);
	}
}
//*****************************************************************************************
void CCustomizeButton::CopyFrom (const CBCGToolbarButton& s)
{
	CBCGToolbarMenuButton::CopyFrom (s);
	const CCustomizeButton& src = (const CCustomizeButton&) s;

	m_uiCustomizeCmdId = src.m_uiCustomizeCmdId;
	m_strCustomizeText = src.m_strCustomizeText;
	m_bIsEmpty = src.m_bIsEmpty;
	m_bIsPipeStyle = src.m_bIsPipeStyle;
	m_bMenuRightAlign = src.m_bMenuRightAlign;
}
//*********************************************************************************
void CCustomizeButton::OnCancelMode ()
{
	CBCGToolbarMenuButton::OnCancelMode ();

	if (m_sizeExtra != CSize (0, 0) && m_pWndParentToolbar != NULL)
	{
		int nIndex = m_pWndParentToolbar->ButtonToIndex (this);
		if (nIndex >= 0)
		{
			m_pWndParentToolbar->InvalidateButton (nIndex);
		}
	}

}
//********************************************************************************
BOOL CCustomizeButton::InvokeCommand (CBCGPopupMenuBar* pMenuBar, 
		const CBCGToolbarButton* pButton)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMenuBar);
	ASSERT_VALID (pButton);

	if (m_pWndParentToolbar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndParentToolbar);

	int nIndex = pMenuBar->ButtonToIndex (pButton);
	if (nIndex < 0)
	{
		return FALSE;
	}

	//-------------------------
	// Close active popup menu:
	//-------------------------
	if (CBCGPopupMenu::GetActiveMenu () != NULL &&
		::IsWindow (CBCGPopupMenu::GetActiveMenu ()->m_hWnd))
	{
		CBCGPopupMenu::GetActiveMenu ()->PostMessage (WM_CLOSE);
	}

	if (m_lstInvisibleButtons.GetCount ()  > 0 )
	{
		CBCGToolbarButton* pButtonHead = (CBCGToolbarButton*)m_lstInvisibleButtons.GetHead ();
		if (pButtonHead->m_nStyle & TBBS_SEPARATOR)
		{
			nIndex++;
		}
	}

	POSITION pos = m_lstInvisibleButtons.FindIndex (nIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	CBCGToolbarButton* pToolbarButton = 
		(CBCGToolbarButton*) m_lstInvisibleButtons.GetAt (pos);
	ASSERT_VALID (pToolbarButton);

	UINT nIDCmd = pToolbarButton->m_nID;

	if (!m_pWndParentToolbar->OnSendCommand (pToolbarButton) &&
		nIDCmd != 0 && nIDCmd != (UINT) -1)
	{
		CBCGToolBar::AddCommandUsage (nIDCmd);

		if (!pToolbarButton->OnClickUp () && 
			(g_pUserToolsManager == NULL ||
			!g_pUserToolsManager->InvokeTool (nIDCmd)))
		{
			m_pWndParentToolbar->GetOwner()->PostMessage (WM_COMMAND, nIDCmd);    // send command
		}
	}

	return TRUE;
}
