/*
	MetaServer.cpp
	(c)1999 Palestar, Richard Lyle
*/

#define GCQS_DLL
#define MEDUSA_TRACE_ON

#include "GCQ/MasterClient.h"
#include "GCQS/MetaServer.h"
#include "Debug/Assert.h"
#undef Time
#include "Standard/Time.h"
#include "Standard/MD5.h"
#include "Standard/UUE.h"
#include "Standard/CommandLine.h"
#include "Network/FileSocket.h"
#include "File/FileDisk.h"
#include "Standard/StringBuffer.h"

#include <time.h>
#include <ctype.h>

//! Define to non-zero to enable logging of all profile changes to the DB.
#define ENABLE_LADDER_LOG				0
//! Any messages that take longer than this amount in time in milliseconds generate an warning to the log..
const dword MESSAGE_WARNING_TIME = 100;

//---------------------------------------------------------------------------------------------------

static void AddSQL( CharString & sCondition, const char * pClause )
{
	if ( sCondition.length() > 0 )
		sCondition += " AND ";
	sCondition += pClause;
}

//-------------------------------------------------------------------------------

MetaServer::MetaServer() : m_ServerId( 0 ), 
	m_LastBanId( 0 ), m_LastMessageId( 0 ), m_LastReloadId( 0 ), m_LastReportId( 0 )
{}

MetaServer::~MetaServer()
{
	LOG_STATUS( "MetaServer", "MetaServer stopped...");
}

//-------------------------------------------------------------------------------

void MetaServer::onConnect( dword clientId )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "MetaServer", "Connecting, Client %u (%s)", clientId, clientAddress(clientId) );
	m_ClientPong[ clientId ] = Time::seconds();
}

