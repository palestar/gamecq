/*
	CacheList.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef CACHELIST_H
#define CACHELIST_H

#include "Standard/String.h"
#include "SelfUpdate/ClientUpdate.h"
#include "tinyxml.h"

//---------------------------------------------------------------------------------------------------

class CCacheList : public CListView
{
	DECLARE_DYNCREATE(CCacheList)

protected:
	CCacheList();           // protected constructor used by dynamic creation
	virtual ~CCacheList();

public:
	// Types
	struct Program
	{
		int				m_nIndex;			// program index
		bool			m_bCanUse;			// does user have rights to see the program
		bool			m_bInstalled;	
		TiXmlElement *	m_pElement;	
		ClientUpdate *	m_pUpdate;			// current downloader object
		dword			m_nLastUpdate;		// time of last update
		bool			m_bLaunch;
		CharString		m_sAddress;
		int				m_nPort;
		void *			m_pProcess;			// the running process
		HICON			m_hIcon;			// icon for this program
		Program *		m_pDepends;			// what program does this program depend on?

		CharString		m_sName;
		CharString		m_sExecutable;
		CharString		m_sDescription;
		CharString		m_sStatus;
		bool			m_bNeedServer;
		bool			m_bTrackProcess;	// do we track the running process and limit only one instance
		CharString		m_sCommandLine;
		CharString		m_sMirror;
		int				m_nDependId;		// Program id of our dependecy
		CharString		m_sDirectory;
		dword			m_nServerType;
		dword			m_nLobbyId;
		dword			m_nUserFlags;
		CharString		m_sCategory;

		Program() : m_nIndex( -1 ), m_bCanUse( true ), m_bInstalled( false ), m_nServerType( 0 ), m_nLobbyId( 0 ), m_nUserFlags( 0 ), 
			m_pElement( NULL ), m_pUpdate( NULL ), m_nLastUpdate( 0 ), m_bLaunch( false ), m_nPort( 0 ), m_pProcess( NULL ),
			m_hIcon( NULL ), m_pDepends( NULL ), m_bNeedServer( false ), m_bTrackProcess( true )
		{};

		Program( const Program & copy ) : m_nIndex( copy.m_nIndex ), m_bInstalled( copy.m_bInstalled ), m_sName( copy.m_sName ), 
			m_sExecutable( copy.m_sExecutable ), m_bCanUse( copy.m_bCanUse ),
			m_sDescription( copy.m_sDescription ), m_sStatus( copy.m_sStatus ), m_sCommandLine( copy.m_sCommandLine ), 
			m_sMirror( copy.m_sMirror ), m_bNeedServer( copy.m_bNeedServer ),
			m_sDirectory( copy.m_sDirectory ),m_nServerType( copy.m_nServerType ),
			m_nLobbyId( copy.m_nLobbyId ), m_nUserFlags( copy.m_nUserFlags ), m_sCategory( copy.m_sCategory ),
			m_pElement( copy.m_pElement ), m_pUpdate( copy.m_pUpdate ), m_nLastUpdate( copy.m_nLastUpdate ),
			m_bLaunch( copy.m_bLaunch ), m_sAddress( copy.m_sAddress ), m_nPort( copy.m_nPort ),
			m_pProcess( copy.m_pProcess ), m_hIcon( copy.m_hIcon ), m_pDepends( copy.m_pDepends ),
			m_bTrackProcess( copy.m_bTrackProcess )
		{};
	};

	// Data
	bool				m_bInitialized;
	CImageList			m_ProgramIcons;
	int					m_SortColumn;
	bool				m_SortAscending;
	TiXmlDocument 		m_CacheConfig;
	CharString			m_sCacheDirectory;			// full path to the cache directory!
	Array< Program * >	m_Programs;

	// Mutators
	bool				initializeCache();
	bool				shutdownCache();

	Program *			findProgram( const char * pName );
	Program *			findProgram( dword nGameId, dword nServerType );

	bool				isInstalled( Program * pProgram );
	bool				isUpdating( Program * pProgram );
	bool				isUpdated( Program * pProgram );
	bool				startUpdate( Program * pProgram );
	bool				deleteFiles( Program * pProgramm );
	bool				stopUpdate( Program * pProgram );

	bool				launchProgram( Program * pProgram, 
							const char * pAddress, 
							unsigned int nPort );
	bool				startProcess( Program * pProgram );

	void				sortPrograms();
	void				updateProgramIcons();

	static CCacheList *	getCacheList();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGamesInstall();
	afx_msg void OnUpdateGamesInstall(CCmdUI *pCmdUI);
	afx_msg void OnGamesLaunch();
	afx_msg void OnUpdateGamesLaunch(CCmdUI *pCmdUI);
	afx_msg void OnGamesDelete();
	afx_msg void OnUpdateGamesDelete(CCmdUI *pCmdUI);
	virtual BOOL DestroyWindow();
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGamesUpdate();
	afx_msg void OnUpdateGamesUpdate(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};



#endif

//---------------------------------------------------------------------------------------------------
//EOF
