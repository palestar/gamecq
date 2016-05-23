#if !defined(AFX_OPTIONS_H__6CBD4D76_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_OPTIONS_H__6CBD4D76_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Options.h : header file
//

#include "OptionsGeneral.h"
#include "OptionsEmots.h"
#include "OptionsColors.h"

/////////////////////////////////////////////////////////////////////////////
// COptions dialog

class COptions : public CPropertySheet
{
// Construction
public:
	COptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptions)
	//}}AFX_DATA


	COptionsGeneral		m_General;
	COptionsEmots		m_Emotions;
	COptionsColors		m_Colors;
	HICON				m_hIcon;

	static void		RestoreDefaultEmotions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptions)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptions)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONS_H__6CBD4D76_7CE8_11D5_BA96_00C0DF22DE85__INCLUDED_)
