// MirrorServer.cpp : Defines the entry point for the application.
//

#include "Debug/Assert.h"
#include "Debug/ExceptionHandler.h"
#include "Standard/CommandLine.h"
#include "Standard/Time.h"
#include "Standard/Settings.h"
#include "Standard/Process.h"
#include "Network/MirrorServer.h"
#include "GCQ/MetaClient.h"

//----------------------------------------------------------------------------

class MirrorServerConcrete : public MirrorServer
{
public:
	// Mutators
	bool start( const char * pConfig )
	{ 
		Settings settings( "MirrorServer", pConfig );

		// initialize the logging first thing before we do anything else..
		std::string logFile( settings.get( "logFile", "MirrorServer.log" ) );
		std::string logExclude( settings.get( "logExclude", "" ) );
		unsigned int nLogLevel = settings.get( "logLevel", LL_STATUS );
		new FileReactor( logFile, nLogLevel, logExclude );

		m_GameID = settings.get( "gameId", (dword)1 );
		m_MetaAddress = settings.get( "metaAddress", "meta-server.palestar.com" );
		m_MetaPort = settings.get( "metaPort", 9000 );
		m_UID = settings.get( "uid", "");
		m_PW = settings.get( "pw", "" );
		m_DownloadFlags = settings.get( "downloadFlags", (dword)0 );
		m_UploadFlags = settings.get( "uploadFlags", 0xffffffff );
		m_RegisterServer = settings.get( "registerServer", 1 ) != 0;

		m_ClientFlags.release();
		m_NextUpdate = 0;

		MirrorServer::Context context;
		context.sCatalog = settings.get( "catalog", "" );
		context.mirror = settings.get( "mirror", ".\\Mirror\\" );
		context.address = settings.get( "address", "" );
		context.port = settings.get( "port", 9001  );
		context.maxClients = settings.get( "maxClients", 1000 );
		context.bVersionControl = settings.get( "VersionControl", (dword)0 ) != 0;
		context.nMinUpdateTime = settings.get( "minUpdateTime", 120 );
		context.sUID = m_UID;
		context.sPW = m_PW;

		// set the log file now, so any errors in the configuration will be logged...
		m_Context.logFile = context.logFile;

		int nLinkCount = settings.get("LinkCount", (dword)0);
		for(int i=0;i<nLinkCount;i++)
		{
			CharString sLink;
			sLink.format( "Link%d", i );

			MirrorServer::Link link;
			link.sCatalog = settings.get( sLink + "Catalog", context.sCatalog + "." + sLink );
			link.sAddress = settings.get( sLink + "Address", "" );
			link.nPort = settings.get( sLink + "Port", (dword)0 );
			link.bUpdate = true;

			if ( link.sAddress.length() > 0 && link.nPort != 0 )
			{
				LOG_STATUS( "MirrorServer", "Link %d configured for %s:%d...", i, link.sAddress.cstr(), link.nPort );
				context.links.push( link );
			}
			else
				LOG_STATUS( "MirrorServer", CharString().format("Failed to find link %d configuration!", i) );
		}

		return MirrorServer::start( context );
	}

