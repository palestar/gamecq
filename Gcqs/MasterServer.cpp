/*
	MasterServer.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#define GCQS_DLL

#include "GCQS/MasterServer.h"
#include "Standard/MD5.h"
#undef Time
#include "Standard/Time.h"

//----------------------------------------------------------------------------

MasterServer::MasterServer() : m_pDB( NULL )
{}

MasterServer::~MasterServer()
{
	if ( m_pDB != NULL )
	{
		// close the database connection
		Database::release( m_pDB );
		m_pDB = NULL;
	}
}

//----------------------------------------------------------------------------

void MasterServer::onConnect( dword client )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "MasterServer", CharString().format("Connecting client %u (%s)", client, clientAddress( client ) ) );

	// hash and send public key to client
	m_ClientKeys[ client ] = WidgetKey().string();
	send( client, MasterClient::CLIENT_RECV_PUBLIC ) << m_ClientKeys[ client ];
}

void MasterServer::onReceive( dword client, byte message, const InStream & input )
{
	switch( message )
	{
	case MasterClient::SERVER_RECV_KEY:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;
			CharString key;
			input >> key;

			AutoLock lock( &m_Lock );

			// firstly make sure the key is valid
			CharString publicKey( m_ClientKeys[ client ] );
			CharString myAddress( clientSocket( client )->address() );
			CharString peerAddress( clientSocket( client )->peerAddress() );
			CharString validKey( MD5( CharString(publicKey + peerAddress + myAddress) ).checksum() );

			int jobDone = 0;
			if ( version == MasterClient::VERSION )
			{
				if ( key != validKey )
					LOG_STATUS( "MasterServer", "Warning key is not valid.." );

				// make sure their IP address is in our database
				CharString query;
				query.format( "SELECT authorized_id FROM authorized WHERE address = '%s'", peerAddress.cstr() );

				Database::Query found = m_pDB->query( query );
				if ( found.rows() > 0 )
					jobDone = found[0][0];
				else
					LOG_STATUS( "MasterServer", "IP not found in database..." );
			}
			else
				LOG_STATUS( "MasterServer", CharString().format("Client %u sent invalid version...", client) );

			if ( jobDone > 0 )
				LOG_STATUS( "MasterServer", CharString().format("Client %u authorized..", client) );
			else
				LOG_STATUS( "MasterServer", CharString().format("Client %u not authorized...", client) );

			send( client, MasterClient::CLIENT_RECV_KEY ) << job << MasterClient::VERSION << jobDone;
		}
		break;
	}
}

void MasterServer::onDisconnect( dword client )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "MasterServer", CharString().format("Disconnecting client %u", client) );
	m_ClientKeys.remove( client );
}

bool MasterServer::start( const Context & context )
{
	m_Context = context;

	LOG_STATUS( "MasterServer", CharString().format("Starting MasterServer, address = %s, port = %d, maxClients = %d",
		m_Context.address.cstr(), m_Context.port, m_Context.maxClients ) );

	// open the connection to the database
	if (! initializeDB() )
	{
		LOG_STATUS( "MasterServer", "Failed to connect to DB!" );
		return false;
	}

	// start the server
	if (! Server::start( new Socket("ZLIB"), m_Context.port, m_Context.maxClients ) )
		return false;

	return true;
}

//----------------------------------------------------------------------------

bool MasterServer::initializeDB()
{
	LOG_STATUS( "MasterServer", CharString().format("Connecting to database, dbname = %s, dbaddress = %s, dbuid = %s, dbport = %u",
		m_Context.dbname.cstr(), m_Context.dbaddress.cstr(), m_Context.dbuid.cstr(), m_Context.dbport) );

	// create the connection
	m_pDB = Database::create( "DatabaseMYSQL" );
	if (! m_pDB->open( m_Context.dbname, m_Context.dbaddress, m_Context.dbport, m_Context.dbuid, m_Context.dbpw ) )
		return false;

	return true;
}

//----------------------------------------------------------------------------
// EOF

