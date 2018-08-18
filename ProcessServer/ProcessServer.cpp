/*
	ProcessServer.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/ExceptionHandler.h"
#include "Standard/CommandLine.h"
#include "Standard/Process.h"
#include "Standard/Settings.h"
#include "Standard/Time.h"
#include "File/Path.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "Network/MirrorClient.h"

#include "GCQS/ProcessServer.h"

//----------------------------------------------------------------------------

static bool localUpdate( const char * pDest, const char * pSrc )
{
	bool bRestart = false;

	ASSERT( pDest[ strlen(pDest) - 1 ] == PATH_SEPERATORC );
	ASSERT( pSrc[ strlen(pSrc) - 1 ] == PATH_SEPERATORC );

	Path srcPath( pSrc );
	CharString sMask( srcPath.directory() + "*" );
	FindFile ff( sMask );

	// make sure destination directory exists
	FileDisk::createDirectory( pDest );

	// move files
	for(int i=0;i<ff.fileCount();i++)
	{
		CharString dstFile;
		dstFile.format( "%s%s", pDest, ff.file(i) );
		CharString srcFile;
		srcFile.format( "%s%s", pSrc, ff.file(i) );

		bool copyFile = false;
		bool copyUpdate = false;
		if ( FileDisk::fileExists( dstFile ) )
		{
			// make sure date of src file is newer
			if ( FileDisk::fileDate( srcFile ) > FileDisk::fileDate( dstFile ) )
			{
				copyFile = true;

				// attempt to delete the old file, if it fails then use MirrorUpdate
				if (! FileDisk::deleteFile( dstFile ) )
					copyUpdate = true;
			}
		}
		else
			copyFile = true;

		bRestart |= copyFile;

		if ( copyFile && copyUpdate )
			FileDisk::copyFile( srcFile, dstFile + ".upd" );
		else if ( copyFile )
			FileDisk::copyFile( srcFile, dstFile );
	}

	// recurse into subdirectories
	for(int i=0;i<ff.directoryCount();i++)
	{
		if ( ff.directory(i)[0] == '.' )
			continue;

		CharString newDst;
		newDst.format( "%s%s" PATH_SEPERATOR, pDest, ff.directory( i ) );
		CharString newSrc;
		newSrc.format( "%s%s" PATH_SEPERATOR, pSrc, ff.directory( i ) );

		bRestart |= localUpdate( newDst, newSrc );
	}

	return bRestart;
}

//----------------------------------------------------------------------------

#if defined(_WIN32)

class MyProcessServer : public ProcessServer
{
public:
	// Construction
	MyProcessServer() : m_nCpuIndex( 0 ), m_nCpuUsage( 0 ), m_nMemoryIndex( 0 ), m_nMemoryUsage( 0 ), m_nLastPM( 0 )
	{}

	// ProcessServer interface
	int	cpuUsage()
	{
		return m_nCpuUsage;
	}
	int memoryUsage()
	{
		return m_nMemoryUsage;
	}

	// Mutators
	bool start( const Context & context )
	{
		if (! ProcessServer::start( context ) )
			return false;
		return true;
	}

	void updatePerformanceMonitor()
	{}

	void sendChat( const char * pChat )
	{
		LOG_STATUS( "ProcessServer", "Sending Chat: %s", pChat );
		m_MetaClient.sendChat( 0, pChat );
	}

protected:
	// Data
	int			m_nCpuIndex, m_nCpuUsage;
	int			m_nMemoryIndex, m_nMemoryUsage;
	dword		m_nLastPM;
};

#else

class MyProcessServer : public ProcessServer
{
public:
	// Construction
	MyProcessServer() : m_nCpuUsage( 0 ), m_nMemoryUsage( 0 ), m_nLastTotalCPU( 0 ), m_nLastIdleCPU( 0 ), m_nLastUpdate( 0 )
	{}

	// ProcessServer interface
	int	cpuUsage()
	{
		return m_nCpuUsage;
	}
	int memoryUsage()
	{
		return m_nMemoryUsage;
	}

	// Mutators
	bool start( const Context & context )
	{
		if (! ProcessServer::start( context ) )
			return false;

		return true;
	}

	void updatePerformanceMonitor()
	{
		dword nCurrentTime = Time::milliseconds();
		if ( (nCurrentTime - m_nLastUpdate) < 1000 )
			return;

		dword nElapsed = nCurrentTime - m_nLastUpdate;
		m_nLastUpdate = nCurrentTime;

		FILE * pFile = fopen( "/proc/meminfo", "r" );
		if ( pFile != NULL )
		{
			dword nMemTotal = 0;
			dword nMemFree = 0;

			char sLine[ 512 ];
			while( fgets( sLine, sizeof(sLine), pFile ) )
			{
				if ( strncmp( sLine, "MemTotal:", 9 ) == 0 )
					nMemTotal = strtoul( sLine + 9, NULL, 10 );
				else if ( strncmp( sLine, "MemFree:", 8 ) == 0 )
					nMemFree = strtoul( sLine + 8, NULL, 10 );
				else if ( nMemFree != 0 && nMemTotal != 0 )
					break;		// no need to keep parsing once we have our two numbers
			}

			fclose( pFile );

			dword nMemUsed = nMemTotal - nMemFree;
			if ( nMemTotal > 0 )
				m_nMemoryUsage = (nMemUsed * 100) / nMemTotal;
		}

		pFile = fopen( "/proc/stat", "r" );
		if ( pFile != NULL )
		{
			qword nTotalCPU = 0;
			qword nIdleCPU = 0;

			char sLine[ 512 ];
			while( fgets( sLine, sizeof(sLine), pFile ) )
			{
				if ( strncmp( sLine, "cpu ", 4 ) == 0 )
				{
					qword CPU[8];
					sscanf( sLine, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", 
						&CPU[0],
						&CPU[1],
						&CPU[2],
						&CPU[3],
						&CPU[4],
						&CPU[5],
						&CPU[6],
						&CPU[7] );

					for(int i=0;i<8;++i)
						nTotalCPU += CPU[i];
					nIdleCPU = CPU[3];

					break;
				}
			}

			fclose( pFile );

			qword nTotalDelta = nTotalCPU - m_nLastTotalCPU;
			qword nIdleDelta = nIdleCPU - m_nLastIdleCPU;
			
			if ( nTotalDelta > 0 )
			{
				m_nCpuUsage = (int)((nTotalDelta - nIdleDelta) * 100 / nTotalDelta);
				m_nLastTotalCPU = nTotalCPU;
				m_nLastIdleCPU = nIdleCPU;
			}
		}
	}

	void sendChat( const char * pChat )
	{
		LOG_STATUS( "ProcessServer", "Sending Chat: %s", pChat );
		m_MetaClient.sendChat( 0, pChat );
	}

	int			m_nCpuUsage;
	int			m_nMemoryUsage;
	qword		m_nLastTotalCPU;
	qword		m_nLastIdleCPU;
	dword		m_nLastUpdate;
};

#endif

//----------------------------------------------------------------------------

int main( int argc, char ** argv )
{
	CharString iniFile = "./ProcessServer.ini";
	if ( argc > 1 )
		iniFile = argv[1];

	Settings settings( "ProcessServer", iniFile );
	
	// initialize the logging first thing before we do anything else..
	std::string logFile( settings.get( "logFile", "ProcessServer.log" ) );
	std::string logExclude( settings.get( "logExclude", "" ) );
	unsigned int nMinLogLevel = settings.get( "logLevel", LL_STATUS );
	new FileReactor( logFile, nMinLogLevel, logExclude );

	bool bRemoteUpdate = settings.get( "remoteUpdate", 1 ) != 0;
	bool bLocalUpdate = settings.get( "localUpdate", (dword)0 ) != 0;

	CharString sPath( FileDisk::currentDirectory() );
	if (! sPath.endsWith( PATH_SEPERATOR ) )
		sPath += PATH_SEPERATOR;

#if !defined(_DEBUG)
	// update the files
	if ( settings.get( "doUpdate", 1 ) != 0 )
	{
		settings.put( "doUpdate", (dword)0 );

		if ( bLocalUpdate )
		{
			CharString updatePath = settings.get( "localUpdatePath", "" );
			if ( updatePath.length() > 0 )
			{
				FileDisk::normalizePath( updatePath.buffer() );
				if (! updatePath.endsWith( PATH_SEPERATOR ) )
					updatePath += PATH_SEPERATOR;

				// copy the files from a directory
				if ( localUpdate( sPath, updatePath ) )
					return -3;		// let the service/script update our files..
			}
		}
	}
#endif

	// do the update next time!
	settings.put( "doUpdate", 1 );

	ProcessServer::Context context;
	context.logFile = logFile.c_str();
	context.name = settings.get( "name", "ProcessServer" );
	context.config = iniFile;
	context.gameId = settings.get( "gameId", 1 );
	context.processGroup = settings.get( "processGroup", 1 );
	context.networkGroup = settings.get( "networkGroup", 1 );
	context.metaAddress = settings.get( "metaAddress", "meta-server.palestar.com" );
	context.metaPort = settings.get( "metaPort", 9000 );
	context.uid = settings.get( "uid", "DSS" );
	context.pw = settings.get( "pw", "darkspace" );
	context.address = settings.get( "address", "" );
	context.port = settings.get( "port", 8000 );
	context.maxClients = settings.get( "maxClients", 1000 );
	context.processFile = iniFile;
	context.syncClock = settings.get ("syncClock", (dword)0 ) != 0;

	// start the server
	MyProcessServer theServer;
	if (! theServer.start( context ) )
		return -1;

	// signal that we are running
	Event serverRunning( "ProcessServerRun" );
	serverRunning.signal();

	dword nNextUpdateCheck = Time::seconds() + settings.get("updateTime",300);
	dword nLastCRC = 0;

	// run the server forever, unless it crashes
	Event serverStop( "ProcessServerStop" );
	while( theServer.running() && !theServer.shutdownCompleted() )
	{
		if (! serverStop.wait( 10 ) )
		{
			LOG_STATUS( "ProcessServer", "Recevied shutdown signal." );
			theServer.shutdown();

			serverStop.clear();
		}

		theServer.update();
		theServer.updatePerformanceMonitor();

#if !defined(_DEBUG)
		if ( bRemoteUpdate && nNextUpdateCheck < Time::seconds() )
		{
			LOG_STATUS("ProcessServer", "Checking for remote updates.");

			// check for new code update
			MirrorClient mirrorClient;
			if ( mirrorClient.open( 
				settings.get( "mirrorAddress", "mirror-server.palestar.com" ),
				settings.get( "mirrorPort", 9200 ), sPath, NULL, true ) )
			{
				// attempt to login, ingore if failed
				mirrorClient.login( settings.get( "uid", "" ), settings.get( "pw", "" ) );

				// get the CRC only, only do a sync if remote files have been changed...
				dword nCRC = mirrorClient.getCRC();
				if ( nCRC != nLastCRC )
				{
					nLastCRC = nCRC;

					LOG_STATUS("ProcessServer", "Synchronizing updates.");

					dword nJobID = mirrorClient.syncronize();
					if ( nJobID != 0 && mirrorClient.waitJob( nJobID, 86400 * 1000 ) )
					{
						int nWarningTime = settings.get( "warningTime", 300 );

						LOG_STATUS( "ProcessServer", "Files updated -- Restarting the server in %d seconds.", nWarningTime );

						CharString sWarningMessage = settings.get( "warningMessage", 
							CharString().format("/notice /%s Updating in $T...", context.name.cstr() ) );
						while( nWarningTime > 0 )
						{
							CharString sTimeLeft;
							sTimeLeft.format("%d %s", 
								nWarningTime > 60 ? nWarningTime / 60 : nWarningTime,
								nWarningTime > 60 ? "minute(s)" : "second(s)");

							// replace the "$T" token with the time remaining...
							CharString sChat = sWarningMessage;
							sChat.replace( "$T", sTimeLeft );

							theServer.sendChat( sChat );

							int nSleepTime = 0;
							if ( nWarningTime > (60 * 10) )
								nSleepTime = 60 * 5;			// sleep for 5 minutes
							else
								nSleepTime = 60;				// sleep for 1 minute

							if ( nSleepTime > nWarningTime )
								nSleepTime = nWarningTime;

							nWarningTime -= nSleepTime;

							dword nEndSleep = Time::seconds() + nSleepTime;
							while( Time::seconds() < nEndSleep )
							{
								if (! serverStop.wait( 10 ) )
								{
									LOG_STATUS( "ProcessServer", "Received stop signal, stopping now." );
									nSleepTime = nWarningTime = 0;		// stop now... no warning
									break;
								}

								theServer.update();
								theServer.updatePerformanceMonitor();
							}
						}

						// start the shutdown, server will exit once the last process has stopped..
						theServer.shutdown();
					}
				}
				mirrorClient.close();
			}
			else
			{
				LOG_ERROR( "ProcessServer", "Failed to connect to MirrorServer!" );
			}
		

			nNextUpdateCheck = Time::seconds() + settings.get("updateTime",300);
		}
#endif
	}

	theServer.stop();

	return 0;
}

//----------------------------------------------------------------------------
//EOF
