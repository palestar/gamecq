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

// BCGToolbarComboBoxButton.h: interface for the CBCGToolbarComboBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGToolbarButton.h"

class CBCGToolbarMenuButton;
class CBCGComboEdit;

class BCGCONTROLBARDLLEXPORT CBCGToolbarComboBoxButton : public CBCGToolbarButton  
{
	friend class CBCGComboEdit;

	DECLARE_SERIAL(CBCGToolbarComboBoxButton)

public:
	CBCGToolbarComboBoxButton();
	CBCGToolbarComboBoxButton(UINT uiID, int iImage, DWORD dwStyle = CBS_DROPDOWNLIST, int iWidth = 0);
	virtual ~CBCGToolbarComboBoxButton();

// Operations:
	int AddItem (LPCTSTR lpszItem, DWORD_PTR dwData = 0);
	int GetCount () const;
	LPCTSTR GetItem (int iIndex = -1) const;
	DWORD_PTR GetItemData (int iIndex = -1) const;
	int GetCurSel () const
	{
		return m_iSelIndex;
	}
	
	void RemoveAllItems ();

	BOOL SelectItem (int iIndex, BOOL bNotify = TRUE);
	BOOL SelectItem (DWORD_PTR dwData);
	BOOL SelectItem (LPCTSTR lpszText);

	BOOL DeleteItem (int iIndex);
	BOOL DeleteItem (DWORD_PTR dwData);
	BOOL DeleteItem (LPCTSTR lpszText);

	void SetDropDownHeight (int nHeight);

// Overrides:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, const CRect& rect);
	virtual CBCGComboEdit* CreateEdit (CWnd* pWndParent, const CRect& rect, DWORD dwEditStyle);

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void SerializeContent (CArchive& ar);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnMove ();
	virtual void OnSize (int iSize);
	virtual HWND GetHwnd ()
	{	
		if (m_bFlat && !GetComboBox()->GetDroppedState() && m_pWndEdit != NULL)
		{
			return m_pWndEdit->GetSafeHwnd ();
		}

		return m_pWndCombo->GetSafeHwnd ();
	}

	virtual void EnableWindow (BOOL bEnable = TRUE)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL)
		{
			m_pWndCombo->EnableWindow (bEnable);
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->EnableWindow (bEnable);
		}
	}

	virtual BOOL IsWindowVisible ()
	{
		return ((m_pWndCombo->GetSafeHwnd () != NULL &&
				m_pWndCombo->GetStyle () & WS_VISIBLE) ||
				(m_pWndEdit->GetSafeHwnd () != NULL &&
				m_pWndEdit->GetStyle () & WS_VISIBLE));
	}

	virtual BOOL IsOwnerOf (HWND hwnd)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndCombo->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndEdit->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		return TRUE;
	}

	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void DuplicateData () {}
	virtual void ClearData () {}

	virtual void OnShow (BOOL bShow);
	virtual BOOL ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const;

	virtual void SetStyle (UINT nStyle);

	virtual BOOL IsAutoSynchSelection () const	{	return TRUE;	}

	virtual void OnGlobalFontsChanged();

protected:
	void Initialize ();
	void AdjustRect ();
	void SetHotEdit (BOOL bHot = TRUE);

// Attributes:
public:
	static void SetFlatMode (BOOL bFlat = TRUE)
	{
		m_bFlat = bFlat;
	}

	static BOOL IsFlatMode ()
	{
		return m_bFlat;
	}

	CComboBox* GetComboBox () const
	{
		return m_pWndCombo;
	}

	LPCTSTR GetText () const
	{
		return m_strEdit;
	}

	void SetText (LPCTSTR lpszText);

	BOOL IsSerializeContent () const
	{
		return m_bSerializeContent;
	}

	void SetSerializeContent (BOOL bSet = TRUE)
	{
		m_bSerializeContent = bSet;
	}

	static CBCGToolbarComboBoxButton* GetByCmd (UINT uiCmd, BOOL bIsFocus = FALSE);
	static BOOL SelectItemAll (UINT uiCmd, int iIndex);
	static BOOL SelectItemAll (UINT uiCmd, DWORD_PTR dwData);
	static BOOL SelectItemAll (UINT uiCmd, LPCTSTR lpszText);
	static int GetCountAll (UINT uiCmd);
	static int GetCurSelAll (UINT uiCmd);
	static LPCTSTR GetItemAll (UINT uiCmd, int iIndex = -1);
	static DWORD_PTR GetItemDataAll (UINT uiCmd, int iIndex = -1);
	static void* GetItemDataPtrAll (UINT uiCmd, int iIndex = -1);
	static LPCTSTR GetTextAll (UINT uiCmd);

	virtual BOOL HasFocus () const;

protected:
	DWORD				m_dwStyle;
	CComboBox*			m_pWndCombo;
	CEdit*				m_pWndEdit;

	CStringList			m_lstItems;
	CList<DWORD_PTR, DWORD_PTR>	m_lstItemData;
	int					m_iWidth;
	int					m_iSelIndex;

	BOOL				m_bSerializeContent;

	BOOL				m_bHorz;
	CString				m_strEdit;
	CRect				m_rectCombo;
	CRect				m_rectButton;

	int					m_nDropDownHeight;
	BOOL				m_bIsHotEdit;

	static BOOL			m_bFlat;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGComboEdit

class BCGCONTROLBARDLLEXPORT CBCGComboEdit : public CEdit
{
// Construction
public:
	CBCGComboEdit(CBCGToolbarComboBoxButton& combo);

// Attributes
protected:
	CBCGToolbarComboBoxButton&	m_combo;
	BOOL						m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGComboEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGComboEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGComboEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChange();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)

/////////////////////////////////////////////////////////////////////////////
