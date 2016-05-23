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

// bcgtoolbar.cpp : definition of CBCGToolBar
//
// This code is based on the Microsoft Visual C++ sample file
// TOOLBAR.C from the OLDBARS example
//

#include "stdafx.h"

#include "bcgbarres.h"
#include "BCGtoolbar.h"
#include "BCGMenuBar.h"
#include "BCGToolbarButton.h"
#include "BCGToolbarDropSource.h"
#include "ButtonAppearanceDlg.h"
#include "CBCGToolbarCustomize.h"
#include "bcglocalres.h"
#include "BCGRegistry.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGKeyboardManager.h"
#include "BCGToolbarMenuButton.h"
#include "BCGToolbarSystemMenuButton.h"
#include "BCGPopupMenu.h"
#include "CustomizeButton.h"
#include "BCGCommandManager.h"
#include "RegPath.h"
#include "trackmouse.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"
#include "BCGUserToolsManager.h"
#include "bcgsound.h"
#include "BCGDockBar.h"
#include "BCGVisualManager.h"
#include "BCGDropDown.h"
#include "BCGWorkspace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define TEXT_MARGIN			3
#define STRETCH_DELTA		6
#define BUTTON_MIN_WIDTH	5
#define LINE_OFFSET			5

#define REG_SECTION_FMT					_T("%sBCGToolBar-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGToolBar-%d%x")
#define REG_PARAMS_FMT					_T("%sBCGToolbarParameters")
#define REG_ENTRY_NAME					_T("Name")
#define REG_ENTRY_BUTTONS				_T("Buttons")
#define REG_ENTRY_ORIG_ITEMS			_T("OriginalItems")
#define REG_ENTRY_TOOLTIPS				_T("Tooltips")
#define REG_ENTRY_KEYS					_T("ShortcutKeys")
#define REG_ENTRY_LARGE_ICONS			_T("LargeIcons")
#define REG_ENTRY_ANIMATION				_T("MenuAnimation")
#define REG_ENTRY_RU_MENUS				_T("RecentlyUsedMenus")
#define REG_ENTRY_MENU_SHADOWS			_T("MenuShadows")
#define REG_ENTRY_SHOW_ALL_MENUS_DELAY	_T("ShowAllMenusAfterDelay")
#define REG_ENTRY_CMD_USAGE_COUNT		_T("CommandsUsage")
#define REG_ENTRY_LOOK2000				_T("Look2000")
#define REG_ENTRY_RESET_ITEMS			_T("OrigResetItems")

static const CString strToolbarProfile	= _T("BCGToolBars");

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar notification messages:

UINT BCGM_TOOLBARMENU		= ::RegisterWindowMessage (_T("BCGTOOLBAR_POPUPMENU"));
UINT BCGM_CUSTOMIZETOOLBAR	= ::RegisterWindowMessage (_T("BCGTOOLBAR_CUSTOMIZE"));
UINT BCGM_CREATETOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_CREATE"));
UINT BCGM_DELETETOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_DELETE"));
UINT BCGM_CUSTOMIZEHELP		= ::RegisterWindowMessage (_T("BCGTOOLBAR_CUSTOMIZEHELP"));
UINT BCGM_RESETTOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETTOOLBAR"));
UINT BCGM_RESETMENU			= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETMENU"));
UINT BCGM_SHOWREGULARMENU	= ::RegisterWindowMessage (_T("BCGTOOLBAR_SHOWREGULARMENU"));
UINT BCGM_RESETCONTEXTMENU	= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETCONTEXTMENU"));
UINT BCGM_RESETKEYBOARD		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETKEYBAORD"));
UINT BCGM_RESETRPROMPT		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETRPROMPT"));


const UINT uiAccPopupTimerDelay  = 1300; 
const UINT uiAccTimerDelay  = 500;   
const UINT uiAccNotifyEvent = 20;

/////////////////////////////////////////////////////////////////////////////
// All CBCGToolBar collection:
CObList	gAllToolbars;

BOOL CBCGToolBar::m_bCustomizeMode = FALSE;
BOOL CBCGToolBar::m_bAltCustomizeMode = FALSE;
BOOL CBCGToolBar::m_bShowTooltips = TRUE;
BOOL CBCGToolBar::m_bShowShortcutKeys = TRUE;
BOOL CBCGToolBar::m_bLargeIcons = FALSE;
BOOL CBCGToolBar::m_bAutoGrayInactiveImages = FALSE;
int  CBCGToolBar::m_nGrayImagePercentage = 0;

BOOL CBCGToolBar::m_bDisableLabelsEdit = FALSE;

#ifndef BCG_NO_CUSTOMIZATION

CBCGToolbarDropSource CBCGToolBar::m_DropSource;

#endif // BCG_NO_CUSTOMIZATION

CBCGToolBarImages	CBCGToolBar::m_Images;
CBCGToolBarImages	CBCGToolBar::m_ColdImages;
CBCGToolBarImages	CBCGToolBar::m_MenuImages;
CBCGToolBarImages	CBCGToolBar::m_DisabledImages;
CBCGToolBarImages	CBCGToolBar::m_DisabledMenuImages;
CBCGToolBarImages	CBCGToolBar::m_LargeImages;
CBCGToolBarImages	CBCGToolBar::m_LargeColdImages;
CBCGToolBarImages	CBCGToolBar::m_LargeDisabledImages;

CBCGToolBarImages*	CBCGToolBar::m_pUserImages = NULL;

CSize CBCGToolBar::m_sizeButton = CSize (23, 22);
CSize CBCGToolBar::m_sizeImage	= CSize (16, 15);
CSize CBCGToolBar::m_sizeCurButton = CSize (23, 22);
CSize CBCGToolBar::m_sizeCurImage	= CSize (16, 15);
CSize CBCGToolBar::m_sizeMenuImage	= CSize (-1, -1);
CSize CBCGToolBar::m_sizeMenuButton	= CSize (-1, -1);

double CBCGToolBar::m_dblLargeImageRatio = 2.;
BOOL CBCGToolBar::m_bExtCharTranslation = FALSE;

CMap<UINT, UINT, int, int> CBCGToolBar::m_DefaultImages;

COLORREF CBCGToolBar::m_clrTextHot = (COLORREF) -1;
extern CBCGToolbarCustomize* g_pWndCustomize;

HHOOK CBCGToolBar::m_hookMouseHelp = NULL;
CBCGToolBar* CBCGToolBar::m_pLastHookedToolbar = NULL;

CList<UINT, UINT> CBCGToolBar::m_lstUnpermittedCommands;
CList<UINT, UINT> CBCGToolBar::m_lstBasicCommands;

CCmdUsageCount	CBCGToolBar::m_UsageCount;

BOOL CBCGToolBar::m_bAltCustomization = FALSE;
CBCGToolBar* CBCGToolBar::m_pSelToolbar = NULL;

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

