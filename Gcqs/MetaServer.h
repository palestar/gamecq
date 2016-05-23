/*
	MetaServer.h
	(c)1999 PaleStar Development, Richard Lyle
*/

#ifndef META_SERVER_H
#define META_SERVER_H

#include <set>

#include "File/Stream.h"
#include "Network/Server.h"
#include "GCQ/MetaClient.h"
#include "GCQDB/Database.h"
#include "GCQS/GCQDll.h"

//----------------------------------------------------------------------------

class DLL MetaServer : public Server
{
public:
	// Types
	typedef MetaClient::Game			Game;
	typedef MetaClient::Profile			Profile;
	typedef MetaClient::ShortProfile	ShortProfile;
	
	struct Context
	{
		Context() : dbport( 10100 ),
			maxConnections( 4 ),
			port( 9000 ),
			maxClients( 500 ),
			gameId( 0 ),
			eventNotifyTime( 3600 )
		{}

		CharString		dbname;					// database name
		CharString		dbaddress;				// database address
		int				dbport;					// database port
		CharString		dbuid, dbpw;			// database user, password
		int				maxConnections;			// maximum number of database connections to keep
		CharString		motdFile;				// message of the day file
		CharString		address;				// address
		int				port;					// port
		int				maxClients;				// max clients
		dword			gameId;					// game id for the server
		int				eventNotifyTime;		// how many seconds before event occurs to begin notifications
	};

	// Construction
						MetaServer();
	virtual				~MetaServer();

	// Server interface
	void				onConnect( dword client );							// called when connection made
	void				onReceive( dword client, byte message, const InStream & );	// called when receiving from client
	void				onDisconnect( dword client );						// called when connection lost

	// Mutators
	bool				start( const Context & context );					// run the server
	void				stop();												// stop the metaserver


	void				sendChat( dword client, const char * pMessage );
	void				sendGlobalChat( const char * pMessage );

	// Static
	static CharString	addSlash( const char * pString );

private:
	// Types
	typedef Hash< dword, dword >			UserGameId;
	typedef Hash< dword, dword >			ClientUserHash;
	typedef Hash< dword, dword >			ClientSessionHash;	
	typedef Hash< dword, Profile >			GameProfileHash;
	typedef Hash< dword, GameProfileHash >	UserProfileHash;
	typedef Hash< dword, Array< dword > >	UserClientHash;
	typedef Hash< dword, dword >			ClientServerHash;
	typedef Hash< dword, dword >			ServerClientHash;
	typedef Hash< dword, dword >			ClientPongHash;
	typedef Hash< dword, Array< dword > >	RoomUserHash;
	typedef Hash< dword, dword >			RoomGameHash;
	typedef Hash< dword, Array< dword > >	UserRoomHash;
	typedef Hash< dword, Array< dword > >	UserIgnoreHash;

	struct EventTimer {
		dword			m_nEventID;
		CharString		m_sEventTitle;
		dword			m_nStartTime;
		dword			m_nNextMessageTime;
	};
	typedef std::map< dword, EventTimer >	EventTimerMap;

	typedef Hash< dword, CharString >		ClientMachineHash;
	typedef Tree< CharString, Array< dword > >	MachineClientTree;
	typedef std::set< dword >				LogFieldSet;
	typedef std::set< dword >				ModeratedRoomSet;

	// Data
	Context				m_Context;			// startup context
	CriticalSection		m_LogLock;
	dword				m_ServerId;			// our server id
	dword				m_LastBanId;
	dword				m_LastMessageId;
	dword				m_LastReloadId;
	dword				m_LastReportId;

	Queue< Database * >	m_Connections;		// database connections

	Hash< dword, CharString >
						m_ClientKey;		// client id -> key
	UserGameId			m_UserGameId;		// client id -> game id
	ClientMachineHash	m_ClientMachine;	// client id -> machine id
	MachineClientTree	m_MachineClient;	// machine id -> client id

	ClientUserHash		m_ClientUser;		// client id -> user id
	ClientSessionHash	m_ClientSession;	// client id -> session id
	UserProfileHash		m_UserProfile;		// userId -> gameId -> Profile
	UserClientHash		m_UserClient;		// userId -> clientId(s)

