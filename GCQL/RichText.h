#if !defined(AFX_RICHTEXT_H__83C087A4_D777_11D4_A298_00C04F6FF8BD__INCLUDED_)
#define AFX_RICHTEXT_H__83C087A4_D777_11D4_A298_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RichText.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRichText view

class CRichText : public CScrollView
{
protected:
	CRichText();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRichText)

	int			m_ContentHeight;
// Attributes
public:
	CString		m_FontName;
	int			m_FontSize;

	COLORREF	m_ForgroundColor;
	COLORREF	m_BackgroundColor;
	bool		m_ScrollToBottom;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRichText)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRichText();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CRichText)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RICHTEXT_H__83C087A4_D777_11D4_A298_00C04F6FF8BD__INCLUDED_)
