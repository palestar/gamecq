// A flicker-free resizing dialog class
// Copyright (c) 1999 Andy Brown <andy@mirage.dabsol.co.uk>
// You may do whatever you like with this file, I just don't care.


#include "stdafx.h"
#include "ResizingDialog.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// used if OEMRESOURCE not defined in precompiled headers

#ifndef OBM_SIZE
#define OBM_SIZE 32766
#endif


/***************/
/* Constructor */
/***************/

CResizingDialog::CResizingDialog(const UINT resID,CWnd *pParent)
: CDialog(resID,pParent)
{
	//{{AFX_DATA_INIT(CResizingDialog)
	//}}AFX_DATA_INIT
	
	// allow all sizing
	
	m_xAllow=sizeResize;
	m_yAllow=sizeResize;
	
	// not initialized yet
	
	m_bInited=false;
}


/***************/
/* Message Map */
/***************/

BEGIN_MESSAGE_MAP(CResizingDialog, CDialog)
//{{AFX_MSG_MAP(CResizingDialog)
ON_WM_SIZE()
ON_WM_GETMINMAXINFO()
ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*****************/
/* WM_INITDIALOG */
/*****************/

BOOL CResizingDialog::OnInitDialog() 
{
	CRect rcIcon,rcDialogClient;
	
	// call the base class
	
	CDialog::OnInitDialog();
	
	// get the dialog size
	
	GetWindowRect(m_rcDialog);
	m_MinSize.x=m_rcDialog.Width();
	m_MinSize.y=m_rcDialog.Height();
	
	// set up the size icon
	
	m_bmSizeIcon.LoadOEMBitmap(OBM_SIZE);
	m_wndSizeIcon.Create(NULL,WS_CHILD | WS_VISIBLE | SS_BITMAP,CRect(0,0,10,10),this,m_idSizeIcon);
	m_wndSizeIcon.SetBitmap(m_bmSizeIcon);
	
	// move the icon to the bottom-right corner
	
	GetClientRect(rcDialogClient);
	m_wndSizeIcon.GetWindowRect(rcIcon);
	ScreenToClient(rcIcon);
	m_wndSizeIcon.SetWindowPos(NULL,rcDialogClient.right-rcIcon.Width(),rcDialogClient.bottom-rcIcon.Height(),0,0,SWP_NOZORDER | SWP_NOSIZE);
	
	// make it auto-position
	
	AddControl(m_idSizeIcon,sizeRepos,sizeRepos);
	
	// all done
	
	m_bInited=true;
	return TRUE;
}


/***********/
/* WM_SIZE */
/***********/

void CResizingDialog::OnSize(UINT nType,int cx,int cy)
{
	CRect rect;
	HDWP hdwp;
	std::vector<CItem>::iterator it;
	
	// call the base class
	
	CDialog::OnSize(nType,cx,cy);
	
	if(m_Items.size())
	{
		// get the new size
		
		GetWindowRect(rect);
		
		// start deferring window pos
		
		hdwp=BeginDeferWindowPos(20);
		
		// children can resize themselves
		
		for(it=m_Items.begin();it!=m_Items.end();it++)
			it->OnSize(hdwp,m_rcDialog,rect,this);
		
		// do the deferred window position change
		
		EndDeferWindowPos(hdwp);
	}
	
	// remember new size
	
	m_rcDialog=rect;
}


/********************/
/* WM_GETMINMAXINFO */
/********************/

void CResizingDialog::OnGetMinMaxInfo(MINMAXINFO *lpMMI)
{
	if(m_bInited)
	{
		lpMMI->ptMinTrackSize=m_MinSize;
		
		if(m_xAllow==sizeNone)
			lpMMI->ptMaxTrackSize.x=lpMMI->ptMaxSize.x=m_MinSize.x;
		
		if(m_yAllow==sizeNone)
			lpMMI->ptMaxTrackSize.y=lpMMI->ptMaxSize.y=m_MinSize.y;
	}
}


/*****************/
/* WM_ERASEBKGND */
/*****************/

BOOL CResizingDialog::OnEraseBkgnd(CDC *pDC)
{
	std::vector<CItem>::const_iterator it;
	
	for(it=m_Items.begin();it!=m_Items.end();it++)
	{
		// skip over the size icon if it's been hidden
		
		if(it->m_resID==m_idSizeIcon && !m_wndSizeIcon.IsWindowVisible())
			continue;
		
		if(it->m_bFlickerFree)
			pDC->ExcludeClipRect(it->m_rcControl);
	}
	
	// call the base class
	
	CDialog::OnEraseBkgnd(pDC);
	return FALSE;
}