	ClientServerHash	m_ClientServer;		// client id -> server id
	ServerClientHash	m_ServerClient;		// server id -> client id
	ClientPongHash		m_ClientPong;		// client id -> last pong
	RoomUserHash		m_RoomUsers;		// users in a specific room
	RoomGameHash		m_RoomGame;			// room id -> game id
	ModeratedRoomSet	m_RoomModerated;	// room that are currently moderated
	UserRoomHash		m_UserRoom;			// user id -> array of room id's
	UserIgnoreHash		m_Ignores;			// user id -> ignores
	EventTimerMap		m_EventTimerMap;	// event id -> EventTimer 
	Array< CharString >	m_Words;			// illegal words / profanity
	LogFieldSet			m_LogFieldSet;		// set of field_id's that should be logged to the user_ladder_log

	// Mutators
	bool				initializeDB();

	Database *			getConnection();				// get a database connection
	void				freeConnection( Database * pConnection );
	void				freeAllConnections();

	void				selectGame( dword nClientId, dword nGameId, dword nJobId );
	bool				getShortProfile( dword gameId, dword userId, ShortProfile & profile );
	bool				getShortProfiles( dword gameId, const Array< dword > & userIds, Array< ShortProfile > & profiles );
	bool				getProfile( dword gameId, dword userId, Profile & profile );
	bool				filterProfileForClient( dword nClientId, Profile & profile );
	bool				filterProfileForClient( dword nClientId, ShortProfile & profile );
	void				filterProfilesForClient( dword nClientId, Array< ShortProfile > & profile, bool bRemoveHidden = false );
	bool				flushProfile( dword userId );
	bool				getFriends( dword userId, Array< dword > & friends );
	bool				getFriends( dword gameId, dword userId, Array< ShortProfile > & friends );
	bool				getClanMembers( dword clanId, Array< dword > & members );
	bool				getClanMembers( dword gameId, dword clanId, Array< ShortProfile > & members );
	bool				getClanMembersOnline( dword clanId, Array< dword > & members );
	bool				getClanIdByName( CharString clanName, dword & clanId );
	bool				getClanName( dword client, dword clanId, CharString & clanName );
	bool				getRoomMembers( dword gameId, dword roomId, Array< ShortProfile > & members );
	bool				getModeratorsInRoom( dword roomId, dword gameId, Array< dword > & mods );
	bool				getStaffOnline( dword gameId, Array< dword > & staff );
	bool				getDevelopersOnline( dword gameId, Array< dword > & devs );
	bool				getModeratorsOnline( dword gameId, Array< dword > & mods );
	bool				getModeratorsOnline( dword gameId, Array< ShortProfile > & mods );

	bool				findUser( const char * pPattern, Array< dword > & found );
	bool				findUserExactFirst( const char * pPattern, Array< dword > & found );
	dword				banUser( dword who, dword userId, dword duration, const char * pWhy );
	bool				addWatchList( dword who, dword userId, dword watch_type, const char * pWhy, dword banId );

	dword				registerSelf();
	dword				registerServer( dword clientId, MetaClient::Server & server );
	void				removeServer( dword clientId );

	void				loginClient( dword clientId, dword job, dword userId, const char * pMID );
	bool				validateClient( dword clientId, dword flags = 0 );	// validate that the client is logged in
	bool				isAdministrator( dword clientId );
	bool				isModerator( dword clientId );
	bool				isServer( dword clientId );
	bool				isRoomModerated( dword roomId );
	dword				getGameId( dword clientId );
	bool				getProfile( dword clientId, MetaClient::Profile & profile );
	dword				getUserId( dword clientid );
	CharString			getUserName( dword clientId );
	dword				getFlags( dword clientId );
	CharString			getPublicKey( dword clientId );
	CharString			getMID( dword clientId );
	void				logoffClient( dword client );
	
	bool				validateName( const char * pName );					// validate that username is legal
	bool				validateMID( const char * pMID );					// validate the users MID

	void				processCommand( dword client, dword roomId, dword recpId, 
							const char * pText );
	void				processMessage( dword client, dword roomId, dword recpId, 
							const char * pText, bool echo = false );
	void				processMessageInternal( dword client, const char * pAuthorName, 
							dword roomId, dword recpId, const char * pText, bool echo = false );
	void				processChat( dword client, dword roomId, dword recpId, const char * pText );

	void				updateDemon();

	class UpdateThread : public SafeThread	
	{
	public:
		// Construction
						UpdateThread( MetaServer * pServer );
		// Thread interface
		int				run();
	private:
		MetaServer *	m_pServer;
	};

	friend class UpdateThread;

};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

