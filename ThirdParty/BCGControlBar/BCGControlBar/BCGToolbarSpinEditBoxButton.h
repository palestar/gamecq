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
// BCGToolbarSpinEditBoxButton.h: interface for the CBCGToolbarSpinEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARSPINEDITBOXBUTTON_H__C8BA7618_0A7D_4B74_AC8A_3D99977F3A91__INCLUDED_)
#define AFX_BCGTOOLBARSPINEDITBOXBUTTON_H__C8BA7618_0A7D_4B74_AC8A_3D99977F3A91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarEditBoxButton.h"
#include "BCGSpinButtonCtrl.h"

class BCGCONTROLBARDLLEXPORT CBCGToolbarSpinEditBoxButton : public CBCGToolbarEditBoxButton  
{
	DECLARE_SERIAL(CBCGToolbarSpinEditBoxButton)

// Construction
public:
	CBCGToolbarSpinEditBoxButton();
	CBCGToolbarSpinEditBoxButton(UINT uiID, int iImage, DWORD dwStyle = ES_AUTOHSCROLL, int iWidth = 0);
	virtual ~CBCGToolbarSpinEditBoxButton();

// Operations
public:
	void SetRange (int nMin, int nMax);
	void GetRange (int& nMin, int& nMax);

protected:
	void Init ();

// Attributes
protected:
   CBCGSpinButtonCtrl	m_wndSpin;
   int					m_nMin;
   int					m_nMax;

// Overrides
protected:
	virtual CEdit* CreateEdit (CWnd* pWndParent, const CRect& rect);
	virtual void OnMove ();
	virtual void GetEditBorder (CRect& rectBorder);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnShowEditbox (BOOL bShow);
};

#endif // !defined(AFX_BCGTOOLBARSPINEDITBOXBUTTON_H__C8BA7618_0A7D_4B74_AC8A_3D99977F3A91__INCLUDED_)
