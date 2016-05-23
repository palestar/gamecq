//*******************************************************************************
// COPYRIGHT NOTES
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //*******************************************************************************

#include "stdafx.h"
#include "BCGCommandManager.h"
#include "BCGRegistry.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REG_PARAMS_FMT						_T("%sBCGCommandManager")
#define REG_ENTRY_COMMANDS_WITHOUT_IMAGES	_T("CommandsWithoutImages")

static const CString strToolbarProfile	= _T("BCGToolBars");

//////////////////////////////////////////////////////////////////////
// One global static CBCGCommandManager Object
//////////////////////////////////////////////////////////////////////
class _STATIC_CREATOR_
{
public:
	CBCGCommandManager s_TheCmdMgr;
};

static _STATIC_CREATOR_ STATIC_CREATOR;

BCGCONTROLBARDLLEXPORT CBCGCommandManager* GetCmdMgr()
{
	return &STATIC_CREATOR.s_TheCmdMgr;
}
//////////////////////////////////////////////////////////////////////


#ifndef _NO_BCG_LEGACY_
UINT CImageHash::GetImageOfCommand(UINT nID, BOOL bUser /*= false*/)
{
	return CMD_MGR.GetCmdImage(nID, bUser);
}
#endif

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

CBCGCommandManager::CBCGCommandManager()
{
}

CBCGCommandManager::~CBCGCommandManager()
{
}

//////////////////////////////////////////////////////////////////////
// ImageHash functions
//////////////////////////////////////////////////////////////////////

//****************************************************************************************
void CBCGCommandManager::SetCmdImage (UINT uiCmd, int iImage, BOOL bUserImage)
{
	if (uiCmd == 0 || uiCmd == (UINT) -1)
	{
		return;
	}

	if (bUserImage)
	{
		// If command is already associated to the "standard" image list,
		// don't assign to to the "user" images
		if (GetCmdImage (uiCmd, FALSE) < 0)
		{
			m_CommandIndexUser.SetAt (uiCmd, iImage);
		}
	}
	else
	{
		if (GetCmdImage (uiCmd, TRUE) < 0)
		{
			m_CommandIndex.SetAt (uiCmd, iImage);
		}
	}
}
//****************************************************************************************
int CBCGCommandManager::GetCmdImage (UINT uiCmd, BOOL bUserImage) const
{
	int iImage = -1;

	if (bUserImage)
	{
		if (!m_CommandIndexUser.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	else
	{
		if (!m_CommandIndex.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	
	return iImage;
}
//***************************************************************************************
void CBCGCommandManager::ClearCmdImage (UINT uiCmd)
{
	m_CommandIndexUser.RemoveKey (uiCmd);
}
//****************************************************************************************
void CBCGCommandManager::ClearAllCmdImages ()
{
	m_CommandIndex.RemoveAll ();
	m_CommandIndexUser.RemoveAll ();
	m_lstCommandsWithoutImages.RemoveAll ();
}
//****************************************************************************************
void CBCGCommandManager::CleanUp ()
{
	ClearAllCmdImages ();
}
//**************************************************************************************
void CBCGCommandManager::EnableMenuItemImage (UINT uiCmd, BOOL bEnable)
{
	POSITION pos = m_lstCommandsWithoutImages.Find (uiCmd);
	
	if (bEnable)
	{
		if (pos != NULL)
		{
			m_lstCommandsWithoutImages.RemoveAt (pos);
		}
	}
	else
	{
		if (pos == NULL)
		{
			m_lstCommandsWithoutImages.AddTail (uiCmd);
		}
	}
}
//*************************************************************************************
BOOL CBCGCommandManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	return reg.Read (REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages);
}
//*************************************************************************************
BOOL CBCGCommandManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages);
	}

	return FALSE;
}