CBCGToolBarParams::CBCGToolBarParams()
{
	m_uiColdResID = 0;
	m_uiHotResID = 0;
	m_uiDisabledResID = 0;
	m_uiLargeColdResID = 0;
	m_uiLargeHotResID = 0;
	m_uiLargeDisabledResID = 0;
	m_uiMenuResID = 0;
	m_uiMenuDisabledResID = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar

IMPLEMENT_SERIAL(CBCGToolBar, CControlBar, VERSIONABLE_SCHEMA | 1)

#pragma warning (disable : 4355)

CBCGToolBar::CBCGToolBar() :
	m_bMenuMode (FALSE),
	m_Impl (this),
	m_bIgnoreSetText (FALSE)
{
	m_iButtonCapture = -1;      // nothing captured
	m_iHighlighted = -1;
	m_iSelected = -1;
	m_iHot = -1;

	m_iDragIndex = -1;
	m_rectDrag.SetRectEmpty ();
	m_pDragButton = NULL;
	m_ptStartDrag = CPoint (-1, -1);
	m_bIsDragCopy = FALSE;

	m_bMasked = FALSE;
	m_bPermament = FALSE;

	m_pCustomizeBtn = NULL;

	//---------------------
	// UISG standard sizes:
	//---------------------
	m_cyTopBorder = m_cyBottomBorder = 1;   // 1 pixel for top/bottom gaps

	m_sizeCurButtonLocked = CSize (23, 22);
	m_sizeCurImageLocked = CSize (16, 15);
	m_sizeButtonLocked = CSize (23, 22);
	m_sizeImageLocked = CSize (16, 15);

	m_bStretchButton = FALSE;
	m_rectTrack.SetRectEmpty ();

	m_iImagesOffset = 0;
	m_uiOriginalResID = 0;

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint (-1, -1);
	m_pWndLastCapture = NULL;
	m_hwndLastFocus = NULL;

	m_bLocked = FALSE;
	m_bShowHotBorder = TRUE;
	m_bGrayDisabledButtons = TRUE;
	m_bLargeIconsAreEnbaled = TRUE;

	m_bTextLabels = FALSE;
	m_bDrawTextLabels = FALSE;
	m_nMaxBtnHeight = 0;

	m_bDisableControlsIfNoHandler = TRUE;
	m_bRouteCommandsViaFrame = TRUE;

	m_bResourceWasChanged = FALSE;

	m_bQuickCustomize = FALSE;

	m_iAccHotItem = -1;
	m_bRoundShape = FALSE;
}

#pragma warning (default : 4355)

//******************************************************************************************
CBCGToolBar::~CBCGToolBar()
{
	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	while(!m_OrigResetButtons.IsEmpty())
	{
		delete m_OrigResetButtons.RemoveHead ();
	}

	RemoveAllButtons ();
}
//******************************************************************************************
BOOL CBCGToolBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CBCGToolBar::CreateEx (pParentWnd, TBSTYLE_FLAT,
							dwStyle,
							CRect(1, 1, 1, 1),
							nID);
}
//******************************************************************************************
BOOL CBCGToolBar::CreateEx (CWnd* pParentWnd, 
							DWORD dwCtrlStyle,
							DWORD dwStyle,
							CRect rcBorders,
							UINT nID)
{
	dwStyle |= CBRS_GRIPPER;

	if (pParentWnd != NULL)
	{
		ASSERT_VALID(pParentWnd);   // must have a parent
	}

	if (rcBorders.left < 1)
	{
		rcBorders.left = 1;	// Otherwise, I have a problem woith a "double" grippers
	}

	if (rcBorders.top < 1)
	{
		rcBorders.top = 1;	// Otherwise, I have a problem woith a "double" grippers
	}

	SetBorders (rcBorders);

	//----------------
	// Save the style:
	//----------------
	m_dwStyle = (dwStyle & CBRS_ALL);
	if (nID == AFX_IDW_TOOLBAR)
	{
		m_dwStyle |= CBRS_HIDE_INPLACE;
	}

	dwStyle &= ~CBRS_ALL;
	dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE;
	dwStyle |= dwCtrlStyle;

	//----------------------------
	// Initialize common controls:
	//----------------------------
	VERIFY (AfxDeferRegisterClass (AFX_WNDCOMMCTLS_REG));

	//-----------------
	// Create the HWND:
	//-----------------
	CRect rect;
	rect.SetRectEmpty();

	//-----------------------------
	// Register a new window class:
	//-----------------------------
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT uiClassStyle = CS_DBLCLKS;
	HCURSOR hCursor = ::LoadCursor (NULL, IDC_ARROW);
	HBRUSH hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

	CString strClassName;
	strClassName.Format (_T("BCGToolBar:%x:%x:%x:%x"), 
		(UINT_PTR)hInst, uiClassStyle, (UINT_PTR)hCursor, (UINT_PTR)hbrBackground);

	//---------------------------------
	// See if the class already exists:
	//---------------------------------
	WNDCLASS wndcls;
	if (::GetClassInfo (hInst, strClassName, &wndcls))
	{
		//-----------------------------------------------
		// Already registered, assert everything is good:
		//-----------------------------------------------
		ASSERT (wndcls.style == uiClassStyle);
	}
	else
	{
		//-------------------------------------------
		// Otherwise we need to register a new class:
		//-------------------------------------------
		wndcls.style = uiClassStyle;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = hCursor;
		wndcls.hbrBackground = hbrBackground;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = strClassName;
		
		if (!AfxRegisterClass (&wndcls))
		{
			AfxThrowResourceException();
		}
	}

	if (!CControlBar::Create (strClassName, NULL, dwStyle, rect, pParentWnd, nID))
	{
		return FALSE;
	}

	if (pParentWnd != NULL)
	{
		SetOwner (pParentWnd);
	}

	return TRUE;
}
//******************************************************************************************
void CBCGToolBar::SetSizes (SIZE sizeButton, SIZE sizeImage)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	m_sizeButton = sizeButton;
	m_sizeImage = sizeImage;

	m_sizeCurButton = sizeButton;
	m_sizeCurImage = sizeImage;

	m_Images.SetImageSize (m_sizeImage);
	m_ColdImages.SetImageSize (m_sizeImage);
	m_DisabledImages.SetImageSize (m_sizeImage);

	CSize sizeImageLarge (	(int) (.5 + m_dblLargeImageRatio * m_sizeImage.cx), 
							(int) (.5 + m_dblLargeImageRatio * m_sizeImage.cy));

	m_LargeImages.SetImageSize (sizeImageLarge);
	m_LargeColdImages.SetImageSize (sizeImageLarge);
	m_LargeDisabledImages.SetImageSize (sizeImageLarge);

	if (m_bLargeIcons)
	{
		m_sizeCurButton.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButton.cx);
		m_sizeCurButton.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButton.cy);

		m_sizeCurImage.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImage.cx);
		m_sizeCurImage.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImage.cy);
	}

	if (m_pUserImages != NULL)
	{
		m_pUserImages->SetImageSize (m_sizeImage);
	}
}
//******************************************************************************************
void CBCGToolBar::SetLockedSizes (SIZE sizeButton, SIZE sizeImage)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	m_sizeButtonLocked = sizeButton;
	m_sizeImageLocked = sizeImage;

	m_sizeCurButtonLocked = sizeButton;
	m_sizeCurImageLocked = sizeImage;

	m_ImagesLocked.SetImageSize (m_sizeImageLocked);
	m_MenuImagesLocked.SetImageSize (m_sizeImageLocked);
	m_ColdImagesLocked.SetImageSize (m_sizeImageLocked);
	m_DisabledImagesLocked.SetImageSize (m_sizeImageLocked);

	CSize sizeImageLarge (
		(int) (.5 + m_dblLargeImageRatio * m_sizeImageLocked.cx), 
		(int) (.5 + m_dblLargeImageRatio * m_sizeImageLocked.cy));

	m_LargeImagesLocked.SetImageSize (sizeImageLarge);
	m_LargeColdImagesLocked.SetImageSize (sizeImageLarge);
	m_LargeDisabledImagesLocked.SetImageSize (sizeImageLarge);

	if (m_bLargeIcons)
	{
		m_sizeCurButtonLocked.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButtonLocked.cx);
		m_sizeCurButtonLocked.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButtonLocked.cy);

		m_sizeCurImageLocked.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImageLocked.cx);
		m_sizeCurImageLocked.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImageLocked.cy);
	}
}
//******************************************************************************************
void CBCGToolBar::SetHeight(int cyHeight)
{
	ASSERT_VALID (this);

	int nHeight = cyHeight;
	
	if (m_dwStyle & CBRS_BORDER_TOP)
	{
		cyHeight -= afxData.cyBorder2;
	}

	if (m_dwStyle & CBRS_BORDER_BOTTOM)
	{
		cyHeight -= afxData.cyBorder2;
	}

	m_cyBottomBorder = (cyHeight - GetRowHeight ()) / 2;
	
	//-------------------------------------------------------
	// If there is an extra pixel, m_cyTopBorder will get it:
	//-------------------------------------------------------
	m_cyTopBorder = cyHeight - GetRowHeight () - m_cyBottomBorder;
	
	if (m_cyTopBorder < 0)
	{
		TRACE(_T("Warning: CBCGToolBar::SetHeight(%d) is smaller than button.\n"),
			nHeight);
		m_cyBottomBorder += m_cyTopBorder;
		m_cyTopBorder = 0;  // will clip at bottom
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}
//******************************************************************************************
BOOL CBCGToolBar::SetUserImages (CBCGToolBarImages* pUserImages)
{
	ASSERT (pUserImages != NULL);
	if (!pUserImages->IsValid ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_sizeImage != pUserImages->GetImageSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_pUserImages = pUserImages;
	return TRUE;
}
//******************************************************************************************
BOOL CBCGToolBar::SetButtons(const UINT* lpIDArray, int nIDCount, BOOL bRemapImages)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	//-----------------------
	// Save customize button:
	//-----------------------
	CCustomizeButton* pCustomizeBtn = NULL;
	if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		pCustomizeBtn = new CCustomizeButton;
		pCustomizeBtn->CopyFrom (*m_pCustomizeBtn);
	}

	RemoveAllButtons ();

	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	if (lpIDArray == NULL)
	{
		while (nIDCount-- > 0)
		{
			InsertSeparator ();
		}
		
		return TRUE;
	}

	int iImage = m_iImagesOffset;

	//--------------------------------
	// Go through them adding buttons:
	//--------------------------------
	for (int i = 0; i < nIDCount; i ++)
	{
		int iCmd = *lpIDArray ++;

		m_OrigButtons.AddTail (new CBCGToolbarButton (iCmd, -1));

		if (iCmd == 0)	// Separator
		{
			InsertSeparator ();
		}
		else if (bRemapImages)
		{
			if (InsertButton (CBCGToolbarButton (iCmd, iImage, NULL, FALSE, 
				m_bLocked)) >= 0 && !m_bLocked)
			{
				m_DefaultImages.SetAt (iCmd, iImage);
			}

			iImage ++;
		}
		else
		{
			if (m_DefaultImages.Lookup (iCmd, iImage)) 
			{
				InsertButton (CBCGToolbarButton (iCmd, iImage, NULL, FALSE, m_bLocked));
			}
		}
	}

	//--------------------------
	// Restore customize button:
	//--------------------------
	if (pCustomizeBtn != NULL)
	{
		InsertButton (pCustomizeBtn);
		m_pCustomizeBtn = pCustomizeBtn;
	}

	if (GetSafeHwnd () != NULL)
	{
		//------------------------------------
		// Allow to produce some user actions:
		//------------------------------------
		OnReset ();
		CWnd* pParentFrame = (m_pDockSite == NULL) ?
			GetParent () : m_pDockSite;
		if (pParentFrame != NULL)
		{
			pParentFrame->SendMessage (BCGM_RESETTOOLBAR, (WPARAM) m_uiOriginalResID);

			while (!m_OrigResetButtons.IsEmpty ())
			{
				delete m_OrigResetButtons.RemoveHead ();
			}

			//-------------------------------------------
			//	Store Buttons state after OnToolbarReset
			//-------------------------------------------
			int i = 0;
			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i++)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);

				if(pButton != NULL && pButton->IsKindOf(RUNTIME_CLASS(CBCGToolbarButton)))
				{
					CRuntimeClass* pRTC = pButton->GetRuntimeClass();
					CBCGToolbarButton* pBtn = (CBCGToolbarButton*)pRTC->CreateObject();
					pBtn->CopyFrom(*pButton);
					m_OrigResetButtons.AddTail(pBtn); 
				}
			}
		}
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGToolBar::LoadBitmap (UINT uiResID, UINT uiColdResID, UINT uiMenuResID, 
							  BOOL bLocked, 
							  UINT uiDisabledResID, UINT uiMenuDisabledResID)
{
	CBCGToolBarParams params;

	params.m_uiColdResID		= uiColdResID;
	params.m_uiHotResID			= uiResID;
	params.m_uiDisabledResID	= uiDisabledResID;
	params.m_uiMenuResID		= uiMenuResID;
	params.m_uiMenuDisabledResID= uiMenuDisabledResID;

	return LoadBitmapEx (params, bLocked);
}
//******************************************************************************************
BOOL CBCGToolBar::LoadToolBar(UINT uiResID, UINT uiColdResID, UINT uiMenuResID, 
							  BOOL bLocked,
							  UINT uiDisabledResID, UINT uiMenuDisabledResID,
							  UINT uiHotResID)
{
	CBCGToolBarParams params;

	params.m_uiColdResID		= uiColdResID;
	params.m_uiHotResID			= uiHotResID;
	params.m_uiDisabledResID	= uiDisabledResID;
	params.m_uiMenuResID		= uiMenuResID;
	params.m_uiMenuDisabledResID= uiMenuDisabledResID;

	return LoadToolBarEx (uiResID, params, bLocked);
}
//*****************************************************************************************
BOOL CBCGToolBar::LoadBitmapEx (CBCGToolBarParams& params, BOOL bLocked)
{
	m_bLocked = bLocked;

	if (m_bLocked)
	{
		//------------------------------------------
		// Don't add bitmap to the shared resources!
		//------------------------------------------
		if (!m_ImagesLocked.Load (params.m_uiHotResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		if (params.m_uiColdResID != 0)
		{
			if (!m_ColdImagesLocked.Load (params.m_uiColdResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_ColdImagesLocked.GetCount ());
		}
		else if (m_bAutoGrayInactiveImages)
		{
			m_ImagesLocked.CopyTo (m_ColdImagesLocked);
			m_ColdImagesLocked.GrayImages (m_nGrayImagePercentage);
		}
		
		if (params.m_uiDisabledResID != 0)
		{
			if (!m_DisabledImagesLocked.Load (params.m_uiDisabledResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}
			
			ASSERT (m_ImagesLocked.GetCount () == m_DisabledImagesLocked.GetCount ());
		}

		//------------------
		// Load large images:
		//-------------------
		if (params.m_uiLargeHotResID != 0)
		{	
			if (!m_LargeImagesLocked.Load (params.m_uiLargeHotResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_LargeImagesLocked.GetCount ());
		}

		if (params.m_uiLargeColdResID != 0)
		{
			ASSERT (params.m_uiColdResID != 0);

			if (!m_LargeColdImagesLocked.Load (params.m_uiLargeColdResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_LargeColdImagesLocked.GetCount ());
		}
		
		if (params.m_uiLargeDisabledResID != 0)
		{
			ASSERT (params.m_uiDisabledResID != 0);

			if (!m_LargeDisabledImagesLocked.Load (params.m_uiLargeDisabledResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}
			
			ASSERT (m_ImagesLocked.GetCount () == m_LargeDisabledImagesLocked.GetCount ());
		}
	
		if (params.m_uiMenuResID != 0)
		{
			if (!m_MenuImagesLocked.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_MenuImagesLocked.GetCount ());
		}

		if (params.m_uiMenuDisabledResID != 0)
		{
			if (!m_MenuImagesLocked.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_MenuImagesLocked.GetCount ());
		}
		
		return TRUE;
	}

	if (!m_Images.Load (params.m_uiHotResID, AfxGetResourceHandle (), TRUE))
	{
		return FALSE;
	}
	
	m_iImagesOffset = m_Images.GetResourceOffset (params.m_uiHotResID);
	ASSERT (m_iImagesOffset >= 0);

	if (params.m_uiColdResID != 0)
	{
		if (!m_ColdImages.Load (params.m_uiColdResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_ColdImages.GetCount ());
		ASSERT (m_Images.GetImageSize ().cy == m_ColdImages.GetImageSize ().cy);
	}
	else if (m_bAutoGrayInactiveImages)
	{
		m_Images.CopyTo (m_ColdImages);
		m_ColdImages.GrayImages (m_nGrayImagePercentage);
	}

	if (params.m_uiMenuResID != 0)
	{
		if (!m_MenuImages.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_MenuImages.GetCount ());
		ASSERT (m_MenuImages.GetImageSize ().cy == m_sizeMenuImage.cy);
	}

	if (params.m_uiDisabledResID != 0)
	{
		if (!m_DisabledImages.Load (params.m_uiDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_DisabledImages.GetCount ());
	}
	
	if (params.m_uiMenuDisabledResID != 0)
	{
		if (!m_DisabledMenuImages.Load (params.m_uiMenuDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_DisabledMenuImages.GetCount ());
	}

	ASSERT (m_Images.GetImageSize ().cy == m_sizeImage.cy);

	//------------------
	// Load large images:
	//-------------------
	if (params.m_uiLargeHotResID != 0)
	{	
		if (!m_LargeImages.Load (params.m_uiLargeHotResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_LargeImages.GetCount ());
	}

	if (params.m_uiLargeColdResID != 0)
	{
		ASSERT (params.m_uiColdResID != 0);

		if (!m_LargeColdImages.Load (params.m_uiLargeColdResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_LargeColdImages.GetCount ());
	}
	
	if (params.m_uiLargeDisabledResID != 0)
	{
		ASSERT (params.m_uiDisabledResID != 0);

		if (!m_LargeDisabledImages.Load (params.m_uiLargeDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_LargeDisabledImages.GetCount ());
	}
	
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGToolBar::LoadToolBarEx (UINT uiToolbarResID, CBCGToolBarParams& params, 
								BOOL bLocked)
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

	ASSERT_VALID(this);

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiToolbarResID);
	ASSERT(lpszResourceName != NULL);

	//---------------------------------------------------
	// determine location of the bitmap in resource fork:
	//---------------------------------------------------
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	ASSERT (pItems != NULL);

	if (bLocked)
	{
		SetLockedSizes (CSize (pData->wWidth + 6, pData->wHeight + 6),
						CSize (pData->wWidth, pData->wHeight));
	}
	else
	{
		SetSizes (	CSize (pData->wWidth + 6, pData->wHeight + 6),
					CSize (pData->wWidth, pData->wHeight));
	}

	BOOL bResult = TRUE;

	if (params.m_uiHotResID == 0)	// Use toolbar resource as hot image
	{
		params.m_uiHotResID = uiToolbarResID;
	}

	if (m_uiOriginalResID != 0 || // Buttons are already created
		LoadBitmapEx (params, bLocked))
	{
		int iImageIndex = m_iImagesOffset;
		for (int i = 0; i < pData->wItemCount; i++)
		{
			pItems[i] = pData->items()[i];

			if (!bLocked && pItems [i] > 0)
			{
				m_DefaultImages.SetAt (pItems[i], iImageIndex ++);
			}
		}

		m_uiOriginalResID = uiToolbarResID;
		bResult = SetButtons(pItems, pData->wItemCount);

		if (!bResult)
		{
			m_uiOriginalResID = 0;
		}
	}

	delete[] pItems;

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}
//****************************************************************************************
int CBCGToolBar::InsertButton (const CBCGToolbarButton& button, int iInsertAt)
{
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	CBCGToolbarButton* pButton = (CBCGToolbarButton*) pClass->CreateObject ();
	ASSERT_VALID(pButton);

	pButton->CopyFrom (button);

	int iIndex = InsertButton (pButton, iInsertAt);
	if (iIndex < 0)
	{
		delete pButton;
	}

	return iIndex;
}
//******************************************************************************************
int CBCGToolBar::ReplaceButton (UINT uiCmd, const CBCGToolbarButton& button,
								 BOOL bAll/* = FALSE*/)
{
	ASSERT_VALID (this);

	int nButtonsCount = 0;

	for (int iStartIndex = 0;;)
	{
		int iIndex = CommandToIndex (uiCmd, iStartIndex);
		if (iIndex < 0)
		{
			break;
		}

		POSITION pos = m_Buttons.FindIndex (iIndex);
		if (pos == NULL)
		{
			ASSERT (FALSE);
			break;
		}

		CBCGToolbarButton* pOldButton = (CBCGToolbarButton*) m_Buttons.GetAt (pos);
		ASSERT_VALID (pOldButton);

		m_Buttons.RemoveAt (pos);
		pOldButton->OnCancelMode ();

		delete pOldButton;

		if (InsertButton (button, iIndex) < 0)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		nButtonsCount++;

		if (bAll)
		{
			iStartIndex = iIndex + 1;
		}
		else
		{
			break;
		}
	}

	if (nButtonsCount == 0)
	{
		TRACE(_T("ReplaceButton: Can't find command %d\n"), uiCmd);
	}

	return nButtonsCount;
}
//******************************************************************************************
int CBCGToolBar::InsertButton (CBCGToolbarButton* pButton, int iInsertAt)
{
	ASSERT (pButton != NULL);

	if (!IsCommandPermitted (pButton->m_nID))
	{
		return -1;
	}

	if (iInsertAt != -1 &&
		(iInsertAt < 0 || iInsertAt > m_Buttons.GetCount ()))
	{
		return -1;
	}

	if (iInsertAt == -1 || iInsertAt == m_Buttons.GetCount ())
	{
		if (m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last!

			iInsertAt = (int) m_Buttons.GetCount () - 1;
		}
		else
		{
			//-------------------------
			// Add to the toolbar tail:
			//-------------------------
			m_Buttons.AddTail (pButton);
			pButton->OnChangeParentWnd (this);

			return (int) m_Buttons.GetCount () - 1;
		}
	}

	POSITION pos = m_Buttons.FindIndex (iInsertAt);
	ASSERT (pos != NULL);

	m_Buttons.InsertBefore (pos, pButton);
	pButton->OnChangeParentWnd (this);

	return iInsertAt;
}
//******************************************************************************************
int CBCGToolBar::InsertSeparator (int iInsertAt)
{
	// Don't allow add a separtor first:
	if (m_Buttons.IsEmpty () || iInsertAt == 0)
	{
		return -1;
	}

	CBCGToolbarButton* pButton = new CBCGToolbarButton;
	ASSERT (pButton != NULL);

	pButton->m_nStyle = TBBS_SEPARATOR;

	int iNewButtonIndex = InsertButton (pButton, iInsertAt);
	if (iNewButtonIndex == -1)
	{
		delete pButton;
	}

	return iNewButtonIndex;
}
//******************************************************************************************
void CBCGToolBar::RemoveAllButtons ()
{
	m_iButtonCapture = -1;      // nothing captured
	m_iHighlighted = -1;
	m_iSelected = -1;

	while (!m_Buttons.IsEmpty ())
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.RemoveHead ();
		ASSERT_VALID (pButton);

		if (pButton != NULL)
		{
			pButton->OnCancelMode ();
			delete pButton;
		}
	}

	m_pCustomizeBtn = NULL;
}
//******************************************************************************************
BOOL CBCGToolBar::RemoveButton (int iIndex)
{
	POSITION pos = m_Buttons.FindIndex (iIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	if (iIndex == m_Buttons.GetCount () - 1 && m_pCustomizeBtn != NULL)
	{
		//-------------------------------------
		// Unable to remove "Customize" button:
		//-------------------------------------
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last!
		ASSERT (FALSE);

		return FALSE;
	}

	CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetAt (pos);
	ASSERT_VALID (pButton);

	m_Buttons.RemoveAt (pos);
	pButton->OnCancelMode ();

	delete pButton;

	if (iIndex == m_iSelected)
	{
		m_iSelected = -1;
	}
	else if (iIndex < m_iSelected && m_iSelected >= 0)
	{
		m_iSelected --;
	}

	if (iIndex == m_iButtonCapture)
	{
		m_iButtonCapture = -1;
	}
	else if (iIndex < m_iButtonCapture && m_iButtonCapture >= 0)
	{
		m_iButtonCapture --;
	}

	if (iIndex == m_iHighlighted)
	{
		m_iHighlighted = -1;
		OnChangeHot (m_iHighlighted);
	}
	else if (iIndex < m_iHighlighted && m_iHighlighted >= 0)
	{
		m_iHighlighted --;
		OnChangeHot (m_iHighlighted);
	}

	//-----------------------------------------
	// If last button is separator - remove it:
	//-----------------------------------------
	pos = m_Buttons.GetTailPosition();
	if (pos != NULL && m_pCustomizeBtn == m_Buttons.GetTail ())
	{
		m_Buttons.GetPrev (pos);
	}
	while (pos != NULL)
	{
		POSITION posSave = pos;
		CBCGToolbarButton* pLastButton = (CBCGToolbarButton*) m_Buttons.GetPrev(pos);
		if (pos != NULL)
		{
			if (pLastButton->m_nStyle & TBBS_SEPARATOR)
			{

				m_Buttons.RemoveAt (posSave);
				delete pLastButton;
			}
			else
			{
				//----------------------
				// Regular button, stop!
				//----------------------
				break;
			}
		}
	}

	//----------------------------
	// Don't leave two separators:
	//----------------------------
	if (iIndex > 0 && iIndex < m_Buttons.GetCount ())
	{
		CBCGToolbarButton* pPrevButton = GetButton (iIndex - 1);
		ASSERT_VALID (pPrevButton);

		CBCGToolbarButton* pNextButton = GetButton (iIndex);
		ASSERT_VALID (pNextButton);

		if ((pPrevButton->m_nStyle & TBBS_SEPARATOR) &&
			(pNextButton->m_nStyle & TBBS_SEPARATOR))
		{
			RemoveButton (iIndex);
		}
	}

	RebuildAccelerationKeys ();
	return TRUE;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar attribute access

int CBCGToolBar::CommandToIndex (UINT nIDFind, int iIndexFirst/* = 0*/) const
{
	ASSERT_VALID(this);

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (i >= iIndexFirst && pButton->m_nID == nIDFind)
		{
			return i;
		}
	}

	return -1;
}
//*****************************************************************
UINT CBCGToolBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pButton->m_nID;
}
//*****************************************************************
void CBCGToolBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);

	ASSERT(nIndex >= 0 && nIndex < m_Buttons.GetCount ());
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		*lpRect = CRect (0, 0, 0, 0);
	}
	else
	{
		*lpRect = pButton->Rect ();
	}
}
//*****************************************************************
void CBCGToolBar::GetInvalidateItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	
	ASSERT(nIndex >= 0 && nIndex < m_Buttons.GetCount ());
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));
	
	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		*lpRect = CRect (0, 0, 0, 0);
	}
	else
	{
		*lpRect = pButton->GetInvalidateRect ();
	}
}
//***************************************************************************
UINT CBCGToolBar::GetButtonStyle(int nIndex) const
{
	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pButton->m_nStyle;
}
//*****************************************************************
int CBCGToolBar::ButtonToIndex (const CBCGToolbarButton* pButton) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGToolbarButton* pListButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pListButton != NULL);

		if (pListButton == pButton)
		{
			return i;
		}
	}

	return -1;
}
//*****************************************************************
void CBCGToolBar::SetButtonStyle(int nIndex, UINT nStyle)
{
	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	UINT nOldStyle = pButton->m_nStyle;
	if (nOldStyle != nStyle)
	{
		if (nStyle & TBBS_DISABLED)
		{
			// Disabled button shouldn't be pressed
			nStyle &= ~TBBS_PRESSED;
		}

		// update the style and invalidate
		pButton->SetStyle (nStyle);

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
		{
			InvalidateButton(nIndex);
		}
	}
}
//****************************************************************
CSize CBCGToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	DWORD dwMode = bStretch ? LM_STRETCH : 0;
	dwMode |= bHorz ? LM_HORZ : 0;

	return CalcLayout (dwMode);
}
//*************************************************************************************
CSize CBCGToolBar::CalcDynamicLayout (int nLength, DWORD dwMode)
{
	if ((nLength == -1) && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT) &&
		((dwMode & LM_HORZDOCK) || (dwMode & LM_VERTDOCK)))
	{
		return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZDOCK);
	}

	return CalcLayout(dwMode, nLength);
}
//*************************************************************************************
void CBCGToolBar::GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const
{
	ASSERT_VALID(this);

	CBCGToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);

		nID = 0;
		nStyle = 0;
		iImage = -1;

		return;
	}

	nID = pButton->m_nID;
	nStyle = pButton->m_nStyle;
	iImage = pButton->GetImage ();
}
//*************************************************************************************
void CBCGToolBar::SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage)
{
	ASSERT_VALID(this);

	CBCGToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pButton);

	pButton->m_nStyle = nStyle;
	pButton->m_nID = nID;
	pButton->SetImage (iImage);

	if ((nStyle & TBBS_SEPARATOR) && iImage > 0) // iImage parameter is a button width!
	{
		AdjustLayout ();
	}

	InvalidateButton(nIndex);
}
//*************************************************************************************
BOOL CBCGToolBar::SetButtonText(int nIndex, LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	ASSERT(lpszText != NULL);

	CBCGToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		return FALSE;
	}

	pButton->m_strText = lpszText;
	return TRUE;
}
//*************************************************************************************
CString CBCGToolBar::GetButtonText( int nIndex ) const
{
	ASSERT_VALID(this);

	CBCGToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	ASSERT_VALID (pButton);

	return pButton->m_strText;
}
//*************************************************************************************
void CBCGToolBar::GetButtonText( int nIndex, CString& rString ) const
{
	ASSERT_VALID(this);

	CBCGToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		rString.Empty ();
		return;
	}

	ASSERT_VALID (pButton);

	rString = pButton->m_strText;
}
//*************************************************************************************
void CBCGToolBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;
	pDCPaint->GetClipBox (rectClip);

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	CRect rectClient;
	GetClientRect (rectClient);

	CDC*		pDC = pDCPaint;
	BOOL		bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC (pDCPaint) &&
		bmp.CreateCompatibleBitmap (pDCPaint, rectClient.Width (),
								  rectClient.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;

		if ((GetStyle () & TBSTYLE_TRANSPARENT) == 0)
		{
			CBCGVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
				rectClient, rectClip);
		}
		else
		{
			m_Impl.GetBackgroundFromParent (pDC);
		}

	}

	OnFillBackground (pDC);

	pDC->SetTextColor (globalData.clrBarText);
	pDC->SetBkMode (TRANSPARENT);

	CRect rect;
	GetClientRect(rect);

	//-----------------------------------
	// Force the full size of the button:
	//-----------------------------------
	if (bHorz)
	{
		rect.bottom = rect.top + GetRowHeight ();
	}
	else
	{
		rect.right = rect.left + GetColumnWidth ();
	}

	CBCGToolBarImages* pImages = 
		GetImageList (m_Images, m_ImagesLocked, m_LargeImages, m_LargeImagesLocked);
	CBCGToolBarImages* pHotImages = pImages;
	CBCGToolBarImages* pColdImages = 
		GetImageList (m_ColdImages, m_ColdImagesLocked, m_LargeColdImages, m_LargeColdImagesLocked);
	CBCGToolBarImages* pDisabledImages = 
		GetImageList (m_DisabledImages, m_DisabledImagesLocked, m_LargeDisabledImages, m_LargeDisabledImagesLocked);
	CBCGToolBarImages* pMenuImages = !m_bLocked ? 
		&m_MenuImages : &m_MenuImagesLocked;
	CBCGToolBarImages* pDisabledMenuImages = !m_bLocked ?
		&m_DisabledMenuImages : &m_DisabledMenuImagesLocked;

	BOOL bDrawImages = pImages->IsValid ();

	pHotImages->SetTransparentColor (globalData.clrBtnFace);

	BOOL bFadeInactiveImages = CBCGVisualManager::GetInstance ()->IsFadeInactiveImage ();

	CBCGDrawState ds;
	if (bDrawImages && 
		!pHotImages->PrepareDrawImage (ds, 
			m_bMenuMode ? m_sizeMenuImage : GetImageSize (),
			bFadeInactiveImages))
	{
		return;     // something went wrong
	}

	CFont* pOldFont;
	if (bHorz)
	{
		pOldFont = (CFont*) pDC->SelectObject (&globalData.fontRegular);
	}
	else
	{
		pOldFont = (CFont*) pDC->SelectObject (&globalData.fontVert);
	}

	if (pColdImages->GetCount () > 0)
	{
		//------------------------------------------
		// Disable fade effect for inactive buttons:
		//------------------------------------------
		CBCGVisualManager::GetInstance ()->SetFadeInactiveImage (FALSE);
	}

	//--------------
	// Draw buttons:
	//--------------
	int iButton = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		rect = pButton->Rect ();
		CRect rectInter;

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			BOOL bHorzSeparator = bHorz;
			CRect rectSeparator = rect;

			if (pButton->m_bWrap && bHorz)
			{
				rectSeparator.left = rectClient.left;
				rectSeparator.right = rectClient.right;

				rectSeparator.top = pButton->Rect ().bottom;
				rectSeparator.bottom = rectSeparator.top + LINE_OFFSET;

				bHorzSeparator = FALSE;
			}

			if (rectInter.IntersectRect (rectSeparator, rectClip) && !pButton->IsHidden())
			{
				DrawSeparator (pDC, rectSeparator, bHorzSeparator);
			}

			continue;
		}

		if (!rectInter.IntersectRect (rect, rectClip))
		{
			continue;
		}

		BOOL bHighlighted = FALSE;
		BOOL bDisabled = (pButton->m_nStyle & TBBS_DISABLED) && !IsCustomizeMode ();

		if (IsCustomizeMode () && !m_bLocked)
		{
			bHighlighted = FALSE;
		}
		else
		{
			if (m_bMenuMode)
			{
				bHighlighted = (iButton == m_iHighlighted);
			}
			else
			{
				bHighlighted = ((iButton == m_iHighlighted ||
								iButton == m_iButtonCapture) &&
								(m_iButtonCapture == -1 ||
								iButton == m_iButtonCapture));
			}
		}

		if (pDC->RectVisible(&rect))
		{
			BOOL bDrawDisabledImages = FALSE;

			if (bDrawImages)
			{
				CBCGToolBarImages* pNewImages = NULL;

				if (pButton->m_bUserButton)
				{
					if (pButton->GetImage () >= 0)
					{
						pNewImages = m_pUserImages;
					}
				}
				else
				{
					if (m_bMenuMode)
					{
						if (bDisabled && pDisabledMenuImages->GetCount () > 0)
						{
							bDrawDisabledImages = TRUE;
							pNewImages = pDisabledMenuImages;
						}
						else if (pMenuImages->GetCount () > 0)
						{
							pNewImages = pMenuImages;
						}
						else
						{
							bDrawDisabledImages = 
								(bDisabled && pDisabledImages->GetCount () > 0);

							pNewImages =  bDrawDisabledImages ? 
											pDisabledImages : pHotImages;
						}
					}
					else	// Toolbar mode
					{
						bDrawDisabledImages = 
							(bDisabled && pDisabledImages->GetCount () > 0);

						pNewImages =  bDrawDisabledImages ? 
										pDisabledImages : pHotImages;

						if (!bHighlighted && !bDrawDisabledImages &&
							(pButton->m_nStyle & TBBS_PRESSED) == 0 &&
							pColdImages->GetCount () > 0 &&
							!pButton->IsDroppedDown ())
						{
							pNewImages = pColdImages;
						}
					}
				}

				if (bDrawImages && pNewImages != pImages && pNewImages != NULL)
				{
					pImages->EndDrawImage (ds);
					
					pNewImages->SetTransparentColor (globalData.clrBtnFace);

					pNewImages->PrepareDrawImage (ds,
						m_bMenuMode ? m_sizeMenuImage : GetImageSize (), bFadeInactiveImages);

					pImages = pNewImages;
				}
			}

			DrawButton (pDC, pButton, bDrawImages ? pImages : NULL, 
						bHighlighted, bDrawDisabledImages);
		}
	}

	//-------------------------------------------------------------
	// Highlight selected button in the toolbar customization mode:
	//-------------------------------------------------------------
	if (m_iSelected >= m_Buttons.GetCount ())
	{
		m_iSelected = -1;
	}

	if (IsCustomizeMode () && m_iSelected >= 0 && !m_bLocked && 
		m_pSelToolbar == this)
	{
		CBCGToolbarButton* pSelButton = GetButton (m_iSelected);
		ASSERT (pSelButton != NULL);

		if (pSelButton != NULL && pSelButton->CanBeStored ())
		{
			CRect rectDrag1 = pSelButton->Rect ();
			if (pSelButton->GetHwnd () != NULL)
			{
				rectDrag1.InflateRect (0, 1);
			}

			pDC->Draw3dRect(&rectDrag1, globalData.clrBarText, globalData.clrBarText);
			rectDrag1.DeflateRect (1, 1);
			pDC->Draw3dRect(&rectDrag1, globalData.clrBarText, globalData.clrBarText);
		}
	}

	if (IsCustomizeMode () && m_iDragIndex >= 0 && !m_bLocked)
	{
		DrawDragMarker (pDC);
	}

	pDC->SelectObject (pOldFont);

	if (bDrawImages)
	{
		pImages->EndDrawImage (ds);
	}

	if (bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		pDCPaint->BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
					   &dcMem, rectClip.left, rectClip.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}

	CBCGVisualManager::GetInstance ()->SetFadeInactiveImage (bFadeInactiveImages);
}
//*************************************************************************************
BOOL CBCGToolBar::IsButtonHighlighted (int iButton) const
{
	BOOL bHighlighted = FALSE;

	if (IsCustomizeMode () && !m_bLocked)
	{
		bHighlighted = FALSE;
	}
	else
	{
		if (m_bMenuMode)
		{
			bHighlighted = (iButton == m_iHighlighted);
		}
		else
		{
			bHighlighted = ((iButton == m_iHighlighted ||
							iButton == m_iButtonCapture) &&
							(m_iButtonCapture == -1 ||
							iButton == m_iButtonCapture));
		}
	}

	return bHighlighted;
}
//*************************************************************************************
BOOL CBCGToolBar::DrawButton(CDC* pDC, CBCGToolbarButton* pButton,
							CBCGToolBarImages* pImages,
							BOOL bHighlighted, BOOL bDrawDisabledImages)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!pButton->IsVisible () || pButton->IsHidden () || !pDC->RectVisible (pButton->Rect ()))
	{
		return TRUE;
	}

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	//---------------------
	// Draw button context:
	//---------------------
	pButton->OnDraw (pDC, pButton->Rect (), pImages, bHorz, 
		IsCustomizeMode () && !m_bAltCustomizeMode && !m_bLocked, 
		bHighlighted, m_bShowHotBorder,
		m_bGrayDisabledButtons && !bDrawDisabledImages);
	return TRUE;
}
//*************************************************************************************
CBCGToolbarButton* CBCGToolBar::GetButton(int nIndex) const
{
	POSITION pos = m_Buttons.FindIndex (nIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetAt (pos);
	ASSERT (pButton != NULL);

	return pButton;
}
//*************************************************************************************
CBCGToolbarButton* CBCGToolBar::InvalidateButton (int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_Buttons.GetCount ())
	{
		return NULL;
	}

	CRect rect;
	GetInvalidateItemRect (nIndex, &rect);

	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton != NULL && pButton == m_pCustomizeBtn)
	{
		rect.right += 10;
		rect.bottom += 10;
	}

	InvalidateRect (rect);

	if (pButton != NULL && pButton == m_pCustomizeBtn &&
		m_pCustomizeBtn->GetExtraSize () != CSize (0, 0))
	{
		rect.InflateRect (m_pCustomizeBtn->GetExtraSize ());
		RedrawWindow (rect, NULL, RDW_FRAME | RDW_INVALIDATE);
	}

	return pButton;
}
//*************************************************************************************
INT_PTR CBCGToolBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	if (!m_bShowTooltips)
	{
		return -1;
	}

	if (CBCGPopupMenu::GetActiveMenu () != NULL &&
		CBCGPopupMenu::GetActiveMenu ()->GetMenuBar () != this)
	{
		return -1;
	}

	// check child windows first by calling CControlBar
	INT_PTR nHit = CControlBar::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	// now hit test against CBCGToolBar buttons
	nHit = ((CBCGToolBar*)this)->HitTest(point);
	if (nHit != -1)
	{
		CBCGToolbarButton* pButton = GetButton ((int) nHit);
		if (pButton == NULL)
		{
			return -1;
		}

		if (pButton->IsWindowVisible ())
		{
			// Button's window handles tooltip
			return -1;
		}

		if (pTI != NULL)
		{
			CString strTipText;
			if (!OnUserToolTip (pButton, strTipText))
			{
				if ((pButton->m_nID == 0 || pButton->m_nID == (UINT) -1 ||
					pButton->m_bUserButton) &&
					!pButton->m_strText.IsEmpty ())
				{
					// Use button text as tooltip!
					strTipText = pButton->m_strText;

					strTipText.Remove (_T('&'));
				}
				else
				{
					if (g_pUserToolsManager != NULL &&
						g_pUserToolsManager->IsUserToolCmd (pButton->m_nID))
					{
						strTipText = pButton->m_strText;
					}
					else
					{
						TCHAR szFullText [256];

						AfxLoadString (pButton->m_nID, szFullText);
						AfxExtractSubString(strTipText, szFullText, 1, '\n');
					}
				}
			}

			if (strTipText.IsEmpty ())
			{
				return -1;
			}

			if (pButton->m_nID != 0 && pButton->m_nID != (UINT) -1 && 
				m_bShowShortcutKeys)
			{
				//--------------------
				// Add shortcut label:
				//--------------------
				CString strLabel;
				CFrameWnd* pParent = GetParentFrame () == NULL ?
					NULL : BCGGetTopLevelFrame (GetParentFrame ());

				if (pParent != NULL &&
					(CBCGKeyboardManager::FindDefaultAccelerator (
						pButton->m_nID, strLabel, pParent, TRUE) ||
					CBCGKeyboardManager::FindDefaultAccelerator (
						pButton->m_nID, strLabel, pParent->GetActiveFrame (), FALSE)))
				{
					strTipText += _T(" (");
					strTipText += strLabel;
					strTipText += _T(')');
				}
			}

			pTI->lpszText = (LPTSTR) ::calloc ((strTipText.GetLength () + 1), sizeof (TCHAR));
			_tcscpy (pTI->lpszText, strTipText);

			GetItemRect ((int) nHit, &pTI->rect);
			pTI->uId = (pButton->m_nID == (UINT) -1) ? 0 : pButton->m_nID;
			pTI->hwnd = m_hWnd;
		}

		nHit = (pButton->m_nID == (UINT) -1) ? 0 : pButton->m_nID;
	}

