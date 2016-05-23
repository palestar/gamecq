/*
	CacheFrame.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "stdafx.h"
#include "GCQL.h"
#include "CacheFrame.h"
#include "CacheList.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CCacheFrame, CFrameWnd)

CCacheFrame::CCacheFrame()
{}

CCacheFrame::~CCacheFrame()
{}


BEGIN_MESSAGE_MAP(CCacheFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()


int CCacheFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndCacheBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1) ) ||
		!m_wndCacheBar.LoadToolBar(IDR_CACHEBAR, 0, 0, true))
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndCacheBar.SetWindowText(_T("Navgiation"));
	m_wndCacheBar.EnableTextLabels();

	return 0;
}

BOOL CCacheFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	pContext->m_pNewViewClass = RUNTIME_CLASS(CCacheList);
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}


