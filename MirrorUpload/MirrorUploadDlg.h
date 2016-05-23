// MirrorUploadDlg.h : header file
//

#if !defined(AFX_MIRRORUPLOADDLG_H__1CFDA365_5522_484A_9FC6_DC01C6270B34__INCLUDED_)
#define AFX_MIRRORUPLOADDLG_H__1CFDA365_5522_484A_9FC6_DC01C6270B34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UploadThread.h"
#include "Resource.h"



/////////////////////////////////////////////////////////////////////////////
// CMirrorUploadDlg dialog

class CMirrorUploadDlg : public CDialog
{
// Construction
public:
	CMirrorUploadDlg( CWnd* pParent = NULL);	// standard constructor
	~CMirrorUploadDlg();

	struct Profile
	{
		CharString	sName;
		CharString	sLogFile;
		CharString	sPath;
		CharString	sAddress;
		int			nPort;
		CharString	sUID;
		CharString	sPW;

		UploadThread *	pThread;

		Profile & operator=( const Profile & copy )
		{
			sName = copy.sName;
			sLogFile = copy.sLogFile;
			sPath = copy.sPath;
			sAddress = copy.sAddress;
			nPort = copy.nPort;
			sUID = copy.sUID;
			sPW = copy.sPW;
			pThread = copy.pThread;
			return *this;
		}
	};

	Array< Profile >	m_Profiles;

// Dialog Data
	//{{AFX_DATA(CMirrorUploadDlg)
	enum { IDD = IDD_MIRRORUPLOAD_DIALOG };
	CListCtrl	m_cProfileList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorUploadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMirrorUploadDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNewProfile();
	afx_msg void OnDeleteProfile();
	afx_msg void OnUpload();
	afx_msg void OnEditProfile();
	afx_msg void OnListOpen(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenLog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIRRORUPLOADDLG_H__1CFDA365_5522_484A_9FC6_DC01C6270B34__INCLUDED_)