#if _MSC_VER < 1300
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
	if (pToolTip != NULL && pToolTip->GetSafeHwnd () != NULL)
	{
		pToolTip->SetFont (&globalData.fontTooltip, FALSE);
	}
#endif

	return nHit;
}
//*************************************************************************************
int CBCGToolBar::HitTest(CPoint point) // in window relative coords
{
	int iButton = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->Rect ().PtInRect (point) && !pButton->IsHidden ())
		{
			return (pButton->m_nStyle & TBBS_SEPARATOR) ? -1 : iButton;
		}
	}

	return -1;      // nothing hit
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar message handlers

BEGIN_MESSAGE_MAP(CBCGToolBar, CControlBar)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CBCGToolBar)
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_APPEARANCE, OnToolbarAppearance)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_DELETE, OnToolbarDelete)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE, OnToolbarImage)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, OnToolbarImageAndText)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_START_GROUP, OnToolbarStartGroup)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_TEXT, OnToolbarText)
	ON_WM_LBUTTONUP()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_RESET, OnBcgbarresToolbarReset)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_COMMAND(ID_BCGBARRES_COPY_IMAGE, OnBcgbarresCopyImage)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_NEW_MENU, OnBcgbarresToolbarNewMenu)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
// yurig: support for standard toolbar queries
	ON_MESSAGE(TB_BUTTONCOUNT, OnGetButtonCount)
	ON_MESSAGE(TB_GETITEMRECT, OnGetItemRect)
	ON_MESSAGE(TB_GETBUTTON, OnGetButton)
	ON_MESSAGE(TB_GETBUTTONTEXT, OnGetButtonText)
	ON_REGISTERED_MESSAGE(BCGM_RESETRPROMPT, OnPromptReset)
END_MESSAGE_MAP()

void CBCGToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int iButton = HitTest(point);

	if (m_pSelToolbar != this && IsCustomizeMode ())
	{
		CBCGToolBar* pSelToolbar = m_pSelToolbar;
		m_pSelToolbar = this;

		if (pSelToolbar != NULL)
		{
			ASSERT_VALID (pSelToolbar);

			int iOldSelected = pSelToolbar->m_iSelected;
			pSelToolbar->m_iSelected = -1;
			pSelToolbar->InvalidateButton (iOldSelected);
		}
	}

	if (iButton < 0) // nothing hit
	{
		m_iButtonCapture = -1;

		if (IsCustomizeMode () && !m_bLocked)
		{
			int iSelected = m_iSelected;
			m_iSelected = -1;

			if (iSelected != -1)
			{
				InvalidateButton (iSelected);
				UpdateWindow ();
			}

			OnChangeHot (-1);
		}

		CControlBar::OnLButtonDown(nFlags, point);
		return;
	}

	CBCGToolbarButton* pButton = GetButton(iButton);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	//-----------------------------------------------------------------
	// Check for "Alt-customizible mode" (when ALT key is holded down):
	//-----------------------------------------------------------------
	m_bAltCustomizeMode = FALSE;
	if (m_bAltCustomization &&
		AllowAltCustomization () &&
		!m_bCustomizeMode &&
		GetAsyncKeyState (VK_MENU) & 0x8000)	// ALT is pressed
	{
		m_bAltCustomizeMode = TRUE;
	}

#ifndef BCG_NO_CUSTOMIZATION
	if ((!IsCustomizeMode () && !m_bAltCustomizeMode) || m_bLocked)
#endif // BCG_NO_CUSTOMIZATION

	{
		m_iButtonCapture = iButton;

		// update the button before checking for disabled status
		UpdateButton(m_iButtonCapture);
		if ((pButton->m_nStyle & TBBS_DISABLED) &&
			!pButton->IsKindOf(RUNTIME_CLASS(CBCGDropDownToolbarButton)))
		{
			m_iButtonCapture = -1;
			return;     // don't press it
		}

		pButton->m_nStyle |= TBBS_PRESSED;

		InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		ShowCommandMessageString (pButton->m_nID);

		if (pButton->OnClick (this, FALSE /* No delay*/))
		{
			if (m_Buttons.Find (pButton) != NULL)
			{
				pButton->m_nStyle &= ~TBBS_PRESSED;
			}

			m_iButtonCapture = -1;
			m_iHighlighted = -1;

			OnChangeHot (m_iHighlighted);

			InvalidateButton (iButton);
			UpdateWindow(); // immediate feedback
		}
		else
		{
			m_pWndLastCapture = SetCapture ();
		}
	}

#ifndef BCG_NO_CUSTOMIZATION
	else
	{
		int iSelected = m_iSelected;
		m_iSelected = iButton;

		CRect rect;
		GetItemRect (iButton, &rect);

		if (iSelected != -1)
		{
			InvalidateButton (iSelected);
		}

		m_pDragButton = GetButton (m_iSelected);
		ASSERT (m_pDragButton != NULL);

		m_bIsDragCopy = (nFlags & MK_CONTROL);

		if (!m_pDragButton->IsEditable ())
		{
			m_iSelected = -1;
			m_pDragButton = NULL;

			if (iSelected != -1)
			{
				InvalidateButton (iSelected);
			}
			return;
		}

		InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		if (m_pDragButton->CanBeStretched () &&
			abs (point.x - rect.right) <= STRETCH_DELTA &&
			!m_bAltCustomizeMode)
		{
			m_bStretchButton = TRUE;

			m_rectTrack = m_pDragButton->Rect ();

			if (m_pDragButton->GetHwnd () != NULL)
			{
				m_rectTrack.InflateRect (2, 2);
			}

			m_pWndLastCapture = SetCapture ();
			::SetCursor (globalData.m_hcurStretch);
		}
		else if (m_pDragButton->CanBeStored () &&
			m_pDragButton->OnBeforeDrag ())
		{
			COleDataSource srcItem;

			m_pDragButton->m_bDragFromCollection = TRUE;
			m_pDragButton->PrepareDrag (srcItem);
			m_pDragButton->m_bDragFromCollection = FALSE;

			ShowCommandMessageString (pButton->m_nID);

			m_DropSource.m_bDragStarted = FALSE;
			m_ptStartDrag = point;

			HWND hwndSaved = m_hWnd;

			if (m_bAltCustomizeMode)
			{
				m_bCustomizeMode = TRUE;
			}

			DROPEFFECT dropEffect = srcItem.DoDragDrop 
				(DROPEFFECT_COPY|DROPEFFECT_MOVE, &rect, &m_DropSource);

			if (!::IsWindow (hwndSaved))
			{
				if (m_bAltCustomizeMode)
				{
					m_bCustomizeMode = FALSE;
					m_bAltCustomizeMode = FALSE;
				}

				return;
			}

			CPoint ptDrop;
			::GetCursorPos (&ptDrop);
			ScreenToClient (&ptDrop);

			if (m_DropSource.m_bDragStarted &&
				!rect.PtInRect (ptDrop))
			{
				if (dropEffect != DROPEFFECT_COPY && 
					m_pDragButton != NULL &&
					!m_DropSource.m_bEscapePressed)
				{
					//---------------------
					// Remove source button:
					//---------------------
					RemoveButton (ButtonToIndex (m_pDragButton));
					AdjustLayout ();

					RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				}
				else if (m_pDragButton != NULL)
				{
					InvalidateRect (m_pDragButton->Rect ());
				}
			}
			else
			{
				m_iHighlighted = iButton;
				OnChangeHot (m_iHighlighted);
			}

			m_pDragButton = NULL;
			m_ptStartDrag = CPoint (-1, -1);
		}
		else
		{
			m_pDragButton = NULL;
		}
	}

