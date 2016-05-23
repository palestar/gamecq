#if !defined(AFX_ROOMPASSWORD_H__FDF42A60_644B_490B_ACAD_26838B666140__INCLUDED_)
#define AFX_ROOMPASSWORD_H__FDF42A60_644B_490B_ACAD_26838B666140__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomPassword.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomPassword dialog

class CRoomPassword : public CDialog
{
// Construction
public:
	CRoomPassword(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRoomPassword)
	enum { IDD = IDD_ROOM_PASSWORD };
	CString	m_Password;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomPassword)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoomPassword)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMPASSWORD_H__FDF42A60_644B_490B_ACAD_26838B666140__INCLUDED_)
