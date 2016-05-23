// GCQLDoc.cpp : implementation of the CGCQLDoc class
//

#include "stdafx.h"
#include "GCQL.h"
#include "GCQLDoc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGCQLDoc

IMPLEMENT_DYNCREATE(CGCQLDoc, CDocument)

BEGIN_MESSAGE_MAP(CGCQLDoc, CDocument)
	//{{AFX_MSG_MAP(CGCQLDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCQLDoc construction/destruction

CGCQLDoc::CGCQLDoc()
{
	// TODO: add one-time construction code here

}

CGCQLDoc::~CGCQLDoc()
{
}

BOOL CGCQLDoc::OnNewDocument()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGCQLDoc diagnostics

#ifdef _DEBUG
void CGCQLDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGCQLDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGCQLDoc commands

BOOL CGCQLDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	return FALSE;
}


void CGCQLDoc::SetTitle(LPCTSTR lpszTitle) 
{
	CDocument::SetTitle( CString( CGCQLApp::sm_Game.name ) );
}