#endif // BCG_NO_CUSTOMIZATION

	if (m_bAltCustomizeMode)
	{
		m_bAltCustomizeMode = FALSE;
		SetCustomizeMode (FALSE);

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}
//**************************************************************************************
void CBCGToolBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (IsCustomizeMode () && !m_bLocked)
	{
		if (m_bStretchButton)
		{
			ASSERT_VALID (m_pDragButton);

			if (point.x - m_pDragButton->Rect ().left >= BUTTON_MIN_WIDTH)
			{
				CClientDC dc (this);

				CRect rectTrackOld = m_rectTrack;
				m_rectTrack.right = point.x;
				dc.DrawDragRect (&m_rectTrack, CSize (2, 2), &rectTrackOld, CSize (2, 2));
			}

			::SetCursor (globalData.m_hcurStretch);
		}

		return;
	}

	if (m_ptLastMouse != CPoint (-1, -1) &&
		abs (m_ptLastMouse.x - point.x) < 1 &&
		abs (m_ptLastMouse.y - point.y) < 1)
	{
		m_ptLastMouse = point;
		return;
	}

	m_ptLastMouse = point;

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest (point);

	if (m_iHighlighted == -1 && GetFocus () == this)
	{
		m_iHighlighted = iPrevHighlighted; 
		return;
	}

	CBCGToolbarButton* pButton = m_iHighlighted == -1 ?
							NULL : GetButton (m_iHighlighted);
	if (pButton != NULL &&
		(pButton->m_nStyle & TBBS_SEPARATOR || 
		(pButton->m_nStyle & TBBS_DISABLED && 
		!AllowSelectDisabled ())))
	{
		m_iHighlighted = -1;
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGTrackMouse (&trackmouseevent);	
	}
	
	if (iPrevHighlighted != m_iHighlighted)
	{
		BOOL bNeedUpdate = FALSE;

		if (m_iButtonCapture != -1)
		{
			CBCGToolbarButton* pTBBCapt = GetButton (m_iButtonCapture);
			ASSERT (pTBBCapt != NULL);
			ASSERT (!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));

			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);
			if (m_iHighlighted == m_iButtonCapture)
			{
				nNewStyle |= TBBS_PRESSED;
			}

			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle (m_iButtonCapture, nNewStyle);
				bNeedUpdate = TRUE;
			}
		}

		if ((m_bMenuMode || m_iButtonCapture == -1 || 
			iPrevHighlighted == m_iButtonCapture) &&
			iPrevHighlighted != -1)
		{
			InvalidateButton (iPrevHighlighted);
			bNeedUpdate = TRUE;
		}

		if ((m_bMenuMode || m_iButtonCapture == -1 || 
			m_iHighlighted == m_iButtonCapture) &&
			m_iHighlighted != -1)
		{
			InvalidateButton (m_iHighlighted);
			bNeedUpdate = TRUE;

			if (globalData.IsAccessibilitySupport ())
			{
				BOOL bDropDown = FALSE;
				CBCGToolbarMenuButton* pMenuButton = 
					DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, GetButton (m_iHighlighted));
				if (pMenuButton != NULL && pMenuButton->m_bDrawDownArrow)
				{
					bDropDown = TRUE;		
				}

				if (m_iHighlighted != m_iAccHotItem)
				{
					m_iAccHotItem = m_iHighlighted;
					KillTimer (uiAccNotifyEvent);
					if (bDropDown)
					{
						SetTimer (uiAccNotifyEvent, uiAccPopupTimerDelay, NULL);
					}
					else
					{
						SetTimer (uiAccNotifyEvent, uiAccTimerDelay, NULL);
					}
				}	
			}
		}

		if (m_iHighlighted != -1 && 
			(m_bMenuMode || m_iHighlighted == m_iButtonCapture || m_iButtonCapture == -1))
		{
			ASSERT (pButton != NULL);
			ShowCommandMessageString (pButton->m_nID);
		}
		else if ((m_iButtonCapture == -1 || (m_bMenuMode && m_iHighlighted == -1))
			&& m_hookMouseHelp == NULL)
		{
			GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}

		OnChangeHot (m_iHighlighted);
		if (bNeedUpdate)
		{
			UpdateWindow ();
		}
	}
}
//*************************************************************************************
void CBCGToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsCustomizeMode () && !m_bLocked)
	{
		if (m_bStretchButton)
		{
			ASSERT_VALID (m_pDragButton);

			CRect rect = m_pDragButton->Rect ();
			rect.right = point.x;

			if (rect.Width () >= BUTTON_MIN_WIDTH &&
				abs (m_pDragButton->Rect ().right - point.x) > STRETCH_DELTA)
			{
				m_pDragButton->OnSize (rect.Width ());
				AdjustLayout ();
			}

			m_rectTrack.SetRectEmpty ();

			m_pDragButton = NULL;
			m_bStretchButton = FALSE;

			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			::ReleaseCapture ();

			if (m_pWndLastCapture != NULL)
			{
				m_pWndLastCapture->SetCapture ();
				m_pWndLastCapture = NULL;
			}
		}
		return;
	}

	if (m_iButtonCapture == -1)
	{
		CControlBar::OnLButtonUp(nFlags, point);

		m_ptLastMouse = CPoint (-1, -1);
		OnMouseMove (0, point);
		return;     // not captured
	}

	::ReleaseCapture();
	if (m_pWndLastCapture != NULL)
	{
		m_pWndLastCapture->SetCapture ();
		m_pWndLastCapture = NULL;
	}

	m_iHighlighted = HitTest (point);

	CBCGToolbarButton* pButton = GetButton(m_iButtonCapture);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
	UINT nIDCmd = 0;

	UINT nNewStyle = (pButton->m_nStyle & ~TBBS_PRESSED);
	if (m_iButtonCapture == m_iHighlighted)
	{
		// we did not lose the capture
		if (HitTest(point) == m_iButtonCapture)
		{
			// give button a chance to update
			UpdateButton(m_iButtonCapture);

			// then check for disabled state
			if (!(pButton->m_nStyle & TBBS_DISABLED))
			{
				// pressed, will send command notification
				nIDCmd = pButton->m_nID;

				if (pButton->m_nStyle & TBBS_CHECKBOX)
				{
					// auto check: three state => down
					if (nNewStyle & TBBS_INDETERMINATE)
						nNewStyle &= ~TBBS_INDETERMINATE;

					nNewStyle ^= TBBS_CHECKED;
				}
			}
		}
	}

	if (m_hookMouseHelp == NULL)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	int iButtonCapture = m_iButtonCapture;
	m_iButtonCapture = -1;
	m_iHighlighted = -1;

	HWND hwndSaved = m_hWnd;
	RestoreFocus ();

	if (HitTest(point) == iButtonCapture && 
		!OnSendCommand (pButton) &&
		nIDCmd != 0 && nIDCmd != (UINT) -1)
	{
		InvalidateButton (iButtonCapture);
		UpdateWindow(); // immediate feedback

		AddCommandUsage (nIDCmd);

		if (!pButton->OnClickUp () &&
			(g_pUserToolsManager == NULL ||
			!g_pUserToolsManager->InvokeTool (nIDCmd)))
		{
			GetOwner()->SendMessage (WM_COMMAND, nIDCmd);    // send command
		}
	}
	else
	{
		if (::IsWindow (hwndSaved) && !::IsIconic (hwndSaved) &&
			::IsZoomed (hwndSaved))
		{
			pButton->OnClickUp ();
		}
   }


	if (::IsWindow (hwndSaved) &&				// "This" may be destoyed now!
		iButtonCapture < m_Buttons.GetCount ())	// Button may disappear now!
	{
		SetButtonStyle(iButtonCapture, nNewStyle);
		UpdateButton(iButtonCapture);
		InvalidateButton (iButtonCapture);
		UpdateWindow(); // immediate feedback

		m_ptLastMouse = CPoint (-1, -1);
		OnMouseMove (0, point);
	}
}
//*************************************************************************************
void CBCGToolBar::OnCancelMode()
{
	CControlBar::OnCancelMode();

	if (m_bStretchButton)
	{
		m_pDragButton = NULL;
		m_bStretchButton = FALSE;

		m_rectTrack.SetRectEmpty ();

		::ReleaseCapture ();
		if (m_pWndLastCapture != NULL)
		{
			m_pWndLastCapture->SetCapture ();
			m_pWndLastCapture = NULL;
		}
	}

	if (m_iButtonCapture >= 0 ||
		m_iHighlighted >= 0)
	{
		if (m_iButtonCapture >= 0)
		{
			CBCGToolbarButton* pButton = GetButton(m_iButtonCapture);
			if (pButton == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
				UINT nNewStyle = (pButton->m_nStyle & ~TBBS_PRESSED);
				if (GetCapture() == this)
				{
					::ReleaseCapture();

					if (m_pWndLastCapture != NULL)
					{
						m_pWndLastCapture->SetCapture ();
						m_pWndLastCapture = NULL;
					}
				}

				SetButtonStyle(m_iButtonCapture, nNewStyle);
			}
		}

		m_iButtonCapture = -1;
		m_iHighlighted = -1;

		OnChangeHot (m_iHighlighted);
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
	
		pButton->OnCancelMode ();
	}

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//*************************************************************************************
void CBCGToolBar::OnSysColorChange()
{
	globalData.UpdateSysColors ();

	CBCGVisualManager::GetInstance ()->OnUpdateSystemColors ();
	UpdateImagesColor ();

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//*************************************************************************************
void CBCGToolBar::UpdateImagesColor ()
{
	m_Images.OnSysColorChange ();
	m_ColdImages.OnSysColorChange ();
	m_ImagesLocked.OnSysColorChange ();
	m_ColdImagesLocked.OnSysColorChange ();
	m_MenuImages.OnSysColorChange ();
	m_DisabledMenuImages.OnSysColorChange ();
	m_MenuImagesLocked.OnSysColorChange ();
	m_DisabledImagesLocked.OnSysColorChange ();
	m_DisabledMenuImagesLocked.OnSysColorChange ();

	m_LargeImages.OnSysColorChange ();
	m_LargeColdImages.OnSysColorChange ();
	m_LargeDisabledImages.OnSysColorChange ();
	m_LargeImagesLocked.OnSysColorChange ();
	m_LargeColdImagesLocked.OnSysColorChange ();
	m_LargeDisabledImagesLocked.OnSysColorChange ();

	if (m_pUserImages != NULL)
	{
		m_pUserImages->OnSysColorChange ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar idle update through CToolCmdUI class

#define CToolCmdUI COldToolCmdUI

class CToolCmdUI : public CCmdUI        // class private to this file !
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetRadio(BOOL bOn = TRUE);
};

void CToolCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGToolBar* pToolBar = (CBCGToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;

	if (!bOn)
		nNewStyle |= TBBS_DISABLED;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
}
//*************************************************************************************
void CToolCmdUI::SetCheck(int nCheck)
{
	ASSERT (nCheck >= 0);
	if (nCheck > 2)
	{
		nCheck = 1;
	}

	CBCGToolBar* pToolBar = (CBCGToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) &
				~(TBBS_CHECKED | TBBS_INDETERMINATE);
	if (nCheck == 1)
		nNewStyle |= TBBS_CHECKED;
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX);
}
//*************************************************************************************
void CToolCmdUI::SetRadio(BOOL bOn)
{
	SetCheck(bOn ? 1 : 0); // this default works for most things as well

	CBCGToolBar* pToolBar = (CBCGToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	CBCGToolbarButton* pButton = pToolBar->GetButton (m_nIndex);
	ASSERT_VALID (pButton);

	pButton->SetRadio ();
}
//*************************************************************************************
void CToolCmdUI::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	CBCGToolBar* pToolBar = (CBCGToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	if (pToolBar->GetIgnoreSetText ())
	{
		return;
	}

	CBCGToolbarButton* pButton = pToolBar->GetButton (m_nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pButton);

	//JRG Modified 3/21/2000
	//Remove any amperstands and trailing label (ex.:"\tCtrl+S")
	CString strNewText(lpszText);

	int iOffset = strNewText.Find (_T('\t'));
	if (iOffset != -1)
	{
		strNewText = strNewText.Left (iOffset);
	}
	//JRG Modified 3/21/2000

	CString strOldText = pButton->m_strText.SpanExcluding(_T("\t"));
	if (strOldText == strNewText)		//JRG Modified 3/21/2000
	{
		return;
	}

	pButton->m_strText = strNewText;	//JRG Modified 3/21/2000
	pToolBar->AdjustLayout ();
}
//*************************************************************************************
void CBCGToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)m_Buttons.GetCount ();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		CBCGToolbarButton* pButton = GetButton(state.m_nIndex);
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->IsUserToolCmd (pButton->m_nID))
		{
			bDisableIfNoHndler = FALSE;
		}

		state.m_nID = pButton->m_nID;

		// ignore separators and system commands
		if (!(pButton->m_nStyle & TBBS_SEPARATOR) &&
			pButton->m_nID != 0 &&
			!IsSystemCommand (pButton->m_nID) &&
			pButton->m_nID < AFX_IDM_FIRST_MDICHILD)
		{
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, 
		bDisableIfNoHndler && m_bDisableControlsIfNoHandler);
}
//*************************************************************************************
void CBCGToolBar::UpdateButton(int nIndex)
{
	CWnd* pTarget = GetCommandTarget ();

	// send the update notification
	if (pTarget != NULL)
	{
		CToolCmdUI state;
		state.m_pOther = this;
		state.m_nIndex = nIndex;
		state.m_nIndexMax = (UINT)m_Buttons.GetCount ();
		CBCGToolbarButton* pButton = GetButton(nIndex);

		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		if (!IsSystemCommand (pButton->m_nID) &&
			pButton->m_nID < AFX_IDM_FIRST_MDICHILD)
		{
			BOOL bAutoMenuEnable = FALSE;
			if (pTarget->IsFrameWnd ())
			{
				bAutoMenuEnable = ((CFrameWnd*) pTarget)->m_bAutoMenuEnable;
			}

			state.m_nID = pButton->m_nID;
			state.DoUpdate(pTarget, bAutoMenuEnable &&
						(g_pUserToolsManager == NULL ||
						!g_pUserToolsManager->IsUserToolCmd (pButton->m_nID)));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolBar diagnostics

#ifdef _DEBUG
void CBCGToolBar::AssertValid() const
{
	CControlBar::AssertValid();
}

void CBCGToolBar::Dump(CDumpContext& dc) const
{
	CControlBar::Dump (dc);

	CString strName;

	if (::IsWindow (m_hWnd))
	{
		GetWindowText (strName);
	}

	dc << "\n**** Toolbar ***" << strName;
	dc << "\nButtons: " << m_Buttons.GetCount () << "\n";

	dc.SetDepth (dc.GetDepth () + 1);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->Dump (dc);
		dc << "\n";
	}

	dc.SetDepth (dc.GetDepth () - 1);
	dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

int CBCGToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	CFrameWnd* pParentFrame = m_pDockSite;
	if (pParentFrame == NULL && GetParentFrame () != NULL)
	{
		pParentFrame = GetDockingFrame ();
	}

	if (pParentFrame != NULL)
	{
		CBCGToolBarImages::EnableRTL (pParentFrame->GetExStyle () & WS_EX_LAYOUTRTL);
	}

	_AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (pState->m_bNeedTerm)	// AfxOleInit was called
	{
#ifndef BCG_NO_CUSTOMIZATION
		m_DropTarget.Register (this);
#endif // BCG_NO_CUSTOMIZATION
	}

	m_penDrag.CreatePen (PS_SOLID, 1, globalData.clrBarText);

	m_bRoundShape = 
		CBCGVisualManager::GetInstance ()->IsToolbarRoundShape (this);

	if (m_bRoundShape)
	{
		SetRoundedRgn ();
	}
	else
	{
		SetWindowRgn (NULL, FALSE);
	}

	gAllToolbars.AddTail (this);

	return 0;
}
//****************************************************************************************
DROPEFFECT CBCGToolBar::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
#ifndef BCG_NO_CUSTOMIZATION
	m_iDragIndex = -1;
	m_DropSource.m_bDeleteOnDrop = FALSE;
#endif // BCG_NO_CUSTOMIZATION

	return OnDragOver(pDataObject, dwKeyState, point);
}
//****************************************************************************************
void CBCGToolBar::OnDragLeave() 
{
#ifndef BCG_NO_CUSTOMIZATION

	m_iDragIndex = -1;
	
	CRect rect = m_rectDrag;
	rect.InflateRect (2, 2);
	InvalidateRect (&rect);

	UpdateWindow ();

	m_rectDrag.SetRectEmpty ();
	m_iDragIndex = -1;

	m_DropSource.m_bDeleteOnDrop = TRUE;

#endif // BCG_NO_CUSTOMIZATION
}
//****************************************************************************************
DROPEFFECT CBCGToolBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, 
								   CPoint point) 
{
#ifdef BCG_NO_CUSTOMIZATION

	return DROPEFFECT_NONE;

#else
	if (m_bLocked)
	{
		return DROPEFFECT_NONE;
	}

	CBCGToolbarButton* pButton = CBCGToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bAllowDrop = pButton->CanBeDropped (this);
	delete pButton;

	if (!bAllowDrop)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bCopy = (dwKeyState & MK_CONTROL);

	m_bIsDragCopy = bCopy;

	if (m_pDragButton == NULL)	// Drag from the other toolbar
	{
		//------------------
		// Remove selection:
		//------------------
		int iSelected = m_iSelected;
		m_iSelected = -1;

		if (iSelected != -1)
		{
			InvalidateButton (iSelected);
			UpdateWindow ();
		}
	}

	//---------------------
	// Find the drop place:
	//---------------------
	CRect rect = m_rectDrag;
	int iIndex = FindDropIndex (point, m_rectDrag);

	if (rect != m_rectDrag)
	{
		//--------------------
		// Redraw drop marker:
		//--------------------
		m_iDragIndex = iIndex;

		rect.InflateRect (2, 2);
		InvalidateRect (&rect);

		rect = m_rectDrag;
		rect.InflateRect (2, 2);
		InvalidateRect (&m_rectDrag);

		UpdateWindow ();
	}

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest (point);

	if (iPrevHighlighted != m_iHighlighted)
	{
		OnChangeHot (m_iHighlighted);
	}

	if (iIndex == -1)
	{
		return DROPEFFECT_NONE;
	}

	return (bCopy) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

#endif // BCG_NO_CUSTOMIZATION
}
//****************************************************************************************
BOOL CBCGToolBar::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	ASSERT_VALID(this);

	int iDragIndex = m_iDragIndex;
	if (iDragIndex < 0)
	{
		return FALSE;
	}

	CBCGToolbarButton* pDragButton = m_pDragButton;
	m_pDragButton = NULL;

	OnDragLeave();

	//----------------------------------------------------
	// Create a button object from the OLE clipboard data:
	//----------------------------------------------------
	CBCGToolbarButton* pButton = CreateDroppedButton (pDataObject);
	if (pButton == NULL)
	{
		return FALSE;
	}

	pButton->m_bDragFromCollection = FALSE;

	if (pDragButton != NULL && dropEffect != DROPEFFECT_COPY)
	{
		int iOldIndex = ButtonToIndex (pDragButton);
		if (iDragIndex == iOldIndex || iDragIndex == iOldIndex + 1)
		{
			AddRemoveSeparator (pDragButton, m_ptStartDrag, point);
			delete pButton;
			return TRUE;
		}
		
		RemoveButton (iOldIndex);
		if (iDragIndex > iOldIndex)
		{
			iDragIndex --;
		}

		iDragIndex = min (iDragIndex, (int) m_Buttons.GetCount ());
	}

	if (InsertButton (pButton, iDragIndex) == -1)
	{
		ASSERT (FALSE);
		delete pButton;
		return FALSE;
	}

	AdjustLayout ();

	if (m_bAltCustomizeMode)
	{
		//------------------------------
		// Immideatly save button state:
		//------------------------------
		pButton->SaveBarState ();
	}

	m_iSelected = -1;
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGToolBar::SetCustomizeMode (BOOL bSet)
{
	if (m_bCustomizeMode == bSet)
	{
		return FALSE;
	}

	//---------------------------------------------------------------------
	// First step - inform all toolbars about start/end customization mode:
	//---------------------------------------------------------------------
	for (BOOL bToolbarsListWasChanged = TRUE;
		bToolbarsListWasChanged;)
	{
		int iOrigCount = (int) gAllToolbars.GetCount ();
		bToolbarsListWasChanged = FALSE;

		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); 
			posTlb != NULL && !bToolbarsListWasChanged;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				pToolBar->OnCustomizeMode (bSet);

				//-------------------------------------------------
				// CBCGToolBar::OnCustomizeMode can add/remove some
				// "sub-toolbars". So, let's start loop again!
				//-------------------------------------------------
				if (gAllToolbars.GetCount () != iOrigCount)
				{
					bToolbarsListWasChanged = TRUE;
				}
			}
			}
	}

	m_bCustomizeMode = bSet;

	//-----------------------------------
	// Second step - redraw all toolbars:
	//-----------------------------------
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar->m_pCustomizeBtn != NULL)
			{
				pToolBar->AdjustLayout ();
			}

			pToolBar->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	if (!bSet)
	{
		m_pSelToolbar = NULL;
	}

	return TRUE;
}
//********************************************************************************
int CBCGToolBar::GetCommandButtons (UINT uiCmd, CObList& listButtons)
{
	listButtons.RemoveAll ();
	if (uiCmd == 0)
	{
		return 0;
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			for (POSITION pos = pToolBar->m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) pToolBar->m_Buttons.GetNext (pos);
				if (pButton == NULL)
				{
					break;
				}

				ASSERT_VALID (pButton);

				if (pButton->m_nID == uiCmd)
				{
					listButtons.AddTail (pButton);
				}
			}
		}
	}

	return (int) listButtons.GetCount ();
}
//********************************************************************************
int CBCGToolBar::FindDropIndex (const CPoint p, CRect& rectDrag) const
{
	int iDragButton = -1;
	rectDrag.SetRectEmpty ();

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	CPoint point = p;
	if (point.y < 0)
	{
		point.y = 0;
	}

	if (m_Buttons.IsEmpty () || 
		(m_Buttons.GetCount () == 1 && m_pCustomizeBtn != NULL))
	{
		GetClientRect (&rectDrag);
		iDragButton = 0;
	}
	else
	{
		if (bHorz)
		{
			int iOffset = GetRowHeight ();
			int iButton = 0;
			CRect rectPrev;

			POSITION pos;
			for (pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				if (!pButton->IsHidden () && pButton->IsVisible ())
				{
					CRect rect = pButton->Rect ();

					if (iButton > 0 && rect.top > rectPrev.bottom)
					{
						iOffset	= rect.top - rectPrev.bottom;
						break;
					}

					rectPrev = rect;
				}
			}

			int iCursorRow = point.y / (GetRowHeight () + iOffset);
			int iRow = 0;
			iButton = 0;

			for (pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				if (!pButton->IsHidden () && pButton->IsVisible ())
				{
					CRect rect = pButton->Rect ();
					
					if (iButton > 0 && rect.top >= rectPrev.bottom)
					{
						iRow ++;
					}

					if (iRow > iCursorRow)
					{
						rectDrag = rectPrev;
						rectDrag.left = rectDrag.right;
						iDragButton = iButton - 1;
						break;
					}

					if (iRow == iCursorRow)
					{
						if (point.x < rect.left)
						{
							iDragButton = iButton;
							rectDrag = rect;
							rectDrag.right = rectDrag.left;
							break;
						}
						else if (point.x <= rect.right)
						{
							rectDrag = rect;
							if (point.x - rect.left > rect.right - point.x)
							{
								iDragButton = iButton + 1;
								rectDrag.left = rectDrag.right;
							}
							else
							{
								iDragButton = iButton;
								rectDrag.right = rectDrag.left;
							}
							break;
						}
					}

					rectPrev = rect;
				}
			}

			if (iDragButton == -1 && iRow == iCursorRow)
			{
				rectDrag = rectPrev;
				rectDrag.left = rectDrag.right;
				iDragButton = iButton;
			}
		}
		else
		{
			int iButton = 0;
			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				CRect rect = pButton->Rect ();

				if (point.y < rect.top)
				{
					iDragButton = iButton;
					rectDrag = rect;
					rectDrag.bottom = rectDrag.top;
					break;
				}
				else if (point.y <= rect.bottom)
				{
					rectDrag = rect;
					if (point.y - rect.top > rect.bottom - point.y)
					{
						iDragButton = iButton + 1;
						rectDrag.top = rectDrag.bottom;
					}
					else
					{
						iDragButton = iButton;
						rectDrag.bottom = rectDrag.top;
					}
					break;
				}
			}
		}
	}

	if (iDragButton >= 0)
	{
		const int iCursorSize = 6;

		CRect rectClient;	// Client area rectangle
		GetClientRect (&rectClient);

		if (m_pCustomizeBtn != NULL && iDragButton == m_Buttons.GetCount ())
		{
			iDragButton = max (0, (int) m_Buttons.GetCount () - 1);
		}

		if (bHorz)
		{
			rectDrag.left = 
				max (rectClient.left, rectDrag.left - iCursorSize / 2);

			rectDrag.right = rectDrag.left + iCursorSize;
			if (rectDrag.right > rectClient.right)
			{
				rectDrag.right = rectClient.right;
				rectDrag.left = rectDrag.right - iCursorSize;
			}
		}
		else
		{
			rectDrag.top = 
				max (rectClient.top, rectDrag.top - iCursorSize / 2);

			rectDrag.bottom = rectDrag.top + iCursorSize;
			if (rectDrag.bottom > rectClient.bottom)
			{
				rectDrag.bottom = rectClient.bottom;
				rectDrag.top = rectDrag.bottom - iCursorSize;
			}
		}
	}

	if (m_pCustomizeBtn != NULL && iDragButton == m_Buttons.GetCount ())
	{
		iDragButton = -1;
		rectDrag.SetRectEmpty ();
	}

	return iDragButton;
}
//***********************************************************************
void CBCGToolBar::DrawDragMarker (CDC* pDC)
{
	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	CPen* pOldPen = (CPen*) pDC->SelectObject (&m_penDrag);

	for (int i = 0; i < 2; i ++)
	{
		if (bHorz)
		{
			pDC->MoveTo (m_rectDrag.left + m_rectDrag.Width () / 2 + i - 1, m_rectDrag.top);
			pDC->LineTo (m_rectDrag.left + m_rectDrag.Width () / 2 + i - 1, m_rectDrag.bottom);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.right - i, m_rectDrag.top + i);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.bottom - i - 1);
			pDC->LineTo (m_rectDrag.right - i, m_rectDrag.bottom - i - 1);
		}
		else
		{
			pDC->MoveTo (m_rectDrag.left, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);
			pDC->LineTo (m_rectDrag.right, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.left + i, m_rectDrag.bottom - i);

			pDC->MoveTo (m_rectDrag.right - i - 1, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.right - i - 1, m_rectDrag.bottom - i);
		}
	}

	pDC->SelectObject (pOldPen);
}
//********************************************************************
void CBCGToolBar::OnDestroy() 
{
	m_penDrag.DeleteObject ();

	CControlBar::OnDestroy();

	if (m_pSelToolbar == this)
	{
		m_pSelToolbar = NULL;
	}

	for (POSITION pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar == this)
			{
				gAllToolbars.RemoveAt (posSave);
				break;
			}
		}
	}
}
//********************************************************************
void CBCGToolBar::Serialize (CArchive& ar)
{
	CControlBar::Serialize (ar);

	POSITION pos;
	CString strName;

	if (ar.IsLoading ())
	{
		//-----------------------
		// Save customize button:
		//-----------------------
		CCustomizeButton* pCustomizeBtn = NULL;
		if (m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

			pCustomizeBtn = new CCustomizeButton;
			pCustomizeBtn->CopyFrom (*m_pCustomizeBtn);
		}

		RemoveAllButtons ();
		m_Buttons.Serialize (ar);

		for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
		{
			CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
			if (pButton == NULL)
			{
				// Corrupted data!
				ASSERT (FALSE);
				m_Buttons.RemoveAll ();	// Memory leak! Don't delete wrong objects.

				if (CanBeRestored ())
				{
					RestoreOriginalstate ();
				}
				AdjustLocations ();
				return;
			}

			pButton->m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);	// Fix for the "stuck" buttons.
			pButton->OnChangeParentWnd (this);
		}
		
		BOOL bTextLabels;
		ar >> bTextLabels;
		if (AllowChangeTextLabels ())
		{
			m_bTextLabels = bTextLabels;
		}

		//--------------------------
		// Restore customize button:
		//--------------------------
		if (pCustomizeBtn != NULL)
		{
			InsertButton (pCustomizeBtn);
			m_pCustomizeBtn = pCustomizeBtn;
		}

		AdjustLocations ();

		ar >> strName;

		if (::IsWindow (m_hWnd))
		{
			SetWindowText (strName);
		}

		//--------------------------
		// Remove all "bad" buttons:
		//--------------------------
		for (pos = m_lstUnpermittedCommands.GetHeadPosition (); pos != NULL;)
		{
			UINT uiCmd = m_lstUnpermittedCommands.GetNext (pos);

			int iIndex = CommandToIndex (uiCmd);
			if (iIndex >= 0)
			{
				RemoveButton (iIndex);
			}
		}
	}
	else
	{
		//-----------------------------------
		// Serialize just "Storable" buttons:
		//-----------------------------------
		CObList buttons;

		for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
		{
			CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
			ASSERT_VALID (pButton);

			if (pButton->CanBeStored ())
			{
				buttons.AddTail (pButton);
			}
		}

		buttons.Serialize (ar);
		ar << m_bTextLabels;

		if (::IsWindow (m_hWnd))
		{
			GetWindowText (strName);
		}

		ar << strName;
	}
}
//*********************************************************************
BOOL CBCGToolBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			Serialize (ar);
			ar.Flush ();
		}

		UINT uiDataSize = (UINT) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData != NULL)
		{
			CBCGRegistrySP regSP;
			CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

			if (reg.CreateKey (strSection))
			{
				if (::IsWindow (m_hWnd))
				{
					CString strToolbarName;
					GetWindowText (strToolbarName);

					reg.Write (REG_ENTRY_NAME, strToolbarName);
				}

				bResult = reg.Write (REG_ENTRY_BUTTONS, lpbData, uiDataSize);
				if (bResult && 
					GetWorkspace () != NULL &&
					GetWorkspace ()->IsResourceSmartUpdate ())
				{
					// Save orginal (before customization) state:
					SaveOriginalState (reg);
				}

				SaveResetOriginalState(reg);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGToolBar::SaveState ()!\n"));
	}

	return bResult;
}
//*********************************************************************
BOOL CBCGToolBar::RemoveStateFromRegistry (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	return reg.DeleteKey (strSection);
}
//*********************************************************************
BOOL CBCGToolBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	LPBYTE	lpbData = NULL;
	UINT	uiDataSize;

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	if (!reg.Read (REG_ENTRY_BUTTONS, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		Serialize (ar);
		bResult = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGToolBar::LoadState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGToolBar::LoadState ()!\n"));

		m_Buttons.RemoveAll ();	// Memory leak! Don't delete wrong objects.
		if (CanBeRestored ())
		{
			RestoreOriginalstate ();
		}
	}

	if (lpbData != NULL)
	{
		delete lpbData;
	}

	if (bResult &&
		GetWorkspace () != NULL &&
		GetWorkspace ()->IsResourceSmartUpdate ())
	{
		LoadLastOriginalState (reg);
	}

	LoadResetOriginalState(reg);

	AdjustLayout ();
	return bResult;
}
//*******************************************************************************************
void CBCGToolBar::OnContextMenu(CWnd*, CPoint point)
{
	if (m_bLocked && IsCustomizeMode ())
	{
		MessageBeep ((UINT) -1);
		return;
	}

	OnChangeHot (-1);

	if (!IsCustomizeMode ())
	{
		CFrameWnd* pParentFrame = m_pDockSite;
		if (pParentFrame == NULL && GetParentFrame () != NULL)
		{
			pParentFrame = GetDockingFrame ();
		}

		if (pParentFrame != NULL)
		{
			ASSERT_VALID(pParentFrame);

			pParentFrame->SendMessage (BCGM_TOOLBARMENU,
				(WPARAM) GetSafeHwnd (),
				MAKELPARAM(point.x, point.y));
		}

		return;
	}

	SetFocus ();

	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	int iButton = HitTest(ptClient);

	int iSelected = m_iSelected;
	m_iSelected = iButton;

	if (iSelected != -1)
	{
		InvalidateButton (iSelected);
	}

	if (m_iSelected != -1)
	{
		InvalidateButton (m_iSelected);
	}

	if (m_pSelToolbar != this)
	{
		CBCGToolBar* pSelToolbar = m_pSelToolbar;
		m_pSelToolbar = this;

		if (pSelToolbar != NULL)
		{
			ASSERT_VALID (pSelToolbar);

			int iOldSelected = pSelToolbar->m_iSelected;
			pSelToolbar->m_iSelected = -1;
			pSelToolbar->InvalidateButton (iOldSelected);
		}
	}

	UpdateWindow ();

	if (iButton < 0) // nothing hit
	{
		return;
	}

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (!pButton->IsEditable ())
	{
		m_iSelected = -1;
		InvalidateButton (iButton);
		UpdateWindow ();

		return;
	}

	if (pButton->CanBeStored ())
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CBCGLocalResource locaRes;

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_BCGBARRES_POPUP_BCGTOOL_BAR));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		if (pButton->IsLocked ())
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_BYCOMMAND | MF_GRAYED);
		}

		if (!EnableContextMenuItems (pButton, pPopup))
		{
			return;
		}

		//Disable StartGroup Item if left button is not visible
		int nPrevIndex = m_iSelected-1;
		if (nPrevIndex >= 0)
		{
			CBCGToolbarButton* pPrevButton = GetButton (nPrevIndex);
			if (pPrevButton != NULL && !pPrevButton->IsVisible ())
			{
				pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_BYCOMMAND | MF_GRAYED);
			}
		}

		pPopup->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,
								point.x, point.y, this);
	}
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarAppearance() 
{
#ifndef BCG_NO_CUSTOMIZATION

	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	CBCGLocalResource locaRes;

	CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
	if (dlg.DoModal () == IDOK)
	{
		AdjustLayout ();
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}

#endif // BCG_NO_CUSTOMIZATION
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarDelete() 
{
	ASSERT (m_iSelected >= 0);
	RemoveButton (m_iSelected);
	
	m_iSelected = -1;

	AdjustLayout ();
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarImage() 
{
#ifndef BCG_NO_CUSTOMIZATION

	CBCGLocalResource locaRes;

	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	BOOL bSaveText = pButton->m_bText;
	BOOL bSaveImage = pButton->m_bImage;

	pButton->m_bText = FALSE;
	pButton->m_bImage = TRUE;

	if (pButton->GetImage () < 0)
	{
		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
		if (dlg.DoModal () != IDOK)
		{
			pButton->m_bText = bSaveText;
			pButton->m_bImage = bSaveImage;
			return;
		}
	}

	AdjustLayout ();

#endif // BCG_NO_CUSTOMIZATION
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarImageAndText() 
{
#ifndef BCG_NO_CUSTOMIZATION

	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	BOOL bSaveText = pButton->m_bText;
	BOOL bSaveImage = pButton->m_bImage;

	pButton->m_bText = TRUE;
	pButton->m_bImage = TRUE;

	if (pButton->GetImage () < 0)
	{
		CBCGLocalResource locaRes;

		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
		if (dlg.DoModal () != IDOK)
		{
			pButton->m_bText = bSaveText;
			pButton->m_bImage = bSaveImage;
			return;
		}
	}

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	if (pButton->m_strText.IsEmpty ())
	{
		MessageBeep ((UINT) -1);

		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	AdjustLayout ();

#endif // BCG_NO_CUSTOMIZATION
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarStartGroup() 
{
	ASSERT (m_iSelected > 0);

	CBCGToolbarButton* pPrevButton = NULL;
	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGToolbarButton* pCurrButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pCurrButton);

		if (i == m_iSelected)
		{
			ASSERT (pPrevButton != NULL);	// m_iSelected > 0!

			if (pPrevButton->m_nStyle & TBBS_SEPARATOR)
			{
				if (pPrevButton->IsVisible ())
				{
					VERIFY (RemoveButton (m_iSelected - 1));
				}
			}
			else
			{
				InsertSeparator (m_iSelected ++);
			}

			break;
		}

		pPrevButton = pCurrButton;
	}

	AdjustLayout ();
}
//*******************************************************************************************
void CBCGToolBar::OnToolbarText() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	pButton->m_bText = TRUE;
	pButton->m_bImage = FALSE;

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	if (pButton->m_strText.IsEmpty ())
	{
		MessageBeep ((UINT) -1);

		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	AdjustLayout ();
}
//************************************************************************************
void CBCGToolBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CControlBar::OnWindowPosChanged(lpwndpos);
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

	RedrawCustomizeButton ();

	if (!IsCustomizeMode () || g_pWndCustomize == NULL || m_bLocked)
	{
		return;
	}

#ifndef BCG_NO_CUSTOMIZATION

	if (lpwndpos->flags & SWP_HIDEWINDOW)
	{
		g_pWndCustomize->ShowToolBar (this, FALSE);

		if (m_pSelToolbar == this)
		{
			m_pSelToolbar = NULL;
			m_iSelected = -1;
		}
	}
	
	if (lpwndpos->flags & SWP_SHOWWINDOW)
	{
		g_pWndCustomize->ShowToolBar (this, TRUE);
	}

#endif // BCG_NO_CUSTOMIZATION
}
//**************************************************************************************
HBRUSH CBCGToolBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CControlBar::OnCtlColor (pDC, pWnd, nCtlColor);
	if (!IsCustomizeMode () || m_bLocked)
	{
		return hbr;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pButton->IsOwnerOf (pWnd->GetSafeHwnd ()))
		{
			HBRUSH hbrButton = pButton->OnCtlColor (pDC, nCtlColor);
			return (hbrButton == NULL) ? hbr : hbrButton;
		}
	}

	return hbr;
}
//**************************************************************************************
int CBCGToolBar::GetCount () const
{
	return (int) m_Buttons.GetCount ();
}
//*************************************************************************************
BOOL CBCGToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	BOOL bStretch = m_bStretchButton;

	if (!bStretch && IsCustomizeMode () && m_iSelected != -1 && !m_bLocked)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (HitTest (ptCursor) == m_iSelected)
		{
			CBCGToolbarButton* pButton = GetButton (m_iSelected);
			ASSERT_VALID (pButton);

			if (pButton->CanBeStretched () &&
				abs (ptCursor.x - pButton->Rect ().right) <= STRETCH_DELTA)
			{
				bStretch = TRUE;
			}
		}
	}

	if (bStretch)
	{
		::SetCursor (globalData.m_hcurStretch);
		return TRUE;
	}
	
	return CControlBar::OnSetCursor(pWnd, nHitTest, message);
}
//****************************************************************************************
BOOL CBCGToolBar::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		if (m_bStretchButton)
		{
			OnCancelMode ();
		}
		else
		{
			CBCGToolbarMenuButton* pMenuButon = GetDroppedDownMenu ();
			if (pMenuButon != NULL)
			{
				return CControlBar::PreTranslateMessage(pMsg);
			}

			Deactivate ();
			RestoreFocus ();
		}

		return TRUE;
	}

	if(pMsg->message == BCGM_RESETRPROMPT)
	{
		OnPromptReset(0,0);

		return TRUE;
	}


	return CControlBar::PreTranslateMessage(pMsg);
}
//**************************************************************************************
BOOL CBCGToolBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (IsCustomizeMode () && !m_bLocked)
	{
		return CControlBar::OnCommand (wParam, lParam);
	}

	BOOL bAccelerator = FALSE;
	int nNotifyCode = HIWORD (wParam);

	// Find the control send the message:
	HWND hWndCtrl = (HWND)lParam;
	if (hWndCtrl == NULL)
	{
		if (wParam == IDCANCEL)	// ESC was pressed
		{
			RestoreFocus ();
			return TRUE;
		}

		if (wParam != IDOK ||
			(hWndCtrl = ::GetFocus ()) == NULL)
		{
			return FALSE;
		}

		bAccelerator = TRUE;
		nNotifyCode = 0;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		HWND hwdList = pButton->GetHwnd ();
		if (hwdList == NULL)	// No control
		{
			continue;
		}

		if (hwdList == hWndCtrl || ::IsChild (hwdList, hWndCtrl))
		{
			if (!bAccelerator)
			{
				ASSERT (LOWORD (wParam) == pButton->m_nID);
				if (!pButton->NotifyCommand (nNotifyCode))
				{
					break;
				}
			}

			GetOwner()->PostMessage (WM_COMMAND,
				MAKEWPARAM (pButton->m_nID, nNotifyCode), lParam);
			return TRUE;
		}
	}

	return TRUE;
}
//*************************************************************************************
CBCGToolBar* CBCGToolBar::FromHandlePermanent (HWND hwnd)
{
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		if (pToolBar->GetSafeHwnd () == hwnd)
		{
			return pToolBar;
		}
	}

	return NULL;
}
//**********************************************************************************
CSize CBCGToolBar::CalcLayout(DWORD dwMode, int nLength)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	
	if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		m_pCustomizeBtn->m_bIsEmpty = FALSE;
	}

	if (dwMode & LM_HORZDOCK)
	{
		ASSERT(dwMode & LM_HORZ);
	}

	m_nMaxBtnHeight = CalcMaxButtonHeight ();

	CSize sizeResult(0,0);

	if (!(m_dwStyle & CBRS_SIZE_FIXED))
	{
		BOOL bDynamic = m_dwStyle & CBRS_SIZE_DYNAMIC;

		if (bDynamic && (dwMode & LM_MRUWIDTH))
			SizeToolBar(m_nMRUWidth);
		else if (bDynamic && (dwMode & LM_HORZDOCK))
			SizeToolBar(32767);
		else if (bDynamic && (dwMode & LM_VERTDOCK))
		{
			SizeToolBar(0);
		}
		else if (bDynamic && (nLength != -1))
		{
			CRect rect; rect.SetRectEmpty();
			CalcInsideRect(rect, (dwMode & LM_HORZ));
			BOOL bVert = (dwMode & LM_LENGTHY);

			int nLen = nLength + (bVert ? rect.Height() : rect.Width());

			SizeToolBar(nLen, bVert);
		}
		else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
			SizeToolBar(m_nMRUWidth);
		else
			SizeToolBar((dwMode & LM_HORZ) ? 32767 : 0);
	}

	sizeResult = CalcSize ((dwMode & LM_HORZ) == 0);
	
	if (m_pCustomizeBtn != NULL &&
		(int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
		m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty ())
	{
		ASSERT_VALID (m_pCustomizeBtn);

		// Hide "Customize button and calc. size again:
		m_pCustomizeBtn->m_bIsEmpty = TRUE;
		sizeResult = CalcSize ((dwMode & LM_HORZ) == 0);
	}

	if (dwMode & LM_COMMIT)
	{
		if ((m_dwStyle & CBRS_FLOATING) && (m_dwStyle & CBRS_SIZE_DYNAMIC) &&
			(dwMode & LM_HORZ))
		{
			m_nMRUWidth = sizeResult.cx;
		}
	}

	//BLOCK: Adjust Margins
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, (dwMode & LM_HORZ));
		sizeResult.cy -= rect.Height();
		sizeResult.cx -= rect.Width();

		CSize size = CControlBar::CalcFixedLayout((dwMode & LM_STRETCH), (dwMode & LM_HORZ));
		sizeResult.cx = max(sizeResult.cx, size.cx);
		sizeResult.cy = max(sizeResult.cy, size.cy);
	}

	RebuildAccelerationKeys ();
	return sizeResult;
}
//**********************************************************************************
CSize CBCGToolBar::CalcSize (BOOL bVertDock)
{
	if (m_Buttons.IsEmpty ())
	{
		return GetButtonSize ();
	}

	CClientDC dc (this);
	CFont* pOldFont = (CFont*) dc.SelectObject (
		bVertDock ? &globalData.fontVert : &globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	CSize sizeResult (GetColumnWidth (), GetRowHeight ());

	CRect rect; rect.SetRectEmpty();
	CalcInsideRect (rect, !bVertDock);

	int iStartX = bVertDock ? 0 : 1;
	int iStartY = bVertDock ? 1 : 0;

	CPoint cur (iStartX, iStartY);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		if (pos == NULL && m_pCustomizeBtn != NULL && IsFloating ())
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == pButton);	// Should be last
			break;
		}

		CSize sizeDefault (GetColumnWidth (), 
				m_bDrawTextLabels ? GetButtonSize ().cy : GetRowHeight ());
		CSize sizeButton = pButton->OnCalculateSize (&dc, sizeDefault, !bVertDock);

		if (m_bDrawTextLabels)
		{
			sizeButton.cy = m_nMaxBtnHeight;
		}

		if (!bVertDock)
		{
			if ((cur.x == iStartX || pButton->m_bWrap)
				&& pButton->m_nStyle & TBBS_SEPARATOR)
			{
				sizeButton = CSize (0, 0);
			}

			sizeResult.cx = max (cur.x + sizeButton.cx, sizeResult.cx);
			sizeResult.cy = max (cur.y + sizeButton.cy, sizeResult.cy);

			cur.x += sizeButton.cx;

			if (pButton->m_bWrap)
			{
				cur.x = iStartX;
				cur.y += GetRowHeight () + LINE_OFFSET;
			}
		}
		else
		{
			sizeResult.cx = max (cur.x + sizeButton.cx, sizeResult.cx);
			sizeResult.cy = max (cur.y + sizeButton.cy, sizeResult.cy);

			cur.x = iStartX;
			cur.y += sizeButton.cy;
		}
	}

	dc.SelectObject (pOldFont);
	return sizeResult;
}
//**********************************************************************************
int CBCGToolBar::WrapToolBar (int nWidth, int nHeight /*= 32767*/)
{
	int nResult = 0;
	
	BOOL bVertDock = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;

	CClientDC dc (this);

	CFont* pOldFont = (CFont*) dc.SelectObject (
		bVertDock ? &globalData.fontVert : &globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	CBCGToolbarButton* pPrevButton = NULL;

	CRect rect;
	GetClientRect(rect);

	int x = 0;
	int y = rect.top;

    if (IsFloating())
    {
        nHeight = 32767;
    }

    if (!IsFloating()  &&  !bVertDock  &&  m_pCustomizeBtn)
    {
		CSize sizeButton = m_pCustomizeBtn->OnCalculateSize (&dc, 
			CSize (GetColumnWidth (), GetRowHeight ()), !bVertDock);
        nWidth -= sizeButton.cx;
    }


	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		pButton->m_bWrap = FALSE;

		if (pos == NULL && m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == pButton);	// Should be last
			break;
		}

		// Don't process invisivle buttons
		if (!pButton->IsVisible ())  
		{
			continue;
		}

		CSize sizeButton = pButton->OnCalculateSize (&dc, 
			CSize (GetColumnWidth (), GetRowHeight ()), !bVertDock);
		
		if (x == 0 && (pButton->m_nStyle & TBBS_SEPARATOR))
		{
			// Don't show separator on the first column!
			sizeButton = CSize (0, 0);
		}

		if (x + sizeButton.cx > nWidth &&
            y + sizeButton.cy < nHeight &&
			!(pButton->m_nStyle & TBBS_SEPARATOR))
		{
			if (pPrevButton != NULL)
			{
				pPrevButton->m_bWrap = TRUE;
				x = 0;
				y += sizeButton.cy + LINE_OFFSET;
				nResult ++;
			}
		}

		pPrevButton = pButton;
		x += sizeButton.cx;
	}

	dc.SelectObject (pOldFont);
	return nResult + 1;
}
//**********************************************************************************
void  CBCGToolBar::SizeToolBar (int nLength, BOOL bVert)
{
	CSize size;

	if (!bVert)
	{
		int nMin, nMax, nTarget, nCurrent, nMid;

		// Wrap ToolBar vertically
		nMin = 0;
		nCurrent = WrapToolBar(nMin);

		// Wrap ToolBar as specified
		nMax = nLength;
		nTarget = WrapToolBar(nMax);

		if (nCurrent != nTarget)
		{
			while (nMin < nMax)
			{
				nMid = (nMin + nMax) / 2;
				nCurrent = WrapToolBar(nMid);

				if (nCurrent == nTarget)
					nMax = nMid;
				else
				{
					if (nMin == nMid)
					{
						WrapToolBar(nMax);
						break;
					}
					nMin = nMid;
				}
			}
		}

		size = CalcSize (bVert);
		WrapToolBar (size.cx);
	}
	else
	{
		int iWidth = 32767;
		WrapToolBar (iWidth);

		size = CalcSize (FALSE);
		if (nLength > size.cy)
		{
			iWidth = 0;

			do
			{
				iWidth += GetButtonSize ().cx;
				WrapToolBar (iWidth);
				size = CalcSize (FALSE);
			}
			while (nLength < size.cy);
		}

		WrapToolBar (size.cx);
	}
}
//**********************************************************************************
void CBCGToolBar::OnSize(UINT nType, int cx, int cy) 
{
	SetRoundedRgn ();

	CControlBar::OnSize(nType, cx, cy);
	
	if (IsCustomizeMode () && !m_bLocked)
	{
		OnCancelMode ();
	}

	AdjustLocations ();

	//------------------------------------------------------
	// Adjust system menu of the floating toolbar miniframe:
	//------------------------------------------------------
	if (IsFloating ())
	{
		CMiniFrameWnd* pMiniFrame = 
			DYNAMIC_DOWNCAST (CMiniFrameWnd, GetParentFrame ());
		if (pMiniFrame != NULL)
		{
			CMenu* pSysMenu = pMiniFrame->GetSystemMenu(FALSE);
			ASSERT (pSysMenu != NULL);

			pSysMenu->DeleteMenu (SC_RESTORE, MF_BYCOMMAND);
			pSysMenu->DeleteMenu (SC_MINIMIZE, MF_BYCOMMAND);
			pSysMenu->DeleteMenu (SC_MAXIMIZE, MF_BYCOMMAND);

			if (!CanBeClosed ())
			{
				pSysMenu->EnableMenuItem (SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
			}
		}
	}
}
//**********************************************************************************
void CBCGToolBar::AdjustLocations ()
{
	ASSERT_VALID(this);

	if (m_Buttons.IsEmpty () || GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	CRect rectClient;
	GetClientRect (rectClient);

	int xRight = rectClient.right;

	CClientDC dc (this);
	CFont* pOldFont;
	if (bHorz)
	{
		pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	}
	else
	{
		pOldFont = (CFont*) dc.SelectObject (&globalData.fontVert);
	}
	
	ASSERT (pOldFont != NULL);

	int iStartOffset;
	if (bHorz)
	{
		iStartOffset = rectClient.left + 1;
	}
	else
	{
		iStartOffset = rectClient.top + 1;
	}

	int iOffset = iStartOffset;
	int y = rectClient.top;

	CSize sizeCustButton (0, 0);

	if (m_pCustomizeBtn != NULL && !IsFloating () && !IsCustomizeMode ())
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		m_pCustomizeBtn->m_lstInvisibleButtons.RemoveAll ();

		BOOL bIsEmpty = m_pCustomizeBtn->m_bIsEmpty;
		m_pCustomizeBtn->m_bIsEmpty = FALSE;

		sizeCustButton = m_pCustomizeBtn->OnCalculateSize (&dc,
			CSize (	bHorz ? GetColumnWidth () : rectClient.Width (), 
			bHorz ? rectClient.Height () : GetRowHeight ()), bHorz);

		m_pCustomizeBtn->m_bIsEmpty = bIsEmpty;
	}

	BOOL bPrevWasSeparator = FALSE;
	int nRowActualWidth = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		BOOL bVisible = TRUE;

		CSize sizeButton = pButton->OnCalculateSize (&dc, 
			CSize (GetColumnWidth (), GetRowHeight ()), bHorz);
		if (pButton->m_bTextBelow && bHorz)
		{
			sizeButton.cy =  GetRowHeight ();
		}

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (iOffset == iStartOffset || bPrevWasSeparator)
			{
				sizeButton = CSize (0, 0);
				bVisible = FALSE;
			}
			else
			{
				bPrevWasSeparator = TRUE;
			}
		}

		int iOffsetPrev = iOffset;

		CRect rectButton;
		if (bHorz)
		{
			rectButton.left = iOffset;
			rectButton.right = rectButton.left + sizeButton.cx;
			rectButton.top = y;
			rectButton.bottom = rectButton.top + sizeButton.cy;
			
			iOffset += sizeButton.cx;
			nRowActualWidth += sizeButton.cx;
		}
		else
		{
			rectButton.left = rectClient.left;
			rectButton.right = rectClient.left + sizeButton.cx;
			rectButton.top = iOffset;
			rectButton.bottom = iOffset + sizeButton.cy;

			iOffset += sizeButton.cy;
		}

		if (m_pCustomizeBtn != NULL && pButton != m_pCustomizeBtn &&
			!IsFloating () && !IsCustomizeMode ())
		{
			CSize fakeSizeCustButton (sizeCustButton);

			//-------------------------------------------------------------------------
			// I assume, that the customize button is at the tail position at any time.
			//-------------------------------------------------------------------------
			if ((int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
				(pos != NULL && m_Buttons.GetAt (pos) == m_pCustomizeBtn) && 
				m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty ())
			{
				fakeSizeCustButton = CSize (0,0);
			}

			if ((bHorz && rectButton.right > xRight - fakeSizeCustButton.cx) ||
				(!bHorz && rectButton.bottom > rectClient.bottom - fakeSizeCustButton.cy))
			{
				bVisible = FALSE;
				iOffset = iOffsetPrev;
				
				m_pCustomizeBtn->m_lstInvisibleButtons.AddTail (pButton);
			}
		}

		pButton->Show (bVisible);
		pButton->SetRect (rectButton);

		if (bVisible)
		{
			bPrevWasSeparator = (pButton->m_nStyle & TBBS_SEPARATOR);
		}

		if ((pButton->m_bWrap || pos == NULL) && bHorz)
		{
			//-----------------------
			// Center buttons in row:
			//-----------------------
			int nShift = (xRight - nRowActualWidth - iStartOffset) / 2;
			if (IsFloating () && nShift > 0 && m_bTextLabels)
			{
				for (POSITION posRow = posSave; posRow != NULL;)
				{
					BOOL bThis = (posRow == posSave);

					CBCGToolbarButton* pButtonRow = (CBCGToolbarButton*) m_Buttons.GetPrev (posRow);
					ASSERT (pButtonRow != NULL);

					if (pButtonRow->m_bWrap && !bThis)
					{
						break;
					}

					CRect rect = pButtonRow->Rect ();
					rect.OffsetRect (nShift, 0);
					pButtonRow->SetRect (rect);
				}
			}

			iOffset = iStartOffset;
			nRowActualWidth = 0;
			y += GetRowHeight () + LINE_OFFSET;
		}
	}

	if (m_pCustomizeBtn != NULL)
	{
		CRect rectButton = rectClient;

		if ((int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
			m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty () ||
			IsFloating () ||
			IsCustomizeMode ())
		{
			// Hide customize button:
			m_pCustomizeBtn->SetRect (CRect (0, 0, 0, 0));
			m_pCustomizeBtn->Show (FALSE);
		}
		else
		{
			if (bHorz)
			{
				rectButton.right = xRight - 1;
				rectButton.left = rectButton.right - sizeCustButton.cx + 1;
			}
			else
			{
				rectButton.bottom --;
				rectButton.top = rectButton.bottom - sizeCustButton.cy;
			}

			m_pCustomizeBtn->SetRect (rectButton);
			m_pCustomizeBtn->Show (TRUE);
		}
	}

	dc.SelectObject (pOldFont);
	RedrawCustomizeButton ();
}
//**********************************************************************************
DWORD CBCGToolBar::RecalcDelayShow(AFX_SIZEPARENTPARAMS* lpLayout)
{
	DWORD dwRes = CControlBar::RecalcDelayShow (lpLayout);

	if (!IsFloating ())
	{
		AdjustLocations ();
	}

	return dwRes;
}
//*********************************************************************************
void CBCGToolBar::AddRemoveSeparator (const CBCGToolbarButton* pButton,
						const CPoint& ptStart, const CPoint& ptDrop)
{
	ASSERT_VALID (pButton);
	
	int iIndex = ButtonToIndex (pButton);
	if (iIndex <= 0)
	{
		return;
	}

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
	int iDelta = (bHorz) ? ptDrop.x - ptStart.x : ptDrop.y - ptStart.y;

	if (abs (iDelta) < STRETCH_DELTA)
	{
		// Ignore small move....
		return;
	}

	if (iDelta > 0)	// Add a separator left of button
	{
		const CBCGToolbarButton* pLeftButton = GetButton (iIndex - 1);
		ASSERT_VALID (pLeftButton);

		if (pLeftButton->m_nStyle & TBBS_SEPARATOR)
		{
			// Already have separator, do nothing...
			return;
		}

		InsertSeparator (iIndex);
	}
	else	// Remove a separator in the left side
	{
		const CBCGToolbarButton* pLeftButton = GetButton (iIndex - 1);
		ASSERT_VALID (pLeftButton);

		if ((pLeftButton->m_nStyle & TBBS_SEPARATOR) == 0)
		{
			// Not a separator, do nothing...
			return;
		}

		if (pLeftButton->IsVisible ())
		{
			RemoveButton (iIndex - 1);
		}
	}

	AdjustLayout ();

	m_iSelected = -1;
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//***************************************************************************************
void CBCGToolBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int iButton = HitTest(point);
	if (iButton >= 0)
	{
		CBCGToolbarButton* pButton = GetButton (iButton);
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		BOOL bIsSysMenu = pButton->IsKindOf (RUNTIME_CLASS (CBCGToolbarSystemMenuButton));
		pButton->OnDblClick (this);

		if (!bIsSysMenu)
		{
			OnLButtonDown (nFlags, point);
		}

		//----------------------------------------------------------
		// Don't permit dock/undock when user double clicks on item!
		//----------------------------------------------------------
	}
	else
	{
		CControlBar::OnLButtonDblClk(nFlags, point);
	}
}
//***************************************************************************************
void CBCGToolBar::DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz)
{
	CBCGVisualManager::GetInstance ()->OnDrawSeparator (pDC, this, rect, bHorz);
}
//***************************************************************************************
CBCGToolbarButton* CBCGToolBar::CreateDroppedButton (COleDataObject* pDataObject)
{
#ifdef BCG_NO_CUSTOMIZATION
	return NULL;
#else
	CBCGToolbarButton* pButton = CBCGToolbarButton::CreateFromOleData (pDataObject);
	ASSERT (pButton != NULL);

	//---------------------------
	// Remove accelerator string:
	//---------------------------
	int iOffset = pButton->m_strText.Find (_T('\t'));
	if (iOffset >= 0)
	{
		pButton->m_strText = pButton->m_strText.Left (iOffset);
	}

	if (!pButton->m_bDragFromCollection)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	if (pButton->m_bDragFromCollection && 
		pButton->GetImage () == -1 &&
		pButton->m_strText.IsEmpty ())
	{
		//----------------------------------------------
		// User-defined button by default have no image
		// and text and empty. To avoid the empty button
		// appearance, ask user about it's properties:
		//----------------------------------------------
		CBCGLocalResource locaRes;
		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));

		if (dlg.DoModal () != IDOK)
		{
			delete pButton;
			return NULL;
		}
	}

	if (pButton->GetImage () < 0)
	{
		pButton->m_bText = TRUE;
		pButton->m_bImage = FALSE;
	}

	return pButton;

