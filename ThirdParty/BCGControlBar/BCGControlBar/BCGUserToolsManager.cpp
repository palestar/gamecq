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

// BCGUserToolsManager.cpp: implementation of the CBCGUserToolsManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGToolBar.h"
#include "BCGUserToolsManager.h"
#include "BCGRegistry.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CString strUserToolsProfile = _T("BCGUserToolsManager");
static const CString strUserToolsEntry = _T("Tools");

CBCGUserToolsManager*	g_pUserToolsManager = NULL;
extern CObList	gAllToolbars;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGUserToolsManager::CBCGUserToolsManager(const UINT uiCmdToolsDummy,
										   const UINT uiCmdFirst, const UINT uiCmdLast,
										   CRuntimeClass* pToolRTC,
										   UINT uArgMenuID, UINT uInitDirMenuID) :
	m_uiCmdToolsDummy (uiCmdToolsDummy),
	m_uiCmdFirst (uiCmdFirst),
	m_uiCmdLast (uiCmdLast),
	m_pToolRTC (pToolRTC),
	m_uiArgumentsMenuID (uArgMenuID),
	m_uiInitialDirMenuID (uInitDirMenuID)    
{
	ASSERT (g_pUserToolsManager == NULL);
	g_pUserToolsManager = this;

	VERIFY (m_pToolRTC != NULL);
	VERIFY (m_pToolRTC->IsDerivedFrom (RUNTIME_CLASS (CBCGUserTool)));

	ASSERT (m_uiCmdFirst <= m_uiCmdLast);

	//---------------------
	// Load default filter:
	//---------------------
	CBCGLocalResource locaRes;
	m_strFilter.LoadString (IDS_BCGBARRES_CMD_FILTER);

	m_strDefExt = _T("*.exe");
}
//****************************************************************************************
CBCGUserToolsManager::~CBCGUserToolsManager()
{
	while (!m_lstUserTools.IsEmpty ())
	{
		delete m_lstUserTools.RemoveHead ();
	}

	g_pUserToolsManager = NULL;
}
//***********************************************************************************************
BOOL CBCGUserToolsManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strUserToolsProfile, lpszProfileName);

	while (!m_lstUserTools.IsEmpty ())
	{
		delete m_lstUserTools.RemoveHead ();
	}

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.Open (strProfileName))
	{
		return FALSE;
	}

	if (!reg.Read (strUserToolsEntry, m_lstUserTools))
	{
		//---------------------------------------------------------
		// Tools objects may be corrupted, so, I don't delete them.
		// Memory leak is possible!
		//---------------------------------------------------------
		m_lstUserTools.RemoveAll ();
		return FALSE;
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGUserToolsManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strUserToolsProfile, lpszProfileName);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strProfileName))
	{
		return FALSE;
	}

	return reg.Write (strUserToolsEntry, m_lstUserTools);
}
//*****************************************************************************************
CBCGUserTool* CBCGUserToolsManager::CreateNewTool ()
{
	ASSERT (m_pToolRTC != NULL);

	if (m_lstUserTools.GetCount () >= GetMaxTools ())
	{
		TRACE(_T("Too many user-defined tools. The max. number is %d"), GetMaxTools ());
		return FALSE;
	}

	UINT uiCmdId = 0;

	//-----------------------------------
	// Find a first available command id:
	//-----------------------------------
	for (uiCmdId = m_uiCmdFirst; uiCmdId <= m_uiCmdLast; uiCmdId ++)
	{
		BOOL bIsCmdAvailable = TRUE;

		for (POSITION pos = m_lstUserTools.GetHeadPosition (); pos != NULL;)
		{
			CBCGUserTool* pListTool = (CBCGUserTool*) m_lstUserTools.GetNext (pos);
			ASSERT_VALID (pListTool);

			if (pListTool->GetCommandId () == uiCmdId)
			{
				bIsCmdAvailable = FALSE;
				break;
			}
		}

		if (bIsCmdAvailable)
		{
			break;
		}
	}

	if (uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	CBCGUserTool* pTool = (CBCGUserTool*) m_pToolRTC->CreateObject ();
	ASSERT_VALID (pTool);

	pTool->m_uiCmdId = uiCmdId;

	m_lstUserTools.AddTail (pTool);
	return pTool;
}
//*****************************************************************************************
BOOL CBCGUserToolsManager::RemoveTool (CBCGUserTool* pTool)
{
	ASSERT_VALID (pTool);
	POSITION pos = m_lstUserTools.Find (pTool);

	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);

	UINT uiCmdId = pTool->GetCommandId ();
	delete pTool;

	//------------------------------------
	// Remove user tool from all toolbars:
	//------------------------------------
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		BOOL bToolIsFound = FALSE;

		int iIndex = -1;
		while ((iIndex = pToolBar->CommandToIndex (uiCmdId)) >= 0)
		{
			pToolBar->RemoveButton (iIndex);
			bToolIsFound = TRUE;
		}

		if (bToolIsFound)
		{
			pToolBar->AdjustLayout ();
		}
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGUserToolsManager::MoveToolUp (CBCGUserTool* pTool)
{
	ASSERT_VALID (pTool);

	POSITION pos = m_lstUserTools.Find (pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posPrev = pos;
	m_lstUserTools.GetPrev (posPrev);
	if (posPrev == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);
	m_lstUserTools.InsertBefore (posPrev, pTool);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGUserToolsManager::MoveToolDown (CBCGUserTool* pTool)
{
	ASSERT_VALID (pTool);

	POSITION pos = m_lstUserTools.Find (pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posNext = pos;
	m_lstUserTools.GetNext (posNext);
	if (posNext == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);
	m_lstUserTools.InsertAfter (posNext, pTool);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGUserToolsManager::InvokeTool (UINT uiCmdId)
{
	CBCGUserTool* pTool = FindTool (uiCmdId);
	if (pTool == NULL)
	{
		return FALSE;
	}

	return pTool->Invoke ();
}
//******************************************************************************
CBCGUserTool* CBCGUserToolsManager::FindTool (UINT uiCmdId) const
{
	if (uiCmdId < m_uiCmdFirst || uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	for (POSITION pos = m_lstUserTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGUserTool* pListTool = (CBCGUserTool*) m_lstUserTools.GetNext (pos);
		ASSERT_VALID (pListTool);

		if (pListTool->GetCommandId () == uiCmdId)
		{
			return pListTool;
		}
	}

	return NULL;
}