void MetaServer::onReceive( dword clientId, byte message, const InStream & input )
{
	//TRACE( CharString().format("MetaServer::onReceive, clientId = %u, message = %x", clientId, dword( message )) );

	dword nStartTime = Time::milliseconds();
	switch( message )
	{
	case MetaClient::SERVER_SEND_KEY:
		{
			dword job;
			input >> job;

			// send back the key
			send( clientId, MetaClient::CLIENT_RECV_KEY) 
				<< job << getPublicKey( clientId );
		}
		break;
	case MetaClient::SERVER_LOGIN:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;

			if ( version == MetaClient::version() )
			{
				CharString mid;
				input >> mid;
				CharString id;
				input >> id;
				CharString md5;
				input >> md5;

				// user names are limited to 50 characters or less
				id.left( 50 );
				
				if( validateMID( mid ) )
				{
					LOG_STATUS( "MetaServer", CharString().format("Client %u login, id = %s", clientId, id.cstr() ) );
					
					CharString find( addSlash( id ) );

					// get all username/loginname matches from the database
					Database * pDB = getConnection();
					Database::Query result = pDB->query( CharString().format( "SELECT user_id, user_password, user_newpasswd FROM users WHERE loginname='%s' OR username='%s'", 
						find.cstr(), find.cstr() ) );
					
					// get the clients public key
					CharString publicKey( getPublicKey( clientId ) );
					// find the userId
					dword userId = 0;
					for(int i=0;i<result.rows() && userId == 0;i++)
					{
						CharString password = (const char *)result[i][1];
						CharString trueMD5 = MD5( publicKey + password ).checksum();
						
						// found a username match, check the passwords!
						if ( md5 == trueMD5 )
						{
							// found the user, login the client
							userId = result[i][0];
							loginClient( clientId, job, userId, mid );
						}

						CharString newPassword = (const char *)result[i][2];
						if ( newPassword.length() > 0 )
						{	
							trueMD5 = MD5( publicKey + newPassword ).checksum();
							if ( md5 == trueMD5 )
							{
								userId = result[i][0];
								loginClient( clientId, job, userId, mid );

								// new password is being used, update the user record in the DB..
								LOG_STATUS( "MetaServer", "Password reset completed for user %u.", userId );
								pDB->execute( CharString().format( "UPDATE users SET user_password='%s',user_newpasswd='' WHERE user_id=%u", newPassword.cstr(), userId ) );
							}
						}
					}
					
					freeConnection( pDB );

					if( userId == 0 )
					{
						send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_FAILED;
						LOG_STATUS( "MetaServer", "Invalid password from client %u, id = %s, mid = %s", clientId, id.cstr(), mid.cstr() );
					}
				}
				else
				{
					send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_FAILED;
					LOG_STATUS( "MetaServer", "Invalid MID from client %u, id = %s, mid = %s", clientId, id.cstr(), mid.cstr() );
				}
			}
			else
			{
				send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_FAILED;
				LOG_STATUS( "MetaServer", "Invalid version from client %u", clientId );
			}
		}
		break;
	case MetaClient::SERVER_LOGIN_SESSION:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;

			if ( version == MetaClient::version() )
			{
				CharString mid;
				input >> mid;
				dword sessionId;
				input >> sessionId;

				if( validateMID( mid ) )
				{
					LOG_STATUS( "MetaServer", CharString().format("Client %u login by session, sessionId = %u", clientId, sessionId ) );
					
					Database * pDB = getConnection();
					Database::Query result( pDB->query( CharString().format( "SELECT user_id FROM sessions WHERE sess_id=%u", sessionId ) ) );
					
					dword userId = 0;
					if ( result.rows() > 0 )
						userId = result[0][0];
					
					freeConnection( pDB );
					
					loginClient( clientId, job, userId, mid );
				}
				else
				{
					send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_FAILED;
					LOG_STATUS( "MetaServer", "Invalid MID from client %u, sessionId = %s, mid = %", clientId, sessionId, mid.cstr() );
				}
			}
			else
			{
				LOG_STATUS( "MetaServer", "Invalid version from client %u", clientId );
			}
		}
		break;
	case MetaClient::SERVER_LOGIN_PROXY:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			bool sessionLogin;
			input >> sessionLogin;

			if ( sessionLogin )
			{
				dword sessionId;
				input >> sessionId;

				LOG_STATUS( "MetaServer", CharString().format("Client %u login by proxy, session = %u", clientId, sessionId ) );

				Database * pDB = getConnection();
				Database::Query result( pDB->query( CharString().format( "SELECT user_id FROM sessions WHERE sess_id=%u", sessionId ) ) );

				dword userId = 0;
				if ( result.rows() > 0 )
					userId = result[0][0];

				freeConnection( pDB );

				MetaClient::Profile profile;
				if ( userId != 0 && getProfile( getGameId( clientId ), userId, profile ) )
					send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_OKAY << profile;
				else
					send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_ERROR;
			}
			else
			{
				CharString id;
				input >> id;
				CharString md5;
				input >> md5;

				id.left( 50 );

				LOG_STATUS( "MetaServer", "Client %u login by proxy with MD5, id = %s", clientId, id.cstr() );


				Database * pDB = getConnection();

				CharString find = addSlash( id );
				Database::Query result( pDB->query( CharString().format( "SELECT user_id, user_password FROM users WHERE loginname='%s' OR username='%s'", 
					find.cstr(), find.cstr() ) ) );

				dword userId = 0;
				for(int i=0;i<result.rows();i++)
				{
					CharString password( (const char *)result[i][1] );
					
					// found a username match, check the passwords!
					if ( md5 == password )
					{
						userId = result[i][0];
						break;
					}
				}

				freeConnection( pDB );

				MetaClient::Profile profile;
				if ( userId != 0 && getProfile( getGameId( clientId ), userId, profile ) )
					send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_OKAY << profile;
				else
					send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_ERROR;
			}
		}
		break;
	case MetaClient::SERVER_LOGIN_CREATE:
		{
			dword job;
			input >> job;
			CharString mid;
			input >> mid;
			Profile profile;
			input >> profile;
			CharString password;
			input >> password;

			LOG_STATUS( "MetaServer", "Client %u creating new login, name = %s, password = %s, email = %s", 
				clientId, profile.name.cstr(), password.cstr(), profile.email.cstr() );

			if( validateMID( mid ) )
			{
				Database * pDB = getConnection();
				
				// make sure this machine isn't banned
				Database::Query banned( pDB->query( CharString().format("SELECT ban_end FROM banlist WHERE ban_machine='%s' AND ban_end > unix_timestamp()", mid.cstr())) );
				if ( banned.rows() == 0 )
				{
					if ( validateName( profile.name ) )
					{
						CharString pattern = addSlash( profile.name );
						Database::Query result( pDB->query( CharString().format( "SELECT user_id FROM users WHERE (loginname='%s' OR username='%s')", 
							pattern.cstr(), pattern.cstr() ) ) );
						if ( result.rows() == 0 )
						{
							// add the user to the user table
							pDB->execute( CharString().format("INSERT INTO users(loginname, username, user_regdate, user_password, user_email)"
								" VALUES('%s','%s','%s','%s','%s')", 
								pattern.cstr(), pattern.cstr(), Time::format( Time::seconds(), "%B %d, %Y").cstr(),
								addSlash( password ).cstr(), addSlash( profile.email ).cstr() ) );
							
							dword userId = pDB->insertId();
							loginClient( clientId, job, userId, mid );
						}
						else
						{
							// username already taken
							send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_DUPLICATE_LOGIN;
						}
					}
					else
					{
						// name is illegal
						send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_ILLEGAL;
					}
					
				}
				else
				{
					LOG_STATUS( "MetaServer", "Rejecting client %u, machine %s is banned!", clientId, mid.cstr() );
					send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_BANNED;
				}
				
				
				freeConnection( pDB );
				
			}
			else
			{
				send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_FAILED;
				LOG_STATUS( "MetaServer", "Invalid MID from client %u", clientId );
			}

		}
		break;
	case MetaClient::SERVER_SEND_GAMES:
		{
			dword job;
			input >> job;

			Database * pDB = getConnection();

			Database::Query games( pDB->query( "SELECT * FROM games ORDER BY name") );
			bool bIsAdmin = isAdministrator( clientId );

			Array< MetaClient::Game > gameList;
			for(int i=0;i<games.rows();i++)
			{
				if ( ((int)games[i]["is_public"]) == 0 && !bIsAdmin )
					continue;		// skip not public and not admin..

				MetaClient::Game & game = gameList.push();
				game.id = games[i]["game_id"];
				game.name = games[i]["name"];
				game.newlogin = games[i]["newlogin_url"];
				game.home = games[i]["home_url"];
				game.download = games[i]["download_url"];
				game.admin = games[i]["admin_url"];
				game.manual = games[i]["manual_url"];
				game.clans = games[i]["clan_url"];
				game.profile = games[i]["profile_url"];
				game.news = games[i]["news_url"];
				game.forum = games[i]["forum_url"];
				game.registry = games[i]["registry"];
				game.address = games[i]["address"];
				game.port = games[i]["port"];
				game.chatRoom = games[i]["chat_room"];
			}

			// send the game list to the client
			send( clientId, MetaClient::CLIENT_RECV_GAMES ) << job << gameList;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_SERVERS:
		{
			dword job;
			input >> job;
			CharString sName;
			input >> sName;
			dword gameId;
			input >> gameId;
			dword type;
			input >> type;

			CharString sQuery = "SELECT * from servers";

			CharString sCondition;
			if ( sName.length() > 0 )
				AddSQL( sCondition, CharString().format("name='%s'", addSlash( sName ).cstr()) );
			if ( gameId != 0 )
				AddSQL( sCondition, CharString().format("game_id=%u", gameId ) );
			if ( type != 0 )
				AddSQL( sCondition, CharString().format("type=%u", type ) );

			if ( sCondition.length() > 0 )
				sCondition = CharString(" WHERE ") + sCondition;

			Database * pDB = getConnection();
			Database::Query servers( pDB->query( sQuery + sCondition ) );

			Array< MetaClient::Server > serverList;
			for(int i=0;i<servers.rows();i++)
			{
				MetaClient::Server & server = serverList.push();
				server.gameId = servers[i]["game_id"];
				server.id = servers[i]["server_id"];
				server.type = servers[i]["type"];
				server.flags = servers[i]["flags"];
				server.name = servers[i]["name"];
				server.name.unslash();
				server.shortDescription = servers[i]["short_description"];
				server.shortDescription.unslash();
				server.description = servers[i]["description"];
				server.description.unslash();
				server.address = servers[i]["address"];
				server.port = servers[i]["port"];
				server.maxClients = servers[i]["max_clients"];
				server.clients = servers[i]["clients"];
				server.lastUpdate = servers[i]["last_updated"];
				server.data = servers[i]["data"];
				server.data.unslash();
			}
			// send the game list to the client
			send( clientId, MetaClient::CLIENT_RECV_SERVERS ) << job << serverList;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_REGISTER_SERVER:
		if ( validateClient( clientId ) )
		{
			MetaClient::Server server;
			input >> server;

			registerServer( clientId, server );
		}
		break;
	case MetaClient::SERVER_REGISTER_SERVER2:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			MetaClient::Server server;
			input >> server;

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << registerServer( clientId, server );
		}
		break;
	case MetaClient::SERVER_REMOVE_SERVER:
		if ( validateClient( clientId ) )
			removeServer( clientId );
		break;
	case MetaClient::SERVER_SEND_PROFILES:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString id;
			input >> id;
			id.left( 50 );
			
			Database * pDB = getConnection();

			Database::Query users( pDB->query( CharString().format("SELECT user_id FROM users WHERE username LIKE '%s' ORDER BY username LIMIT 256", addSlash(id).cstr() ) ) );

			dword gameId = getGameId(clientId);

			Array< MetaClient::ShortProfile > profiles;
			profiles.allocate( users.rows() );

			for(int i=0;i<users.rows();i++)
			{
				MetaClient::ShortProfile profile;
				getShortProfile( gameId, users[i][0], profiles[i] );
			}

			filterProfilesForClient( clientId, profiles, true );
			freeConnection( pDB );

			send( clientId, MetaClient::CLIENT_RECV_PROFILES) << job << profiles;
		}
		break;
	case MetaClient::SERVER_SEND_PROFILE:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			bool isSessionId;
			input >> isSessionId;
			dword id;
			input >> id;

			dword userId = 0;
			if ( isSessionId )
			{
				Database * pDB = getConnection();

				Database::Query sessions( pDB->query( CharString().format( "SELECT user_id from sessions WHERE sess_id = %u", id ) ) );
				if ( sessions.rows() > 0 )
					userId = sessions[0][0];

				freeConnection( pDB );
			}
			else
				userId = id;

			MetaClient::Profile profile;
			if ( userId != 0 && getProfile( getGameId( clientId ), userId, profile ) )
				send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_OKAY << profile;
			else
				send( clientId, MetaClient::CLIENT_RECV_PROFILE) << job << MetaClient::RESULT_ERROR;
		}
		break;
	case MetaClient::SERVER_RECV_PROFILE:
		if ( validateClient( clientId, MetaClient::SERVER|MetaClient::ADMINISTRATOR ) )
		{
			dword job;
			input >> job;
			MetaClient::Profile profile;
			input >> profile;

			dword gameId = getGameId( clientId );
			dword clientFlags = getFlags( clientId );

			bool bUpdated = false;

			Database * pDB = getConnection();
			if ( clientFlags & MetaClient::SERVER )
			{
				//LOG_STATUS( "MetaServer", CharString().format("SERVER_RECV_PROFILE, player=%s, score=%f, clientId=%u", profile.name, profile.score, clientId) );

				// update the user_data table
				pDB->execute( CharString().format("UPDATE user_data SET score='%f',rank='%s' WHERE game_id=%u AND user_id=%u", 
					profile.score, 
					profile.rank.cstr(),
					gameId,	profile.userId ) );
				bUpdated = true;
			}
			if ( clientFlags & MetaClient::ADMINISTRATOR )
			{
				pDB->execute( CharString().format("UPDATE user_data SET moderator=%u,server=%u,admin=%u,news=%u,editor=%u,event=%u,developer=%u WHERE game_id=%u AND user_id=%u", 
					(profile.flags & MetaClient::MODERATOR) ? 1 : 0,
					(profile.flags & MetaClient::SERVER) ? 1 : 0, 
					(profile.flags & MetaClient::ADMINISTRATOR) ? 1 : 0,
					(profile.flags & MetaClient::NEWS_ADMIN) ? 1 : 0,
					(profile.flags & MetaClient::EDITOR) ? 1 : 0,
					(profile.flags & MetaClient::EVENT) ? 1 : 0,
					(profile.flags & MetaClient::DEVELOPER) ? 1 : 0,
					gameId,	profile.userId ) );
				bUpdated = true;
			}
			if ( bUpdated )
				pDB->execute(CharString().format("INSERT INTO user_reload(user_id,time) VALUES (%u,UNIX_TIMESTAMP())", profile.userId ) );

			freeConnection( pDB );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
		}
		break;
	case MetaClient::SERVER_ADD_PROFILE:
		if ( validateClient( clientId, MetaClient::SERVER ) )
		{
			dword userId;
			input >> userId;
			dword fieldId;
			input >> fieldId;
			float fAdd;
			input >> fAdd;

			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();
			
			pDB->execute( CharString().format("UPDATE user_ladder SET value = value + %f WHERE user_id=%u AND field_id=%u",
				fAdd, userId, fieldId) );

			// if this is a logged field, insert another record into the DB concerning this change..
			if ( m_LogFieldSet.find( fieldId ) != m_LogFieldSet.end() )
			{
				pDB->execute( CharString().format("INSERT INTO user_ladder_log(user_id,field_id,value,time) VALUES(%u,%u,%f,unix_timestamp())",
					userId, fieldId, fAdd) );
			}

			freeConnection( pDB );

			// update the in-memory profile as well...
			lock();

			UserProfileHash::Iterator iGames = m_UserProfile.find( userId );
			if ( iGames.valid() )
			{
				GameProfileHash::Iterator iProfile = (*iGames).find( gameId );
				if ( iProfile.valid() )
				{
					Profile & profile = *iProfile;

					for(int i=0;i<profile.fields.size();++i)
					{
						if ( profile.fields[i].id == fieldId )
						{
							float fValue = (float)atof( profile.fields[i].value.cstr() );
							fValue += fAdd;

							profile.fields[i].value = CharString().format( "%f", fValue );
							break;
						}
					}
				}
			}
			unlock();
		}
		break;
	case MetaClient::SERVER_UPDATE_SCORE:
		if ( validateClient( clientId, MetaClient::SERVER ) )
		{
			dword userId;
			input >> userId;
			float fScore;
			input >> fScore;
			CharString sRank;
			input >> sRank;

			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();

			pDB->execute( CharString().format("UPDATE user_data SET score='%f',rank='%s' WHERE game_id=%u AND user_id=%u", 
				fScore, sRank.cstr(), gameId, userId ) );
			
			freeConnection( pDB );

			// update the in-memory profile as well...
			lock();

			UserProfileHash::Iterator iGames = m_UserProfile.find( userId );
			if ( iGames.valid() )
			{
				GameProfileHash::Iterator iProfile = (*iGames).find( gameId );
				if ( iProfile.valid() )
				{
					Profile & profile = *iProfile;
					profile.score = fScore;
					profile.rank = sRank;
				}
			}
			unlock();
		}
		break;
	case MetaClient::SERVER_SEND_CLANS:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString id;
			input >> id;


			Database * pDB = getConnection();

			Database::Query rows( pDB->query( CharString().format("SELECT clan_id,name,home FROM clans WHERE name LIKE '%s' ORDER BY name", addSlash(id).cstr() ) ) );

			Array< MetaClient::ShortClan > clans;
			for(int i=0;i<rows.size();i++)
			{
				MetaClient::ShortClan & clan = clans.push();
				clan.clanId = rows[i][0];
				clan.name = rows[i][1];
				clan.home = rows[i][2];
			}

			send( clientId, MetaClient::CLIENT_RECV_CLANS) << job << clans;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_CLAN:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword clanId;
			input >> clanId;

			Database * pDB = getConnection();

			dword gameId = getGameId( clientId );

			Database::Query result( pDB->query( CharString().format("SELECT * from clans WHERE clan_id=%u", clanId ) ) );
			if ( result.rows() == 1 )
			{
				int approved = result[0]["approved"];
				int closed = result[0]["closed"];

				MetaClient::Clan clan;
				clan.clanId = result[0]["clan_id"];
				clan.flags = (approved ? MetaClient::CLAN_VALID : 0);
				clan.flags |= (closed ? MetaClient::CLAN_CLOSED : 0);
				clan.name = result[0]["name"];
				clan.longName = result[0]["long_name"];
				clan.motto = result[0]["motto"];
				clan.home = result[0]["home"];
				//clan.faction = result[0]["faction"];

				// get the clan score
				Database::Query score( pDB->query( CharString().format("SELECT sum(score) AS score FROM clan_members,user_data WHERE "
						"user_data.user_id = clan_members.user_id AND is_valid=1 AND game_id=%u AND clan_id=%u", gameId, clanId ) ) );

				if ( score.rows() > 0 )
					clan.score = score[0]["score"];
				else
					clan.score = 0;
	
				// get the clan data
				Database::Query data( pDB->query( CharString().format( "SELECT * from clan_data,factions WHERE "
					"clan_data.clan_id = %u AND clan_data.game_id = %u AND clan_data.faction_id=factions.faction_id", clanId, gameId) ) );
				if ( data.rows() > 0 )
				{
					clan.faction = data[0]["fid"];

					// uudecode the data
					UUD uud;
					if ( uud.decode( data[0]["data"] ) )
						clan.data.initialize( uud.decoded(), uud.decodedBytes() );

				}
				else
				{
					// no clan data record found, create one!
					pDB->execute( CharString().format("INSERT clan_data(clan_id,game_id) VALUES( %u, %u)", clanId, gameId) );
				}

				send( clientId, MetaClient::CLIENT_RECV_CLAN) << job << MetaClient::RESULT_OKAY << clan;
			}
			else
				send( clientId, MetaClient::CLIENT_RECV_CLAN) << job << MetaClient::RESULT_ERROR;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_MEMBERS:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword clanId;
			input >> clanId;

			Array< MetaClient::ShortProfile > members;
			getClanMembers( getGameId(clientId), clanId,  members );
			filterProfilesForClient( clientId, members, false );

			send( clientId, MetaClient::CLIENT_RECV_PROFILES ) << job << members;
		}
		break;
	case MetaClient::SERVER_RECV_CLAN:
		if ( validateClient( clientId, MetaClient::SERVER|MetaClient::ADMINISTRATOR ) )
		{
			dword job;
			input >> job;
			MetaClient::Clan clan;
			input >> clan;

			dword clientFlags = getFlags( clientId );
			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();

			// allow server to change clan data
			if ( clientFlags & MetaClient::SERVER )
			{
				pDB->execute( CharString().format("UPDATE clan_data SET data='%s' WHERE clan_id=%u AND game_id = %u",
					CharString( UUE( clan.data.data(), clan.data.size() ).encoded() ).slash().cstr(), clan.clanId, gameId ) );
			}
			// allow only admins to approve a clan
			if ( clientFlags & MetaClient::ADMINISTRATOR )
			{
				pDB->execute( CharString().format("UPDATE clans SET approved=%u WHERE clan_id=%u",
					(clan.flags & MetaClient::CLAN_VALID) ? 1 : 0, clan.clanId ) );
			}

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_CLAN_RECRUIT:
		if( validateClient(clientId) )
		{
			dword job, clanid, promotee, promotid;
			input >> job;
			input >> clanid;
			input >> promotee;
			promotid = getUserId( promotee );

			Database *  pDB = getConnection();
	
			//check to make sure this client is an admin of the clan
			Database::Query result( pDB->query( CharString().format("SELECT is_admin from clan_members WHERE clan_id=%u AND user_id=%u", clanid, clientId) ) );
			if( result.rows() > 0 )
			{
				//check to make sure the promotee is actually in the clan
				int result = pDB->query( CharString().format("SELECT user_id from clan_members WHERE clan_id=%u AND user_id=%u", clanid, promotid) ).rows();
				if( result > 0 ) //user_id exists so I FEEL GREAT, I FEEL GOD, I. CAN. DO. THIS.
				{

					pDB->execute(CharString().format("UPDATE clan_members SET is_valid=1 WHERE user_id=$u AND clan_id=$u", promotid, clanid) );
					pDB->execute(CharString().format("INSERT INTO user_reload(user_id,time) VALUES (%u,UNIX_TIMESTAMP())", promotid) );

					Database::Query result(pDB->query(CharString().format("SELECT forum_id FROM clans WHERE clan_id=%u", clanid) ) );

					if( result.rows() > 0 )
					{
						int forum_id = result[0][0];
						pDB->execute(CharString().format("INSERT INTO forum_access(forum_id,user_id,can_post) values (%u, %u, 1)" , forum_id, promotid) );
					}
				}
				else
				{
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
					freeConnection(pDB);
					return;
				}

			}
			else
			{
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
				freeConnection(pDB);
				return;
			}
			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
			freeConnection(pDB);
		}
		break;
	case MetaClient::SERVER_CLAN_PROMOTE:
		if( validateClient(clientId) )
		{
			dword job, clanid, promotee, promotid;
			input >> job;
			input >> clanid;
			input >> promotee;
			promotid = getUserId( promotee );

			Database *  pDB = getConnection();
	
			//check to make sure this client is an admin of the clan
			Database::Query result( pDB->query( CharString().format("SELECT is_admin from clan_members WHERE clan_id=%u AND user_id=%u", clanid, clientId) ) );
			if( result.rows() > 0 )
			{
				//check to make sure the promotee is actually in the clan
				int result = pDB->query( CharString().format("SELECT user_id from clan_members WHERE clan_id=%u AND user_id=%u", clanid, promotid) ).rows();
				if( result > 0 ) //user_id exists so I FEEL GREAT, I FEEL GOD, I. CAN. DO. THIS.
				{
					pDB->execute(CharString().format("UPDATE clan_members SET is_admin=1 WHERE user_id=$u AND clan_id=$u", promotid, clanid) );
				}
				else
				{
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
					freeConnection(pDB);
					return;
				}

			}
			else
			{
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
				freeConnection(pDB);
				return;
			}
			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
			freeConnection(pDB);
		}
		break;
	case MetaClient::SERVER_CREATE_CLAN:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			MetaClient::Clan clan;
			input >> clan;

			dword clanId = 0;
			dword clientFlags = getFlags( clientId );
			dword userId = getUserId( clientId );
			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();

			// make sure clan name or long name isn't already taken
			Database::Query result( pDB->query( CharString().format("SELECT clan_id from clans WHERE name LIKE '%s' OR long_name LIKE '%s'",
				addSlash( clan.name ).cstr(), addSlash( clan.longName ).cstr() ) ) );
			if ( result.rows() == 0 )
			{
				pDB->query( CharString().format("INSERT clans(game_id,name,long_name,motto,home,closed,approved) VALUES(%u,'%s','%s','%s','%s',%u,%u)",
					gameId, addSlash( clan.name ).cstr(), addSlash( clan.longName ).cstr(), 
					addSlash( clan.motto ).cstr(), addSlash( clan.home ).cstr(),
					(clan.flags & MetaClient::CLAN_CLOSED ? 1 : 0),
					(clientFlags & MetaClient::ADMINISTRATOR ? 1 : 0)) );

				// get the clanId
				dword clanId = pDB->insertId();
				if ( clanId != 0 )
				{
					// add this user as the admin
					pDB->execute( CharString().format("INSERT clan_members(clan_id,user_id,is_valid,is_admin) VALUES(%u,%u,1,1)",
						clanId, userId) );
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
				}
				else
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;

				// create the clan_data record
				pDB->execute( CharString().format("INSERT INTO clan_data(clan_id,game_id,faction_id,faction_time) VALUES(%u,%u,%u,UNIX_TIMESTAMP())",
					clanId, gameId, clan.faction) );
			}
			else
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_JOIN_CLAN:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword clanId;
			input >> clanId;
			int curMembers = 0;
			int curLevel = 0;
			int maxMembers = 0;
			dword gameId = getGameId( clientId );
			dword userId = getUserId( clientId );

			Database * pDB = getConnection();

			if ( clanId != 0 )	// request to join a clan. Check if the clan is currently accepting new recruits
			{
				// get current member count
				Database::Query q_cur_members( pDB->query( CharString().format("SELECT COUNT( is_valid ) FROM clan_members WHERE clan_id=$u", clanId) ) );
				if ( q_cur_members.rows() == 1 )
					curMembers = q_cur_members[0][0];
					
				// get current clan level
				Database::Query q_cur_level( pDB->query( CharString().format("SELECT level FROM clan_data WHERE clan_id=%u AND game_id=%u", clanId, gameId) ) );
				if ( q_cur_level.rows() == 1 )
					curLevel = q_cur_level[0][0];
					
				// get max members for that level
				Database::Query q_max_levels( pDB->query( CharString().format("SELECT max_members FROM clan_levels WHERE level=%u", curLevel) ) );
				if ( q_max_levels.rows() == 1 )
					maxMembers = q_max_levels[0][0];
				
				// are the current members under the max member limit?
				if ( curMembers < maxMembers )
				{
					Database::Query result( pDB->query( CharString().format("SELECT clan_id from clans WHERE clan_id=%u AND closed=0", clanId) ) );
					if ( result.rows() == 1 )
						pDB->execute( CharString().format("REPLACE clan_members(clan_id,user_id) VALUES(%u,%u)", clanId, userId) );
					else
						send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
				}
				else
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
			}
			else
				pDB->execute( CharString().format("DELETE FROM clan_members WHERE user_id=%u", userId) );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_ADDRESS:
		{
			dword job;
			input >> job;

			CharString address( clientAddress( clientId ) );
			send( clientId, MetaClient::CLIENT_RECV_ADDRESS ) << job << address;
		}
		break;
	case MetaClient::SERVER_CHANGE_PASSWORD:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString md5;
			input >> md5;

			Database * pDB = getConnection();

			pDB->execute( CharString().format("UPDATE users SET user_password='%s' WHERE user_id=%u", 
				md5.cstr(), getUserId( clientId )) );
			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_CHANGE_NAME:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString name;
			input >> name;

			LOG_STATUS( "MetaServer", CharString().format("Client %u changing name to '%s'", clientId, name.cstr() ) );

			// validate the name
			if ( validateName( name ) )
			{
				Database * pDB = getConnection();

				// make sure the username doesn't already exist
				dword userId = getUserId( clientId );
				CharString pattern = name.slash();
				
				int nConflicts = pDB->query( CharString().format( "SELECT user_id FROM users WHERE username='%s'", pattern.cstr() ) ).rows();
				nConflicts += pDB->query( CharString().format("SELECT user_id FROM users WHERE loginname='%s' AND user_id != %u", pattern.cstr(), userId ) ).rows();

				if ( nConflicts == 0 )
				{
					pDB->execute( CharString().format("UPDATE users SET username='%s' WHERE user_id=%u", 
						pattern.cstr(), userId) );
					pDB->execute( CharString().format("INSERT INTO user_reload(user_id,time) VALUES(%u,unix_timestamp())",
						userId) );
					
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
				}
				else
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::LOGIN_DUPLICATE_LOGIN;

				freeConnection( pDB );
			}
			else
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::LOGIN_ILLEGAL;
		}
		break;
	case MetaClient::SERVER_CHANGE_EMAIL:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString email;
			input >> email;

			Database * pDB = getConnection();

			dword userId = getUserId( clientId );
			pDB->execute( CharString().format("UPDATE users SET user_email='%s' WHERE user_id=%u", 
				addSlash( email ).cstr(), userId) );
			pDB->execute( CharString().format("INSERT INTO user_reload(user_id,time) VALUES(%u,unix_timestamp())",
				userId) );
			
			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SET_STATUS:
		if ( validateClient( clientId ) )
		{
			dword userId;
			input >> userId;
			CharString status;
			input >> status;

			// limit the status text to 50 characters
			status.left( 50 );

			if ( userId == 0 )
				userId = getUserId( clientId );

			Database * pDB = getConnection();
			
			pDB->execute( CharString().format( "UPDATE user_status SET status='%s', updated=unix_timestamp() WHERE user_id=%u",
				addSlash(status).cstr(), userId ) );
			pDB->execute( CharString().format("INSERT INTO user_reload(user_id,time) VALUES(%u,unix_timestamp())",
				userId) );

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_FRIENDS:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			
			Array< MetaClient::ShortProfile > friends;
			getFriends( getGameId( clientId ), getUserId( clientId ), friends );
			filterProfilesForClient( clientId, friends, false );

			send( clientId, MetaClient::CLIENT_RECV_PROFILES ) << job << friends;
		}
		break;
	case MetaClient::SERVER_ADD_FRIEND:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword friendId;
			input >> friendId;

			Database * pDB = getConnection();
			
			Database::Query rows( pDB->query( CharString().format("SELECT key_id FROM ignores WHERE user_id=%u AND ignore_id=%u", 
				friendId, getUserId( clientId ) ) ) ); 
			
			// users can't add others to their friend list if they are on ignore
			if( rows.size() == 0 )
			{
				Database::Query friends( pDB->query( CharString().format("SELECT count(user_id) FROM friends WHERE user_id=%u", 
					getUserId( clientId ) ) ) ); 
				
				// users can only have 100 users max on their friend list
				if( isAdministrator( clientId ) || friends.size() == 0 || ((dword)friends[0][0]) < 100 )
				{
					pDB->execute( CharString().format("REPLACE friends(user_id,friend_id) VALUES(%u,%u)", 
						getUserId( clientId ), friendId) );

					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
				}
				else
				{
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
					sendChat( clientId, "/You can not add more than 100 users to your friendlist..." );
				}
			}
			else
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_DEL_FRIEND:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword friendId;
			input >> friendId;

			Database * pDB = getConnection();
			pDB->execute( CharString().format("DELETE FROM friends WHERE user_id = %u AND friend_id = %u", 
				getUserId( clientId ), friendId) );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_DEL_SELFFROMFRIEND:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString who;
			input >> who;
			
			dword userId = 0;

			Array< dword > found;
			if ( findUserExactFirst( who, found ) )
			{
				if ( found.size() == 1 )
					userId = found[0];
			}
			
			// users can't remove themselves from admins friendlist
			if( userId != 0 )
			{
				Profile profile;
				getProfile( getGameId( clientId ), userId, profile );
				if ( profile.flags & MetaClient::ADMINISTRATOR )
					userId = 0;	// fail if target is moderator
			}

			if( userId != 0 )
			{
				Database * pDB = getConnection();
				Database::Query rows( pDB->query( CharString().format("SELECT user_id FROM friends WHERE user_id=%u AND friend_id=%u", 
					userId, getUserId( clientId ) ) ) ); 
				
				if( rows.size() > 0 )
				{
					pDB->execute( CharString().format("DELETE FROM friends WHERE user_id = %u AND friend_id = %u", 
						userId, getUserId( clientId ) ) );
		
					// only send the user a message that a user got removed from his friendlist if he was on it before
					processMessage( clientId, 0, userId, CharString().format( "/<b>%s @%u</b> has been removed from your friendlist by request...",
						getUserName( clientId ).cstr(), getUserId( clientId ) ) );
				}
				freeConnection( pDB );

				// always send confirmation, regardless if the user was on the other users friendlist or not
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			}
			else
			{
				sendChat( clientId, "/Make sure you typed the username correctly, or use the userId to prevent mismatches." );
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
			}
		}
		break;
	case MetaClient::SERVER_SEND_IGNORES:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;

			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();
			Database::Query rows( pDB->query( CharString().format("SELECT ignore_id FROM ignores WHERE user_id=%u", 
				getUserId( clientId ) ) ) ); 

			Array< MetaClient::ShortProfile > ignores;
			for(int i=0;i<rows.size();i++)
			{
				MetaClient::ShortProfile profile;
				if ( getShortProfile( gameId, rows[i][0], profile ) )
					ignores.push( profile );
			}
			freeConnection( pDB );

			filterProfilesForClient( clientId, ignores, false );
			send( clientId, MetaClient::CLIENT_RECV_PROFILES ) << job << ignores;
		}
		break;
	case MetaClient::SERVER_ADD_IGNORE:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword ignoreId;
			input >> ignoreId;

			bool bOk = true;
			
			// Staff cannot be ignored
			MetaClient::ShortProfile profile;
			if( getShortProfile( getGameId( clientId), ignoreId, profile ) )
				if ( ( profile.flags & MetaClient::MODERATOR) != 0 )
					bOk = false;

			if( bOk )
			{
				Database * pDB = getConnection();

				pDB->execute( CharString().format("REPLACE ignores(user_id,ignore_id) VALUES(%u,%u)", 
					getUserId( clientId), ignoreId) );
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

				freeConnection( pDB );
			}
			else
				send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
		}
		break;
	case MetaClient::SERVER_DEL_IGNORE:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword ignoreId;
			input >> ignoreId;

			Database * pDB = getConnection();

			pDB->execute( CharString().format("DELETE FROM ignores WHERE user_id = %u AND ignore_id = %u", 
				getUserId( clientId ), ignoreId) );
			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_STAFFONLINE:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			
			Array< MetaClient::ShortProfile > staff;
			getModeratorsOnline( getGameId( clientId ), staff );
			filterProfilesForClient( clientId, staff, true );

			send( clientId, MetaClient::CLIENT_RECV_PROFILES ) << job << staff;
		}
		break;
	case MetaClient::SERVER_BAN_USER:
		if ( validateClient( clientId, MetaClient::MODERATOR ) )
		{
			dword job;
			input >> job;
			dword userId;
			input >> userId;
			dword duration;
			input >> duration;

			int result = MetaClient::RESULT_ERROR;

			// get the information for the player that is being banned
			MetaClient::ShortProfile profile;
			if ( getShortProfile( getGameId(clientId), userId, profile ) )
			{
				// Moderators can not be banned via server
				if ( (profile.flags & MetaClient::MODERATOR) == 0 )
				{
					banUser( m_ClientUser[ clientId ], userId, duration, "Unspecified" );
					result = MetaClient::RESULT_OKAY;
				}
			}

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << result;
		}
		break;
	case MetaClient::SERVER_BAN_SERVER:
		if ( validateClient( clientId, MetaClient::MODERATOR ) )
		{
			dword job;
			input >> job;
			dword serverId;
			input >> serverId;
			dword duration;
			input >> duration;

			int result = MetaClient::RESULT_ERROR;

			Database * pDB = getConnection();
			Database::Query rows( pDB->query( CharString().format("SELECT mid from servers WHERE server_id=%u", serverId) ) );
			if ( rows.size() > 0 )
			{
				CharString mid;
				mid = rows[0][0];

				pDB->execute( CharString().format("INSERT banlist(ban_machine,ban_start,ban_end,ban_time_type) VALUES('%s',%u,(unix_timestamp() + %u),4)",
					mid.cstr(), Time::seconds(), duration ) );
				result = MetaClient::RESULT_OKAY;
			}

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << result;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_WATCHLIST:
		if ( validateClient( clientId, MetaClient::MODERATOR ) )
		{
			dword job;
			input >> job;
			MetaClient::WatchListQuery wQuery;
			input >> wQuery;

			int result = MetaClient::RESULT_ERROR;

			CharString sQuery = "SELECT watchlist.*,users.username AS addedbyname,users2.username AS currusername "
				"FROM watchlist,users, users AS users2 WHERE watchlist.addedby = users.user_id AND watchlist.userid = users2.user_id";
			
			// if at least one option is selected, but not all of them
			if( ( wQuery.watch || wQuery.kick || wQuery.ban ) && !( wQuery.watch && wQuery.kick && wQuery.ban ) )
			{
				bool bOrNeeded = false;
				sQuery.append(" AND (");
				if( wQuery.watch )
				{
					sQuery.append( CharString().format(" %swatchlist.watch_type = 0", bOrNeeded ? "OR ":"" ) );
					bOrNeeded = true;
				}
				if( wQuery.kick )
				{
					sQuery.append( CharString().format(" %swatchlist.watch_type = 1", bOrNeeded ? "OR ":"" ) );
					bOrNeeded = true;
				}
				if( wQuery.ban )
				{
					sQuery.append( CharString().format(" %swatchlist.watch_type = 2", bOrNeeded ? "OR ":"" ) );
					bOrNeeded = true;
				}

				sQuery.append(" )");
			}

			if( wQuery.onlyActive )
				sQuery.append( " AND watchlist.is_active = 1" );

			if( wQuery.linkMissing )
				sQuery.append( " AND ( watchlist.postinglink='' OR watchlist.postinglink IS NULL )" );
			
			if( wQuery.userId != 0 )
				sQuery.append( CharString().format( " AND watchlist.userid = %u", wQuery.userId ) );

			if( wQuery.namePattern.length() > 0 )
				sQuery.append( CharString().format( " AND ( watchlist.username LIKE '%%%s%%' OR users2.username LIKE '%%%s%%' )",
							addSlash(wQuery.namePattern).cstr(), addSlash(wQuery.namePattern).cstr() ) );

			if( wQuery.limit > 0 )
				sQuery.append( CharString().format( " ORDER BY watchlist.watch_id DESC LIMIT %u", wQuery.limit ) );

			Database * pDB = getConnection();
			Database::Query rows( pDB->query( sQuery ) ); 

			Array< MetaClient::WatchList > watchListEntrys;
			for(int i=0;i<rows.size();i++)
			{
				int is_active = rows[i]["is_active"];
				
				MetaClient::WatchList watchList;
				watchList.watchId		= rows[i]["watch_id"];
				watchList.userId		= rows[i]["userid"];
				watchList.userName		= rows[i]["username"];
				watchList.currUsername	= rows[i]["currusername"];
				watchList.addedBy		= rows[i]["addedby"];
				watchList.addedTime		= rows[i]["addedtime"];
				watchList.addedByName	= rows[i]["addedbyname"];
				watchList.watchType		= rows[i]["watch_type"];
				watchList.watchReason	= rows[i]["reason"];
				watchList.postingLink	= rows[i]["postinglink"];
				watchList.userIP		= rows[i]["userip"];
				watchList.userMID		= rows[i]["usermachine"];
				watchList.isActive		= (is_active ? true : false);

				watchListEntrys.push( watchList );
			}
			freeConnection( pDB );

			send( clientId, MetaClient::CLIENT_RECV_WATCHLIST ) << job << watchListEntrys;
		}
		break;
	case MetaClient::SERVER_EDIT_WATCHLIST:
		if ( validateClient( clientId, MetaClient::MODERATOR ) )
		{
			dword job;
			input >> job;
			int command;
			input >> command;
			int watchId;
			input >> watchId;
			
			Database * pDB = getConnection();

			// 0 - change postingLink
			if ( command == 0 )
			{
				CharString sOldText;
				input >> sOldText;
				CharString sNewText;
				input >> sNewText;
				
				// length = 0 could mean the old entry is still NULL, so use a modified UPDATE
				if( sOldText.length() == 0 )
					pDB->execute( CharString().format("UPDATE watchlist SET postinglink='%s' WHERE watch_id=%u AND ( postinglink='' OR postinglink IS NULL )", 
							addSlash( sNewText ).cstr(), watchId ) );
				else
					pDB->execute( CharString().format("UPDATE watchlist SET postinglink='%s' WHERE watch_id=%u AND postinglink='%s'", 
							addSlash( sNewText ).cstr(), watchId, addSlash( sOldText ).cstr() ) );
				
				// mysql_info() would be better but seems not to work here
				Database::Query rows( pDB->query( CharString().format("SELECT watch_id FROM watchlist WHERE watch_id=%u AND postinglink='%s'", watchId, sNewText.cstr()  ) ) ); 
				
				if( rows.size() == 1 )
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
				else
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
			}
			// 1 - change active status
			if ( command == 1 )
			{
				bool bOldStatus;
				input >> bOldStatus;
				
				pDB->execute( CharString().format("UPDATE watchlist SET is_active=%u WHERE watch_id=%u AND is_active=%u", 
					bOldStatus ? 0:1, watchId, bOldStatus ? 1:0 ) );
				
				// mysql_info() would be better but seems not to work here
				Database::Query rows( pDB->query( CharString().format("SELECT watch_id FROM watchlist WHERE watch_id=%u AND is_active=%u", watchId, bOldStatus ? 0:1 ) ) ); 
				
				if( rows.size() == 1 )
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
				else
					send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_ERROR;
			}

			freeConnection( pDB );
		}
		break;
	case MetaClient::PING:
		{
			dword tick;
			input >> tick;

			send( clientId, MetaClient::PONG ) << tick;
		}
		break;
	case MetaClient::PONG:
		{
			dword tick;
			input >> tick;

			//LOG_STATUS( "MetaServer", CharString().format("Received PONG, clientId = %u, tick = %u", clientId, tick ) );

			lock();
			
			// save the pong time
			m_ClientPong[ clientId ] = tick;

			// update the clients session
			dword nSessionID = m_ClientSession[ clientId ];
			Database * pDB = getConnection();

			unlock();

			pDB->execute( CharString().format("UPDATE sessions SET start_time=unix_timestamp() WHERE sess_id=%u", nSessionID ) );
			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_ROOMS:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;

			dword clientFlags = getFlags( clientId );
			bool bIsAdmin = (clientFlags & MetaClient::ADMINISTRATOR) != 0;
			bool bIsServer = (clientFlags & MetaClient::SERVER) != 0;
			dword gameId = getGameId( clientId );
			
			Database * pDB = getConnection();

			// get all rooms with the number of members
			CharString query;

			// Only Administrators can see hidden admins.
			if ( (clientFlags & MetaClient::ADMINISTRATOR) == 0 )
			{
				query.format("SELECT rooms.*, COUNT(user_status.hidden) as members "
					"FROM rooms LEFT JOIN room_members ON rooms.room_id = room_members.room_id "
					"LEFT JOIN user_status ON user_status.user_id = room_members.user_id AND user_status.hidden = 0 "
					"WHERE game_id=%u GROUP BY name", gameId );
			}
			else
			{
				query.format("SELECT rooms.*, COUNT(room_members.room_id) as members "
					"FROM rooms LEFT JOIN room_members ON rooms.room_id = room_members.room_id "
					"WHERE game_id=%u GROUP BY name", gameId );
			}
							
			Database::Query rows( pDB->query( query ) );

			Array< MetaClient::Room > rooms;
			for(int i=0;i<rows.size();i++)
			{
				bool bPrivate = ((int)rows[i]["is_private"]) != 0;
				if ( bPrivate && !bIsAdmin && !bIsServer )
					continue;		// don't sent private rooms unless they are the admin/server

				MetaClient::Room & room = rooms.push();

				room.roomId = rows[i]["room_id"];
				room.name = rows[i]["name"];
				room.language = rows[i]["language"];
				room.members = rows[i]["members"];
				
				room.flags = 0;
				if ( ((int)rows[i]["is_static"]) != 0 )
					room.flags |= MetaClient::FLAG_ROOM_STATIC;
				if ( ((int)rows[i]["is_moderated"]) != 0 )
				{
					room.flags |= MetaClient::FLAG_ROOM_MODERATED;
					if ( m_RoomModerated.find( room.roomId ) == m_RoomModerated.end() )
						m_RoomModerated.insert( room.roomId );
				}
				if ( bPrivate )
					room.flags |= MetaClient::FLAG_ROOM_PRIVATE;
				if ( strlen<char>( rows[i]["password"] ) > 0 )
					room.flags |= MetaClient::FLAG_ROOM_PASSWORD;
			}

			send( clientId, MetaClient::CLIENT_RECV_ROOMS ) << job << rooms;

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_SEND_PLAYERS:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword roomId;
			input >> roomId;

			Array< ShortProfile > players;
			if ( m_RoomGame.find( roomId ).valid() )
			{
				dword nGameID = m_RoomGame[ roomId ];

				getRoomMembers( nGameID, roomId, players );
				filterProfilesForClient( clientId, players, true );
			}

			send( clientId, MetaClient::CLIENT_RECV_PROFILES ) << job << players;
		}
		break;
	case MetaClient::SERVER_CREATE_ROOM:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString name;
			input >> name;
			CharString md5;
			input >> md5;
			bool bStatic;
			input >> bStatic;
			bool bModerated;
			input >> bModerated;
			bool bPrivate;
			input >> bPrivate;

			if ( bStatic && !isAdministrator( clientId ) )
				bStatic = false;		// only admins can make static rooms
			if ( bModerated && !isModerator( clientId ) && !isAdministrator( clientId ) )
				bModerated = false;		// only mods can make moderated rooms

			// limit room names
			name.left( 50 );

			dword userId = getUserId( clientId );
			int jobResult = MetaClient::RESULT_ERROR;

			// create the room
			Database * pDB = getConnection();

			// attempt to insert the room 
			pDB->execute( CharString().format("INSERT IGNORE INTO rooms(name,game_id,password,is_static,is_moderated,is_private) VALUES('%s',%u,'%s',%u,%u,%u) ",
				addSlash( name ).cstr(), getGameId( clientId ), md5.cstr(), bStatic ? 1 : 0, bModerated ? 1 : 0, bPrivate ? 1 : 0) );
			// now query for the id, be sure to use the provided password to prevent spoofing by clients to join rooms they shouldn't.
			Database::Query q = pDB->query( CharString().format( "SELECT room_id FROM rooms WHERE name='%s' AND password='%s'", 
				addSlash( name ).cstr(), md5.cstr() ) );

			if ( q.rows() > 0 )
			{
				// get the room id
				jobResult = q[0]["room_id"];

				// prevent multiple rows for a single user & room
				pDB->execute( CharString().format( "DELETE FROM room_members WHERE user_id=%u AND room_id=%u", 
					userId, jobResult ) );
				pDB->execute( CharString().format("INSERT INTO room_members(user_id,room_id,server_id) values(%u,%u,%u)", 
					userId, jobResult, m_ServerId) );
			}
			freeConnection( pDB );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << jobResult;
		}
		break;
	case MetaClient::SERVER_JOIN_ROOM:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword roomId;
			input >> roomId;
			CharString md5;
			input >> md5;

			int jobResult = -1;

			Database * pDB = getConnection();

			// get the game id and password for the room
			Database::Query result( pDB->query( CharString().format("SELECT * from rooms where room_id=%u", roomId) ) );
			if ( result.rows() > 0 )
			{
				dword gameId = result[0]["game_id"];
				CharString password = (const char *)result[0]["password"];
				CharString roomName = (const char *)result[0]["name"];
				bool is_moderated = ((int)result[0]["is_moderated"]) != 0;

				dword userId = getUserId( clientId );
				dword clientFlags = getFlags( clientId );
				bool is_moderator = (clientFlags & MetaClient::MODERATOR) != 0;
				bool is_admin = (clientFlags & MetaClient::ADMINISTRATOR) != 0;

				// check clientId password against the room password, moderators are allowed to join all rooms
				if ( is_admin || is_moderator || password == md5 )
				{
					jobResult = roomId;

					// prevent multiple rows for a single user & room
					pDB->execute( CharString().format( "DELETE FROM room_members WHERE user_id=%u AND room_id=%u", 
						userId, roomId ) );
					// add player to room members
					pDB->execute( CharString().format("INSERT INTO room_members(user_id,room_id,server_id) values(%u,%u,%u)", 
						userId, roomId, m_ServerId) );
				}
			}
			freeConnection( pDB );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << jobResult;
		}
		break;
	case MetaClient::SERVER_LEAVE_ROOM:
		if ( validateClient( clientId ) )
		{
			dword roomId;
			input >> roomId;

			Database * pDB = getConnection();

			// leave the room
			pDB->execute( CharString().format("DELETE FROM room_members WHERE user_id = %u AND room_id = %u", 
				m_ClientUser[ clientId ], roomId ) );

			freeConnection( pDB );
		}
		break;
	case MetaClient::SERVER_RECV_CHAT:
		if ( validateClient( clientId ) )
		{
			dword roomId;
			input >> roomId;
			dword recpId;
			input >> recpId;
			CharString text;
			input >> text;

			// keep the chat under 512 characters
			text.left( 512 );
			// process the chat
			processChat( clientId, roomId, recpId, text );
		}
		break;
	case MetaClient::SERVER_LOGOFF:
		logoffClient( clientId );
		break;
	case MetaClient::SERVER_SELECT_GAME:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			dword gameId;
			input >> gameId;

			selectGame( clientId, gameId, job );
		}
		break;
	case MetaClient::SERVER_RECV_INSTALLER:
		if ( validateClient( clientId ) )
		{
			CharString installer;
			input >> installer;

			dword userId = getUserId( clientId );
			dword gameId = getGameId( clientId );

			Database * pDB = getConnection();
			pDB->execute( CharString().format("REPLACE user_installer(user_id,game_id,installer_id) VALUES(%u,%u,'%s')",
				userId, gameId, addSlash( installer ).cstr()) );
			freeConnection( pDB );
		}
		break;
	case MetaClient::SEND_TIME:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;

			send( clientId, MetaClient::RECV_TIME ) << job << Time::seconds();
		}
		break;
	case MetaClient::SERVER_RECV_MDBG:
		if ( validateClient( clientId ) )
		{
			dword job;
			input >> job;
			CharString mdbg;
			input >> mdbg;

			LOG_STATUS( "MetaServer", " MDBG-Info: %d | %s | %s", getUserId(clientId), getMID(clientId).cstr(), mdbg.cstr() );

			send( clientId, MetaClient::CLIENT_JOB_DONE ) << job << MetaClient::RESULT_OKAY;
		}
		break;
	default:
		LOG_ERROR( "MetaServer", CharString().format("Invalid message type received, Client %u (%s)", clientId, clientAddress( clientId )) );
		removeClient( clientId );
		break;
	}

	dword nElapsed = Time::milliseconds() - nStartTime;
	if ( nElapsed > MESSAGE_WARNING_TIME )
		LOG_WARNING( "MetaServer", CharString().format( "WARNING: Message 0x%0.2x from client %u took %u ms", message, clientId, nElapsed ) );
}