	// MirrorServer interface
	virtual void onUpdate()
	{
		// update meta server
		m_MetaClient.update();

		AutoLock lock( &m_Lock );
		if ( m_UID.length() > 0 && m_NextUpdate < Time::seconds() )
		{
			if (! m_MetaClient.loggedIn() )
			{
				LOG_STATUS( "MirrorServer", "Connecting to MetaServer, address = %s, port = %d", m_MetaAddress.cstr(), m_MetaPort );
				if ( m_MetaClient.open( m_MetaAddress, m_MetaPort ) > 0 && m_MetaClient.login( m_UID, m_PW ) > 0 )
				{
					LOG_STATUS( "MirrorServer", "Connected to meta server..." );
				}
				else
					LOG_STATUS( "MirrorServer", "Failed to login to meta server..." );
			}


			if ( m_RegisterServer && m_MetaClient.loggedIn() )
			{
				// select the correct game
				m_MetaClient.selectGame( m_GameID );

				// register this server
				MetaClient::Server server;
				server.gameId = m_GameID;
				server.type = MetaClient::MIRROR_SERVER;
				server.flags = 0;
				server.name = CharString().format("%s:%d", context().address.cstr(), context().port);
				server.description = CharString().format("MirrorServer2, mirror = %s, logFile = %s", 
					context().mirror.cstr(), context().logFile.cstr() );
				server.address = context().address;
				server.port = context().port;
				server.maxClients = context().maxClients;
				server.clients = clientCount();

				m_MetaClient.registerServer( server );
			}
			else if ( m_MetaClient.loggedIn() )
			{
				// select the correct game
				m_MetaClient.selectGame( m_GameID );

				// keep the connection alive 
				CharString address;
				m_MetaClient.getAddress( address );
			}

			// register every 5 minutes
			if ( m_MetaClient.loggedIn() )
				m_NextUpdate = Time::seconds() + (60 * 5);
			else
				m_NextUpdate = Time::seconds() + 30;
		}
	}

	bool login( dword client, const char * pUser, const char * pMD5 )
	{
		// force a reconnect to the main server next update
		if (! m_MetaClient.loggedIn() )
		{
			m_NextUpdate = 0;
			onUpdate();
		}

		MetaClient::Profile profile;
		if ( m_MetaClient.loginByProxy( pUser, pMD5, profile ) > 0 )
		{
			m_ClientFlags[ client ] = profile.flags;
			return true;
		}

		// force a reconnect to the main server now
		return false;
	}
	bool login( dword client, dword nSID )
	{
		// force a reconnect to the main server next update
		if (! m_MetaClient.loggedIn() )
		{
			m_NextUpdate = 0;
			onUpdate();
		}

		MetaClient::Profile profile;
		if ( m_MetaClient.loginByProxy( nSID, profile ) > 0 )
		{
			m_ClientFlags[ client ] = profile.flags;
			return true;
		}

		return false;
	}
	void logout( dword client )
	{
		// remove the client from the hash table
		m_ClientFlags.remove( client );
	}

	bool canDownload( dword client )
	{
		if ( m_DownloadFlags == 0 )
			return true;
		if ( m_ClientFlags.find( client ).valid() )
			return (m_ClientFlags[ client ] & m_DownloadFlags) == m_DownloadFlags;
		return false;
	}

	bool canUpload( dword client )
	{
		if ( m_UploadFlags == 0 )
			return true;
		if ( m_ClientFlags.find( client ).valid() )
			return (m_ClientFlags[ client ] & m_UploadFlags) == m_UploadFlags;
		return false;
	}
	bool canLock( dword nClientId )
	{
		return canUpload( nClientId );
	}
	bool canDelete( dword nClientId )
	{
		return canUpload( nClientId );
	}
	bool canLabel( dword nClientId )
	{
		return canUpload( nClientId );
	}
	bool canRollback( dword nClientId )
	{
		return canUpload( nClientId );
	}

private:
	// Data
	dword		m_GameID;
	CharString	m_MetaAddress;
	int			m_MetaPort;
	CharString	m_UID;
	CharString	m_PW;

	dword		m_DownloadFlags;
	dword		m_UploadFlags;
	bool		m_RegisterServer;
	MetaClient	m_MetaClient;

	Hash< dword, dword >
				m_ClientFlags;
	dword		m_NextUpdate;
};

//----------------------------------------------------------------------------

#pragma warning( disable: 4800 )

int main( int argc, char ** argv )
{
	if ( argc < 2 )
	{
		printf( "Usage: %s <iniFile>", argv[0] );
		return 1;
	}

	Event serverStop( CharString().format("StopProcess%u", Process::getCurrentProcessId()) );

	// start the server
	MirrorServerConcrete theServer;
	if (! theServer.start( argv[1] ) )
		return -1;

	// run the server forever, unless it crashes
	while( theServer.running() )
	{
		if (! serverStop.wait( 10 ) )
			break;

		theServer.update();
	}

	theServer.stop();
	return 0;
}