/**************************/
/* Add a resizing control */
/**************************/

void CResizingDialog::AddControl(const UINT resID,const eSizeType xsize,const eSizeType ysize,const bool bFlickerFree)
{
	CItem item;
	
	// create the new item
	
	GetDlgItem(resID)->GetWindowRect(item.m_rcControl);
	ScreenToClient(item.m_rcControl);
	
	item.m_bFlickerFree=bFlickerFree;
	item.m_resID=resID;
	item.m_xSize=xsize;
	item.m_ySize=ysize;
	
	if(xsize==sizeRelative)
		item.m_xRelative= item.m_rcControl.left - m_rcDialog.Width()/2;

	if(ysize==sizeRelative)
		item.m_yRelative= item.m_rcControl.top - m_rcDialog.Height()/2;


	// add to the array
	
	m_Items.push_back(item);
}


/**********************/
/* Set allowed sizing */
/**********************/

void CResizingDialog::AllowSizing(const eSizeType xsize,const eSizeType ysize)
{
	m_xAllow=xsize;
	m_yAllow=ysize;
}


/************************/
/* Hide the sizing icon */
/************************/

void CResizingDialog::HideSizeIcon(void)
{
	m_wndSizeIcon.ShowWindow(SW_HIDE);
}


/*********************/
/* CItem constructor */
/*********************/

CResizingDialog::CItem::CItem()
{
}


/**************************/
/* CItem copy constructor */
/**************************/

CResizingDialog::CItem::CItem(const CItem& src)
{
	Assign(src);
}


/*************************/
/* CItem Equals Operator */
/*************************/

CResizingDialog::CItem& CResizingDialog::CItem::operator=(const CItem& src)
{
	Assign(src);
	return *this;
}


/****************/
/* Assign CItem */
/****************/

void CResizingDialog::CItem::Assign(const CItem& src)
{
	m_rcControl=src.m_rcControl;
	m_resID=src.m_resID;
	m_xSize=src.m_xSize;
	m_ySize=src.m_ySize;
	m_bFlickerFree=src.m_bFlickerFree;
	m_xRelative=src.m_xRelative;
	m_yRelative=src.m_yRelative;
}


/****************/
/* CItem OnSize */
/****************/

void CResizingDialog::CItem::OnSize(HDWP hdwp,const CRect& rcParentOld,const CRect& rcParentNew,CWnd *pDlg)
{
	CSize diff;
	CRect rcControl;
	CWnd *pWnd;
	int   newpos,newsize;
	
	// get the control window
	
	pWnd=pDlg->GetDlgItem(m_resID);
	
	// get the size difference
	
	diff.cx=rcParentNew.Width()-rcParentOld.Width();
	diff.cy=rcParentNew.Height()-rcParentOld.Height();
	
	// preset for no change
	
	rcControl=m_rcControl;
	
	// process horizontal option
	
	switch(m_xSize)
	{
    case sizeResize:
		rcControl.right+=diff.cx;
		break;
		
    case sizeRepos:
		rcControl.left+=diff.cx;
		rcControl.right+=diff.cx;
		break;
		
	case sizeRelative:
		newpos=m_xRelative+(rcParentNew.Width()/2);
		newsize=rcControl.Width();
		rcControl.left=newpos;
		rcControl.right=newpos+newsize;
		break;
	}
	
	// process vertical option
	
	switch(m_ySize)
	{
    case sizeResize:
		rcControl.bottom+=diff.cy;
		break;
		
    case sizeRepos:
		rcControl.top+=diff.cy;
		rcControl.bottom+=diff.cy;
		break;
		
    case sizeRelative:
		newpos=m_yRelative+(rcParentNew.Height()/2); // FIX
		newsize=rcControl.Height();
		rcControl.top=newpos;
		rcControl.bottom=newpos+newsize;
		break;
	}
	
	// set new size
	
	if(rcControl!=m_rcControl)
	{
		DeferWindowPos(hdwp,*pWnd,NULL,rcControl.left,rcControl.top,rcControl.Width(),rcControl.Height(),SWP_NOZORDER);
		m_rcControl=rcControl;
	}
}
