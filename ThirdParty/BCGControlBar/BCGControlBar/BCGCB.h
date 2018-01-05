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

#ifndef __BCGCB_H
#define __BCGCB_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

#include "bcgcbver.h"	// Library version info.

#ifdef _BCGCB_IN_OTHER_DLL
BCGCONTROLBARDLLEXPORT void BCGControlBarDllInitialize ();
#endif // _BCGCB_IN_OTHER_DLL

#ifndef _BCGCB_NO_AUTOLINK_

#define _BCGCB_LIBNAME1_	"BCGCB700"

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_
	#define _BCGCB_LIBNAME2_	_BCGCB_LIBNAME1_
#else
	#define _BCGCB_LIBNAME2_	_BCGCB_LIBNAME1_##"Static"
#endif

#ifdef _UNICODE
	#define _BCGCB_LIBNAME3_	_BCGCB_LIBNAME2_##"U"
#else
	#define _BCGCB_LIBNAME3_	_BCGCB_LIBNAME2_
#endif

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_
	#define _BCGCB_LIBNAME4_	_BCGCB_LIBNAME3_
#elif defined _BCGCONTROLBAR_STATIC_
	#define _BCGCB_LIBNAME4_	_BCGCB_LIBNAME3_##"s"
#else
	#define _BCGCB_LIBNAME4_	_BCGCB_LIBNAME3_
#endif

#if _MSC_VER < 1300
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_##""
#elif _MSC_VER == 1300
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_##"70"
#elif _MSC_VER < 1400
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_##"71"
#else
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_##"80"
#endif

#ifdef _DEBUG
#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"D.lib"
#else
#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##".lib"
#endif

#pragma comment(lib, _BCGCB_LIBNAME_)
#pragma message("Automatically linking with " _BCGCB_LIBNAME_)

#endif // _BCGCB_NO_AUTOLINK_

//------------------
// BCG control bars:
//------------------
#include "BCGToolBar.h"
#include "BCGStatusBar.h"
#include "BCGMenuBar.h"
#include "BCGDialogBar.h"
#include "BCGOutlookBar.h"
#include "BCGColorBar.h"
#include "BCGCaptionBar.h"
#include "BCGTasksPane.h"

//-------------------------
// BCG control bar buttons:
//-------------------------
#include "BCGToolbarButton.h"
#include "BCGToolbarComboBoxButton.h"
#include "BCGToolbarDateTimeCtrl.h"
#include "BCGToolbarMenuButton.h"
#include "BCGToolbarRegularMenuButton.h"
#include "BCGToolbarEditBoxButton.h"
#include "BCGToolbarSpinEditBoxButton.h"
#include "BCGDropDown.h"
#include "BCGColorMenuButton.h"
#include "BCGToolbarFontCombo.h"

//-------------------------------------------------------------------
// BCG frame windows (replaces CFrameWnd, CMDIFrameWnd, CMDIChildWnd,
// COleIPFrameWnd and OleDocIPFrameWnd):
//-------------------------------------------------------------------
#include "BCGFrameWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGMDIChildWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"

#include "BCGAppBarWnd.h"

//-------------------------
// BCG customization stuff:
//-------------------------
#include "CBCGToolbarCustomize.h"

#include "BCGContextMenuManager.h"
#include "BCGKeyboardManager.h"
#include "BCGMouseManager.h"

#include "BCGUserTool.h"
#include "KeyHelper.h"

//-----------------------
// BCG workspace manager
//-----------------------
#include "BCGWorkspace.h"
#include "BCGRegistry.h"
#include "RebarState.h"

//-------------
// BCG helpers:
//-------------
#include "BCGTearOffManager.h"
#include "BCGDrawManager.h"

//-----------------------
// BCG menu replacements:
//-----------------------
#include "BCGPopupMenu.h"
#include "BCGPopupMenuBar.h"
#include "BCGToolBarImages.h"

//------------------
// BCG docking bars:
//------------------
#include "BCGSizingControlBar.h"
#include "BCGTabWnd.h"

//--------------
// BCG controls:
//--------------
#include "BCGButton.h"
#include "BCGColorButton.h"
#include "BCGMenuButton.h"
#include "BCGURLLinkButton.h"
#include "BCGEditListBox.h"
#include "BCGAnimCtrl.h"
#include "PowerColorPicker.h"
#include "BCGFontComboBox.h"
#include "BCGHeaderCtrl.h"
#include "BCGListCtrl.h"
#include "BCGPropList.h"
#include "BCGMaskEdit.h"
#include "BCGSpinButtonCtrl.h"
#include "BCGSplitterWnd.h"
#include "BCGPopupWindow.h"
#include "BCGPopupDlg.h"

//-------------
// BCG dialogs:
//-------------
#include "BCGDialog.h"
#include "BCGPropertySheet.h"
#include "BCGPropertyPage.h"
#include "BCGFileDialog.h"
#include "BCGWindowsManagerDlg.h"
#include "BCGPrintPreviewView.h"
#include "ImageEditDlg.h"
#include "BCGColorDialog.h"
#include "BCGKeyMapDlg.h"

//-----------
// BCG views:
//-----------
#include "BCGTabView.h"

//--------------------
// Visualization stuf:
//--------------------
#include "BCGVisualManager.h"
#include "BCGVisualManagerXP.h"
#include "BCGWinXPVisualManager.h"
#include "BCGVisualManager2003.h"
#include "BCGVisualManagerVS2005.h"
#include "BCGSkinManager.h"

//----------------
// Shell controls:
//----------------
#include "BCGShellManager.h"
#include "BCGShellTree.h"
#include "BCGShellList.h"

//----------------
// Accessibility:
//----------------
#include "BCGAccessibility.h"

#endif // __BCGCB_H
