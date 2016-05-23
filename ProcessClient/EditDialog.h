#if !defined(AFX_EDITDIALOG_H__E8FF649B_2AD0_493A_8A9D_842F7E57A5D0__INCLUDED_)
#define AFX_EDITDIALOG_H__E8FF649B_2AD0_493A_8A9D_842F7E57A5D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDialog.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CEditDialog dialog

class CEditDialog : public CDialog
{
// Construction
public:
	CEditDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditDialog)
	enum { IDD = IDD_EDIT };
	CEdit	m_TextControl;
	CString	m_Text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDIALOG_H__E8FF649B_2AD0_493A_8A9D_842F7E57A5D0__INCLUDED_)
