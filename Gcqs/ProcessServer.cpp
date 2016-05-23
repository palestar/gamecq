/*
	ProcessServer.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#if defined(WIN32)
#include <windows.h>
#endif

#define GCQS_DLL
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "File/Path.h"
#include "Standard/Process.h"
#include "Standard/Time.h"
#include "Standard/RegExpM.h"
#include "Standard/StringBuffer.h"
#include "Standard/Settings.h"
#include "GCQS/ProcessServer.h"

#define IMPORT_PROCESS_DAT			// define to import old process.dat format

//----------------------------------------------------------------------------

const dword MAX_LOG_SIZE = 32 * 1024;

//----------------------------------------------------------------------------

ProcessServer::ProcessServer() : m_NextProcessId( 1 ), m_NextLogId( 1 ), m_Shutdown( false ), m_RebootOnShutdown( false ), m_bShutdownCompleted( false )
{}

//----------------------------------------------------------------------------

void ProcessServer::onConnect( dword client )
{
	LOG_STATUS( "ProcessServer", CharString().format("Connecting client %u from %s", client, clientAddress( client )) );

	AutoLock lock( &m_Lock );
	m_ClientValid[ client ] = false;
}

void ProcessServer::onReceive( dword client, byte message, const InStream & input )
{
	//LOG_STATUS( "ProcessServer","onReceive, client = %u (%s), message = 0x%x", client, clientAddress(client), message );

	switch( message )
	{
	case ProcessClient::SERVER_LOGIN:
		{
			dword job;
			input >> job;
			CharString uid;
			input >> uid;
			CharString md5;
			input >> md5;

			bool result = false;

			MetaClient::Profile profile;
			if ( m_MetaClient.loginByProxy( uid, md5, profile ) > 0 )
			{
				LOG_STATUS( "ProcessServer", CharString().format("Login %s, client %u (%s)", profile.name.cstr(), client, clientAddress(client)) );
				result = (profile.flags & (MetaClient::ADMINISTRATOR|MetaClient::SERVER)) != 0;
			}
			else
				LOG_STATUS( "ProcessServer", CharString().format("Login failed, client %u (%s)", client, clientAddress(client)) );

			AutoLock lock( &m_Lock );

			m_ClientValid[ client ] = result;
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << result;
		}
		break;
	case ProcessClient::SERVER_SESSION_LOGIN:
		{
			dword job;
			input >> job;
			dword sessionId;
			input >> sessionId;

			bool result = false;

			MetaClient::Profile profile;
			if ( m_MetaClient.loginByProxy( sessionId, profile ) > 0 )
			{
				LOG_STATUS( "ProcessServer", CharString().format("Login %s, client %u (%s)", profile.name.cstr(), client, clientAddress(client)) );
				result = (profile.flags & MetaClient::ADMINISTRATOR) != 0;
			}
			else
				LOG_STATUS( "ProcessServer", CharString().format("Login failed, client %u (%s)", client, clientAddress(client)) );

			AutoLock lock( &m_Lock );

			m_ClientValid[ client ] = result;
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << result;
		}
		break;
	case ProcessClient::SERVER_SEND_PROCESS_LIST:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			// send process list to the server
			AutoLock lock( &m_Lock );
			send( client, ProcessClient::CLIENT_RECV_PROCESS_LIST ) << job << m_ProcessList;
		}
		break;
	case ProcessClient::SERVER_SEND_CONFIG:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			CharString config;
			CharString configFile;

			if ( processId != 0 )
			{
				Process proc;
				if ( findProcess( processId, proc ) )
					configFile = proc.config;
			}
			else
				configFile = m_Context.config;

			LOG_STATUS( "ProcessServer", CharString().format("Send Config, client %u (%s), configFile = %s", 
				client, clientAddress(client), configFile.cstr()) );

			// attempt to load the configuration file
			char * pConfig = FileDisk::loadTextFile( configFile );
			if ( pConfig != NULL )
			{
				config = pConfig;
				delete [] pConfig;
			}

			send( client, ProcessClient::CLIENT_RECV_CONFIG ) << job << config;
		}
		break;
	case ProcessClient::SERVER_RECV_CONFIG:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;
			CharString config;
			input >> config;

			bool jobDone = false;

			CharString configFile;
			if ( processId != 0 )
			{
				Process proc;
				if ( findProcess( processId, proc ) )
					configFile = proc.config;
			}
			else
				configFile = m_Context.config;

			LOG_STATUS( "ProcessServer", "Recv Config, client %u (%s), configFile = %s", 
				client, clientAddress(client), configFile.cstr() );

			// save the new file
			jobDone = FileDisk::saveTextFile( configFile, CharString( config ) );

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_SEND_LOG:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			CharString log;
			CharString logFile;
			if ( processId != 0 )
			{
				// send log of one of our processes
				Process proc;
				if ( findProcess( processId, proc ) )
					logFile = proc.log;
			}
			else
				logFile = m_Context.logFile;

			FileDisk file;
			if ( file.open( logFile ) )
			{
				dword size = file.size();
				if ( size > MAX_LOG_SIZE )
				{
					file.setPosition( size - MAX_LOG_SIZE );
					size = MAX_LOG_SIZE;
				}

				char * pLog = new char[ size + 1];
				pLog[ size ] = 0;

				file.read( pLog, size );
				file.close();

				// save to string
				log = pLog;
				// release allocated memory
				delete [] pLog;
			}
			else
				log = "Failed to open log file!";


			send( client, ProcessClient::CLIENT_RECV_LOG ) << job << log;
		}
		break;
	case ProcessClient::SERVER_ADD_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			Process proc;
			input >> proc;

			AutoLock lock( &m_Lock );

			proc.processId = m_NextProcessId++;
			m_ProcessList.push( proc );

			LOG_STATUS( "ProcessServer", "Add Process, client = %u (%s), processId = %u, name = %s, exec = %s", 
				client, clientAddress(client), proc.processId, proc.name.cstr(), proc.executable.cstr() );

			saveProcessList();
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << true;
		}
		break;
	case ProcessClient::SERVER_SET_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			Process proc;
			input >> proc;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			int pi = findProcess( proc.processId );
			if ( pi >= 0 )
			{
				m_ProcessList[pi] = proc;
				jobDone = true;

				LOG_STATUS( "ProcessServer", "Set Process, client = %u (%s), processId = %u, name = %s, exec = %s", 
					client, clientAddress(client), proc.processId, proc.name.cstr(), proc.executable.cstr() );

				saveProcessList();
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_DEL_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			int pi = findProcess( processId );
			if ( pi >= 0 )
			{
				LOG_STATUS( "ProcessServer", "Delete Process, name = %s, client = %u (%s), processId = %u", 
					m_ProcessList[pi].name.cstr(), client, clientAddress(client), processId );

				// stop the actual process if any
				if ( m_ProcessInfo.find( processId ).valid() )
				{
					::Process::stop( m_ProcessInfo[ processId ].m_pHandle );
					m_ProcessInfo.remove( processId );
				}

				// remove from the list
				m_ProcessList.remove( pi );
				jobDone = true;

				saveProcessList();
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_STOP_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			int pi = findProcess( processId );
			if ( pi >= 0 )
			{
				m_ProcessList[ pi ].flags |= ProcessClient::PF_DISABLED;
				jobDone = true;

				LOG_STATUS( "ProcessServer", "Stop Process, name = %s, client = %u (%s), processId = %u", 
					m_ProcessList[pi].name.cstr(), client, clientAddress(client), processId );

				saveProcessList();
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_START_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			int pi = findProcess( processId );
			if ( pi >= 0 )
			{
				m_ProcessList[ pi ].flags &= ~ProcessClient::PF_DISABLED;
				jobDone = true;

				LOG_STATUS( "ProcessServer", "Start Process, name = %s, client = %u (%s), processId = %u", 
					m_ProcessList[pi].name.cstr(), client, clientAddress(client), processId );
				saveProcessList();
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_RESTART_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			Process proc;
			if ( findProcess( processId, proc ) )
			{
				if ( m_ProcessInfo.find( processId ).valid() )
				{
					jobDone = stopProcess( processId );

					LOG_STATUS( "ProcessServer", "Restart Process, name = %s, client = %u (%s), processId = %u,", 
						proc.name.cstr(), client, clientAddress(client), processId );
				}
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_SEND_STATUS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			AutoLock lock( &m_Lock );

			ProcessClient::Status status;
			status.processGroup = m_Context.processGroup;
			status.networkGroup = m_Context.networkGroup;
			status.cpuUsage = cpuUsage();
			status.memoryUsage = memoryUsage();	
			status.processCount = 0;

			for(int i=0;i<m_ProcessList.size();i++)
				if ( (m_ProcessList[i].flags & ProcessClient::PF_RUNNING) != 0 )
					status.processCount++;

			send( client, ProcessClient::CLIENT_RECV_STATUS ) << job << status;
		}
		break;
	case ProcessClient::SERVER_STOP_ALL:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			bool jobDone = false;

			AutoLock lock( &m_Lock );
			for(int i=0;i<m_ProcessList.size();i++)
				m_ProcessList[i].flags |= ProcessClient::PF_DISABLED;
			saveProcessList();

			jobDone = true;

			LOG_STATUS( "ProcessServer", CharString().format("Stop All, client = %u (%s)", client, clientAddress(client)) );
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_START_ALL:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			bool jobDone = false;

			AutoLock lock( &m_Lock );
			for(int i=0;i<m_ProcessList.size();i++)
				m_ProcessList[i].flags &= ~ProcessClient::PF_DISABLED;
			saveProcessList();

			jobDone = true;
			LOG_STATUS( "ProcessServer", CharString().format("Start All, client = %u (%s)", client, clientAddress(client)) );
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_RESTART_ALL:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			bool jobDone = false;

			AutoLock lock( &m_Lock );
			for(int i=0;i<m_ProcessList.size();i++)
				stopProcess( m_ProcessList[i].processId );

			jobDone = true;
			LOG_STATUS( "ProcessServer", CharString().format("Restart All, client = %u (%s)", client, clientAddress(client)) );
			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_REBOOT:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			bool jobDone = reboot();
			if ( jobDone )
				LOG_STATUS( "ProcessServer", CharString().format("Server Rebooting, client = %u (%s)", client, clientAddress(client)) );

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_EXIT:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;

			// signal all running processes to stop
			bool jobDone = shutdown();
			if ( jobDone )
				LOG_STATUS( "ProcessServer", CharString().format("Server Exiting, client = %u (%s)", client, clientAddress(client)) );

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_TERMINATE_PROCESS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			bool jobDone = false;

			AutoLock lock( &m_Lock );

			Process proc;
			if ( findProcess( processId, proc ) )
			{
				// just terminate the process
				if ( m_ProcessInfo.find( processId ).valid() )
				{
					::Process::stop( m_ProcessInfo[ processId ].m_pHandle );
					jobDone = true;

					LOG_STATUS( "ProcessServer", "Terminated Process, name = %s, client = %u (%s), processId = %u,", 
						proc.name.cstr(), client, clientAddress(client), processId );
				}
			}

			send( client, ProcessClient::CLIENT_JOB_DONE ) << job << jobDone;
		}
		break;
	case ProcessClient::SERVER_OPEN_LOG:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			dword processId;
			input >> processId;

			CharString logFile;
			if ( processId != 0 )
			{
				// send log of one of our processes
				Process proc;
				if ( findProcess( processId, proc ) )
					logFile = proc.log;
			}
			else
				logFile = m_Context.logFile;

			dword logId = m_NextLogId++;

			AutoLock lock( &m_Lock );

			FileDisk & file = m_LogFile[ logId ];
			if ( file.open( logFile ) )
			{
				LOG_STATUS( "ProcessServer", "Open Log, logFile = %s, logId = %u, clientId = %u", logFile.cstr(), logId, client );

				m_ActiveLog.push( logId );
				m_LogClient[ logId ] = client;
				m_ClientLog[ client ].push( logId );
			}
			else
			{
				LOG_STATUS( "ProcessServer", "Open Log Failed, logFile = %s, clientId = %u", logFile.cstr(), client );

				// failed to open file
				m_LogFile.remove( logId );
				// set id to 0 for error
				logId = 0;
			}

			send( client, ProcessClient::CLIENT_RECV_LOG_ID ) << job << logId;
		}
		break;
	case ProcessClient::SERVER_CLOSE_LOG:
		if ( validateClient( client ) )
		{
			dword logId;
			input >> logId;

			AutoLock lock( &m_Lock );

			LOG_STATUS( "ProcessServer", CharString().format("Close Log, logId = %u, client = %u", logId, client) );

			m_ActiveLog.removeSearch( logId );
			m_LogFile.remove( logId );
			m_LogClient.remove( logId );
			m_ClientLog[ client ].removeSearch( logId );
		}
		break;
	case ProcessClient::SERVER_SEARCH_LOGS:
		if ( validateClient( client ) )
		{
			dword job;
			input >> job;
			ProcessClient::SearchLogRequest req;
			input >> req;

			LOG_STATUS( "ProcessServer", CharString().format("Search Log, clientId = %u", client) );

			CharString result;
			if( req.filemask.find('/') >= 0 || req.filemask.find('\\') >= 0 )
			{	// this should never happen, unless the user has a hacked client
				LOG_STATUS( "ProcessServer", CharString().format("Search Log, invalid filemask received from clientId = %u", client) );
				result = "Failed";
			}
			else
			{
				result = searchLogFiles( req.filemask, req.searchString, req.isRegExp, req.searchLevel, req.resolveClientId );
			}

			send( client, ProcessClient::CLIENT_RECV_SEARCHRESULT ) << job << result;
		}
		break;
	case ProcessClient::PING:
		send( client, ProcessClient::PONG );
		break;
	case ProcessClient::PONG:
		break;
	default:
		{
			LOG_ERROR( "ProcessServer", CharString().format("Bad Message, client = %u (%s), message = %d", client, clientAddress(client), message) );
			removeClient( client );
		}
		break;
	}
}

void ProcessServer::onDisconnect( dword client )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "ProcessServer", CharString().format("Disconnecting client %u", client) );

	// remove client from hash table
	m_ClientValid.remove( client );

	// remove any open log files
	Array< dword > & logs = m_ClientLog[ client ];
	for(int i=0;i<logs.size();i++)
	{
		dword logId = logs[ i ];

		LOG_STATUS( "ProcessServer", CharString().format("Close Log, logId = %u, client = %u", logId, client) );

		m_ActiveLog.removeSearch( logId );
		m_LogClient.remove( logId );
		m_LogFile.remove( logId );
	}
	m_ClientLog.remove( client );
}

//----------------------------------------------------------------------------

int ProcessServer::processCount() const
{
	return m_ProcessList.size();
}

const ProcessClient::Process & ProcessServer::process( int n ) const
{
	return m_ProcessList[ n ];
}

//----------------------------------------------------------------------------

bool ProcessServer::start( const Context & context )
{
	m_Context = context;
	m_Shutdown = false;
	m_RebootOnShutdown = false;

	// load process list
	loadProcessList();

	// start the server
	if (! Server::start( new Socket("ZLIB"), m_Context.port, m_Context.maxClients ) )
		return false;

	// start the update thread
	UpdateThread * pThread = new UpdateThread( this );
	pThread->resume();

	return true;
}

void ProcessServer::stop()
{
	// disconnect all clients and stop listening for connections
	Server::stop();

	// save the process configuration
	saveProcessList();
	// close connection to the metaserver
	m_MetaClient.close();

	// stop all running processes
	for(int i=0;i<m_ProcessList.size();i++)
	{
		Process & proc = m_ProcessList[ i ];
		if ( m_ProcessInfo.find( proc.processId).valid() )
			::Process::stop( m_ProcessInfo[ proc.processId ].m_pHandle );
	}
	m_ProcessInfo.release();
}

bool ProcessServer::shutdown()
{
	m_RebootOnShutdown = false;
	m_Shutdown = true;
	return true;
}

bool ProcessServer::reboot()
{
	m_RebootOnShutdown = true;
	m_Shutdown = true;
	return true;
}

void ProcessServer::addProcess( Process & proc )
{
	AutoLock lock( &m_Lock );

	proc.processId = m_NextProcessId++;
	m_ProcessList.push( proc );
	saveProcessList();
}

bool ProcessServer::removeProcess( dword processId )
{
	AutoLock lock( &m_Lock );
	int pi = findProcess( processId );
	if ( pi >= 0 )
	{
		m_ProcessList.remove( pi );
		saveProcessList();
	}

	return pi >= 0;
}

//----------------------------------------------------------------------------

int ProcessServer::cpuUsage()
{
	return 0;
}

int ProcessServer::memoryUsage()
{
	return 0;
}

//----------------------------------------------------------------------------

bool ProcessServer::rebootMachine()
{
#if defined(WIN32)
	HANDLE hToken; 
	if (! OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ) 
		return false;

	// Get the LUID for the shutdown privilege. 
	TOKEN_PRIVILEGES tkp; 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1; // one privilege to set 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

	// Cannot test the return value of AdjustTokenPrivileges. 
	if (GetLastError() != ERROR_SUCCESS) 
		return false;

	// Shut down the system and force all applications to close. 
	if (! ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0) ) 
		return false;

	return true;
#else
	return false;
#endif
}


const int	REGISTER_TIME = 300;
const int	SYNC_CLOCKS_TIME = 86400;		// sync the clocks every 24 hours

void ProcessServer::updateDemon()
{
	dword lastRegister = 0;
	dword lastTimeSync = 0;
	dword pingTime = 0;

	while ( running() && !m_bShutdownCompleted )
	{
		Thread::sleep( 1000 );

		if (! m_MetaClient.loggedIn() )
		{
			LOG_STATUS( "ProcessServer", "Establishing connection to the metaserver" );

			// attempt to connect
			if ( m_MetaClient.open( m_Context.metaAddress, m_Context.metaPort ) > 0 )
			{
				if ( m_MetaClient.login( m_Context.uid, m_Context.pw ) < 0 )
				{
					LOG_STATUS( "ProcessServer", "Failed to login to the metaserver" );
					m_MetaClient.close();	// failed to login
				}
				else
				{
					LOG_STATUS( "ProcessServer", "Connected to the metaserver" );
					// select the correct game
					m_MetaClient.selectGame( m_Context.gameId );
				}
			}
		}
		
		m_MetaClient.update();
		if ( m_MetaClient.loggedIn() )
		{
			if ( lastRegister < (Time::seconds() - REGISTER_TIME) )
			{
				AutoLock lock( &m_Lock );

				MetaClient::Server server;
				server.gameId = m_Context.gameId;
				server.type = MetaClient::PROCESS_SERVER;
				server.flags = 0;
				server.name = m_Context.name;
				server.shortDescription.format( "CPU: %d%%, MEM: %d%%", cpuUsage(), memoryUsage() );

				server.description = server.shortDescription + "\n\n";
				for(int i=0;i<m_ProcessList.size();i++)
				{
					Process & proc = m_ProcessList[ i ];
					if ( (proc.flags & ProcessClient::PF_RUNNING) == 0 )
						continue;
					server.description += CharString().format( "%s\n", proc.name.cstr() );
				}

				server.address = m_Context.address;
				server.port = m_Context.port;
				server.maxClients = m_Context.maxClients;
				server.clients = clientCount();
				server.data = CharString().format("processGroup=%u;networkGroup=%u;processCount=%u;cpuUsage=%d;memoryUsage=%d", 
					m_Context.processGroup, m_Context.networkGroup, m_ProcessList.size(), cpuUsage(), memoryUsage() );

				lock.release();

				if ( m_MetaClient.registerServer( server ) < 0 )
					lastRegister = (Time::seconds() - REGISTER_TIME) + 30;		// failed, try again in 30 seconds
				else
					lastRegister = Time::seconds();
			}

			if ( m_Context.syncClock && lastTimeSync < (Time::seconds() - SYNC_CLOCKS_TIME) )
			{
				// get the current time from the metaserver
				dword currentTime = m_MetaClient.getTime();
				if ( currentTime > 0 )
				{
					LOG_STATUS( "ProcessServer", "Syncronizing system time to %s", Time::format( currentTime, "%c" ).cstr() );

					if (! Time::setTime( currentTime ) )
						LOG_STATUS( "ProcessServer", "Failed to set the system time!" );

					lastTimeSync = Time::seconds();
				}
				else
				{
					LOG_STATUS( "ProcessServer", "Failed to syncronize system time from MetaServer!" );
					lastTimeSync = (Time::seconds() - SYNC_CLOCKS_TIME) + 300;		// try again in another 5 min
				}
			}
		}

		AutoLock lock( &m_Lock );

		bool shutdownComplete = true;			// have all child processes stopped

		// check for log updates
		for(int i=0;i<m_ActiveLog.size();i++)
		{
			dword logId = m_ActiveLog[ i ];

			FileDisk & file = m_LogFile[ logId ];

			try {
				if ( file.position() > file.size() )
					file.setPosition( 0 );		// file has been rotated... reset read position

				dword read = file.size() - file.position();
				if ( read > 0 )
				{
					if ( read > MAX_LOG_SIZE )
					{
						file.setPosition( file.size() - MAX_LOG_SIZE );
						read = MAX_LOG_SIZE;
					}
				
					char * pLines = new char[ read + 1 ];
					pLines[ read ] = 0;

					file.read( pLines, read );

					send( m_LogClient[ logId ], ProcessClient::CLIENT_RECV_LOG_UPDATE ) << logId << CharString(pLines);

					delete [] pLines;
				}
			}
			catch( FileDisk::FileError )
			{
				LOG_STATUS( "ProcessServer", CharString().format("Log Read Error, logFile = %s, logId = %u", file.fileName(), logId) );

				// close the previously open file
				file.close();
				// attempt to reopen the file
				file.open( file.fileName() );
			}
		}

		// check process list
		for(int i=0;i<m_ProcessList.size();i++)
		{
			Process & proc = m_ProcessList[ i ];
			if ( m_ProcessInfo.find( proc.processId ).valid() )
			{
				ProcessInfo & info = m_ProcessInfo[ proc.processId ];

				proc.flags |= ProcessClient::PF_RUNNING;
				if ( (proc.flags & ProcessClient::PF_DISABLED) == 0 )
				{
					// make sure the process is still running
					if (! ::Process::active( info.m_pHandle ) )
					{
						proc.flags &= ~ProcessClient::PF_RUNNING;

						if ( ! m_Shutdown )
						{
							if ( info.m_nRestartTime != 0 && Time::seconds() < info.m_nRestartTime )
								continue;		// not time to restart yet, continue onto the next process..

							// process has exited, increment the number of restarts, then calculate the next restart
							// time increasing the wait time each time the process exits
							info.m_nRestarts += 1;
							info.m_nRestartTime = Time::seconds() + (60 * (info.m_nRestarts * info.m_nRestarts) );

							int exitCode = ::Process::exitCode( info.m_pHandle );
							::Process::close( info.m_pHandle );

							CharString message;
							message.format( "Process %u exit, name = %s, exec = %s, arg = %s, exitCode = %d, restarts = %u", 
								proc.processId, proc.name.cstr(), proc.executable.cstr(), proc.arguments.cstr(), exitCode, info.m_nRestarts );
							LOG_STATUS( "ProcessServer", message );


							// send report if exit code is negative
							if ( exitCode < 0 )
								m_MetaClient.sendChat( 0, CharString().format("/report %s", message.cstr()) );

							// restart the process..
							Path exePath( proc.executable );
							void * pStart = ::Process::start( CharString().format("%s %s", exePath.file().cstr(), proc.arguments.cstr()), 
								exePath.directory() );
							if ( pStart == NULL )
							{
								LOG_STATUS( "ProcessServer", "Process %u failed to restart", proc.processId );

								m_ProcessInfo.remove( proc.processId );
								proc.flags |= ProcessClient::PF_DISABLED;
							}
							else
							{
								LOG_STATUS( "ProcessServer", "Process %u restarted", proc.processId );
								info.m_pHandle = pStart;
							}
						}
						else
						{
							int exitCode = ::Process::exitCode( info.m_pHandle );

							LOG_STATUS( "ProcessServer", "Process Stopped, name = %s, exitCode = %d", proc.name.cstr(), exitCode );
							::Process::close( info.m_pHandle );

							m_ProcessInfo.remove( proc.processId );
						}
						
					}
					else if ( m_Shutdown )
					{
						shutdownComplete = false;	
						stopProcess( proc.processId );
					}
					else if ( info.m_nRestartTime != 0 && Time::seconds() > info.m_nRestartTime )
					{
						// process has been running long enough to clear the restart time.
						info.m_nRestartTime = 0;
						info.m_nRestarts = 0;
					}
				}
				else
				{
					if (! ::Process::active( info.m_pHandle ) )
					{
						LOG_STATUS( "ProcessServer", "Process Stopped, name = %s", proc.name.cstr() );
						::Process::close( info.m_pHandle );

						m_ProcessInfo.remove( proc.processId );
					}
					else
						stopProcess( proc.processId );
				}
			}
			else
			{
				proc.flags &= ~ProcessClient::PF_RUNNING;
				if ( (proc.flags & ProcessClient::PF_DISABLED) == 0 && !m_Shutdown )
				{
					LOG_STATUS( "ProcessServer", "Starting Process %u, name = %s, exec = %s, arg = %s", 
						proc.processId, proc.name.cstr(), proc.executable.cstr(), proc.arguments.cstr() );

					Path exePath( proc.executable );
					void * pStart = ::Process::start( CharString().format("%s %s", exePath.file().cstr(), proc.arguments.cstr()),
						exePath.directory() );
					if ( pStart == NULL )
					{
						proc.flags |= ProcessClient::PF_DISABLED;

						LOG_STATUS( "ProcessServer", "Process %u Failed to Start, name = %s, exec = %s", 
							proc.processId, proc.name.cstr(), proc.executable.cstr() );
					}
					else
						m_ProcessInfo[ proc.processId ].m_pHandle = pStart;
				}
			}
		}

		pingTime++;
		if ( pingTime > 15 )
		{
			// ping all clients
			for(int i=0;i<clientCount();i++)
				send( client(i), ProcessClient::PING );
			
			pingTime = 0;
		}

		// check for shutdown
		if ( m_Shutdown && shutdownComplete )
		{
			m_bShutdownCompleted = true;

			if ( m_RebootOnShutdown )
				rebootMachine();
		}

		lock.release();
	}
}

//----------------------------------------------------------------------------

bool ProcessServer::saveProcessList()
{
	AutoLock lock( &m_Lock );

	// load the process configuration
	Settings pf( "ProcessList", m_Context.processFile );

	pf.put( "NextProcessId", m_NextProcessId );
	pf.put( "ProcessCount", m_ProcessList.size() );
	for(int i=0;i<m_ProcessList.size();i++)
	{
		Process & process = m_ProcessList[ i ];
		pf.put( CharString().format("ProcessId%d", i), process.processId );
		pf.put( CharString().format("ProcessName%d", i), process.name );
		pf.put( CharString().format("ProcessExe%d", i), process.executable );
		pf.put( CharString().format("ProcessArg%d", i), process.arguments );
		pf.put( CharString().format("ProcessConfig%d", i), process.config );
		pf.put( CharString().format("ProcessLog%d", i), process.log );
		pf.put( CharString().format("ProcessFlags%d", i ), process.flags);
	}

	return true;
}

bool ProcessServer::loadProcessList()
{
	AutoLock lock( &m_Lock );

	m_ProcessList.release();

	Settings pf( "ProcessList", m_Context.processFile );
	m_NextProcessId = pf.get( "NextProcessId", m_NextProcessId );

	int nProcessCount = pf.get( "ProcessCount", (dword)0 );
	for(int i=0;i<nProcessCount;i++)
	{
		Process process;
		process.processId = pf.get( CharString().format("ProcessId%d", i), i + 1 );
		process.name = pf.get( CharString().format("ProcessName%d", i), "" );
		process.executable = pf.get( CharString().format("ProcessExe%d", i), "" );
		process.arguments = pf.get( CharString().format("ProcessArg%d", i), "" );
		process.config = pf.get( CharString().format("ProcessConfig%d", i), "" );
		process.log = pf.get( CharString().format("ProcessLog%d", i), "" );
		
		process.flags = (u16)pf.get( CharString().format("ProcessFlags%d", i), 0xffff );
		if ( process.flags == 0xffff )
		{
			process.flags = 0;
			if ( pf.get( CharString().format("ProcessRunning%d", i), (dword)0 ) != 0 )
				process.flags |= ProcessClient::PF_RUNNING;
			if ( pf.get( CharString().format("ProcessDisabled%d", i), (dword)1 ) != 0 )
				process.flags |= ProcessClient::PF_DISABLED;
		}

		m_ProcessList.push( process );
	}

	return true;
}

//----------------------------------------------------------------------------

bool ProcessServer::validateClient( dword client )
{
	AutoLock lock( &m_Lock );
	Hash<dword,bool>::Iterator find = m_ClientValid.find( client );
	if ( find.valid() && *find )
		return true;

	LOG_STATUS( "ProcessServer", CharString().format("Failed validation, client = %d (%s)", client, clientAddress(client)) );
	removeClient( client );

	return false;
}

int ProcessServer::findProcess( dword processId ) 
{
	AutoLock lock( &m_Lock );

	for(int i=0;i<m_ProcessList.size();i++)
		if ( m_ProcessList[i].processId == processId )
			return i;

	return -1;
}

bool ProcessServer::findProcess( dword processId, Process & proc ) 
{
	AutoLock lock( &m_Lock );

	for(int i=0;i<m_ProcessList.size();i++)
		if ( m_ProcessList[i].processId == processId )
		{
			proc = m_ProcessList[i];
			return true;
		}

	return false;
}

bool ProcessServer::stopProcess( dword processId )
{
	AutoLock lock( &m_Lock );

	if ( m_ProcessInfo.find( processId ).valid() )
	{
		ProcessInfo & info = m_ProcessInfo[ processId ];

		Event stopProcess( CharString().format("StopProcess%u", ::Process::getProcessId( info.m_pHandle )) );
		stopProcess.signal();

		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

ProcessServer::UpdateThread::UpdateThread( ProcessServer * pServer ) 
	: m_pServer( pServer )
{}

int ProcessServer::UpdateThread::run()
{
	m_pServer->updateDemon();
	delete this;
	return 0;
}

inline int findMatchingLine( CharString & sText, int nPosition, CharString & sLine )
{
	int startPos = sText.reverseFind('\n',nPosition);	// left linebreak
	if( startPos > 0 )
		startPos++;		// Skip linebreak

	int endPos = sText.find( '\n', nPosition );				// right linebreak
	if( endPos < 0 )
		endPos = startPos+sText.length() - 1;

	sLine = sText.copy(sText);
	sLine.mid( startPos, ( endPos - startPos ) + 1 );

	return endPos + 1;
}

inline int findString( CharString & sText, int nCurPos, const CharString & sSearch, RegExpM * cr, bool bUseCr )
{
	if( bUseCr )
		return cr->regFind( sText.buffer() + nCurPos );
	else
	{
		int nAbsolutePos = sText.find( sSearch, nCurPos);
		if( nAbsolutePos > 0 )
			nAbsolutePos -= nCurPos;	// absolute -> relative

		return nAbsolutePos;
	}
}
//----------------------------------------------------------------------------
CharString ProcessServer::searchLogFiles( CharString sFilemask, CharString sSearch, bool bRegExp, char nSearchLevel,
									 bool bResolveClients )
{
	CharString sResult;
	
	RegExpM rxSearch;
	RegExpM rxClientId, rxLogin;
	Tree< unsigned int, String > trClientLookup;

	sSearch.lower();	// all search is performed case insensitive

	if( bRegExp )
		rxSearch.regComp( sSearch );

	if( bResolveClients )
		rxClientId.regComp( STR("[Cc]lient ([0-9]+)[, ]") );
	

	FindFile ff;
	CharString sMask = CharString().format("./Logs/%s.log",sFilemask.cstr());
	ff.findFiles( sMask, false, false );	// sorted by date

	int nTotalFoundSize = 0;
	int nTotalSearchedSize = 0;
	int nTotalFilesSearched = 0;
	CharString sLine;
	dword tStart = Time::seconds();
	bool bAbort = false;

	// search through all found files
	for( int i = 0 ; i < ff.fileCount() && !bAbort ; i++ )
	{
		CharString sLocalFile( ff.file( i ) );

		if( nSearchLevel == 2 )		// only list the files, not search them ?
		{
			CharString sFile = CharString().format("./Logs/%s", sLocalFile.cstr()) ;
			sResult += String().format("*** [%s] *** %s\r\n", Time::format( FileDisk::fileDate( sFile ),"%c").cstr(), sLocalFile.cstr() );

			continue;
		}
		
		bool fileNamePrintedYet = false;
		CharString sText;
		CharString sTextOrig;
		int nCurPos = 0;
		try {
			CharString sFile = CharString().format("./Logs/%s",sLocalFile.cstr() );
			char * pTemp = FileDisk::loadTextFile( sFile );
			sText.copy( pTemp ).lower();	// pText is all lowercase, as it's used for case insensitive search
			sTextOrig = CharString().format("%s", pTemp );		// pTextOrig is orig-case, as it's used to build sResult
			delete pTemp;

			int nTextLen = sText.length();
			nTotalSearchedSize += nTextLen;
			nTotalFilesSearched++;

			int pos = -1;
			// loop to find all occurences of the string
			while( nCurPos < nTextLen && ( pos = findString( sText, nCurPos, sSearch, &rxSearch, bRegExp )  ) >= 0 )
			{
				if( !fileNamePrintedYet )
				{
					if( nSearchLevel == 0 )
						sResult += STR("\r\n\r\n");

					CharString sFile = CharString().format("./Logs/%s", sLocalFile.cstr()) ;
					sResult += String().format("*** [%s] *** %s\r\n", Time::format( FileDisk::fileDate( sFile ),"%c").cstr(), sLocalFile.cstr() );

					fileNamePrintedYet = true;
				}
				
				if( nSearchLevel == 1 )		// only check if there is a match in the file at all ?
					break;
				
				// find the matching line, this time in the original-case text
				nCurPos = findMatchingLine( sTextOrig, nCurPos + pos, sLine );	
				
				// need to resolve clientIds when found ?
				if( bResolveClients )
				{
					int nClientIdPos = rxClientId.regFind( sLine );
					if( nClientIdPos >= 0 )		// line holds a clientId ?
					{
						int nClientIdLen = rxClientId.getFindLen() - 8;	// match length - static chars

						// extract the clientId
						CharString sClientId = sLine;
						sClientId.right( sClientId.length() - ( nClientIdPos + 7 ) );
						sClientId.left( nClientIdLen );
						
						int nClientId = CharString::strint(sClientId);
						CharString sResolvedClient(STR(""));
						if( trClientLookup.find( nClientId ).valid() )	// know this clientId already ?
						{
							sResolvedClient = trClientLookup[ nClientId ];
						}
						else
						{
							// haven't seen it yet. Look it up.
							rxLogin.regComp( String().format( STR("\n../../.. ..:..:.. : Login client %d user ([^,\n]+), userId = ([0-9]+), "), nClientId ) );
							if( rxLogin.regFind( sTextOrig ) >= 0 )
							{
								sResolvedClient = rxLogin.getReplaceString( STR(" ( \\1 @\\2 )") );
							}
							else
							{
								// not found yet ? Try alternate way to resolve it
								rxLogin.regComp( String().format( STR("\n../../.. ..:..:.. : Client %d login, id = ([^\r]+)"), nClientId ) );
								if( rxLogin.regFind( sTextOrig ) >= 0 )
								{
									sResolvedClient = rxLogin.getReplaceString( STR(" ( \\1 )") );
								}	// no further "else". If it's not found the result is left blank
							}	

							trClientLookup[ nClientId ] = sResolvedClient;
						}

						if( sResolvedClient != "" )		// got something to insert ?
							sLine.insert( sResolvedClient, nClientIdPos + nClientIdLen + 7 );
					}
				}	// done resolving clientId

				// add the found line to the result
				nTotalFoundSize += sLine.length();
				sResult += sLine;
				
				// abort conditions
				if( nTotalFoundSize > 300000 )
				{
					sResult += STR("\r\n\r\n*** Result too large, search aborted");
					bAbort = true;
					break;
				}

			}
		}
		catch( ... )
		{
			return STR("Failed");
		}
	}

	if( sResult.length() == 0 )
		sResult = STR("No results");
	else
		sResult += STR("\r\nDone...");
	
	sResult += CharString().format( STR("\r\nSearched %d file(s) containing %d bytes within %d seconds. Result is %d bytes."),
		nTotalFilesSearched, nTotalSearchedSize, Time::seconds() - tStart, nTotalFoundSize );
	
	return sResult;
}

//----------------------------------------------------------------------------
//EOF