#endif // BCG_NO_CUSTOMIZATION
}
//****************************************************************************************
CBCGToolbarButton* CBCGToolBar::GetHighlightedButton () const
{
	if (m_iHighlighted < 0)
	{
		return NULL;
	}
	else
	{
		return GetButton (m_iHighlighted);
	}
}
//****************************************************************************************
void CBCGToolBar::RebuildAccelerationKeys ()
{
	m_AcellKeys.RemoveAll ();

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		if ((pButton->m_nStyle & TBBS_SEPARATOR) ||
			!pButton->m_bText)
		{
			continue;
		}

		int iAmpOffset = pButton->m_strText.Find (_T('&'));
		if (iAmpOffset >= 0 && iAmpOffset < pButton->m_strText.GetLength () - 1)
		{
			TCHAR szChar [2] = { pButton->m_strText.GetAt (iAmpOffset + 1), '\0' };
			CharUpper (szChar);
			UINT uiHotKey = (UINT) (szChar [0]);
			m_AcellKeys.SetAt (uiHotKey, pButton);
		}
	}
}
//****************************************************************************************
void CBCGToolBar::OnCustomizeMode (BOOL bSet)
{
	m_iButtonCapture = -1;
	m_iHighlighted = -1;
	m_iSelected = -1;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		pButton->OnCancelMode ();

		if ((pButton->m_nStyle & TBBS_DISABLED) == 0)
		{
			pButton->EnableWindow (!bSet);
		}
	}
}
//****************************************************************************************
BOOL CBCGToolBar::RestoreOriginalstate ()
{
	if (m_uiOriginalResID == 0)
	{
		return FALSE;
	}

	BOOL bRes = LoadToolBar (m_uiOriginalResID);

	AdjustLayout ();
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	return bRes;
}
//*****************************************************************************************
void CBCGToolBar::ShowCommandMessageString (UINT uiCmdId)
{
	if (m_hookMouseHelp != NULL)
	{
		return;
	}

	if (uiCmdId == (UINT) -1)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}

	UINT uiTrackId = uiCmdId;
	if (IsSystemCommand (uiCmdId))
	{
		uiTrackId = ID_COMMAND_FROM_SC (uiCmdId);
		ASSERT (uiTrackId >= AFX_IDS_SCFIRST &&
				uiTrackId < AFX_IDS_SCFIRST + 31);
	}
	else if (uiCmdId >= AFX_IDM_FIRST_MDICHILD)
	{
		// all MDI Child windows map to the same help id
		uiTrackId = AFX_IDS_MDICHILD;
	}

	GetOwner()->SendMessage (WM_SETMESSAGESTRING, (WPARAM) uiTrackId);
}
//*****************************************************************************************
afx_msg LRESULT CBCGToolBar::OnMouseLeave(WPARAM,LPARAM)
{
	if (m_hookMouseHelp != NULL || 
		(m_bMenuMode && !IsCustomizeMode () && GetDroppedDownMenu () != NULL))
	{
		return 0;
	}

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint (-1, -1);

	if (m_iHighlighted >= 0 && GetFocus () != this)
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = -1;

		OnChangeHot (m_iHighlighted);

		CBCGToolbarButton* pButton = InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		if (pButton == NULL || !pButton->IsDroppedDown ())
		{
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
	}

	return 0;
}
//*****************************************************************************************
BOOL CBCGToolBar::CanBeRestored () const
{
	return (m_uiOriginalResID != 0);
}
//*****************************************************************************************
BOOL CBCGToolBar::IsLastCommandFromButton (CBCGToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	HWND hwnd = pButton->GetHwnd ();

	if (!::IsWindow(hwnd))
	{
		return FALSE;
	}

	const MSG* pMsg = CWnd::GetCurrentMessage();
	if (pMsg == NULL)
	{
		return FALSE;
	}

	return (hwnd == (HWND) pMsg->lParam || hwnd == pMsg->hwnd);
}
//*****************************************************************************************
BOOL CBCGToolBar::AddToolBarForImageCollection (UINT uiResID, UINT uiBmpResID/*= 0*/,
												UINT uiColdResID/*= 0*/, UINT uiMenuResID/*= 0*/,
												UINT uiDisabledResID/*= 0*/, UINT uiMenuDisabledResID/*= 0*/)
{
	CBCGToolBar tlbTmp;
	return tlbTmp.LoadToolBar (uiResID, uiColdResID, uiMenuResID, FALSE, 
								uiDisabledResID, uiMenuDisabledResID, uiBmpResID);
}
//*****************************************************************************************
void CBCGToolBar::SetHotTextColor (COLORREF clrText)
{
	m_clrTextHot = clrText;
}
//*****************************************************************************************
COLORREF CBCGToolBar::GetHotTextColor ()
{
	return m_clrTextHot == (COLORREF) -1 ?
		globalData.clrBarText : m_clrTextHot;
}
//*****************************************************************************************
void CBCGToolBar::OnBcgbarresToolbarReset() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (g_pUserToolsManager == NULL ||
		g_pUserToolsManager->FindTool (pButton->m_nID) == NULL)
	{
		int iImage;
		if (m_DefaultImages.Lookup (pButton->m_nID, iImage))
		{
			pButton->m_bUserButton = FALSE;
			pButton->SetImage (iImage);
			pButton->m_bImage = TRUE;
		}
		else
		{
			pButton->m_bImage = FALSE;
		}
	}

	pButton->m_bText = m_bMenuMode || !pButton->m_bImage;

	//----------------------
	// Restore default text:
	//----------------------
	OnSetDefaultButtonText (pButton);

	AdjustLayout ();
	CMD_MGR.ClearCmdImage (pButton->m_nID);

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//***************************************************************************************
afx_msg LRESULT CBCGToolBar::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
	OnCancelMode ();

	int nIndex = HitTest ((DWORD) lParam);
	if (nIndex < 0)	// Click into the empty space or separator,
	{				// don't show HELP
		MessageBeep ((UINT) -1);
		return -1;
	}

	CBCGToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	if (pButton->OnContextHelp (this))
	{
		return -1;	// Continue help mode
	}

	LRESULT lres = CControlBar::OnHelpHitTest (wParam, lParam);

	if (lres > 0)
	{
		SetHelpMode (FALSE);
	}

	return lres;
}
//****************************************************************************************
LRESULT CALLBACK CBCGToolBar::BCGToolBarMouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION)
	{
		return CallNextHookEx (m_hookMouseHelp, nCode, wParam, lParam);
	}

	MOUSEHOOKSTRUCT* lpMS = (MOUSEHOOKSTRUCT*) lParam;
	ASSERT (lpMS != NULL);

	if (wParam == WM_MOUSEMOVE)
	{
		//------------------------------------------
		// Find a toolbar matched to the mouse hook:
		//------------------------------------------
		CBCGToolBar* pWndToolBar = 
			DYNAMIC_DOWNCAST (CBCGToolBar, CWnd::WindowFromPoint (lpMS->pt));
		if (pWndToolBar != NULL)
		{
			CPoint ptClient = lpMS->pt;
			pWndToolBar->ScreenToClient (&ptClient);
			pWndToolBar->OnMouseMove (0, ptClient);
		}

		if (m_pLastHookedToolbar != NULL &&
			m_pLastHookedToolbar != pWndToolBar)
		{
			m_pLastHookedToolbar->m_bTracked = FALSE;
			m_pLastHookedToolbar->m_ptLastMouse = CPoint (-1, -1);

			if (m_pLastHookedToolbar->m_iHighlighted >= 0)
			{
				int iButton = m_pLastHookedToolbar->m_iHighlighted;
				m_pLastHookedToolbar->m_iHighlighted = -1;

				m_pLastHookedToolbar->OnChangeHot (m_pLastHookedToolbar->m_iHighlighted);

				m_pLastHookedToolbar->InvalidateButton (iButton);
				m_pLastHookedToolbar->UpdateWindow(); // immediate feedback
			}
		}

		m_pLastHookedToolbar = pWndToolBar;
	}

	return 0;
}
//***************************************************************************************
void CBCGToolBar::SetHelpMode (BOOL bOn)
{
	if (bOn)
	{
		if (m_hookMouseHelp == NULL)	// Not installed yet, set it now!
		{
			m_hookMouseHelp = ::SetWindowsHookEx (WH_MOUSE, BCGToolBarMouseProc, 
				0, GetCurrentThreadId ());
			if (m_hookMouseHelp == NULL)
			{
				TRACE (_T("CBCGToolBar: Can't set mouse hook!\n"));
			}
		}
	}
	else if (m_hookMouseHelp != NULL)
	{
		::UnhookWindowsHookEx (m_hookMouseHelp);
		m_hookMouseHelp = NULL;

		m_pLastHookedToolbar = NULL;

		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			POSITION posSave = posTlb;

			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);
				pToolBar->OnCancelMode ();
			}

			posTlb = posSave;
			gAllToolbars.GetNext (posTlb);
		}
	}
}
//***************************************************************************************
void CBCGToolBar::SetNonPermittedCommands (CList<UINT, UINT>& lstCommands)
{
	m_lstUnpermittedCommands.RemoveAll ();
	m_lstUnpermittedCommands.AddTail (&lstCommands);
}
//***************************************************************************************
void CBCGToolBar::SetBasicCommands (CList<UINT, UINT>& lstCommands)
{
	m_lstBasicCommands.RemoveAll ();
	m_lstBasicCommands.AddTail (&lstCommands);
}
//***************************************************************************************
void CBCGToolBar::AddBasicCommand (UINT uiCmd)
{
	if (m_lstBasicCommands.Find (uiCmd) == NULL)
	{
		m_lstBasicCommands.AddTail (uiCmd);
	}
}
//***************************************************************************************
void CBCGToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	m_Impl.CalcNcSize (lpncsp);
}
//***************************************************************************************
void CBCGToolBar::OnNcPaint() 
{
	m_Impl.DrawNcArea ();
}
//****************************************************************************************
BCGNcHitTestType CBCGToolBar::OnNcHitTest(CPoint /*point*/) 
{
	return HTCLIENT;
}
//***************************************************************************************
void CBCGToolBar::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		pButton->m_bTextBelow = ((pButton->m_nStyle & TBBS_SEPARATOR) == 0) &&
								m_bTextLabels && bHorz;
	}	

