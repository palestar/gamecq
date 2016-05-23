#if !defined(AFX_OPTIONSGENERAL_H__F6A70B25_19AA_4A6B_AECF_18D509E14693__INCLUDED_)
#define AFX_OPTIONSGENERAL_H__F6A70B25_19AA_4A6B_AECF_18D509E14693__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsGeneral.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral dialog

class COptionsGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsGeneral)

// Construction
public:
	COptionsGeneral();
	~COptionsGeneral();

// Dialog Data
	//{{AFX_DATA(COptionsGeneral)
	enum { IDD = IDD_OPTIONS_GENERAL };
	BOOL	m_ChatSound;
	BOOL	m_RoomAnnounce;
	BOOL	m_AlwaysLog;
	int		m_Language;
	int		m_GameRefresh;
	int		m_AutoAwayTime;
	int		m_ChatBuffer;
	int		m_TextSize;
	BOOL	m_AutoLogin;
	BOOL	m_bMessageSound;
	BOOL	m_bTaskBarMessages;
	BOOL	m_bMinimized;
	BOOL	m_bAway;
	BOOL	m_bCheckForUpdates;
	BOOL	m_bEnableWordFilter;
	int		m_nCheckUpdateInterval;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsGeneral)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsGeneral)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSGENERAL_H__F6A70B25_19AA_4A6B_AECF_18D509E14693__INCLUDED_)
