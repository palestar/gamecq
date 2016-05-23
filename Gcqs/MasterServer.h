/*
	MasterServer.h

	This class implements a MasterServer, which is used by the MetaServer to verify and communicate with all the other MetaServers
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef MASTERSERVER_H
#define MASTERSERVER_H

#include "File/Stream.h"
#include "Network/Server.h"
#include "GCQDB/Database.h"
#include "GCQ/MasterClient.h"
#include "GCQS/GCQDll.h"

//----------------------------------------------------------------------------

class DLL MasterServer : public Server
{
public:
	// Types
	struct Context
	{
		CharString		dbname;					// database name
		CharString		dbaddress;				// database address
		int				dbport;					// database port
		CharString		dbuid, dbpw;			// database user, password
		CharString		address;				// address
		int				port;					// port
		int				maxClients;				// max clients

		Context &		operator=( const Context & copy );
	};

	// Construction
	MasterServer();
	virtual ~MasterServer();

	// Server interface
	void				onConnect( dword client );							// called when connection made
	void				onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	void				onDisconnect( dword client );						// called when connection lost

	// Mutators
	bool				start( const Context & context );					// run the server

private:
	// Data
	Database *			m_pDB;				// connection to the SQL server
	Context				m_Context;			// startup context
	Hash< dword, CharString >
						m_ClientKeys;		// client public keys
	
	// Mutators
	bool				initializeDB();
};

//----------------------------------------------------------------------------

inline MasterServer::Context & MasterServer::Context::operator=( const Context & copy )
{
	dbname = copy.dbname;
	dbaddress = copy.dbaddress;
	dbport = copy.dbport;
	dbuid = copy.dbuid;
	dbpw = copy.dbpw;
	address = copy.address;
	port = copy.port;
	maxClients = copy.maxClients;
	return *this;
}

#endif

//----------------------------------------------------------------------------
// EOF

