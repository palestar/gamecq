/*
	ServerList.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef SERVERLIST_H
#define SERVERLIST_H

//---------------------------------------------------------------------------------------------------

#include "GCQ/MetaClient.h"
#include "Standard/Event.h"
#include "Standard/CriticalSection.h"
#include "TFXDataTip.h"
#include "CacheList.h"

class CServerList : public CListView
{
protected:
	DECLARE_DYNCREATE(CServerList)

	CServerList();           // protected constructor used by dynamic creation
	virtual ~CServerList();

	bool						m_bInitialized;
	Array< MetaClient::Server >	m_Servers;
	Array< CCacheList::Program * >
								m_Programs;

	int							m_SortColumn;
	bool						m_SortAscending;
	TFXDataTip					m_Tip;

	UINT						m_RefreshTimer;

	dword						m_nGameIdFilter;
	dword						m_nServerTypeFilter;

// Attributes
public:
	// Accessors
	bool						isServerSelected();
	dword						getGameIdFilter() const;
	dword						getServerTypeFilter() const;

	// Mutators
	void						setGameIdFilter( dword nGameId );
	void						setServerTypeFilter( dword nServerType );

	static CServerList *		getServerList();

private:
	// Mutators
	void						sortServers();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameList)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg void OnGamesJoinDC(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnServerConnect();
	afx_msg void OnUpdateServerConnect(CCmdUI *pCmdUI);
	afx_msg void OnServerRefresh();
	afx_msg void OnServerKick();
	afx_msg void OnUpdateServerKick(CCmdUI *pCmdUI);
	afx_msg void OnServerBan();
	afx_msg void OnUpdateServerBan(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

//---------------------------------------------------------------------------------------------------

inline dword CServerList::getGameIdFilter() const
{
	return m_nGameIdFilter;
}

inline dword CServerList::getServerTypeFilter() const
{
	return m_nServerTypeFilter;
}

inline void CServerList::setGameIdFilter( dword nGameId )
{
	m_nGameIdFilter = nGameId;
}

inline void CServerList::setServerTypeFilter( dword nServerType )
{
	m_nServerTypeFilter = nServerType;
}

//---------------------------------------------------------------------------------------------------


#endif

//---------------------------------------------------------------------------------------------------
//EOF
