// GCQLDoc.h : interface of the CGCQLDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GCQLDOC_H__5B4DD8ED_720C_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_GCQLDOC_H__5B4DD8ED_720C_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGCQLDoc : public CDocument
{
protected: // create from serialization only
	CGCQLDoc();
	DECLARE_DYNCREATE(CGCQLDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCQLDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void SetTitle(LPCTSTR lpszTitle);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGCQLDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGCQLDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCQLDOC_H__5B4DD8ED_720C_11D5_BA96_00C0DF22DE85__INCLUDED_)