#ifndef BCG_NO_REBAR

	CReBar* pBar = DYNAMIC_DOWNCAST (CReBar, GetParent ());
	if (pBar != NULL)
	{
		CReBarCtrl& wndReBar = pBar->GetReBarCtrl ();
		UINT uiReBarsCount = wndReBar.GetBandCount ();

		REBARBANDINFO bandInfo;
		bandInfo.cbSize = sizeof (bandInfo);
		bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

		UINT uiBand = 0;

		for (uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
		{
			wndReBar.GetBandInfo (uiBand, &bandInfo);
			if (bandInfo.hwndChild == GetSafeHwnd ())
			{
				break;
			}
		}

		bandInfo.fMask ^= RBBIM_CHILD;

		if (uiBand >= uiReBarsCount)
		{
			ASSERT (FALSE);
		}
		else
		{
			CSize size = CControlBar::CalcFixedLayout (FALSE, TRUE);

			m_nMaxBtnHeight = CalcMaxButtonHeight ();
			CSize sizeMin = CalcSize (FALSE);

			CRect rect; rect.SetRectEmpty();
			CalcInsideRect (rect, TRUE);
			sizeMin.cy -= rect.Height();
			sizeMin.cx -= rect.Width();

			sizeMin.cx = max(sizeMin.cx, size.cx);
			sizeMin.cy = max(sizeMin.cy, size.cy);

			bandInfo.cxMinChild = m_sizeButton.cx;
			bandInfo.cyMinChild = sizeMin.cy;

			bandInfo.cxIdeal = sizeMin.cx;

			wndReBar.SetBandInfo (uiBand, &bandInfo);
		}
	}
	else

#endif // BCG_NO_REBAR

	{
		CFrameWnd* pParent = GetParentFrame ();
		if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
		{
			pParent->RecalcLayout ();
		}
	}

	AdjustLocations ();
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//****************************************************************************************
void CBCGToolBar::OnBcgbarresCopyImage() 
{
	ASSERT (m_iSelected >= 0);

	CBCGToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
	ASSERT (pButton->GetImage () >= 0);

	//-----------------------------
	// Is this button "user tool"?
	//-----------------------------
	CBCGUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !pButton->m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (pButton->m_nID);
		if (pUserTool != NULL)
		{
			pUserTool->CopyIconToClipboard ();
			return;
		}
	}

	CBCGToolBarImages* pImages = (pButton->m_bUserButton) ? 
			m_pUserImages : &m_Images;
	ASSERT (pImages != NULL);

	CWaitCursor wait;
	pImages->CopyImageToClipboard (pButton->GetImage ());
}
//****************************************************************************************
BOOL CBCGToolBar::OnSetDefaultButtonText (CBCGToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nID == 0 || pButton->m_nID == (UINT) -1)
	{
		return FALSE;
	}

	TCHAR szFullText [256];
	CString strTipText;

	if (AfxLoadString (pButton->m_nID, szFullText) &&
		AfxExtractSubString (strTipText, szFullText, 1, '\n'))
	{
		pButton->m_strText = strTipText;
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
void CBCGToolBar::SetMenuSizes (SIZE sizeButton, SIZE sizeImage)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	//-----------------------------------------------------------------
	// Button must be big enough to hold image + 3 pixels on each side:
	//-----------------------------------------------------------------
	ASSERT(sizeButton.cx >= sizeImage.cx + 6);
	ASSERT(sizeButton.cy >= sizeImage.cy + 6);

	m_sizeMenuButton = sizeButton;
	m_sizeMenuImage = sizeImage;

	m_MenuImages.SetImageSize (m_sizeMenuImage);
	m_DisabledMenuImages.SetImageSize (m_sizeMenuImage);
}
//****************************************************************************************
CSize CBCGToolBar::GetMenuImageSize ()
{
	if (m_sizeMenuImage.cx == -1)
	{
		return m_sizeImage;
	}
	else
	{
		return m_sizeMenuImage;
	}
}
//****************************************************************************************
CSize CBCGToolBar::GetMenuButtonSize ()
{
	if (m_sizeMenuButton.cx == -1)
	{
		return m_sizeButton;
	}
	else
	{
		return m_sizeMenuButton;
	}
}
//****************************************************************************************
BOOL CBCGToolBar::EnableContextMenuItems (CBCGToolbarButton* pButton, CMenu* pPopup)
{
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	if (!pButton->OnCustomizeMenu (pPopup))
	{
		if (!pButton->m_bImage || pButton->GetImage () < 0)
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_COPY_IMAGE, MF_BYCOMMAND | MF_GRAYED);
		}

		if (pButton->m_nID == (UINT) -1 || pButton->m_nID == 0)
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_BYCOMMAND | MF_GRAYED);
		}

		if (pButton->m_bText || (pButton->m_bTextBelow && bHorz))
		{
			if (pButton->m_bImage)
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_CHECKED  | MF_BYCOMMAND);
			}
			else
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_CHECKED  | MF_BYCOMMAND);
			}
		}
		else
		{
			ASSERT (pButton->m_bImage);
			pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_CHECKED | MF_BYCOMMAND);
		}

		if (pButton->m_bTextBelow && bHorz)
		{
			//------------------------
			// Text is always visible!
			//------------------------
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_BYCOMMAND | MF_GRAYED);
		}

		if (IsPureMenuButton (pButton))
		{
			//--------------------------
			// Disable text/image items:
			//--------------------------
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);

			pButton->m_bText = TRUE;
		}
	}

	//---------------------------
	// Adjust "Start group" item:
	//---------------------------
	CBCGToolbarButton* pPrevButton = NULL;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pCurrButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pCurrButton);

		if (pCurrButton == pButton)
		{
			if (pPrevButton == NULL)	// First button
			{
				pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_BYCOMMAND | MF_GRAYED);
			}
			else if (pPrevButton->m_nStyle & TBBS_SEPARATOR)
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_CHECKED  | MF_BYCOMMAND);
			}

			break;
		}

		pPrevButton = pCurrButton;
	}

	return TRUE;
}
//***************************************************************************************
void CBCGToolBar::OnChangeHot (int iHot)
{
	if (m_iHot == iHot && m_iHot >= 0)
	{
		iHot = -1;
	}

	m_iHot = iHot;

	CBCGToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu ();
	if (pCurrPopupMenu == NULL && !CBCGToolBar::IsCustomizeMode ())
	{
		return;
	}

	if (iHot < 0 || iHot >= m_Buttons.GetCount())
	{
		m_iHot = -1;
		if (pCurrPopupMenu != NULL && CBCGToolBar::IsCustomizeMode () &&
			!m_bAltCustomizeMode)
		{
			pCurrPopupMenu->OnCancelMode ();
		}

		return;
	}

	CBCGToolbarMenuButton* pMenuButton =
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, GetButton (iHot));

	if (pMenuButton != pCurrPopupMenu)
	{
		if (pCurrPopupMenu != NULL)
		{
			int iHighlighted = m_iHighlighted;

			if (!CBCGToolBar::IsCustomizeMode ())
			{
				m_iHighlighted = -1;
			}

			pCurrPopupMenu->OnCancelMode ();

			m_iHighlighted = iHighlighted;
		}

		if (pMenuButton != NULL &&
			(!CBCGToolBar::IsCustomizeMode () || 
			!pMenuButton->IsKindOf (RUNTIME_CLASS (CBCGToolbarSystemMenuButton))))
		{
			pMenuButton->OnClick (this);
		}
	}
	else
	{
		if (CBCGToolBar::IsCustomizeMode () &&
			pCurrPopupMenu != NULL && pCurrPopupMenu->IsDroppedDown ())
		{
			pCurrPopupMenu->OnCancelMode ();
		}
	}

	if (IsCustomizeMode () && m_iDragIndex < 0)
	{
		int nSelected = m_iHighlighted;
		m_iSelected = m_iHot;

		if (nSelected != -1)
		{
			InvalidateButton (nSelected);
		}

		CBCGToolbarButton* pSelButton = GetButton (m_iSelected);
		if (pSelButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		if (pSelButton->m_nStyle & TBBS_SEPARATOR)
		{
			m_iSelected = -1;
		}
		else
		{
			InvalidateButton (m_iSelected);
		}
	}
	// --- End ----

	if (m_iHot >= 0 && m_iHot != m_iHighlighted)
	{
		int iCurrHighlighted = m_iHighlighted;
		if (iCurrHighlighted >= 0)
		{
			InvalidateButton (iCurrHighlighted);
		}

		m_iHighlighted = m_iHot;

		InvalidateButton (m_iHighlighted);
		UpdateWindow ();
	}
}
//*****************************************************************************************
BOOL CBCGToolBar::PrevMenu ()
{
	int iHot;
	CBCGToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu (&iHot);
	if (pCurrPopupMenu == NULL)
	{
		return FALSE;
	}

	int iHotOriginal = iHot;
	int iTotalItems = GetCount ();

	while (--iHot != iHotOriginal)
	{
		if (iHot < 0)
		{
			iHot = iTotalItems - 1;
		}

		
		CBCGToolbarButton* pButton = GetButton (iHot);
		if (DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton) != NULL &&
			(pButton->m_nStyle & TBBS_DISABLED) == 0 && !pButton->IsHidden ())
		{
			break;
		}
	}

	if (iHot == iHotOriginal)	// Only one menu item on the toolbar,
	{							// do nothing
		return TRUE;
	}

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPopupMenu::ANIMATION_TYPE animType = CBCGPopupMenu::GetAnimationType ();
	CBCGPopupMenu::SetAnimationType (CBCGPopupMenu::NO_ANIMATION);

	OnChangeHot (iHot);

	//-----------------------
	// Select the first item:
	//-----------------------
	if (m_iHot >= 0)
	{
		CBCGToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, GetButton (m_iHot));
		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
		}
	}

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPopupMenu::SetAnimationType (animType);
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGToolBar::NextMenu ()
{
	int iHot;
	CBCGToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu (&iHot);
	if (pCurrPopupMenu == NULL)
	{
		return FALSE;
	}

	int iHotOriginal = iHot;
	int iTotalItems = GetCount ();

	while (++iHot != iHotOriginal)
	{
		if (iHot >= iTotalItems)
		{
			iHot = 0;
		}

		CBCGToolbarButton* pButton = GetButton (iHot);
		if (DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton) != NULL &&
			(pButton->m_nStyle & TBBS_DISABLED) == 0 && !pButton->IsHidden ())
		{
			break;
		}
	}

	if (iHot == iHotOriginal)	// Only one menu item on the toolbar,
	{							// do nothing
		return TRUE;
	}

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPopupMenu::ANIMATION_TYPE animType = CBCGPopupMenu::GetAnimationType ();
	CBCGPopupMenu::SetAnimationType (CBCGPopupMenu::NO_ANIMATION);

	OnChangeHot (iHot);

	//-----------------------
	// Select the first item:
	//-----------------------
	if (m_iHot >= 0)
	{
		CBCGToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, GetButton (m_iHot));
		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
		}
	}

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPopupMenu::SetAnimationType (animType);
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGToolBar::SetHot (CBCGToolbarButton *pMenuButton)
{
	if (pMenuButton == NULL)
	{
		m_iHot = -1;
		return TRUE;
	}

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pMenuButton == pButton)
		{
			if (m_iHot != i)
			{
				OnChangeHot (i);
			}
			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGToolBar::DropDownMenu (CBCGToolbarButton* pButton)
{
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (pButton);

	//----------------------------
	// Simulate menu button click:
	//----------------------------
	CBCGToolbarMenuButton* pMenuButton =
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
	if (pMenuButton == NULL || !pMenuButton->OnClick (this))
	{
		return FALSE;
	}

	//----------------------------
	// Select the first menu item:
	//----------------------------
	if (pMenuButton->IsDroppedDown ())
	{
		pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
	}

	SetHot (pMenuButton);
	return TRUE;
}
//********************************************************************************************
BOOL CBCGToolBar::ProcessCommand (CBCGToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nID == 0 ||
		pButton->m_nID == (UINT) -1)
	{
		return FALSE;
	}

	BCGPlaySystemSound (BCGSOUND_MENU_COMMAND);

	//-----------------------
	// Send command to owner:
	//-----------------------
	AddCommandUsage (pButton->m_nID);
	GetOwner()->PostMessage (WM_COMMAND, pButton->m_nID);

	return TRUE;
}
//********************************************************************************************
CBCGToolbarMenuButton* CBCGToolBar::GetDroppedDownMenu (int* pIndex) const
{
	if (m_Buttons.IsEmpty ())
	{
		return NULL;
	}

	int iIndex = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			if (pIndex != NULL)
			{
				*pIndex = iIndex;
			}

			return pMenuButton;
		}
	}

	if (pIndex != NULL)
	{
		*pIndex = -1;
	}

	return NULL;
}
//******************************************************************
void CBCGToolBar::Deactivate ()
{
	if (m_iHighlighted >= 0 && m_iHighlighted < m_Buttons.GetCount ())
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = m_iHot = -1;

		InvalidateButton (iButton);
		UpdateWindow ();

		GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	RestoreFocus ();
}
//*********************************************************************
BOOL CBCGToolBar::SaveParameters (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		bResult =	reg.Write (REG_ENTRY_TOOLTIPS, m_bShowTooltips) &&
					reg.Write (REG_ENTRY_KEYS, m_bShowShortcutKeys) &&
					reg.Write (REG_ENTRY_LARGE_ICONS, m_bLargeIcons) &&
					reg.Write (REG_ENTRY_ANIMATION, (int) CBCGPopupMenu::GetAnimationType ()) &&
					reg.Write (REG_ENTRY_RU_MENUS, CBCGMenuBar::m_bRecentlyUsedMenus) &&
					reg.Write (REG_ENTRY_MENU_SHADOWS, CBCGMenuBar::m_bMenuShadows) &&
					reg.Write (REG_ENTRY_SHOW_ALL_MENUS_DELAY, CBCGMenuBar::m_bShowAllMenusDelay) &&
					reg.Write (REG_ENTRY_LOOK2000, CBCGVisualManager::GetInstance ()->IsLook2000 ()) &&
					reg.Write (REG_ENTRY_CMD_USAGE_COUNT, m_UsageCount);
	}

	return bResult;
}
//*****************************************************************************************
BOOL CBCGToolBar::LoadParameters (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	BOOL bLook2000 = FALSE;
	int iAnimType = CBCGPopupMenu::NO_ANIMATION;

	bResult =	reg.Read (REG_ENTRY_TOOLTIPS, m_bShowTooltips) &&
				reg.Read (REG_ENTRY_KEYS, m_bShowShortcutKeys) &&
				reg.Read (REG_ENTRY_LARGE_ICONS, m_bLargeIcons) &&
				reg.Read (REG_ENTRY_ANIMATION, iAnimType) &&
				reg.Read (REG_ENTRY_RU_MENUS, CBCGMenuBar::m_bRecentlyUsedMenus) &&
				reg.Read (REG_ENTRY_MENU_SHADOWS, CBCGMenuBar::m_bMenuShadows) &&
				reg.Read (REG_ENTRY_SHOW_ALL_MENUS_DELAY, CBCGMenuBar::m_bShowAllMenusDelay) &&
				reg.Read (REG_ENTRY_LOOK2000, bLook2000) &&
				reg.Read (REG_ENTRY_CMD_USAGE_COUNT, m_UsageCount);

	CBCGPopupMenu::SetAnimationType ((CBCGPopupMenu::ANIMATION_TYPE) iAnimType);
	SetLargeIcons (m_bLargeIcons);

	CBCGVisualManager::GetInstance ()->SetLook2000 (bLook2000);
	return bResult;
}
//**********************************************************************************
void CBCGToolBar::OnSetFocus(CWnd* pOldWnd) 
{
	CControlBar::OnSetFocus(pOldWnd);

	if (pOldWnd != NULL &&
		::IsWindow (pOldWnd->GetSafeHwnd ()) &&
		DYNAMIC_DOWNCAST (CBCGToolBar, pOldWnd) == NULL &&
		DYNAMIC_DOWNCAST (CBCGToolBar, pOldWnd->GetParent ()) == NULL)
	{
		m_hwndLastFocus = pOldWnd->GetSafeHwnd ();
	}
}
//**********************************************************************************
void CBCGToolBar::RestoreFocus ()
{
	if (::IsWindow (m_hwndLastFocus))
	{
		::SetFocus (m_hwndLastFocus);
	}

	m_hwndLastFocus = NULL;
}
//*********************************************************************************
void CBCGToolBar::OnBcgbarresToolbarNewMenu() 
{
#ifndef BCG_NO_CUSTOMIZATION

	CBCGToolbarMenuButton* pMenuButton = new CBCGToolbarMenuButton;
	pMenuButton->m_bText = TRUE;
	pMenuButton->m_bImage = FALSE;

	CBCGLocalResource locaRes;
	CButtonAppearanceDlg dlg (pMenuButton, m_pUserImages, this, 0, IsPureMenuButton (pMenuButton));
	if (dlg.DoModal () != IDOK)
	{
		delete pMenuButton;
		return;
	}

	m_iSelected = InsertButton (pMenuButton, m_iSelected);

	AdjustLayout ();
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	pMenuButton->OnClick (this, FALSE);

#endif // BCG_NO_CUSTOMIZATION
}
//**********************************************************************************
void CBCGToolBar::SetToolBarBtnText (UINT nBtnIndex,
									LPCTSTR szText,
									BOOL bShowText,
									BOOL bShowImage)
{
	CBCGToolbarButton* pButton = GetButton (nBtnIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (bShowText)
	{
		if (szText == NULL)
		{
			OnSetDefaultButtonText(pButton);
		}
		else
		{
			SetButtonText(nBtnIndex, szText);
		}
	}

	pButton->m_bText = bShowText;
	pButton->m_bImage = bShowImage;
}
//*************************************************************************************
void CBCGToolBar::SetLargeIcons (BOOL bLargeIcons/* = TRUE*/)
{
	m_bLargeIcons = bLargeIcons;

	if (m_bLargeIcons)
	{
		m_sizeCurButton.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeButton.cx);
		m_sizeCurButton.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeButton.cy);

		m_sizeCurImage.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeImage.cx);
		m_sizeCurImage.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeImage.cy);
	}
	else
	{
		m_sizeCurButton = m_sizeButton;
		m_sizeCurImage = m_sizeImage;
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar->m_bLocked)
			{
				// Locked toolbars have its individual sizes
				if (m_bLargeIcons)
				{
					pToolBar->m_sizeCurButtonLocked.cx = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cx);
					pToolBar->m_sizeCurButtonLocked.cy = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cy);

						pToolBar->m_sizeCurImageLocked.cx = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cx);
						pToolBar->m_sizeCurImageLocked.cy = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cy);
				}
				else
				{
					pToolBar->m_sizeCurButtonLocked = pToolBar->m_sizeButtonLocked;
					pToolBar->m_sizeCurImageLocked = pToolBar->m_sizeImageLocked;
				}
			}

			pToolBar->AdjustLayout ();
		}
	}
}
//************************************************************************************
BOOL CBCGToolBar::IsCommandRarelyUsed (UINT uiCmd)
{
	if (IsCustomizeMode () ||
		uiCmd == 0 || uiCmd == (UINT) -1 ||
		IsStandardCommand (uiCmd) ||
		m_lstBasicCommands.IsEmpty ())
	{
		return FALSE;
	}

	if ((uiCmd == ID_BCGBARRES_TASKPANE_BACK) ||
		(uiCmd == ID_BCGBARRES_TASKPANE_FORWARD) ||
		(uiCmd == ID_BCGBARRES_TASKPANE_OTHER))
	{
		return FALSE;
	}

	return !IsBasicCommand (uiCmd) &&
		!m_UsageCount.IsFreqeuntlyUsedCmd (uiCmd);
}
//************************************************************************************
BOOL CBCGToolBar::SetCommandUsageOptions (UINT nStartCount, UINT nMinUsagePercentage)
{
	return m_UsageCount.SetOptions (nStartCount, nMinUsagePercentage);
}
//***********************************************************************************
void CBCGToolBar::EnableLargeIcons (BOOL bEnable)
{
	ASSERT (GetSafeHwnd () == NULL);	// Should not be created yet!
	m_bLargeIconsAreEnbaled = bEnable;
}
//*************************************************************************************
void CBCGToolBar::EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, 
										 const CString& strCustomizeText, BOOL bQuickCustomize)
{
	if (bEnable)
	{
		if (m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);

			m_pCustomizeBtn->m_uiCustomizeCmdId = uiCustomizeCmd;
			m_pCustomizeBtn->m_strText =  strCustomizeText;
		}
		else
		{
			if (InsertButton (CCustomizeButton (uiCustomizeCmd, strCustomizeText)) < 0)
			{
				ASSERT (FALSE);
				return;
			}

			m_pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, m_Buttons.GetTail ());
			ASSERT_VALID (m_pCustomizeBtn);
		}

		m_bQuickCustomize = bQuickCustomize;
	}
	else if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		m_Buttons.RemoveTail ();
		delete m_pCustomizeBtn;
		m_pCustomizeBtn = NULL;
	}
}
//*******************************************************************************************
void CBCGToolBar::EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, UINT uiCustomizeTextResId, BOOL bQuickCustomize)
{
	CString strCustomizeText;
	strCustomizeText.LoadString (uiCustomizeTextResId);

	EnableCustomizeButton (bEnable, uiCustomizeCmd, strCustomizeText, bQuickCustomize);
}
//*******************************************************************************************
void CBCGToolBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	CControlBar::OnWindowPosChanging(lpwndpos);
	
