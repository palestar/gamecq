/*
	ProcessServer.h
	(c)1999 PaleStar Development, Richard Lyle
*/

#ifndef PROCESS_SERVER_H
#define PROCESS_SERVER_H

#include "File/Stream.h"
#include "Network/Server.h"
#include "GCQ/ProcessClient.h"
#include "GCQ/MetaClient.h"
#include "GCQS/GCQDll.h"

class Connection;		// forward declare

//----------------------------------------------------------------------------

class DLL ProcessServer : public Server
{
public:
	// Types
	struct Context
	{
		CharString			logFile;				// server log file
		CharString			name;					// server name
		CharString			config;					// server configuration file
		dword				gameId;					// gameid
		dword				processGroup;			// our process group
		dword				networkGroup;			// our network group
		CharString			metaAddress;			// metaserver address
		int					metaPort;				// port
		CharString			uid,pw;					// username/password
		CharString			address;				// address
		int					port;					// port
		int					maxClients;				// max clients
		CharString			processFile;			// file for saving/loading process information
		bool				syncClock;				// sync system time off the metaserver

		Context &		operator=( const Context & copy );
	};
	typedef ProcessClient::Process		Process;

	// Construction
	ProcessServer();

	// Server interface
	void				onConnect( dword client );							// called when connection made
	void				onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	void				onDisconnect( dword client );						// called when connection lost

	// Accessors
	bool				shutdownCompleted() const;
	int					processCount() const;
	const Process &		process( int n ) const;

	// Mutators
	bool				start( const Context & context );					// run the server
	void				stop();												// stop the server now, this terminates any child processes..
	bool				shutdown();											// shutdown this server nicely
	bool				reboot();											// shutdown this server nicely then reboot the machine

	void				addProcess( Process & proc );						// add process to process list
	bool				removeProcess( dword processId );					// remove process from process list

	virtual int			cpuUsage();											// virtual that should return the percentage of cpu being used
	virtual int			memoryUsage();										// virtual that should return the percentage of memory used

	// Static
	static bool			rebootMachine();									// reboot this machine

protected:
	//! Types
	struct ProcessInfo 
	{
		ProcessInfo() : m_pHandle( NULL ), m_nRestartTime( 0 ), m_nRestarts( 0 ) 
		{}

		void *			m_pHandle;
		dword			m_nRestartTime;
		dword			m_nRestarts;
	};

	// Data
	Context				m_Context;
	CriticalSection		m_LogLock;
	MetaClient			m_MetaClient;
	dword				m_NextProcessId;
	dword				m_NextLogId;
	bool				m_Shutdown;
	bool				m_RebootOnShutdown;
	bool				m_bShutdownCompleted;

	Hash< dword, bool >	
						m_ClientValid;
	Array< Process >	m_ProcessList;
	Hash< dword, ProcessInfo >
						m_ProcessInfo;
	Array< dword >		m_ActiveLog;
	Hash< dword, dword >
						m_LogClient;			// logId -> clientId
	Hash< dword, FileDisk >
						m_LogFile;				// logId -> FileDisk 
	Hash< dword, Array< dword > >
						m_ClientLog;			// clientId -> logId

	// Mutators
	bool				saveProcessList();
	bool				loadProcessList();

	// Helpers
	bool				validateClient( dword client );
	int					findProcess( dword processId );
	bool				findProcess( dword processId, Process & proc );
	bool				stopProcess( dword processId );

	CharString			searchLogFiles( CharString sFilemask, CharString sSearch, bool bRegExp,
							char nSearchLevel, bool bResolveClients );

	void				updateDemon();

	class UpdateThread : public SafeThread	
	{
	public:
		// Construction
						UpdateThread( ProcessServer * pServer );
		// Thread interface
		int				run();
	private:
		ProcessServer *	m_pServer;
	};

	friend class UpdateThread;

};

//----------------------------------------------------------------------------

inline bool ProcessServer::shutdownCompleted() const
{
	return m_bShutdownCompleted;
}

inline ProcessServer::Context & ProcessServer::Context::operator=( const Context & copy )
{
	logFile = copy.logFile;
	name = copy.name;
	config = copy.config;
	gameId = copy.gameId;
	processGroup = copy.processGroup;
	networkGroup = copy.networkGroup;
	metaAddress = copy.metaAddress;
	metaPort = copy.metaPort;
	uid = copy.uid;
	pw = copy.pw;
	address = copy.address;
	port = copy.port;
	maxClients = copy.maxClients;
	processFile = copy.processFile;
	syncClock = copy.syncClock;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