void MetaServer::onDisconnect( dword clientId )
{
	AutoLock lock( &m_Lock );

	LOG_STATUS( "MetaServer", "Disconnecting client %u", clientId );

	m_ClientKey.remove( clientId );
	m_ClientPong.remove( clientId );

	ClientMachineHash::Iterator iMachine = m_ClientMachine.find( clientId );
	if ( iMachine.valid() )
	{
		CharString sMID = *iMachine;

		Array< dword > & clients = m_MachineClient[ sMID ];
		clients.removeSearch( clientId );
		if ( clients.size() == 0 )
		{
			// last client from the same machine has disconnected
			m_MachineClient.remove( sMID );		
		}
		m_ClientMachine.remove( clientId );
	}
	lock.release();

	logoffClient( clientId );
}

//----------------------------------------------------------------------------


bool MetaServer::start( const Context & context )
{
	m_Context = context;

	LOG_STATUS( "MetaServer", "Starting MetaServer, dbname = %s, address = %s, port = %d, maxClients = %d",
		context.dbname.cstr(), context.address.cstr(), context.port, context.maxClients );

	// open the connection to the database
	if (! initializeDB() )
	{
		LOG_ERROR( "MetaServer", "Failed to connect to DB!" );
		return false;
	}

	// start the server
	if (! Server::start( new Socket("ZLIB"), m_Context.port, m_Context.maxClients ) )
		return false;

	// start the update thread
	UpdateThread * pThread = new UpdateThread( this );
	pThread->resume();

	return true;
}