#ifndef BCG_NO_REBAR

	CReBar* pBar = DYNAMIC_DOWNCAST (CReBar, GetParent ());
	if (pBar != NULL)
	{
		AdjustLocations ();
	}

#endif
}
//*******************************************************************************************
void CBCGToolBar::EnableTextLabels (BOOL bEnable/* = TRUE*/)
{
	if (m_bMenuMode)
	{
		ASSERT (FALSE);
		return;
	}

	m_bTextLabels = bEnable;
	AdjustLayout ();
}
//****************************************************************************************
int CBCGToolBar::CalcMaxButtonHeight ()
{
	ASSERT_VALID (this);

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;
	m_bDrawTextLabels = FALSE;

	if (!m_bTextLabels || !bHorz)
	{
		return 0;
	}

	int nMaxBtnHeight = 0;
	CClientDC dc (this);

	CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	//-----------------------------------------------------------------------
	// To better look, I'm assuming that all rows shoud be of the same height.
	// Calculate max. button height:
	//-----------------------------------------------------------------------
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
		
		if (pButton->m_bTextBelow)
		{
			if (pButton->m_strText.IsEmpty ())
			{
				OnSetDefaultButtonText (pButton);
			}

			CSize sizeButton = pButton->OnCalculateSize (&dc, 
				GetButtonSize (), bHorz);

			nMaxBtnHeight = max (nMaxBtnHeight, sizeButton.cy);
		}
	}

	m_bDrawTextLabels = (nMaxBtnHeight > GetButtonSize ().cy);
	dc.SelectObject (pOldFont);
	return nMaxBtnHeight;
}
//***************************************************************************************
void CBCGToolBar::ResetAllImages()
{
	m_Images.Clear();
	m_ColdImages.Clear();
	m_MenuImages.Clear();
	m_DisabledMenuImages.Clear();
	m_LargeImages.Clear();
	m_LargeColdImages.Clear();
	m_LargeDisabledImages.Clear();

	m_DefaultImages.RemoveAll();
}
//**********************************************************************************
void CBCGToolBar::ResetImages ()
//
// Reset all toolbar images exept user-derfined to the default
//
{
	ASSERT_VALID (this);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->ResetImageToDefault ();
	}

	if (IsFloating ())
	{
		AdjustLayout ();
	}
}
//**********************************************************************************
BOOL CBCGToolBar::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//**********************************************************************************
BOOL CBCGToolBar::OnUserToolTip (CBCGToolbarButton* pButton, CString& strTTText) const
{
	ASSERT_VALID (pButton);

	CFrameWnd* pTopFrame = BCGGetTopLevelFrame (this);
	if (pTopFrame == NULL)
	{
		return FALSE;
	}

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->GetToolbarButtonToolTipText (pButton, strTTText);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->GetToolbarButtonToolTipText (pButton, strTTText);
		}
		else	// Maybe, OLE frame...
		{
			CBCGOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pTopFrame);
			if (pOleFrame != NULL)
			{
				return pOleFrame->GetToolbarButtonToolTipText (pButton, strTTText);
			}
			else
			{
				CBCGOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pTopFrame);
				if (pOleDocFrame != NULL)
				{
					return pOleDocFrame->GetToolbarButtonToolTipText (pButton, strTTText);
				}
			}
		}
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGToolBar::OnKillFocus(CWnd* pNewWnd) 
{
	CControlBar::OnKillFocus(pNewWnd);
	
	if (!IsCustomizeMode ())
	{
		CBCGPopupMenu* pMenu = DYNAMIC_DOWNCAST	(CBCGPopupMenu, pNewWnd);
		if (pMenu == NULL || pMenu->GetParentToolBar () != this)
		{
			Deactivate ();
		}
	}
}
//********************************************************************************
void CBCGToolBar::ResetAll ()
{
	CMD_MGR.ClearAllCmdImages ();

	POSITION pos = NULL;

	//------------------------------------------
	// Fill image hash by the default image ids:
	//------------------------------------------
	for (pos = CBCGToolBar::m_DefaultImages.GetStartPosition (); pos != NULL;)
	{
		UINT uiCmdId;
		int iImage;

		CBCGToolBar::m_DefaultImages.GetNextAssoc (pos, uiCmdId, iImage);
		CMD_MGR.SetCmdImage (uiCmdId, iImage, FALSE);
	}

	for (pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (pToolBar->CanBeRestored ())
		{
			pToolBar->RestoreOriginalstate ();
		}
	}
}
//****************************************************************************************
BOOL CBCGToolBar::TranslateChar (UINT nChar)
{
	// ----------------------------
	// Ensure the key is printable:
	// ----------------------------
	WORD wChar = 0;
	BYTE lpKeyState [256];
	::GetKeyboardState (lpKeyState);

	int nRes = ::ToAsciiEx (nChar,
				MapVirtualKey (nChar, 0),
				lpKeyState,
				&wChar,
				1,
				::GetKeyboardLayout (AfxGetThread()->m_nThreadID));

	BOOL bKeyIsPrintable = nRes > 0;

	if (!bKeyIsPrintable)
	{
		return FALSE;
	}

	UINT nUpperChar = CBCGKeyboardManager::TranslateCharToUpper (nChar);

	CBCGToolbarButton* pButton = NULL;
	if (!m_AcellKeys.Lookup (nUpperChar, pButton))
	{
		return FALSE;
	}

	ASSERT_VALID (pButton);

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPopupMenu::ANIMATION_TYPE animType = CBCGPopupMenu::GetAnimationType ();
	CBCGPopupMenu::SetAnimationType (CBCGPopupMenu::NO_ANIMATION);

	BOOL bRes = DropDownMenu (pButton);

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPopupMenu::SetAnimationType (animType);

	if (bRes)
	{
		return TRUE;
	}

	return ProcessCommand (pButton);
}
//**************************************************************************************
const CObList& CBCGToolBar::GetAllToolbars ()
{
	return gAllToolbars;
}
//**************************************************************************************
void CBCGToolBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CControlBar::OnSettingChange(uFlags, lpszSection);

	if (uFlags == SPI_SETNONCLIENTMETRICS)
	{
		globalData.UpdateFonts();
		AdjustLayout ();
	}
}
//******************************************************************************
BOOL CBCGToolBar::IsUserDefined () const
{
	ASSERT_VALID (this);

	CFrameWnd* pTopFrame = BCGGetTopLevelFrame (this);
	if (pTopFrame == NULL)
	{
		return FALSE;
	}

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->m_Impl.IsUserDefinedToolbar (this);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->m_Impl.IsUserDefinedToolbar (this);
		}
		else	// Maybe, OLE frame...
		{
			CBCGOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pTopFrame);
			if (pOleFrame != NULL)
			{
				return pOleFrame->m_Impl.IsUserDefinedToolbar (this);
			}
			else
			{
				CBCGOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pTopFrame);
				if (pOleDocFrame != NULL)
				{
					return pOleDocFrame->m_Impl.IsUserDefinedToolbar (this);
				}
			}
		}
	}

	return FALSE;
}
//******************************************************************************
void CBCGToolBar::CleanUpImages ()
{
	m_Images.Clear ();
	m_ColdImages.Clear ();
	m_MenuImages.Clear ();
	m_DisabledImages.Clear ();
	m_DisabledMenuImages.Clear ();
	m_LargeImages.Clear();
	m_LargeColdImages.Clear();
	m_LargeDisabledImages.Clear();

	m_DefaultImages.RemoveAll ();
	m_UsageCount.Reset ();

	CBCGToolBarImages::CleanUp ();
}
//*********************************************************************
void CBCGToolBar::CleanUpLockedImages ()
{
	if (!m_bLocked)
	{
		return;
	}

	m_ImagesLocked.Clear ();
	m_ColdImagesLocked.Clear ();
	m_DisabledImagesLocked.Clear ();
	m_LargeImagesLocked.Clear ();
	m_LargeColdImagesLocked.Clear ();
	m_LargeDisabledImagesLocked.Clear ();
	m_MenuImagesLocked.Clear ();
	m_DisabledMenuImagesLocked.Clear ();
}
//*********************************************************************
LRESULT CBCGToolBar::OnGetButtonCount(WPARAM,LPARAM)
{
	return GetCount();
}
//*********************************************************************
LRESULT CBCGToolBar::OnGetItemRect(WPARAM wParam, LPARAM lParam)
{
	GetItemRect ((int) wParam, (LPRECT)lParam);
	return TRUE;
}
//*********************************************************************
LRESULT CBCGToolBar::OnGetButton(WPARAM wParam, LPARAM lParam)
{
	int idx = int(wParam);
	TBBUTTON * pButton = (TBBUTTON *)lParam;
	UINT style = GetButtonStyle( idx );
	pButton->fsStyle = LOBYTE( LOWORD( style ) );
	pButton->fsState = LOBYTE( HIWORD( style ) );
    pButton->idCommand = GetItemID( idx );
	pButton->iBitmap = 0;
	pButton->dwData = 0;
	pButton->iString = 0;
	return TRUE;
}
//*********************************************************************
LRESULT CBCGToolBar::OnGetButtonText(WPARAM wParam, LPARAM lParam)
{
	int idx = CommandToIndex ((int) wParam);
	_tcscpy ((LPTSTR) lParam, GetButtonText (idx));
	return strlen( (LPSTR)lParam );
}
//*********************************************************************
void CBCGToolBar::SetLook2000 (BOOL bLook2000)
{
	CBCGVisualManager::GetInstance ()->SetLook2000 (bLook2000);
}
//*********************************************************************
BOOL CBCGToolBar::IsLook2000 ()
{
	return CBCGVisualManager::GetInstance ()->IsLook2000 ();
}
//*********************************************************************
BOOL CBCGToolBar::SmartUpdate (const CObList& lstPrevButtons)
{
	m_bResourceWasChanged = FALSE;

	POSITION posPrev = NULL;
	POSITION posCurr = NULL;

	//-----------------------------
	// Looking for deleted buttons:
	//-----------------------------
	for (posPrev = lstPrevButtons.GetHeadPosition (); posPrev != NULL;)
	{
		CBCGToolbarButton* pButtonPrev = 
			DYNAMIC_DOWNCAST (CBCGToolbarButton, lstPrevButtons.GetNext (posPrev));
		ASSERT_VALID (pButtonPrev);

		//----------------------------
		// Find item in the curr.data:
		//----------------------------
		BOOL bFound = FALSE;
		POSITION posCurr = NULL;

		for (posCurr = m_OrigButtons.GetHeadPosition (); posCurr != NULL;)
		{
			CBCGToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			if (pButtonCurr->CompareWith (*pButtonPrev))
			{
				bFound = TRUE;
				break;
			}
		}

		if (!bFound)	// Not found, item was deleted
		{
			m_bResourceWasChanged = TRUE;

			int iIndex = CommandToIndex (pButtonPrev->m_nID);
			if (iIndex >= 0)
			{
				RemoveButton (iIndex);
			}
		}
	}

	//-----------------------------
	// Looking for the new buttons:
	//-----------------------------
	int i = 0;
	for (posCurr = m_OrigButtons.GetHeadPosition (); posCurr != NULL; i++)
	{
		CBCGToolbarButton* pButtonCurr = 
			DYNAMIC_DOWNCAST (CBCGToolbarButton, m_OrigButtons.GetNext (posCurr));
		ASSERT_VALID (pButtonCurr);

		//----------------------------
		// Find item in the prev.data:
		//----------------------------
		BOOL bFound = FALSE;

		for (posPrev = lstPrevButtons.GetHeadPosition (); posPrev != NULL;)
		{
			CBCGToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (pButtonCurr->CompareWith (*pButtonPrev))
			{
				bFound = TRUE;
				break;
			}
		}

		if (!bFound)	// Not found, new item!
		{
			m_bResourceWasChanged = TRUE;

			UINT uiCmd = pButtonCurr->m_nID;
			int iIndex = min ((int) m_Buttons.GetCount (), i);

			if (uiCmd == 0)	// Separator
			{
				InsertSeparator (iIndex);
			}
			else
			{
				int iImage = -1;
				m_DefaultImages.Lookup (uiCmd, iImage);

				InsertButton (
					CBCGToolbarButton (uiCmd, iImage, NULL, FALSE, m_bLocked), iIndex);
			}
		}
	}

	//--------------------------------
	// Compare current and prev. data:
	//--------------------------------
	if (lstPrevButtons.GetCount () != m_OrigButtons.GetCount ())
	{
		m_bResourceWasChanged = TRUE;
	}
	else
	{
		for (posCurr = m_OrigButtons.GetHeadPosition (),
			posPrev = lstPrevButtons.GetHeadPosition (); posCurr != NULL;)
		{
			ASSERT (posPrev != NULL);

			CBCGToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			CBCGToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (!pButtonCurr->CompareWith (*pButtonPrev))
			{
				m_bResourceWasChanged = TRUE;
				break;
			}
		}
	}

	return m_bResourceWasChanged;
}
//************************************************************************************
void CBCGToolBar::SaveOriginalState (CBCGRegistry& reg)
{
	if (!m_OrigButtons.IsEmpty ())
	{
		reg.Write (REG_ENTRY_ORIG_ITEMS, m_OrigButtons);
	}
}
//*************************************************************************************
BOOL CBCGToolBar::LoadLastOriginalState (CBCGRegistry& reg)
{
	BOOL bIsUpdated = FALSE;

	CObList lstOrigButtons;	// Original (resource) data in the last session
	if (reg.Read (REG_ENTRY_ORIG_ITEMS, lstOrigButtons))
	{
		bIsUpdated = SmartUpdate (lstOrigButtons);
	}

	while (!lstOrigButtons.IsEmpty ())
	{
		delete lstOrigButtons.RemoveHead ();
	}

	return bIsUpdated;
}
//***************************************************************************************
CBCGToolBarImages* CBCGToolBar::GetImageList (CBCGToolBarImages& images, CBCGToolBarImages& imagesLocked, 
							 CBCGToolBarImages& largeImages, CBCGToolBarImages& largeImagesLocked) const
{
	if (m_bLocked)
	{
		return (!m_bMenuMode && m_bLargeIcons && largeImagesLocked.GetCount () > 0) ?
				&largeImagesLocked : &imagesLocked;
	}
	else
	{
		return (!m_bMenuMode && m_bLargeIcons && largeImages.GetCount () > 0) ?
				&largeImages : &images;
	}
}
//********************************************************************************
void CBCGToolBar::OnGlobalFontsChanged ()
{
	ASSERT_VALID (this);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
	
		pButton->OnGlobalFontsChanged ();
	}
}
//********************************************************************************
void CBCGToolBar::AutoGrayInactiveImages (BOOL bEnable/* = TRUE*/,
										   int nGrayPercentage/* = 0 */,
										   BOOL bRedrawAllToolbars/* = TRUE*/)
{
	m_bAutoGrayInactiveImages = bEnable;
	m_nGrayImagePercentage = nGrayPercentage;

	if (m_bAutoGrayInactiveImages)
	{
		m_Images.CopyTo (m_ColdImages);
		m_ColdImages.GrayImages (m_nGrayImagePercentage);
	}
	else
	{
		m_ColdImages.Clear ();
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar->IsLocked())
			{
				ASSERT_VALID (pToolBar);

				if (m_bAutoGrayInactiveImages)
				{
					pToolBar->m_ImagesLocked.CopyTo (pToolBar->m_ColdImagesLocked);
					pToolBar->m_ColdImagesLocked.GrayImages (m_nGrayImagePercentage);
				}
				else
				{
					pToolBar->m_ColdImagesLocked.Clear ();
				}
			}

			if (bRedrawAllToolbars)
			{
				pToolBar->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
			}
		}
	}
}
//*************************************************************************************
LRESULT CBCGToolBar::OnPromptReset (WPARAM, LPARAM)
{
	//Get Toolbar caption
	CString strCaption;
	GetWindowText (strCaption);
	strCaption.TrimLeft ();	strCaption.TrimRight ();

	CString strPrompt;

	{
		CBCGLocalResource locaRes;  

		if(!strCaption.GetLength ())
		{
			strCaption.LoadString (IDS_BCGBARRES_UNTITLED_TOOLBAR);
		}

		strPrompt.Format (IDS_BCGBARRES_RESET_TOOLBAR_FMT, strCaption);
	}

	//Ask for reset
	if (AfxMessageBox (strPrompt, MB_OKCANCEL|MB_ICONWARNING) == IDOK)
	{
		RestoreOriginalstate();
	}

	return 0;
}
//*************************************************************************************
void CBCGToolBar::SaveResetOriginalState (CBCGRegistry& reg)
{
	if (!m_OrigResetButtons.IsEmpty ())
	{
		reg.Write (REG_ENTRY_RESET_ITEMS, m_OrigResetButtons);
	}
}
//*************************************************************************************
BOOL CBCGToolBar::LoadResetOriginalState (CBCGRegistry& reg)
{
	CObList lstOrigButtons;
	if (reg.Read (REG_ENTRY_RESET_ITEMS, lstOrigButtons))
	{
		if (lstOrigButtons.GetCount() > 0)
		{
			while (!m_OrigResetButtons.IsEmpty ())
			{
				delete m_OrigResetButtons.RemoveHead ();
			}

			int i = 0;
			for (POSITION pos = lstOrigButtons.GetHeadPosition (); pos != NULL; i++)
			{
				CBCGToolbarButton* pButton = (CBCGToolbarButton*) lstOrigButtons.GetNext (pos);

				if(pButton != NULL && pButton->IsKindOf(RUNTIME_CLASS(CBCGToolbarButton)))
				m_OrigResetButtons.AddTail(pButton);
			}
		}
	}

	return TRUE;
}
//****************************************************************************
void CBCGToolBar::OnChangeVisualManager ()
{
	m_bRoundShape = 
		CBCGVisualManager::GetInstance ()->IsToolbarRoundShape (this);

	if (m_bRoundShape)
	{
		SetRoundedRgn ();
	}
	else
	{
		SetWindowRgn (NULL, FALSE);
	}

	if (!IsLocked ())
	{
		AdjustLayout ();
	}

	UpdateImagesColor ();
}
//****************************************************************************
void CBCGToolBar::SetRoundedRgn ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (!m_bRoundShape || m_pDockBar == NULL)
	{
		SetWindowRgn (NULL, FALSE);
		return;
	}

	CRect rectWindow;
	GetWindowRect (rectWindow);

	CRgn rgn;
	rgn.CreateRoundRectRgn (0, 0, rectWindow.Width () + 1, rectWindow.Height () + 1, 
		4, 4);

	SetWindowRgn (rgn, FALSE);
}
//****************************************************************************
void CBCGToolBar::RedrawCustomizeButton ()
{
	if (GetSafeHwnd () == NULL || m_pCustomizeBtn == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pCustomizeBtn);

	CRect rect = m_pCustomizeBtn->GetInvalidateRect ();
	rect.InflateRect (m_pCustomizeBtn->GetExtraSize ());

	rect.right += 10;
	rect.bottom += 10;

	RedrawWindow (rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
}

