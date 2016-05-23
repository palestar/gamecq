#if !defined(AFX_OPTIONSCOLORS_H__1EC8E17F_B61E_4A48_8A13_1D6E23CBCF0E__INCLUDED_)
#define AFX_OPTIONSCOLORS_H__1EC8E17F_B61E_4A48_8A13_1D6E23CBCF0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsColors.h : header file
//

#include "ColorBtn.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsColors dialog

class COptionsColors : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsColors)

// Construction
public:
	COptionsColors();
	~COptionsColors();

	CColorBtn	m_Color1;
	CColorBtn	m_Color2;
	CColorBtn	m_Color3;
	CColorBtn	m_Color4;
	CColorBtn	m_Color5;
	CColorBtn	m_Color6;
	CColorBtn	m_Color7;
	CColorBtn	m_Color8;
	CColorBtn	m_Color9;
	CColorBtn	m_Color10;
	CColorBtn	m_Color11;

// Dialog Data
	//{{AFX_DATA(COptionsColors)
	enum { IDD = IDD_OPTIONS_COLORS };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsColors)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsColors)
	virtual BOOL OnInitDialog();
	afx_msg void OnDefaults();
	afx_msg void OnApplyColors();
	afx_msg void OnLoad();
	afx_msg void OnSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSCOLORS_H__1EC8E17F_B61E_4A48_8A13_1D6E23CBCF0E__INCLUDED_)