void MetaServer::stop()
{
	LOG_STATUS( "MetaServer", "Stopping MetaServer..." );

	// disconnect all clients and stop listening for connections
	Server::stop();

	Database * pDB = getConnection();

	// remove ourselfs from the DB
	pDB->execute( CharString().format("DELETE FROM servers WHERE server_id = %u", m_ServerId ) );
	pDB->execute( CharString().format("DELETE FROM room_members where server_id = %u", m_ServerId) );
	pDB->execute( CharString().format("DELETE FROM user_status where server_id = %u", m_ServerId) );

	freeConnection( pDB );

	// free all database connection
	freeAllConnections();
}

void MetaServer::sendChat( dword clientId, const char * pMessage )
{
	MetaClient::Chat chat;
	chat.author = "SERVER";
	chat.authorId = 0;
	chat.recpId = 0;
	chat.roomId = 0;
	chat.time = Time::seconds();
	chat.text = pMessage;

	send( clientId, MetaClient::CLIENT_RECV_CHAT ) << chat;
}

void MetaServer::sendGlobalChat( const char * pMessage )
{
	AutoLock lock( &m_Lock );
	for(int i=0;i<clientCount();i++)
		sendChat( client(i), pMessage );
}

//----------------------------------------------------------------------------

// check a string and prepend a \ before every " and ' so that it's stored correctly in the DB
CharString MetaServer::addSlash( const char * pString )
{
	return CharString( pString ).slash();
}

//----------------------------------------------------------------------------

bool MetaServer::initializeDB()
{
	// register ourself in the database
	m_ServerId = registerSelf(); 
	if ( m_ServerId == 0 )
		return false;

	Database * pDB = getConnection();

	// clean up any messes we made in the db if we had crashed
	pDB->execute( CharString().format("delete from room_members where server_id = %u", m_ServerId) );
	pDB->execute( CharString().format("delete from user_status where server_id = %u", m_ServerId) );

	m_LastMessageId = 0;
	m_LastReloadId = 0;
	m_LastReportId = 0;

	// get the last message id
	Database::Query result( pDB->query( "SELECT MAX(message_id) AS last_message from chat") );
	if ( result.size() > 0 )
		m_LastMessageId = result[0][0];
	// get the last reload id
	Database::Query reload( pDB->query( "SELECT MAX(reload_id) as last_reload FROM user_reload") );
	if ( reload.size() > 0 )
		m_LastReloadId = reload[0][0];
	// get the last report id
	Database::Query report( pDB->query( "SELECT MAX(report_id) as last_report FROM reports") );
	if ( report.size() > 0 )
		m_LastReportId = report[0][0];

	// load the word list
	Database::Query words( pDB->query( "SELECT * from words" ) );
	for(int i=0;i<words.size();i++)
		m_Words.push( (const char *)words[i]["word"] );

	// get the fields that should be logged
	Database::Query fields( pDB->query( "SELECT field_id FROM ladder_fields WHERE is_logged != 0" ) );
	for(int i=0;i<fields.size();++i)
        m_LogFieldSet.insert( (dword)fields[i]["field_id"] );

	freeConnection( pDB );

	return true;
}

Database * MetaServer::getConnection()
{
	Database * pDB = NULL;

	lock();

	m_Connections.reset();
	if ( m_Connections.valid() )
	{
		pDB = *m_Connections;
		m_Connections.pop();
	}
	
	unlock();

	// make a new connection if needed..
	if ( pDB == NULL )
	{
		LOG_STATUS( "MetaServer", "Creating new connection to the database..." );

		pDB = Database::create( "DatabaseMYSQL" );
		if ( pDB == NULL )
		{
			LOG_ERROR( "MetaServer", "Failed to create database interface!" );
			abort();
		}
	}

	// loop forever until we have a connection..
	while( !pDB->connected() )
	{
		// try to connect to the DB, if we fail then update this server and keep trying..
		if (! pDB->open( m_Context.dbname, m_Context.dbaddress, m_Context.dbport, m_Context.dbuid, m_Context.dbpw ) )
		{
			LOG_ERROR( "MetaServer", "Failed to open the database!" );
			Thread::sleep( 50 );
		}
	}

	return pDB;
}

void MetaServer::freeConnection( Database * pDB )
{
	AutoLock lock( &m_Lock );
	// cache some connections, reusing them to save some time
	if ( pDB->connected() && m_Connections.size() < m_Context.maxConnections )
		m_Connections.push( pDB );
	else
		Database::release( pDB );
}

void MetaServer::freeAllConnections()
{
	LOG_STATUS( "MetaServer", "Freeing all database connections..." );

	AutoLock lock( &m_Lock );

	m_Connections.reset();
	while( m_Connections.valid() )
	{
		Database * pDB = *m_Connections;
		m_Connections.pop();

		Database::release( pDB );
	}
}

void MetaServer::selectGame( dword nClientId, dword nGameId, dword nJob )
{
	Database * pDB = getConnection();

	bool bSuccess = false;

	Database::Query r( pDB->query( CharString().format("SELECT * FROM games WHERE game_id=%u", nGameId) ) );
	if ( r.rows() > 0 && (((int)r[0]["is_public"]) != 0 || isAdministrator( nClientId)) )
	{
		Game game;
		game.id = r[0]["game_id"];
		game.name = r[0]["name"];
		game.newlogin = r[0]["newlogin_url"];
		game.home = r[0]["home_url"];
		game.download = r[0]["download_url"];
		game.admin = r[0]["admin_url"];
		game.manual = r[0]["manual_url"];
		game.clans = r[0]["clan_url"];
		game.profile = r[0]["profile_url"];
		game.news = r[0]["news_url"];
		game.forum = r[0]["forum_url"];
		game.registry = r[0]["registry"];
		game.address = r[0]["address"];
		game.port = r[0]["port"];
		game.chatRoom = r[0]["chat_room"];

		dword nUserId = getUserId( nClientId );

		Profile profile;
		if ( getProfile( nGameId, nUserId, profile ) )
		{
			lock();
			// save the users selected game
			m_UserGameId[ nUserId ] = nGameId;
			unlock();

			send( nClientId, MetaClient::CLIENT_RECV_SELF ) << profile;
			send( nClientId, MetaClient::CLIENT_RECV_GAME ) << nJob << game;
			send( nClientId, MetaClient::CLIENT_SEND_INSTALLER );

			bSuccess = true;
		}

	}
	freeConnection( pDB );

	// if we failed, send error code back to the client
	if (! bSuccess )
		send( nClientId, MetaClient::CLIENT_JOB_DONE ) << nJob << MetaClient::RESULT_ERROR;
}

bool MetaServer::getShortProfile( dword gameId, dword userId, ShortProfile & profile )
{
	Profile p;
	if (! getProfile( gameId, userId, p ) )
		return false;

	profile.userId = p.userId;
	profile.name = p.name;
	profile.status = p.status;
	profile.flags = p.flags;
	return true;
}

