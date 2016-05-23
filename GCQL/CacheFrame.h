/*
	CacheFrame.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef CACHEFRAME_H
#define CACHEFRAME_H

//---------------------------------------------------------------------------------------------------

#define CFrameWnd	CBCGFrameWnd

class CCacheFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CCacheFrame)
protected:
	CCacheFrame();           // protected constructor used by dynamic creation
	virtual ~CCacheFrame();

	CBCGToolBar		m_wndCacheBar;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};



#endif

//---------------------------------------------------------------------------------------------------
//EOF
