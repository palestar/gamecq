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
// BCGDlgImpl.h: interface for the CBCGDlgImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGDLGIMPL_H__D8C3B45B_D50A_4579_9FBF_BA86AAB8FF07__INCLUDED_)
#define AFX_BCGDLGIMPL_H__D8C3B45B_D50A_4579_9FBF_BA86AAB8FF07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBCGPopupMenu;

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGDlgImpl  
{
	friend class CBCGDialog;
	friend class CBCGPropertyPage;
	friend class CBCGPropertySheet;

protected:
	CBCGDlgImpl(CWnd& dlg);
	virtual ~CBCGDlgImpl();

	static LRESULT CALLBACK BCGDlgMouseProc (int nCode, WPARAM wParam, LPARAM lParam);

	void SetActiveMenu (CBCGPopupMenu* pMenu);

	BOOL ProcessMouseClick (POINT pt);
	BOOL ProcessMouseMove (POINT pt);

	BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnCommand (WPARAM wParam, LPARAM lParam);
	void OnNcActivate (BOOL& bActive);
	void OnActivate(UINT nState, CWnd* pWndOther);

	void OnDestroy ();

	CWnd&				m_Dlg;
	static HHOOK		m_hookMouse;
	static CBCGDlgImpl*	m_pMenuDlgImpl;
};

#endif // !defined(AFX_BCGDLGIMPL_H__D8C3B45B_D50A_4579_9FBF_BA86AAB8FF07__INCLUDED_)