bool MetaServer::getShortProfiles( dword gameId, const Array< dword > & userIds, Array< MetaClient::ShortProfile > & profiles )
{
	profiles.allocate( userIds.size() );
	for(int i=0;i<userIds.size();++i)
	{
		if (! getShortProfile( gameId, userIds[i], profiles[i] ) )
			LOG_ERROR( "MetaServer", "Failed to get short profile for user %u, gameId %u", userIds[i], gameId );
	}
	return true;
}

bool MetaServer::getProfile( dword gameId, dword userId, Profile & profile )
{
	if ( userId == 0 )
		return false;

	AutoLock lock( &m_Lock );
	UserProfileHash::Iterator iGames = m_UserProfile.find( userId );
	if ( iGames.valid() )
	{
		GameProfileHash::Iterator iProfile = (*iGames).find( gameId );
		if ( iProfile.valid() )
		{
			profile = *iProfile;
			return true;
		}
	}
	lock.release();

	// update cached profile for the client
	bool result = false;
	Database * pDB = getConnection();

	Database::Query user( pDB->query( CharString().format("SELECT user_id,username,user_viewemail,user_email,super_admin,is_muted FROM users WHERE user_id = %u", userId) ) );
	if ( user.rows() == 1 )
	{
		profile.userId = user[0][0];
		profile.name = user[0][1];
		if ( ((int)user[0][2]) != 0 )
			profile.email = user[0][3];
		else
			profile.email = "";
		profile.flags = 0;

		if ( ((int)user[0][4]) != 0 )
			profile.flags |= MetaClient::SUPER_ADMIN;
		if ( ((int)user[0][5]) != 0 )
			profile.flags |= MetaClient::MUTED;

		Database::Query game( pDB->query( CharString().format("SELECT * FROM games WHERE game_id=%u", gameId)) );
		if ( game.rows() > 0 )
		{
			if ( ((int)game[0]["is_free"]) != 0 )
				profile.flags |= MetaClient::SUBSCRIBED;

			Database::Query userData( pDB->query( CharString().format("SELECT user_data.release,user_data.beta,user_data.demo,user_data.moderator"
				",user_data.server,user_data.admin,user_data.editor,user_data.event,user_data.developer,user_data.score,user_data.rank "
				"FROM user_data WHERE user_id=%u AND game_id=%u", userId, gameId) ) );
			if ( userData.rows() > 0 )
			{
				profile.flags |= ((int)userData[0][0]) ? MetaClient::SUBSCRIBED : 0;
				profile.flags |= ((int)userData[0][1]) ? MetaClient::BETA : 0;
				profile.flags |= ((int)userData[0][2]) ? MetaClient::DEMO : 0;
				profile.flags |= ((int)userData[0][3]) ? MetaClient::MODERATOR : 0;
				profile.flags |= ((int)userData[0][4]) ? MetaClient::SERVER : 0;
				profile.flags |= ((int)userData[0][5]) ? MetaClient::ADMINISTRATOR : 0;
				profile.flags |= ((int)userData[0][6]) ? MetaClient::EDITOR : 0;
				profile.flags |= ((int)userData[0][7]) ? MetaClient::EVENT : 0;
				profile.flags |= ((int)userData[0][8]) ? MetaClient::DEVELOPER : 0;
				profile.score = userData[0][9];
				profile.rank = userData[0][10];
			}
			else
			{
				// create user data record
				pDB->execute( CharString().format("INSERT INTO user_data(user_id,game_id) VALUES(%u,%u)", userId, gameId) );
				profile.flags |= MetaClient::DEMO;
			}

			// get user ladder fields
			Database::Query fields( pDB->query( CharString().format("SELECT name, field_id FROM ladder_fields WHERE game_id=%u", gameId) ) );
			for(int i=0;i<fields.rows();i++)
			{
				MetaClient::Field field;
				field.name = fields[i][0];
				field.id = fields[i][1];

				profile.fields.push( field );
			}

			// get the ladder field values
			Database::Query values( pDB->query( CharString().format("SELECT field_id,value from user_ladder where user_id=%u", profile.userId) ) );
			for(int k=0;k<profile.fields.size();k++)
			{
				dword nFieldId = profile.fields[k].id;

				bool bFound = false;
				for(int i=0;i<values.rows() && !bFound;++i)
				{
					if ( nFieldId == (dword)values[i][0] )
					{
						profile.fields[k].value = values[i][1];
						bFound = true;
					}
				}

				// if the field wasn't found, insert a record so it can be updated...
				if (! bFound )
					pDB->execute( CharString().format("INSERT INTO user_ladder(user_id,field_id) VALUES(%u,%u)", userId, nFieldId) );
			}
		}

		// get user status
		Database::Query userStatus( pDB->query( CharString().format("SELECT * FROM user_status WHERE user_id=%u", userId) ) );
		if ( userStatus.rows() > 0 )
		{
			profile.status = userStatus[0]["status"];
			profile.flags |= ((int)userStatus[0]["away"]) ? MetaClient::AWAY : 0;
			profile.flags |= ((int)userStatus[0]["hidden"]) ? MetaClient::HIDDEN : 0;
		}
		else
			profile.status = "Offline";

		// get clan information
		Database::Query clanData( pDB->query( CharString().format("SELECT clan_id,is_admin FROM clan_members WHERE user_id=%u AND is_valid=1", userId) ) );
		if ( clanData.rows() > 0 )
		{
			profile.clanId = clanData[0][0];
			profile.flags |= MetaClient::CLAN_MEMBER;
			profile.flags |= ((int)clanData[0][1]) ? MetaClient::CLAN_ADMIN : 0;

			// get the clan short name, then prefix it onto the players name
			Database::Query clanInfo( pDB->query( CharString().format("SELECT name FROM clans WHERE clan_id=%u AND approved=1", profile.clanId) ) );
			if ( clanInfo.rows() > 0 )
			{
				CharString sClanName = (const char *)clanInfo[0][0];
				profile.name = CharString().format("[%s]%s", sClanName.cstr(), profile.name.cstr());
			}
			else
				profile.clanId = 0;
		}
		else
			profile.clanId = 0;

		// check for existing session
		Database::Query session( pDB->query( CharString().format("SELECT sess_id FROM sessions WHERE user_id=%u", userId) ) );
		if ( session.rows() > 0 )
			profile.sessionId = session[0][0];
		else
			profile.sessionId = 0;

		result = true;
	}
	freeConnection( pDB );

	// cache the profile from now on, unless reload_user forces us to flush the data..
	if ( result )
	{
		lock.set( &m_Lock );
		m_UserProfile[ userId ][ gameId ] = profile;
	}

	return result;
}

bool MetaServer::filterProfileForClient( dword nClientId, Profile & profile )
{
	if ( (profile.flags & MetaClient::HIDDEN) != 0 )
	{
		if ( isAdministrator( nClientId ) )
		{
			profile.status = CharString("[HIDDEN]: ") + profile.status;
			return true;
		}

		profile.status = "Offline";		// player is hidden
		return false;
	}

	return true;
}

bool MetaServer::filterProfileForClient( dword nClientId, ShortProfile & profile )
{
	if ( (profile.flags & MetaClient::HIDDEN) != 0 )
	{
		if ( isAdministrator( nClientId ) )
		{
			profile.status = CharString("[HIDDEN]: ") + profile.status;
			return true;
		}

		profile.status = "Offline";		// player is hidden
		return false;
	}

	return true;
}

void MetaServer::filterProfilesForClient( dword nClientId, Array< ShortProfile > & profiles, bool bRemoveHidden /*= false*/ )
{
	for(int i=0;i<profiles.size();)
	{
		// remove hidden players, unless the client is a admin or a moderator but the profile isn't an admin.. got it? :)
		if ( !filterProfileForClient( nClientId, profiles[i] ) && bRemoveHidden )
		{
			profiles.remove( i );
			continue;
		}
		++i;
	}
}

bool MetaServer::flushProfile( dword userId )
{
	AutoLock lock( &m_Lock );
	m_UserProfile.remove( userId );
	return true;
}

