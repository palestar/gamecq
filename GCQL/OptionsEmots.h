#if !defined(AFX_OPTIONSEMOTS_H__E054BA71_55FC_4A90_A75F_CFF67B393C8C__INCLUDED_)
#define AFX_OPTIONSEMOTS_H__E054BA71_55FC_4A90_A75F_CFF67B393C8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsEmots.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsEmots dialog

class COptionsEmots : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsEmots)

// Construction
public:
	COptionsEmots();
	~COptionsEmots();

// Dialog Data
	//{{AFX_DATA(COptionsEmots)
	enum { IDD = IDD_OPTIONS_EMOTS };
	CListCtrl	m_EmotionList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsEmots)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsEmots)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditEmotion(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddEmotion();
	afx_msg void OnRemoveEmotion();
	afx_msg void OnDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSEMOTS_H__E054BA71_55FC_4A90_A75F_CFF67B393C8C__INCLUDED_)