bool MetaServer::getFriends( dword userId, Array< dword > & friends )
{
	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT friend_id FROM friends WHERE user_id=%u", userId) ) );

	friends.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		friends[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::getFriends( dword gameId, dword userId, Array< ShortProfile > & friends )
{
	Array< dword > ids;
	if (! getFriends( userId, ids ) )
		return false;

	return getShortProfiles( gameId, ids, friends );
}

bool MetaServer::getClanMembers( dword clanId, Array< dword > & clan )
{
	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT user_id from clan_members WHERE clan_id = %u AND is_valid = 1", clanId) ) );

	clan.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		clan[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::getClanMembers( dword gameId, dword clanId, Array< ShortProfile > & members )
{
	Array< dword > ids;
	if (! getClanMembers( clanId, ids ) )
		return false;

	return getShortProfiles( gameId, ids, members );
}

bool MetaServer::getClanMembersOnline( dword clanId, Array< dword > & clan )
{
	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT clan_members.user_id from clan_members, user_status WHERE"
		" clan_id = %u AND clan_members.user_id = user_status.user_id AND clan_members.is_valid = 1", clanId) ) );

	clan.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		clan[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::getClanIdByName( CharString clanName, dword & clanId )
{
	Database * pDB = getConnection();
	Database::Query result( pDB->query( CharString().format("SELECT clan_id from clans WHERE name='%s'", addSlash(clanName).cstr() ) ) );

	if ( result.rows() == 1 )
		clanId = result[0]["clan_id"];
	else
		clanId = 0;
	
	freeConnection( pDB );
	return true;			
}

bool MetaServer::getClanName(dword clientId, dword clanId, CharString & clanName)
{
	Database * pDB = getConnection();
	Database::Query result( pDB->query( CharString().format("SELECT * from clans WHERE clan_id=%u", clanId) ) );

	if ( result.rows() == 1 )
		clanName = result[0]["name"];
	
	freeConnection( pDB );
	return true;			
}

bool MetaServer::getRoomMembers( dword gameId, dword roomId, Array< ShortProfile > & members )
{
	AutoLock lock( &m_Lock );
	RoomUserHash::Iterator iUsers = m_RoomUsers.find( roomId );
	if (! iUsers.valid() )
		return false;		// room not found

	// copy the room members from the hash..
	Array< dword > roomUsers = *iUsers;
	lock.release();

	return getShortProfiles( gameId, roomUsers, members );
}

bool MetaServer::getModeratorsInRoom( dword roomId, dword gameId, Array< dword > & mods )
{
	Array< ShortProfile > members;
	if (! getRoomMembers( gameId, roomId, members ) )
		return false;

	mods.release();
	for(int i=0;i<members.size();++i)
		if ( (members[i].flags & MetaClient::MODERATOR) !=0 )
			mods.push( members[i].userId );

	return true;
}

bool MetaServer::getModeratorsOnline( dword gameId, Array< dword > & mods )
{
	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT DISTINCT sessions.user_id FROM sessions, user_data "
				"WHERE sessions.user_id = user_data.user_id AND sessions.mid IS NOT NULL "
				"AND user_data.moderator = 1 AND user_data.game_id = %u", gameId) ) );

	mods.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		mods[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::getModeratorsOnline( dword gameId, Array< ShortProfile > & mods )
{
	Array<dword> userIds;
	if (! getModeratorsOnline( gameId, userIds ) )
		return false;

	return getShortProfiles( gameId, userIds, mods );
}

bool MetaServer::getStaffOnline( dword gameId, Array< dword > & staff )
{

	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT DISTINCT sessions.user_id FROM sessions, user_data "
				"WHERE sessions.user_id = user_data.user_id AND sessions.mid IS NOT NULL "
				"AND (user_data.moderator=1 OR user_data.developer=1 OR user_data.admin=1) "
				"AND user_data.game_id = %u", gameId) ) );

	staff.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		staff[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::getDevelopersOnline( dword gameId, Array< dword > & devs )
{
	Database * pDB = getConnection();
	Database::Query rows( pDB->query( CharString().format("SELECT DISTINCT sessions.user_id FROM sessions, user_data "
				"WHERE sessions.user_id = user_data.user_id AND sessions.mid IS NOT NULL "
				"AND user_data.developer=1 "
				"AND user_data.game_id = %u", gameId) ) );

	devs.allocate( rows.size() );
	for(int i=0;i<rows.size();i++)
		devs[i] = rows[i][0];

	freeConnection( pDB );
	return true;
}

bool MetaServer::findUser( const char * pPattern, Array< dword > & found )
{
	Database * pDB = getConnection();
	if ( pPattern[0] == '@' )
	{
		dword userId = strtoul( pPattern + 1, NULL, 10 );

		// make sure user does exist
		Database::Query row( pDB->query( CharString().format("SELECT user_id FROM users WHERE user_id = %u", userId) ) );
		if ( row.size() == 1 )
			found.push( userId );
	}
	else
	{
		if ( strlen( pPattern ) >= 4 )
		{
			Database::Query users( pDB->query( CharString().format("SELECT user_id FROM users WHERE username LIKE '%%%s%%'", addSlash( pPattern ).cstr() ) ) );
			for(int i=0;i<users.rows();i++)
				found.push( users[i][0] );
		}
	}

	freeConnection( pDB );
	return found.size() > 0;
}

bool MetaServer::findUserExactFirst( const char * pPattern, Array< dword > & found )
{
	if ( pPattern[0] == '@' )
		return findUser( pPattern, found );

	Database * pDB = getConnection();
	Database::Query users( pDB->query( CharString().format("SELECT user_id FROM users WHERE username = '%s'", addSlash( pPattern ).cstr() ) ) );
	if( users.rows() == 1 )
		found.push( users[0][0] );
	freeConnection( pDB );

	if ( found.size() == 0 )
		return findUser( pPattern, found );

	return true;
}

dword MetaServer::banUser( dword who, dword userId, dword duration, const char * pWhy )
{
	CharString MID, IP;
	dword banId = 0;

	Database * pDB = getConnection();

	// get the MID and IP address if possible from the users session information
	Database::Query session( pDB->query( CharString().format( "SELECT mid, remote_ip from sessions where user_id=%u", userId ) ) );
	if ( session.size() > 0 )
	{
		MID = (const char *)session[0][0];
		IP = (const char *)session[0][1];
	}
	
	if( MID.length() == 0 )		// if there was no session or the session had no MID (forum login)
	{							// then try to retrieve the last_mid from the users table so it can be used for the ban
		Database::Query users( pDB->query( CharString().format( "SELECT last_mid from users where user_id=%u", userId ) ) );
		if ( users.size() > 0 )
			MID = (const char *)users[0][0];
	}

	// add the ban record
	pDB->execute( CharString().format("INSERT banlist(ban_userid,ban_ip,ban_machine,ban_start,ban_end,ban_time_type,ban_from,ban_why)"
		"VALUES(%u,'%s','%s',unix_timestamp(),(unix_timestamp() + %u),4,%u,'%s')",
		userId, IP.cstr(), MID.cstr(), duration, who, addSlash( pWhy ).cstr() ) );
	banId = pDB->insertId();

	freeConnection( pDB );

	LOG_STATUS( "MetaServer", CharString().format("User %u banned user %u, for %u seconds...%s", who, userId, duration, pWhy) );
	return banId;
}

/**
 * who:			userId of the user to add to the list
 * userId:		userId of the person who adds the user
 * watch_type:	0 - watch, 1 - kick, 2 - ban
 * pWhy:		reason for adding
 **/
bool MetaServer::addWatchList( dword who, dword userId, dword watch_type, const char *pWhy, dword banId )
{
	ASSERT( watch_type < 3 );

	bool added = false;

	Database * pDB = getConnection();
	// get username by userId, verify user exists
	Database::Query idToName( pDB->query( CharString().format( "SELECT username from users where user_id=%u", who ) ) );
	if ( idToName.size() > 0 )
	{
		CharString name = (const char *)idToName[0][0];

		// get the MID and IP address if possible from the users session information
		Database::Query session( pDB->query( CharString().format( "SELECT mid, remote_ip from sessions where user_id=%u", who ) ) );
		if ( session.size() > 0 )
		{
			CharString MID, IP;
			MID = (const char *)session[0][0];
			IP = (const char *)session[0][1];
			
			// add the watch record
			pDB->execute( CharString().format("INSERT watchlist(userid,username,addedby,addedtime,watch_type,reason,usermachine,userip,banid)"
				"VALUES(%u,'%s',%u,unix_timestamp(),%u,'%s','%s','%s',%u)",
				who, addSlash(name).cstr(), userId, watch_type, addSlash( pWhy ).cstr(), MID.cstr(), IP.cstr(), banId ) );
		}
		else	// no session info, so add with MID/IP = NULL
		{
			pDB->execute( CharString().format("INSERT watchlist(userid,username,addedby,addedtime,watch_type,reason,banid)"
				"VALUES(%u,'%s',%u,unix_timestamp(),%u,'%s',%u)",
				who, addSlash(name).cstr(), userId, watch_type, addSlash( pWhy ).cstr(), banId ) );
		}

		added = true;
	}
	freeConnection( pDB );

	return added;
}


dword MetaServer::registerSelf()
{
	// register ourself into the database
	MetaClient::Server myself;
	myself.gameId = m_Context.gameId; 
	myself.type = MetaClient::META_SERVER;
	myself.flags = 0;
	myself.name = "MetaServer"; //CharString().format("%s:%u", m_Context.address, m_Context.port );
	myself.shortDescription = "";
	myself.description = "";
	myself.address = m_Context.address;
	myself.port = m_Context.port;
	myself.maxClients = m_Context.maxClients;
	myself.clients = clientCount();
	myself.lastUpdate = Time::seconds();

	return registerServer( 0, myself );
}

dword MetaServer::registerServer( dword clientId, MetaClient::Server & server )
{
	server.id = 0;
	server.lastUpdate = Time::seconds();

	if ( clientId != 0 )
	{
		AutoLock lock( &m_Lock );
		server.flags = getFlags( clientId );

		// get the clients socket address
		if ( server.address.length() == 0 )
			server.address = clientAddress(clientId);
		// set the game id if not provided..
		if ( server.gameId == 0 )
			server.gameId = getGameId( clientId );
	}

	Database * pDB = getConnection();

	Database::Query result( pDB->query( CharString().format("SELECT server_id FROM servers WHERE address = '%s' AND port = %u", 
		addSlash( server.address ).cstr(), server.port) ) );

	if ( result.rows() > 0 )
	{
		server.id = result[0][0];
		
		// using StringBuffer instead of CharString to circumvent the 128kb format string limit
		CharString query;
		query.format( "UPDATE servers SET type=%u,flags=%u,name='%s',short_description='%s',description='%s',"
			"address='%s',port=%u,max_clients=%u,clients=%u,last_updated=unix_timestamp(), game_id=%u, data='",
			server.type, server.flags, 
			addSlash( server.name ).cstr(),
			addSlash( server.shortDescription ).cstr(),
			addSlash( server.description ).cstr(),
			addSlash( server.address ).cstr(), 
			server.port, 
			server.maxClients, server.clients, 
			server.gameId );
		
		// server.data might be longer than 128kb, so it is not used in a format() function
		query += CharString( server.data ).slash();
		query += CharString().format( "', mid='%s' WHERE server_id=%u", getMID( clientId ).cstr(), server.id );

		//LOG_STATUS( "MetaServer", query );
		pDB->execute( query );
	}
	else
	{
		// using StringBuffer instead of CharString to circumvent the 128kb format string limit
		CharString query;
		query.format("INSERT INTO servers(type,flags,name,short_description,description,address,port,max_clients,clients,last_updated,game_id,data,mid)"
			" VALUES(%u,%u,'%s','%s','%s','%s',%u,%u,%u,unix_timestamp(),%u,'",
			server.type, server.flags, 
			addSlash( server.name ).cstr(), 
			addSlash( server.shortDescription ).cstr(),
			addSlash( server.description ).cstr(),
			addSlash( server.address ).cstr(), 
			server.port, 
			server.maxClients, server.clients, 
			server.gameId );

		// server.data might be longer than 128kb, so it is not used in a format() function
		query += CharString( server.data ).slash();
		query += CharString().format( "','%s')", getMID( clientId ).cstr() );

		//LOG_STATUS( "MetaServer", query );
		pDB->query( query );
		if ( pDB->success() )
			server.id = pDB->insertId();
	}

	freeConnection( pDB );

	if ( server.id != 0 )
	{
		if ( clientId != 0 )
		{
			AutoLock lock( &m_Lock );
			m_ClientServer[ clientId ] = server.id;
			m_ServerClient[ server.id ] = clientId;
		}

		LOG_STATUS( "MetaServer", CharString().format("Registering server, serverId: %u, name: %s, data.length = %u, Address: %s, Client: %u (%s)", 
			server.id, server.name.cstr(), server.data.length(), server.address.cstr(), clientId, clientAddress( clientId ) ) );

	}
	else
		LOG_ERROR( "MetaServer", CharString().format("Client %u Failed to register server...", clientId) );


	return server.id;
}

void MetaServer::removeServer( dword clientId )
{
	dword serverId = 0;

	AutoLock lock( &m_Lock );
	
	Hash< dword, dword>::Iterator server = m_ClientServer.find( clientId );
	if ( server.valid() )
	{
		serverId = *server;
		m_ClientServer.remove( clientId );
		m_ServerClient.remove( serverId );

		LOG_STATUS( "MetaServer", CharString().format("Removed server, serverId = %u, clientId = %u", serverId, clientId) );
	}

	lock.release();

	if ( serverId != 0 )
	{
		Database * pDB = getConnection();
		pDB->execute( CharString().format("DELETE FROM servers WHERE server_id = %u", serverId ) );
		freeConnection( pDB );
	}
}

void MetaServer::loginClient( dword clientId, dword job, dword userId, const char * pMID )
{
	// get the client IP address
	lock();
	CharString sAddress( clientAddress(clientId) );
	unlock();

	// sometimes the userId will be 0 if the client is using an outdated session ID and get's disconnected from the MetaServer..
	if ( userId != 0 )
	{
		// get the class DLL C subnet from the IP address
		CharString sSubnet( sAddress );
		sSubnet.left( sSubnet.reverseFind( '.' ) );

		CharString sQuery;
		sQuery = "SELECT ban_end, ban_why FROM banlist WHERE (";
		sQuery += CharString().format( "ban_userid=%u OR ban_machine='%s' OR ban_ip='%s' OR ban_ip='%s'", 
			userId, pMID, sAddress.cstr(), sSubnet.cstr() );
		sQuery += ") AND ban_end > unix_timestamp() ORDER BY ban_end DESC";

		Database * pDB = getConnection();

		Database::Query banned( pDB->query( sQuery ) );
		if ( banned.rows() == 0 )
		{
			// No ban record found, so continue to login this user...
			Profile profile;
			if ( getProfile( m_Context.gameId, userId, profile ) )
			{
				bool bCreateSession = true;
				if ( profile.sessionId != 0 )
				{
					// verify the session record..
					Database::Query qSession( pDB->query( CharString().format( "SELECT user_id from sessions WHERE sess_id=%u", profile.sessionId ) ) );
					if ( qSession.size() > 0 )
					{
						if ( userId == (dword)qSession[0]["user_id"] )
						{
							// found an existing session, just update it..
							bCreateSession = false;
							// user already has a session, update the session
							pDB->execute( CharString().format("UPDATE sessions SET start_time=unix_timestamp(), remote_ip='%s', proxy_ip='', mid='%s' WHERE sess_id=%u",
								clientAddress(clientId), pMID, profile.sessionId) );
						}
					}
				}

				if ( bCreateSession )
				{
					bool sessionCreated = false;
					while(! sessionCreated )
					{
						srand( Time::milliseconds() );
						profile.sessionId = rand() | (rand() << 16);

						pDB->query( CharString().format("INSERT INTO sessions(sess_id,user_id,start_time,remote_ip,mid) VALUES(%u,%u,unix_timestamp(),'%s','%s')",
							profile.sessionId, userId, clientAddress( clientId ), pMID ) );
						sessionCreated = pDB->success();
					}

					// update the in-memory profile with the new sessionId
					lock();
					GameProfileHash & gameProfiles = m_UserProfile[ profile.userId ];
					GameProfileHash::Iterator iProfile = gameProfiles.head();
					while( iProfile.valid() )
					{
						(*iProfile).sessionId = profile.sessionId;
						iProfile.next();
					}
					unlock();
				}

				// update the last login time
				pDB->execute( CharString().format("UPDATE users SET last_login=unix_timestamp(), last_ip='%s', last_mid='%s' WHERE user_id=%u", 
					clientAddress(clientId), pMID, userId) );
				// update the user status
				pDB->execute( CharString().format("INSERT IGNORE INTO user_status(user_id,status,updated,server_id,away,hidden) VALUES(%u,'Online',unix_timestamp(),%u,0,%u)",
					userId, m_ServerId, (profile.flags & MetaClient::ADMINISTRATOR) ? 1 : 0 ) );

				lock();
				// keep the clients machine id
				m_ClientMachine[ clientId ] = pMID;
				m_MachineClient[ pMID ].push( clientId );
				// register this user with this client
				m_ClientUser[ clientId ] = userId;
				m_ClientSession[ clientId ] = profile.sessionId;
				m_UserClient[ userId ].push( clientId );
				// store the clients profile in a hash table
				m_UserGameId[ userId ] = m_Context.gameId;
				unlock();

				LOG_STATUS( "MetaServer", CharString().format("Login client %u user %s, userId = %u, sessionId = %u, IP = %s, MID = %s", 
					clientId, profile.name.cstr(), profile.userId, profile.sessionId, sAddress.cstr(), pMID ) );


				// check for offline messages
				Database::Query messages( pDB->query( 
					CharString().format("SELECT * FROM chat WHERE message_id <= %u AND recp_id=%u AND sent=0 ORDER BY message_id", 
					m_LastMessageId, userId) ) );
				if ( messages.size() > 0 )
				{
					sendChat( clientId, CharString().format("/<b>You have %u offline messages...</b>", messages.size()) );

					// new messages, route the new messages to the clients
					for(int i=0;i<messages.size();i++)
					{
						MetaClient::Chat chat;
						chat.author		= messages[i]["author"];
						chat.authorId	= messages[i]["author_id"];
						chat.time		= messages[i]["time"];
						chat.text		= messages[i]["text"];
						chat.recpId		= messages[i]["recp_id"];
						chat.roomId		= messages[i]["room_id"];

						dword messageId = messages[i]["message_id"];		// save the last process message ID

						// route chat message
						send( clientId, MetaClient::CLIENT_RECV_CHAT) << chat;
					}

					// mark all the messages as sent
					pDB->execute( CharString().format("UPDATE chat SET sent=1 WHERE message_id <= %u AND recp_id = %u", 
						m_LastMessageId, userId ) );
				}

				// send the message of the day
				char * pMOTD = FileDisk::loadTextFile( m_Context.motdFile );
				if ( pMOTD != NULL )
				{
					sendChat( clientId, CharString( pMOTD ) );
					delete pMOTD;
				}

				// send client their login message
				send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_OKAY << profile;

				// if the user to already joined to a room, send their new client the room
				lock();
				UserRoomHash::Iterator iRoom = m_UserRoom.find( userId );
				if ( iRoom.valid() )
				{
					Array< dword > & rooms = *iRoom;

					for(int i=0;i<rooms.size();++i)
					{
						dword nRoomID = rooms[i];
						dword nGameID = m_RoomGame[ nRoomID ];

						Array< ShortProfile > players;
						getRoomMembers( nGameID, nRoomID, players );
						filterProfilesForClient( clientId, players, true );

						send( clientId, MetaClient::CLIENT_ADD_ROOM ) << nRoomID << players;
					}
				}
				unlock();
			}
			else
			{
				LOG_STATUS( "MetaServer", "Failed to login client %u, Failed to get profile for %u IP: %s, MID: %s", clientId, userId, sAddress.cstr(), pMID );
				send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_ERROR;
			}

		}
		else
		{
			LOG_STATUS( "MetaServer", "Rejecting client %u, user %u is banned! IP: %s, MID: %s", clientId, userId, sAddress.cstr(), pMID );
			send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_BANNED;
		}

		freeConnection( pDB );
	}
	else
	{
		LOG_STATUS( "MetaServer", "Failed to login client %u, userId is 0 for %u IP: %s, MID: %s", clientId, userId, sAddress.cstr(), pMID );
		send( clientId, MetaClient::CLIENT_LOGIN ) << job << MetaClient::version() << MetaClient::LOGIN_ERROR;
	}
}

bool MetaServer::isAdministrator( dword clientId )
{
	return (getFlags( clientId ) & MetaClient::ADMINISTRATOR) != 0;
}

bool MetaServer::isModerator( dword clientId )
{
	return (getFlags( clientId ) & MetaClient::MODERATOR) != 0;
}

bool MetaServer::isServer( dword clientId )
{
	return (getFlags( clientId ) & MetaClient::SERVER) != 0;
}

bool MetaServer::isRoomModerated( dword roomId )
{
	AutoLock lock( &m_Lock );
	return m_RoomModerated.find( roomId ) != m_RoomModerated.end();
}

bool MetaServer::validateClient( dword clientId, dword flags /*= 0*/ )
{
	AutoLock lock( &m_Lock );

	ClientUserHash::Iterator iUser = m_ClientUser.find( clientId );
	if ( iUser.valid() && (flags == 0 || (getFlags( clientId ) & flags) != 0) )
		return true;

	LOG_STATUS( "MetaServer", CharString().format("Failed validation, Client %u (%s)", clientId, clientAddress(clientId) ) );

	removeClient( clientId );
	return false;
}

dword MetaServer::getGameId( dword clientId )
{
	AutoLock lock( &m_Lock );

	dword nGameId = 0;
	ClientUserHash::Iterator iUser = m_ClientUser.find( clientId );
	if ( iUser.valid() )
	{
		UserGameId::Iterator iGame = m_UserGameId.find( *iUser );
		if ( iGame.valid() )
			nGameId = *iGame;
	}

	return nGameId;
}

bool MetaServer::getProfile( dword clientId, MetaClient::Profile & profile )
{
	return getProfile( getGameId( clientId ), getUserId( clientId ), profile );
}

dword MetaServer::getUserId( dword clientId )
{
	AutoLock lock( &m_Lock );

	ClientUserHash::Iterator iUser = m_ClientUser.find( clientId );
	if ( iUser.valid() )
		return *iUser;

	return 0;
}

CharString MetaServer::getUserName( dword clientId )
{
	ShortProfile profile;
	if ( getShortProfile( getGameId( clientId ), getUserId( clientId ), profile ) )
		return profile.name;

	return CharString("?");
}

dword MetaServer::getFlags( dword clientId )
{
	ShortProfile profile;
	if ( getShortProfile( getGameId( clientId ), getUserId( clientId ), profile ) )
		return profile.flags;

	return 0;
}

CharString MetaServer::getPublicKey( dword clientId )
{
	AutoLock lock( &m_Lock );
	if (! m_ClientKey.find( clientId ).valid() )
		m_ClientKey[ clientId ] = UniqueNumber().string();//CharString().format("%8.8x", rand() );

	return m_ClientKey[ clientId ];
}

CharString MetaServer::getMID( dword clientId )
{
	AutoLock lock( &m_Lock );
	if ( m_ClientMachine.find( clientId ).valid() )
		return m_ClientMachine[ clientId ];

	return CharString();
}

void MetaServer::logoffClient( dword clientId )
{
	LOG_STATUS( "MetaServer", "Logging off client %u (%s)", clientId, getUserName( clientId ).cstr() );

	lock();

	ClientUserHash::Iterator iUser = m_ClientUser.find( clientId );
	if ( iUser.valid() )
	{
		dword nUserId = *iUser;
		m_ClientUser.remove( clientId );
		m_ClientSession.remove( clientId );

		Array< dword > & clients = m_UserClient[ nUserId ];
		dword nClientCount = clients.size();
		clients.removeSearchSwap( clientId );

		if ( clients.size() == 0 )
		{
			LOG_DEBUG_HIGH( "MetaServer", "Last client disconnected for user %u, removing from all rooms and status.", nUserId );

			// last client connection for this user, clean up the user...
			m_UserClient.remove( nUserId );
			m_UserProfile.remove( nUserId );
			m_UserGameId.remove( nUserId );
			m_UserRoom.remove( nUserId );

			unlock();

			Database * pDB = getConnection();
			// remove client from any chat rooms they may have joined
			pDB->execute( CharString().format("DELETE FROM room_members WHERE user_id = %u", nUserId) );
			// remove client status record, so that they appear Offline
			pDB->execute( CharString().format("DELETE from user_status WHERE user_id = %u", nUserId) );

			freeConnection( pDB );

			lock();
		}		
		else
		{
			LOG_DEBUG_HIGH( "MetaSeerver", "User %u still has %u (%u) client connections.", 
				nUserId, clients.size(), nClientCount );
		}
	}
	else
	{
		LOG_DEBUG_HIGH( "MetaServer", "No user found for client %u", clientId );
	}

	// remove any server the user may have registered
	if ( m_ClientServer.find( clientId ).valid() )
	{
		dword serverId = m_ClientServer[ clientId ];

		// remove from hash table
		m_ClientServer.remove( clientId );
		m_ServerClient.remove( serverId );

		unlock();

		// remove server entry from the database
		Database * pDB = getConnection();
		pDB->execute( CharString().format("DELETE FROM servers WHERE server_id = %u", serverId ) );
		freeConnection( pDB );

		LOG_STATUS( "MetaServer", CharString().format("Removed server, serverId = %u, client = %u", serverId, clientId) );
		return;
	}

	// done
	unlock();
}

bool MetaServer::validateName( const char * pName )
{
	CharString name( pName );
	name.lower();
	
	// name must not exceed tablespace ( no OK for the client if username was stored incorrectly )
	if ( name.length() > 35 )
		return false;
	
	if( name.length() < 3 )
		return false;

	// No 'forbidden' words
	for(int i=0;i<m_Words.size();i++)
		if ( strstr<char>( name, m_Words[i] ) != NULL )
			return false;
	
	// Only printable characters
	for(int i=0;i<name.length();i++)
		if ( ( !isprint( name[i] ) ) || name[i] == '%' )	// % is wildcard for the DB
			return false;
	
	// @ as first char is reserved for UserIDs
	if( name[0] == '@' )
		return false;

	// First and last character must not be a blankspace (prevents faking other users names)
	if( ( name[0] == ' ' ) || ( name[name.length()-1] == ' ' ) )
		return false;
	
	// Prevent HTML tags in playernames
	if( ( name.find('<') >= 0 ) || ( name.find('>') >= 0 ) )
		return false;
	// prevent fake clans
	if( ( name.find('[') >= 0 ) || ( name.find(']') >= 0 ) )
		return false;

	// BBcode isn't allowed also
	if( ( strstr<char>( name, "[b]" ) != NULL ) || ( strstr<char>( name, "[/b]" ) != NULL )
		|| ( strstr<char>( name, "[i]" ) != NULL ) || ( strstr<char>( name, "[/i]" ) != NULL )
		|| ( strstr<char>( name, "[color=" ) != NULL ) || ( strstr<char>( name, "[/color]" ) != NULL ) )
		return false;

	// Prevention of clanfaking, disallow escape codes for [ ]
	if( ( strstr<char>( name, "&#91" ) != NULL ) || ( strstr<char>( name, "&#93" ) != NULL ) )
		return false;

	// Prevention of escaped blankspaces
	if( strstr<char>( name, "&nbsp" ) != NULL || strstr<char>( name, "&#32" ) != NULL )
		return false;

	// name must contain at least 2 letters ( a - z ), this includes the 'local' variants of these
	int nLetters = 0;
	for(int i=0;i<name.length();i++)
		if ( ( ( name[i] >= 'a' ) && ( name[i] <= 'z' ) ) || ( ( name[i] >= 128 ) && ( name[i] <= 151 ) )
			|| ( ( name[i] >= 160 ) && ( name[i] <= 164 ) ) )
			nLetters++;
	
	if( nLetters < 2 )
		return false;
	
	CharString reservedWords1[] = { "darkspace", "palestar", "admin", "transport", "" };
	for(int i = 0; reservedWords1[i] != ""; i++ )
		if( name.find( reservedWords1[i] ) >= 0 )
			return false;

	CharString reservedWords2[] = { "everyone", "anyone", "you", "server", "" };
	for(int i = 0; reservedWords2[i] != ""; i++ )
		if( name.beginsWith( reservedWords2[i] ) )
			return false;

	return true;
}

bool MetaServer::validateMID( const char * pMID )
{
	if( pMID == 0 )	// zero length MID ?
		return false;

	int len = strlen( pMID );
	if( len > 50 )	// MID too long
		return false;

	for( int i = 0 ; i < len ; i++ )	// MID must be a lower case hex value
		if( ( pMID[i] < '0' || pMID[i] > '9' ) && ( pMID[i] < 'a' || pMID[i] > 'f' ) )
			return false;

	return true;
}

//----------------------------------------------------------------------------

void MetaServer::processMessage( dword clientId, dword roomId, dword recpId, const char * pText, bool echo /*= false*/ )
{
	// copy the chat text and convert to lower case
	CharString username = getUserName( clientId );
	dword flags = getFlags( clientId );
	bool is_moderator = (flags & MetaClient::MODERATOR) != 0;
	bool is_admin = (flags & MetaClient::ADMINISTRATOR) != 0;
	bool is_muted = (flags & MetaClient::MUTED) != 0;

	// if room is moderated, then everyone is muted by default..
	if ( (roomId != 0 && isRoomModerated( roomId ) && !is_admin && !is_moderator) || is_muted )
	{
		Database * pDB = getConnection();

		// client is currently muted, send message to moderators in room only
		Database::Query moderators( pDB->query( CharString().format("SELECT room_members.user_id FROM rooms, room_members, user_data "
			"WHERE rooms.room_id=room_members.room_id AND room_members.user_id=user_data.user_id "
			"AND user_data.game_id=rooms.game_id AND moderator=1 AND rooms.room_id=%u", roomId ) ) );

		freeConnection( pDB );

		// emote ?
		if( pText[0] == '/' )
		{
			for(int i=0;i<moderators.rows();i++)
			{
				processMessageInternal( clientId, username, 0, moderators[i][0], 
					CharString().format( "/<font color=ff1030><b>[%s]</b></font> %s", is_muted ? "MUTED" : "MODERATED", pText + 1 ), echo );
			}
		}
		else
		{
			for(int i=0;i<moderators.rows();i++)
			{
				processMessageInternal( clientId, 
					CharString().format( "<font color=ff1030><b>[%s]</b></font> %s", is_muted ? "MUTED" : "MODERATED", username.cstr() ), 0, moderators[i][0], pText, echo );
			}
		}
	}
	else
	{
		// chat.recpId = the userId of the recipent of this chat message, zero for non private messages
		// chat.roomId = the roomId of this chat message
		// if recpId and roomId are both zero, then this is a global message to all clients, requires the MODERATOR flag
		if ( recpId != 0 || roomId != 0 || (flags & (MetaClient::MODERATOR|MetaClient::SERVER)) != 0 )
			processMessageInternal( clientId, username, roomId, recpId, pText, echo );
	}
}

// Internal use only, preconditions according to processMessage() have to be checked before usage !
void MetaServer::processMessageInternal( dword clientId, const char * pAuthorName, dword roomId, dword recpId, const char * pText, bool echo /*= false*/ )
{
	Database * pDB = getConnection();

	pDB->execute( CharString().format("INSERT chat(author,author_id,time,text,recp_id,room_id) values('%s',%u,unix_timestamp(),'%s',%u,%u)",
		addSlash( pAuthorName ).cstr(), echo ? 0 : getUserId( clientId ), 
		addSlash( pText ).cstr(), recpId, roomId ) );

	freeConnection( pDB );
}

void MetaServer::processChat( dword clientId, dword roomId, dword recpId, const char * pText )
{
	LOG_STATUS( "MetaServer", "%s (@%u) sent: %s", getUserName( clientId ).cstr(), getUserId( clientId ), pText );

	if ( pText[0] == '/' && recpId == 0 )
		processCommand( clientId, roomId, recpId, pText );
	else
		processMessage( clientId, roomId, recpId, pText );
}

//---------------------------------------------------------------------------------------------------

static CharString GetElaspedTime( int a_nSeconds )
{
	if ( a_nSeconds > 3600 )
		return CharString().format( "%.1f hours", a_nSeconds / 3600.0f );
	else if ( a_nSeconds > 60 )
		return CharString().format( "%u minutes", a_nSeconds / 60 );
	else 
		return CharString().format( "%u seconds", a_nSeconds );
}

//----------------------------------------------------------------------------

const dword UPDATE_PING_CLIENTS = 10;
const dword UPDATE_BANNED_TIME = 30;
const dword UPDATE_SERVERS_TIME = 5 * 60;
const dword UPDATE_SESSIONS_TIME = 60 * 60;
const dword UPDATE_CHAT_TIME = 5;
const dword UPDATE_ROOMS_TIME = 10 * 60;
const dword UPDATE_IGNORES_TIME = 30 * 60;
const dword UPDATE_PLAYERS_TIME = 5;
const dword UPDATE_EVENTS_TIME = 60;
const dword UPDATE_PROFILES_TIME = 5;
const dword UPDATE_REPORTS_TIME = 30;
const dword UPDATE_WIDGETS_TIME = 30;

const dword CHAT_MESSAGE_DURATION = 60 * 30;							// how long to keep chat messages in the DB
const dword OFFLINE_MESSAGE_DURATION = ((60 * 60) * 24) * 3;		// 3 days for offline messages
const dword SESSION_DURATION = (60 * 60) * 12;						// duration of data in the sessions table
const dword SERVER_DURATION = 15 * 60;								// duration of data in the servers table
const dword REPORT_DURATION = ((60 * 60) * 24) * 7;					// remove reports after 7 days
const dword	RELOAD_DURATION = 60 * 15;								// how long to leave data in the user_reload table
const dword WIDGET_DURATION = 60 * 15;								// remove widgets after this amount of time in the database

const dword CLIENT_PING_TIME = 60;					// how often to ping clients
const dword CLIENT_DISCONNECT_TIME = 60 * 5;		// time to disconnect client if not responding to pings

void MetaServer::updateDemon()
{
	TRACE( "MetaServer::updateDemon starting" );
	
	dword time = Time::seconds();
	dword pingClients = time + UPDATE_PING_CLIENTS;
	dword updateBanned = time + UPDATE_BANNED_TIME;	
	dword updateServer = time;
	dword updateSessions = time + UPDATE_SESSIONS_TIME;
	dword updateChat = time + UPDATE_CHAT_TIME;
	dword updateRooms = time + UPDATE_ROOMS_TIME;
	dword updatePlayers = time + UPDATE_PLAYERS_TIME;
	dword updateIgnores = time + UPDATE_IGNORES_TIME;
	dword updateEvents = time + UPDATE_EVENTS_TIME;
	dword updateProfiles = time + UPDATE_PROFILES_TIME;
	dword updateReports = time + UPDATE_REPORTS_TIME;

	while ( running() )
	{
		Thread::sleep( 250 );

		Database * pDB = getConnection();

		time = Time::seconds();
		if ( time > pingClients )
		{
			AutoLock lock( &m_Lock );

			//TRACE( "Pinging clients..." );

			// ping the clients
			Array< dword > dead;
			for(int i=0;i<clientCount();i++)
			{
				dword clientId = client(i);
				if ( m_ClientPong.find( clientId ).valid() )
				{
					dword lastPong = m_ClientPong[ clientId ];
					dword timeElapsed = time - lastPong;

					if ( timeElapsed > CLIENT_DISCONNECT_TIME )
					{
						LOG_STATUS( "MetaServer", "Disconnecting dead client %u, timeElapsed = %u, lastPong = %u", clientId, timeElapsed, lastPong );
						dead.push( clientId );
					}
					else if ( timeElapsed >= CLIENT_PING_TIME )
						send( clientId, MetaClient::PING ) << time;
				}
			}

			for(int i=0;i<dead.size();i++)
			{
				disconnect( dead[i] );
			}
			
			pingClients = Time::seconds() + UPDATE_PING_CLIENTS;
		}
		if ( time > updateBanned )
		{
			TRACE( "Checking banned list..." );

			// disconnect newly banned users
			CharString query;
			query.format( "SELECT ban_userid,ban_id,ban_machine,ban_ip from banlist WHERE ban_id > %u AND ban_end > %u ORDER BY ban_id", 
				m_LastBanId, Time::seconds() );

			// enumerate through newly banned users
			Database::Query banned( pDB->query( query ) );
			for(int i=0;i<banned.rows();i++)
			{
				dword userId = banned[i][0];
				dword banId = banned[i][1];
				CharString sMID = (const char *)banned[i][2];
				CharString banIp = (const char *)banned[i][3];

				AutoLock lock( &m_Lock );

				UserClientHash::Iterator iClients = m_UserClient.find( userId );
				if ( iClients.valid() )
				{
					Array< dword > clients = *iClients;
					for(int k=0;k<clients.size();++k)
					{
						dword clientId = clients[k];

						// disconnect the user
						LOG_STATUS( "MetaServer", CharString().format("Disconnecting client %u, user is banned", clientId) );
						removeClient( clientId );
					}
				}
				
				MachineClientTree::Iterator iMachines = m_MachineClient.find( sMID );
				if ( iMachines.valid() )
				{
					Array< dword > clients = *iMachines;
					for(int k=0;k<clients.size();++k)
					{
						dword clientId = clients[k];
						LOG_STATUS( "MetaServer", CharString().format("Disconnecting client %u, machine is banned", clientId) );
						removeClient( clientId );
					}
				}

				if ( banIp.length() > 0 )
				{
					for(int k=0;k<clientCount();k++)
					{
						dword clientId = client(k);

						CharString address = clientAddress( clientId );
						CharString subnet = address;
						subnet.left( subnet.reverseFind( '.' ) );

						if ( address == banIp || subnet == banIp )
						{
							LOG_STATUS( "MetaServer", "Disconnecting client %u, IP address %s is banned", clientId, address.cstr() );
							removeClient( clientId );
						}
					}
				}
				
				m_LastBanId = banId;
			}
			updateBanned = Time::seconds() + UPDATE_BANNED_TIME;	
		}
		if ( time > updateServer )
		{
			//TRACE( "Registering self..." );

			// register ourself into the database
			m_ServerId = registerSelf();
			// remove old server records
			pDB->execute( CharString().format("DELETE FROM servers WHERE last_updated < (unix_timestamp() - %u)", SERVER_DURATION) );
			// clean up orphaned room members..
			pDB->execute( "DELETE FROM room_members WHERE NOT EXISTS(select NULL from servers WHERE servers.server_id = room_members.server_id)" );

			updateServer = Time::seconds() + UPDATE_SERVERS_TIME;
		}
		if ( time > updateSessions )
		{
			//TRACE( "Deleting old sessions..." );

			// remove sessions older than 12 hours
			pDB->execute( CharString().format("DELETE LOW_PRIORITY FROM sessions WHERE start_time < (unix_timestamp() - %u)", SESSION_DURATION) );
			updateSessions = Time::seconds() + UPDATE_SESSIONS_TIME;
		}
		if ( time > updateReports )
		{
			//TRACE( "Checking for new reports..." );

			// check for new reports
			Database::Query reports( pDB->query( CharString().format("SELECT * FROM reports WHERE report_id > %u", m_LastReportId)) );
			for(int i=0;i<reports.size();i++)
			{
				dword reportId = reports[i]["report_id"];
				dword authorId = reports[i]["author_id"];

				CharString sMessage = (const char *)reports[i]["message"];
				dword time = reports[i]["time"];
				dword gameId = reports[i]["game_id"];

				AutoLock lock( &m_Lock );
				for(int j=0;j<clientCount();j++)
				{
					dword clientId = client( j );
					if ( getFlags( clientId ) & MetaClient::MODERATOR && getGameId( clientId ) == gameId )
						sendChat( clientId, sMessage );
				}
				m_LastReportId = reportId;
			}
			
			// remove old reports
			pDB->execute( CharString().format("DELETE LOW_PRIORITY FROM reports WHERE time < (unix_timestamp() - %u)", REPORT_DURATION) );
			updateReports = Time::seconds() + UPDATE_REPORTS_TIME;
		}
		if ( time > updateChat )
		{
			//TRACE( "Updating chat..." );

			// new messages, route the new messages to the clients
			Database::Query messages( pDB->query( CharString().format("SELECT * FROM chat WHERE message_id > %u", m_LastMessageId) ) );
			for(int i=0;i<messages.rows();i++)
			{
				MetaClient::Chat chat;
				chat.author		= messages[i]["author"];
				chat.authorId	= messages[i]["author_id"];
				chat.time		= messages[i]["time"];
				chat.text		= messages[i]["text"];
				chat.recpId		= messages[i]["recp_id"];
				chat.roomId		= messages[i]["room_id"];

				dword messageId = messages[i]["message_id"];		// save the last process message ID
				int sent = messages[i]["sent"];

				AutoLock lock( &m_Lock );

				// route chat message
				bool routed = false;
				if ( chat.recpId != 0 )
				{
					// private message, check for this user as a client 
					UserClientHash::Iterator iClients = m_UserClient.find( chat.recpId );
					if ( iClients.valid() )
					{
						if ( chat.recpId != chat.authorId && m_Ignores[ chat.recpId ].search( chat.authorId ) < 0 )
						{
							Array< dword > & clients = *iClients;
							for(int k=0;k<clients.size();++k)
								send( clients[k], MetaClient::CLIENT_RECV_CHAT) << chat;
						}

						routed = true;
					}
				}
				else if ( chat.roomId != 0 )
				{
					// room chat, send to all users in this room
					Array< dword > & members = m_RoomUsers[ chat.roomId ];
					for(int k=0;k<members.size();k++)
					{
						dword memberId = members[k];
						if ( memberId == chat.authorId )
							continue;		// don't send user their own chat
						if ( m_Ignores[ memberId ].search( chat.authorId ) >= 0 )
							continue;		// user is ignored

						UserClientHash::Iterator iClients = m_UserClient.find( memberId );
						if (! iClients.valid() )
							continue;

						Array< dword > & clients = *iClients;
						for(int j=0;j<clients.size();++j)
							send( clients[j], MetaClient::CLIENT_RECV_CHAT) << chat;
					}
					routed = true;
				}
				else
				{
					// global message, send to all clients
					for(int i=0;i<clientCount();i++)
						send( client(i), MetaClient::CLIENT_RECV_CHAT ) << chat;
					routed = true;
				}

				// track the last message processed
				m_LastMessageId = messageId;
				lock.release();

				// if the message was processed, mark it as sent so it can be deleted from the database
				if ( routed && sent == 0 )
					pDB->execute( CharString().format("UPDATE chat SET sent=1 WHERE message_id=%u", messageId) );
			}

			// remove sent chat messages
			pDB->execute( CharString().format("DELETE LOW_PRIORITY FROM chat WHERE time < (unix_timestamp() - %u) AND sent=1", CHAT_MESSAGE_DURATION) );
			// remove old offline messages
			pDB->execute( CharString().format("DELETE LOW_PRIORITY FROM chat WHERE time < (unix_timestamp() - %u)", OFFLINE_MESSAGE_DURATION) );

			updateChat = Time::seconds() + UPDATE_CHAT_TIME;
		}
		if ( time > updateRooms )
		{
			//TRACE( "Checking rooms..." );

			// get a list of all rooms with no members which are not static
			Database::Query emptyRooms( pDB->query( "SELECT * from rooms LEFT JOIN room_members ON rooms.room_id = room_members.room_id "
				"WHERE room_members.room_id IS NULL AND rooms.is_static = 0" ) );

			for(int i=0;i<emptyRooms.rows();i++)
			{
				dword roomId = emptyRooms[i]["room_id"];
				pDB->execute( CharString().format("DELETE FROM rooms WHERE room_id=%u", roomId ) );

				// clean up hash tree
				AutoLock lock( &m_Lock );
				m_RoomUsers.remove( roomId );
				m_RoomGame.remove( roomId );
			}

			updateRooms = Time::seconds() + UPDATE_ROOMS_TIME;
		}

		if ( time > updatePlayers )
		{
			//TRACE( "Updating room members..." );

			Database::Query rooms( pDB->query( "SELECT room_id,game_id from rooms" ) );
			for(int i=0;i<rooms.rows();i++)
			{
				dword roomId = rooms[i][0];
				dword gameId = rooms[i][1];

				Array< dword > currentUsers;

				Database::Query members( pDB->query( CharString().format( "SELECT user_id FROM room_members WHERE room_id=%u", roomId) ) );
				for(int k=0;k<members.size();k++)
					currentUsers.push( members[k][0] );

				AutoLock lock( &m_Lock );
				Array< dword > previousUsers = m_RoomUsers[ roomId ];

				// remove old players
				for(int j=0;j<previousUsers.size();j++)
				{
					if ( currentUsers.search( previousUsers[j] ) >= 0 )
						continue;

					// user is no longer in this room, send CLIENT_DEL_PLAYER message to all current clients
					dword removeId = previousUsers[j];
					m_UserRoom[ removeId ].removeSearch( roomId );

					for(int n=0;n<currentUsers.size();n++)
					{
						UserClientHash::Iterator iClients = m_UserClient.find( currentUsers[n] );
						if (! iClients.valid() )
							continue;

						Array< dword > & clients = *iClients;
						for(int k=0;k<clients.size();++k)
							send( clients[ k ], MetaClient::CLIENT_DEL_PLAYER ) << roomId << removeId;
					}

					// remove room from all clients of user
					UserClientHash::Iterator iClient = m_UserClient.find( removeId );
					if ( iClient.valid() )
					{
						Array< dword > & clients = *iClient;
						for(int k=0;k<clients.size();++k)
							send( clients[k], MetaClient::CLIENT_DEL_ROOM ) << roomId;
					}
				}

				// add new players
				for(int j=0;j<currentUsers.size();j++)
				{
					if ( previousUsers.search( currentUsers[j] ) >= 0 )
						continue;

					dword newMemberId = currentUsers[j];
					m_UserRoom[ newMemberId ].pushUnique( roomId );

					// send room data to the new member
					UserClientHash::Iterator iClient = m_UserClient.find( newMemberId );
					if ( iClient.valid() )
					{
						Array< dword > & clients = *iClient;

						if ( clients.size() > 0 )
						{
							Array< ShortProfile > players;
							getRoomMembers( gameId, roomId, players );
							filterProfilesForClient( clients[0], players, true );

							for(int k=0;k<clients.size();++k)
							{
								send( clients[k], MetaClient::CLIENT_ADD_ROOM ) << roomId << players;
							}
						}
					}

					// new user, send new user to all current clients in this room
					ShortProfile profile;
					if (! getShortProfile( gameId, currentUsers[j], profile ) )
					{
						LOG_ERROR( "MetaServer", "Failed to get short profile for user %u", currentUsers[j] );
						continue;		// failed to get a profile, log an error message?
					}

					for(int n=0;n<currentUsers.size();n++)
					{
						dword memberId = currentUsers[n];

						UserClientHash::Iterator iClients = m_UserClient.find( memberId );
						if (! iClients.valid() )
							continue;

						Array< dword > & clients = *iClients;
						if ( clients.size() > 0 )
						{
							ShortProfile sendProfile( profile );
							if ( filterProfileForClient( clients[0], sendProfile ) )
							{
								for(int k=0;k<clients.size();++k)
									send( clients[ k ], MetaClient::CLIENT_ADD_PLAYER ) << roomId << sendProfile;
							}
						}
					}
				}
				
				// update the hash table for this room with the current member list
				m_RoomUsers[ roomId ] = currentUsers;
				m_RoomGame[ roomId ] = gameId;
			}

			updatePlayers = Time::seconds() + UPDATE_PLAYERS_TIME;
		}

		if ( time > updateIgnores )
		{
			//TRACE( "Updating ignore list..." );

			m_Ignores.release();

			// load the ignore table
			Database::Query ignores( pDB->query( "SELECT * from ignores" ) );

			AutoLock lock( &m_Lock );
			for(int i=0;i<ignores.size();i++)
			{
				dword userId = ignores[i]["user_id"];
				dword ignoreId = ignores[i]["ignore_id"];

				m_Ignores[ userId ].push( ignoreId );
			}

			updateIgnores = Time::seconds() + UPDATE_IGNORES_TIME;
		}
		if ( time > updateEvents )
		{
			//TRACE( "Checking events..." );

			// get all events that will occur within 1 hour
			Database::Query events( pDB->query( CharString().format("SELECT * from events WHERE time > (unix_timestamp() - %u) AND time > (unix_timestamp() - 60) AND is_public = 1", 
				m_Context.eventNotifyTime ) ) );

			AutoLock lock( &m_Lock );
			for(int i=0;i<events.size();i++)
			{
				dword event_id = events[i]["event_id"];
				if ( m_EventTimerMap.find( event_id ) != m_EventTimerMap.end() )
					continue;		// we already have this event in our map..

				EventTimer & timer = m_EventTimerMap[ event_id ];
				timer.m_nEventID = event_id;
				timer.m_nStartTime = events[i]["time"];
				timer.m_sEventTitle = (const char *)events[i]["title"];
				timer.m_nNextMessageTime = timer.m_nStartTime - m_Context.eventNotifyTime;
			}

			updateEvents = Time::seconds() + UPDATE_EVENTS_TIME;
		}

		if ( m_EventTimerMap.size() > 0 )
		{
			AutoLock lock( &m_Lock );
			// send event notification messages when it's time..
			for( EventTimerMap::iterator iTimer = m_EventTimerMap.begin();
				iTimer != m_EventTimerMap.end(); )
			{
				EventTimer & timer = iTimer->second;
				if ( time >= timer.m_nNextMessageTime )
				{
					int nSeconds = timer.m_nStartTime - timer.m_nNextMessageTime;
					if ( nSeconds <= 0 )
					{
						sendGlobalChat( CharString().format("/EVENT: '%s' event is starting now...", timer.m_sEventTitle.cstr() ) );
						m_EventTimerMap.erase( iTimer++ );
						continue;
					}
					else 
					{
						timer.m_nNextMessageTime += Max<int>( nSeconds / 2, 300 );		// send notification at least very 5 minutes until time arrives
						if ( timer.m_nNextMessageTime > timer.m_nStartTime )
							timer.m_nNextMessageTime = timer.m_nStartTime;

						sendGlobalChat( CharString().format("/EVENT: '%s' event will start in %s...", 
							timer.m_sEventTitle.cstr(), GetElaspedTime(nSeconds).cstr() ) );
					}
				}

				++iTimer;		// next timer
			}
		}

		if ( time > updateProfiles )
		{
			//TRACE( "Checking profile updates..." );

			Database::Query reload( pDB->query( CharString().format("SELECT * from user_reload WHERE reload_id > %u", m_LastReloadId) ) );
			for(int i=0;i<reload.size();i++)
			{
				dword reloadId = reload[i]["reload_id"];
				dword userId = reload[i]["user_id"];
				dword time = reload[i]["time"];
				
				AutoLock lock( &m_Lock );

				// flush all profiles for all games for this user..
				flushProfile( userId );

				UserClientHash::Iterator iClient = m_UserClient.find( userId );
				if ( iClient.valid() )
				{
					dword gameId = m_UserGameId[ userId ];

					MetaClient::Profile profile;
					if ( getProfile( gameId, userId, profile ) )
					{
						// send the updated profile to the user's clients
						for(int j=0;j<(*iClient).size();j++)
							send( (*iClient)[j], MetaClient::CLIENT_RECV_SELF) << profile;
					}
				}

				lock.release();

				// is this player joined to a room, send update to all players currently in the room
				Database::Query room( pDB->query( CharString().format("SELECT room_id FROM room_members WHERE user_id=%u", userId)) );
				for(int j=0;j<room.rows();++j)
				{
					dword roomId = room[j][0];
					dword gameId = 0;

					// get the game id for this room
					Database::Query roomGame( pDB->query( CharString().format("SELECT game_id FROM rooms WHERE room_id=%u", roomId)) );
					if ( roomGame.size() > 0 )
						gameId = roomGame[0][0];

					// create a short profile to send to the room members
					ShortProfile shortProfile;
					if ( getShortProfile( gameId, userId, shortProfile ) )
					{
						bool bIsHidden = (shortProfile.flags & MetaClient::HIDDEN ) != 0;
						
						lock.set( &m_Lock );

						Array< dword > & members = m_RoomUsers[ roomId ];
						for(int j=0;j<members.size();j++)
						{
							dword memberId = members[ j ];
							
							UserClientHash::Iterator iClients = m_UserClient.find( memberId );
							if (! iClients.valid() )
								continue;

							ShortProfile memberProfile;
							if (! getShortProfile( gameId, memberId, memberProfile ) )
								continue;

							bool bIsAdmin = (memberProfile.flags & MetaClient::ADMINISTRATOR) != 0;
							for(int k=0;k<(*iClients).size();++k)
							{
								// send visible players to everyone, hidden ones only to admins
								dword clientId = (*iClients)[ k ];
								if( !bIsHidden || bIsAdmin || userId == memberId )		
									send( clientId, MetaClient::CLIENT_ADD_PLAYER ) << roomId << shortProfile;
								else if ( bIsHidden && !bIsAdmin )
									send( clientId, MetaClient::CLIENT_DEL_PLAYER ) << roomId << userId;
							}

						}

						lock.release();
					}
				}
				
				// send updated user to 
				m_LastReloadId = reloadId;
			}

			// delete records older than 24 hours
			pDB->execute( CharString().format("DELETE from user_reload WHERE time < (unix_timestamp() - %u)", RELOAD_DURATION) );
			updateProfiles = Time::seconds() + UPDATE_PROFILES_TIME;
		}

		freeConnection( pDB );
	}

	TRACE( "MetaServer::updateDemon stopping" );
}

//----------------------------------------------------------------------------

MetaServer::UpdateThread::UpdateThread( MetaServer * pServer ) 
	: m_pServer( pServer )
{}

int MetaServer::UpdateThread::run()
{
	m_pServer->updateDemon();
	delete this;
	return 0;
}

//-------------------------------------------------------------------------------
// EOF


