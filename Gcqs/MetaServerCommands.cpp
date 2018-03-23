/*
	MetaServerCommands.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#define GCQS_DLL

#include "GCQS/MetaServer.h"
#include "Debug/Assert.h"
#undef Time
#include "Standard/Time.h"
#include "Standard/CommandLine.h"
#include "Standard/StringBuffer.h"
#include "Standard/StringHelpers.h"

//----------------------------------------------------------------------------

enum Commands
{
	CMD_FIRST,	
	CMD_HELP = CMD_FIRST,			// display help
	
	CMD_ME,							// emotion
	CMD_SEND,						// send message 
	CMD_REPORT,						// send message to moderators online
	CMD_AWAY,						// Set away status
	CMD_BACK,						// Remove away status

	CMD_BROADCAST,					// send message to all clients
	CMD_MODERATE,					// toggle moderate flag on current room
	CMD_MODSAY,						// write an official message to the current room
	CMD_MODSEND,					// send an official message to another player
	CMD_MODTALK,					// send a message to all moderators in the room
	CMD_STAFFSEND,					// sends a message to all staff recently or currently online
	CMD_MUTE,						// mute another player
	CMD_UNMUTE,						// remove mute status
	CMD_CHECK,						// quickchecks a player for Watchlist entrys
	CMD_WATCH,						// add a user to the watchList
	CMD_PUSH,						// disconnect a user
	CMD_KICK,						// kick a user
	CMD_BAN,						// ban a user
	CMD_BANNED,						// list the bans which expire within 24 hours
	CMD_BANNEDLONG,					// list all bans
	CMD_UNBAN,						// unban a user
	CMD_REBAN,						// extend a ban
	CMD_CLONES,						// find users who are online using multiple accounts
	CMD_SERVERS,					// find users + servers with same MID or IP
	CMD_LOGROOM,					// display chat logs for a room
	CMD_LOGUSER,					// display all chat send and received by a user, including private messages
	CMD_SESSION,					// display session information on user
	CMD_HIDE,						// Set hidden status
	CMD_UNHIDE,						// Remove hidden status
	CMD_NOTICE,						// server notice event

	CMD_INVALID,					// invalid command

	// place new commands here
	CMD_TELL,						//tell -- same as send
	CMD_DEVTALK,					// send message to all devs online and recently online

	CMD_COUNT,
	CMD_LAST = CMD_COUNT - 1
};

struct CommandLookup
{
	const char *	pCommand;
	Commands		command;
	dword			flags;
	const char *	pHelp;
};

CommandLookup COMMAND_LOOKUP[] =
{
	{ "/?",				CMD_HELP,		0,							"Display chat help" },
	{ "/me",			CMD_ME,			0,							"Emotion" },
	{ "/send",			CMD_SEND,		0,							"Send message to another user/friend/clan" },
	{ "/tell",			CMD_TELL,		0,						"Tell another user/friend/clan something" },
	{ "/report",		CMD_REPORT,		0,							"Send message to moderators" },
	{ "/away",			CMD_AWAY,		0,							"Set status as away" },
	{ "/back",			CMD_BACK,		0,							"Remove away status" },

	{ "/broadcast",		CMD_BROADCAST,	MetaClient::MODERATOR,		"Broadcast a message" },
	{ "/modtalk",		CMD_MODTALK,	MetaClient::MODERATOR,		"Send a message to all moderators in the room" },
	{ "/moderate",		CMD_MODERATE,	MetaClient::MODERATOR,		"Toggle moderate status for the current room" },
	{ "/modsend",		CMD_MODSEND,	MetaClient::MODERATOR,		"Send an official message to another player" },
	{ "/modsay",		CMD_MODSAY,		MetaClient::MODERATOR,		"Send an official message to the room" },
	{ "/staffsend",		CMD_STAFFSEND,	MetaClient::STAFF,			"Send a message to all staff" },
	{ "/devtalk",		CMD_DEVTALK,	MetaClient::DEVELOPER,		"Send a message to all developers" },
	{ "/mute",			CMD_MUTE,		MetaClient::MODERATOR,		"Set mute status on another player" },
	{ "/unmute",		CMD_UNMUTE,		MetaClient::MODERATOR,		"Remove mute status" },
	{ "/check",			CMD_CHECK,		MetaClient::MODERATOR,		"Quick checks a player for Watchlist entries" },
	{ "/watch",			CMD_WATCH,		MetaClient::MODERATOR,		"Add player to the watchList" },
	{ "/push",			CMD_PUSH,		MetaClient::MODERATOR,		"Ban player for 30 seconds" },
	{ "/kick",			CMD_KICK,		MetaClient::MODERATOR,		"Ban player for 10 minutes" },
	{ "/ban",			CMD_BAN,		MetaClient::MODERATOR,		"Ban player for 24 hours" },
	{ "/banned",		CMD_BANNED,		MetaClient::MODERATOR,		"List all bans which expire within 24 hours" },
	{ "/bannedlong",	CMD_BANNEDLONG,	MetaClient::MODERATOR,		"List all banned users" },
	{ "/unban",			CMD_UNBAN,		MetaClient::MODERATOR,		"Unban a player" },
	{ "/reban",			CMD_REBAN,		MetaClient::ADMINISTRATOR,	"Extend a ban" },
	{ "/clones",		CMD_CLONES,		MetaClient::MODERATOR,		"Display additional accounts a player is currently using" },
	{ "/servers",		CMD_SERVERS,	MetaClient::MODERATOR,		"Find users + servers with same MID or IP" },
	{ "/logroom",		CMD_LOGROOM,	MetaClient::MODERATOR,		"Display chat logs" },
	{ "/loguser",		CMD_LOGUSER,	MetaClient::ADMINISTRATOR,	"Display all chat send and received by a user, including private messages" },
	{ "/session",		CMD_SESSION,	MetaClient::MODERATOR,		"Display session information for a user" },
	{ "/hide",			CMD_HIDE,		MetaClient::EVENT|MetaClient::ADMINISTRATOR,			"Set status to hidden" },
	{ "/unhide",		CMD_UNHIDE,		MetaClient::EVENT|MetaClient::ADMINISTRATOR,	"Remove hidden status" },
	{ "/notice",		CMD_NOTICE,		MetaClient::SERVER,			"Send server notice event" },
};
int COMMAND_LOOKUP_COUNT = sizeof(COMMAND_LOOKUP)/sizeof(COMMAND_LOOKUP[0]);

//----------------------------------------------------------------------------

void MetaServer::processCommand( dword client, dword roomId, dword recpId, const char * pText )
{
	// get a copy of the clients profile
	Profile profile;
	getProfile( client, profile );

	// find the command
	Commands command = CMD_INVALID;
	
	CommandLine arguments( pText );
	CommandLine withQuotes( pText, true );

	if ( arguments.argumentCount() > 0 )
	{
		CharString keyword( arguments.argument( 0 ) );
		
		int n = keyword.length();
		for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
		{
			if ( strnicmp<char>( COMMAND_LOOKUP[ i ].pCommand, keyword, n ) == 0 )
			{
				// found keyword match, check rights
				if ( COMMAND_LOOKUP[ i ].flags != 0 && (profile.flags & COMMAND_LOOKUP[ i ].flags) == 0 )
				{
					sendChat( client, "/Invalid command; you do not have the rights to use this command..." );
					return;
				}

				command = COMMAND_LOOKUP[ i ].command;
				break;
			}
		}
	}

	switch( command )
	{
	case CMD_HELP:
		{
			CharString help;
			
			help += "/<b>GCQ Server Commands</b>\n";
			
			for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
			{
				if ( COMMAND_LOOKUP[ i ].flags == 0 || (profile.flags & COMMAND_LOOKUP[i].flags) != 0 )
				{
					help += CharString().format("<b>%s</b> ... %s\n", COMMAND_LOOKUP[i].pCommand, 
						COMMAND_LOOKUP[i].pHelp );
				}
			}

			sendChat( client, help );
		}
		break;
	case CMD_ME:						// emotion
		{
			bool usage = true;

			if ( arguments.argumentCount() > 1 )
			{
				CharString emotion;
				emotion.format( "/%s %s", profile.name.cstr(), arguments.restring( 1 ).cstr() );

				if ( roomId != 0 )
					processMessage( client, roomId, recpId, emotion );
				sendChat( client, emotion );

				usage = false;
			}

			if ( usage )
				sendChat( client, "/Usage: /me [message]" );
		}
		break;
	case CMD_TELL:
	case CMD_SEND:						// send message 
		{
			bool usage = true;
			if ( arguments.argumentCount() > 2 )
			{
				CharString who( arguments.argument( 1 ) );
				CharString message;
				message.format("/<b>%s @%u</b> Sent \"<b>%s</b>\"", profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );

				if ( strnicmp<char>( who, "friends", who.length() ) == 0 )
				{
					usage = false;

					Array< dword > friends;
					if ( getFriends( profile.userId, friends ) )
					{
						message = CharString().format("/<b>%s @%u</b> sent to Friends: \"<b>%s</b>\"", 
							profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );

						for(int i=0;i<friends.size();i++)
							if( friends[i] != 0 )	// prevent broadcast in case user has @0 on his friendlist
								processMessage( client, 0, friends[i], message );

						sendChat( client, CharString().format("/You sent to Friends: \"<b>%s</b>\"", withQuotes.restring( 2 ).cstr() ) );
					}
					else
						sendChat( client, "/Error, failed to get friends..." );
				}
				else if ( strnicmp<char>( who, "clan", who.length() ) == 0 )
				{
					usage = false;
					if (profile.clanId != 0 )
					{	
						if( ( profile.flags & MetaClient::CLAN_MEMBER ) != 0 )
						{
							Array< dword > members;
							if ( getClanMembersOnline( profile.clanId, members ) )
							{
								if( members.size() > 1 )
								{
									message = CharString().format("/<b>%s @%u</b> sent to Clan: \"<b>%s</b>\"", 
										profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );
									for(int i=0;i<members.size();i++)
										if( members[i] != 0 )	// prevent broadcast in case user has @0 in his clan
											processMessage( client, 0, members[i], message );
										
										sendChat( client, CharString().format("/You sent to %u Clanmembers: \"<b>%s</b>\"",
											members.size() - 1 ,withQuotes.restring( 2 ).cstr() ) );
								}
								else
									sendChat( client, "/Unable to send message: You are the only clanmember online." );
							}
							else
								sendChat( client, "/Error, failed to get clan members..." );
						}
						else
							sendChat( client, "/Error, haven't been accepted by the clan yet..." );
					}
					else
						sendChat( client, "/Error, you do not belong to a clan..." );
				}
				else if ( strnicmp<char>( who, "clanadmin", who.length() ) == 0 )
				{
					usage = false;
					if (profile.clanId != 0 )
					{	
						if( ( profile.flags & MetaClient::CLAN_ADMIN ) != 0 )
						{
							Array< dword > members;
							if ( getClanMembersOnline( profile.clanId, members ) )
							{
								if( members.size() > 1 )
								{
									message = CharString().format("/[<font color=f0e68c>Fleet Admin</font>] <b>%s @%u</b> sent to Clan: \"<b>%s</b>\"", 
										profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );
									for(int i=0;i<members.size();i++)
										if( members[i] != 0 )	// prevent broadcast in case user has @0 in his clan
											processMessage( client, 0, members[i], message );
										
										sendChat( client, CharString().format("/[<font color=f0e68c>Fleet Admin</font>] sent to %u Clanmembers: \"<b>%s</b>\"",
											members.size() - 1 ,withQuotes.restring( 2 ).cstr() ) );
								}
								else
									sendChat( client, "/Unable to send message: You are the only clanmember online." );
							}
							else
								sendChat( client, "/Error, failed to get clan members..." );
						}
						else
							sendChat( client, "/Error, You are not an admin of this clan..." );
					}
					else
						sendChat( client, "/Error, you do not belong to a clan..." );
				}
				else if ( strnicmp<char>( who, "clanoffline", who.length() ) == 0 )
				{
					usage = false;
					if (profile.clanId != 0 )
					{
						if( ( profile.flags & MetaClient::CLAN_MEMBER ) != 0 )
						{
							Array< dword > members;
							if ( getClanMembers( profile.clanId, members ) )
							{
								if( members.size() > 1 )
								{
									message = CharString().format("/<b>%s @%u</b> sent to Clan: \"<b>%s</b>\"", 
										profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );
									for(int i=0;i<members.size();i++)
										if( members[i] != 0 )	// prevent broadcast in case user has @0 in his clan
											processMessage( client, 0, members[i], message );
										
										sendChat( client, CharString().format("/You sent to your Clan: \"<b>%s</b>\"", withQuotes.restring( 2 ).cstr() ) );
								}
								else
									sendChat( client, "/Unable to send message: You are the only clanmember." );
							}
							else
								sendChat( client, "/Error, failed to get clan members..." );
						}
						else
							sendChat( client, "/Error, haven't been accepted by the clan yet..." );
					}
					else
						sendChat( client, "/Error, you do not belong to a clan..." );
				}
				else if ( arguments.argumentCount() > 3 && strnicmp<char>( who, "fleet", who.length() ) == 0 )
				{
					usage = false;
					
					CharString fleet( arguments.argument( 2 ) );
					
					dword clanId = 0;
					if( !fleet.beginsWith("@") )
						getClanIdByName( fleet, clanId );
					else
						clanId = strtol( fleet.buffer() + 1, NULL, 10 );

					if (clanId != 0 )
					{
						Array< dword > members;
						if ( getClanMembers( clanId, members ) )
						{
							message = CharString().format("/<b>%s @%u</b> sent to your fleet: \"<b>%s</b>\"", 
								profile.name.cstr(), profile.userId, withQuotes.restring( 3 ).cstr() );
							for(int i=0;i<members.size();i++)
								if( members[i] != 0 )	// prevent broadcast in case @0 is in the clan
									processMessage( client, 0, members[i], message );
							
							CharString sFleetName;
							getClanName( client, clanId, sFleetName );
							sendChat( client, CharString().format("/You sent to fleet [%s]: \"<b>%s</b>\"", sFleetName.cstr(), withQuotes.restring( 3 ).cstr() ) );
						}
						else
							sendChat( client,"/Error, failed to get fleet members..." );
					}
					else
						sendChat( client, "/Error, fleet does not exist..." );
				}
				else
				{
					usage = false;

					dword userId = 0;

					Array< dword > found;
					if ( findUserExactFirst( who, found ) )
					{
						if ( found.size() == 1 )
							userId = found[0];
						else
							sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
					}
					else
						sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );

					if ( userId != 0 )
					{
						processMessage( client, 0, userId, CharString().format("/<b>%s @%u</b> Sent \"<b>%s</b>\"", 
							profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() ) );

						ShortProfile targetProfile;
						if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
							who = targetProfile.name;

						sendChat( client, CharString().format("/You sent to %s @%u: \"<b>%s</b>\"", who.cstr(), userId, withQuotes.restring( 2 ).cstr() ) );
					}
				}
			}

			if ( usage )
			{
				if( command == CMD_TELL )
					sendChat( client, "/Usage: /tell [friends|clan|clanadmin|clanoffline|user name|@user id|fleet [@fleet id | fleet name] ] [message]" );
				else
					sendChat( client, "/Usage: /send [friends|clan|clanadmin|clanoffline|user name|@user id|fleet [@fleet id | fleet name] ] [message]" );
			}
		}
		break;
	case CMD_REPORT:					// send message to moderators online
		{
			if ( arguments.argumentCount() > 1 )
			{
				dword gameId = getGameId( client );

				sendChat( client, CharString().format("/Sending report \"<b>%s</b>\"", withQuotes.restring( 1 ).cstr())  );

				CharString message;
				message.format( "/<b>[<font color=f57e20>REPORT</font>] %s @%u (<i>%s</i>)</b> reports \"<b>%s</b>\"", profile.name.cstr(), profile.userId, profile.status.cstr() ,withQuotes.restring( 1 ).cstr() );

				Database * pDB = getConnection();
				pDB->execute( CharString().format("INSERT reports(author_id,message,time,game_id) values(%u,'%s',%u,%u)",
					profile.userId, addSlash( message ).cstr(), Time::seconds(), gameId) );
				freeConnection( pDB );
			}
			else
				sendChat( client, "/Usage: /report [playername|@id] [message]" );
		}
		break;
	case CMD_AWAY:
		{
			Database * pDB = getConnection();
			pDB->execute( CharString().format("UPDATE user_status SET away=1 WHERE user_id=%u", profile.userId) );
			pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", profile.userId, Time::seconds()) );
			freeConnection( pDB );

			sendChat( client, "/You are away..." );
		}
		break;
	case CMD_BACK:
		{
			Database * pDB = getConnection();
			pDB->execute( CharString().format("UPDATE user_status SET away=0 WHERE user_id=%u", profile.userId) );
			pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", profile.userId, Time::seconds()) );
			freeConnection( pDB );

			sendChat( client, "/You are back..." );
		}
		break;
	case CMD_BROADCAST:					// send message to all clients
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString message;
				message += "/<font color=ff0000>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=</font>\n";
				message += CharString().format( "<b>%s</b> broadcasts \"<b>%s</b>\"\n", profile.name.cstr(), withQuotes.restring( 1 ).cstr() );
				message += "<font color=ff0000>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=</font>\n";
				message += "\n";

				processMessage( client, 0, 0, message );
			}
			else
				sendChat( client, "/Usage: /broadcast [message]" );
		}
		break;
	case CMD_MODERATE:
		{
			Database * pDB = getConnection();

			Database::Query room( pDB->query( CharString().format("SELECT is_moderated, is_static FROM rooms WHERE room_id=%u", roomId) ) );
			if ( room.rows() > 0 )
			{
				dword is_moderated = ((int)room[0]["is_moderated"]) == 0 ? 1 : 0;
				dword is_static = ((int)room[0]["is_static"]) == 0 ? 0 : 1;
				
				// if user is no admin, he may moderate only non-static rooms, but may "unmoderate" all rooms
				if (!(profile.flags & MetaClient::ADMINISTRATOR) && is_moderated && is_static )
					sendChat( client, "/You can not set a main chatroom to moderated" );
				else
				{
					// get the room members who are not moderators
					Database::Query roomMembers( pDB->query( CharString().format("SELECT room_members.user_id FROM rooms,room_members,user_data WHERE "
						"rooms.room_id=room_members.room_id AND rooms.game_id=user_data.game_id AND room_members.user_id=user_data.user_id "
						"AND user_data.moderator=0 AND rooms.room_id=%u", roomId )) );
					
					pDB->execute( CharString().format("UPDATE rooms SET is_moderated=%u WHERE room_id=%u", is_moderated, roomId) );
					
					// send info to the room
					if ( is_moderated )
					{
						processMessage( client, roomId, 0, "/Room is now moderated...", true );
						m_RoomModerated.insert( roomId );
					}
					else
					{
						processMessage( client, roomId, 0, "/Room is no longer moderated...", true );
						m_RoomModerated.erase( roomId );
					}
				
					// announce action to all moderators in the room
					CharString message;
					message.format( "/<b>%s</b> set the room to %smoderated...", profile.name.cstr(), is_moderated ? "" : "un" );

					Array< dword > mods;
					if ( getModeratorsInRoom( roomId, getGameId( client ), mods ) )
					{
						for(int i=0;i<mods.size();i++)
							if ( mods[i] != profile.userId )
								processMessage( client, 0, mods[i], message );
							
						sendChat( client, CharString().format( "/You set the room to %s...", 
							is_moderated ? "moderated" : "unmoderated" ) );
					}
				}
			}

			freeConnection( pDB );
		}
		break;
	case CMD_MODSAY:					// send official message to the chatroom
		{
			if ( arguments.argumentCount() > 1 && roomId != 0 )
			{
				CharString message = CharString().format( "/\n<b>[<font color=00ff00>MODERATOR</font>]</b> %s: \"<b>%s</b>\"\n\n", 
					profile.name.cstr(), withQuotes.restring( 1 ).cstr() );

				processMessage( client, roomId, 0, message );
				sendChat( client, message );
			}
			else
				sendChat( client, "/Usage in rooms: /modsay [message]" );
		}
		break;
	case CMD_MODSEND:						// send official message to user
		{
			if ( arguments.argumentCount() > 2 )
			{
				CharString who( arguments.argument( 1 ) );

				dword userId = 0;

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
						userId = found[0];
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );

				if ( userId != 0 )
				{	
					CharString message;
					message += "/\n";
					message += "<font color=ff0000>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=</font>\n";
					message += CharString().format( "<b>[<font color=00ff00>MODERATOR</font>] %s @%u</b> Sent \"<b>%s</b>\"\n", 
						profile.name.cstr(), profile.userId, withQuotes.restring( 2 ).cstr() );
					message += "<font color=ff0000>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=</font>\n";
					message += "\n";
					processMessage( client, 0, userId, message );
					
					ShortProfile targetProfile;
					if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
						who = targetProfile.name;

					sendChat( client, CharString().format("/[MODSEND] to %s @%u: \"<b>%s</b>\"", who.cstr(), userId, arguments.restring( 2 ).cstr() ) );
				}
			}
			else	
				sendChat( client, "/Usage: /modsend [user name|@user id] [message]" );
		}
		break;
	case CMD_MODTALK:					// send a message to all moderators in the room
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString message;
				message.format( "/<b><font color=00ff00>[MODTALK]</font> %s @%u</b> Sent \"<b>%s</b>\"", 
					profile.name.cstr(), profile.userId, withQuotes.restring( 1 ).cstr() );

				Array< dword > mods;
				if ( getModeratorsOnline( getGameId( client ), mods ) )
				{
					for(int i=0;i<mods.size();i++)
						if ( mods[i] != profile.userId )
							processMessage( client, 0, mods[i], message );

					sendChat( client, CharString().format("/<b>Sent <font color=00ff00>[MODTALK]</font></b>: \"<b>%s</b>\"", withQuotes.restring( 1 ).cstr() ) );
				}
			}
			else
				sendChat( client, "/Usage: /modtalk [message]" );
		}
		break;
	case CMD_STAFFSEND:					// send a message to all moderators recently online
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString message;
				message.format( "/<b><font color=0080ff>[STAFFSEND]</font> %s @%u</b> Sent \"<b>%s</b>\"", 
					profile.name.cstr(), profile.userId, withQuotes.restring( 1 ).cstr() );

				Array< dword > staff;
				if ( getStaffOnline( getGameId( client ), staff ) )
				{
					for(int i=0;i<staff.size();i++)
						if ( staff[i] != profile.userId )
							processMessage( client, 0, staff[i], message );

					sendChat( client, CharString().format("/<b>Sent <font color=0080ff>[STAFFSEND]</font></b>: \"<b>%s</b>\"", withQuotes.restring( 1 ).cstr() ) );
				}
			}
			else
				sendChat( client, "/Usage: /staffsend [message]" );
		}
		break;
	case CMD_DEVTALK:					// send a message to all devs online
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString message;
				message.format( "/<b><font color=b040b0>[DEVTALK]</font> %s @%u</b> Sent \"<b>%s</b>\"", 
					profile.name.cstr(), profile.userId, withQuotes.restring( 1 ).cstr() );

				Array< dword > devs;
				if ( getDevelopersOnline( getGameId( client ), devs ) )
				{
					for(int i=0;i<devs.size();i++)
						if ( devs[i] != profile.userId )
							processMessage( client, 0, devs[i], message );

					sendChat( client, CharString().format("/<b>Sent <font color=b040b0>[DEVTALK]</font></b>: \"<b>%s</b>\"", 
						withQuotes.restring( 1 ).cstr() ) );
				}
			}
			else
				sendChat( client, "/Usage: /devtalk [message]" );
		}
		break;
	case CMD_MUTE:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
					{
						ShortProfile mute;
						if ( getShortProfile( 0, found[0], mute ) )
						{
							if ( ( mute.flags & MetaClient::MUTED ) == 0)
							{
								sendChat( client, CharString().format("/Muted %s...", mute.name.cstr()) );
								
								Database * pDB = getConnection();
								pDB->execute( CharString().format("UPDATE users SET is_muted=1 WHERE user_id=%u", mute.userId) );
								pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", mute.userId, Time::seconds()) );
								freeConnection( pDB );

								processMessage( client, 0, mute.userId, CharString().format("/You have been muted by %s...", profile.name.cstr()) );
								
								// notify all Moderators in the room that the user has been muted
								if ( roomId != 0 )
								{
									dword gameId = getGameId( client );
									Array< dword > mods;
									if ( getModeratorsInRoom( roomId, gameId, mods ) )
									{	
										CharString message;
										message.format("/<b>%s @%u</b> has been <b>muted</b> by %s...", mute.name.cstr(), mute.userId, profile.name.cstr() );
										for(int i=0;i<mods.size();i++)
											if ( profile.userId != mods[i] )
												processMessage( client, 0, mods[i], message );
									}
								}
							}
							else
								sendChat( client, CharString().format("/Error, %s is already muted...", mute.name.cstr() ) );
						}
						else
							sendChat( client, "/Error, failed to mute user..." );
					}
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find '%s'...", who.cstr()) );
			}
			else
				sendChat( client, "/Usage: /mute [user name|@user id]" );
		}
		break;
	case CMD_UNMUTE:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
					{
						ShortProfile mute;
						if ( getShortProfile( 0, found[0], mute ) )
						{
							if ( ( mute.flags & MetaClient::MUTED ) != 0)
							{
								sendChat( client, CharString().format("/Unmuted %s...", mute.name.cstr()) );
								
								Database * pDB = getConnection();
								pDB->execute( CharString().format("UPDATE users SET is_muted=0 WHERE user_id=%u", mute.userId) );
								pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", mute.userId, Time::seconds()) );
								freeConnection( pDB );

								processMessage( client, 0, mute.userId, CharString().format("/You have been unmuted by %s...", profile.name.cstr()) );
								
								// notify all Moderators in the room that the user has been unmuted
								if ( roomId != 0 )
								{
									dword gameId = getGameId( client );
									Array< dword > mods;
									if ( getModeratorsInRoom( roomId, gameId, mods ) )
									{	
										CharString message;
										message.format("/<b>%s @%u</b> has been <b>unmuted</b> by %s...", mute.name.cstr(), mute.userId, profile.name.cstr() );
										for(int i=0;i<mods.size();i++)
											if ( profile.userId != mods[i] )
												processMessage( client, 0, mods[i], message );
									}
								}
								
							}
							else
								sendChat( client, CharString().format("/Error, %s is currently not muted...", mute.name.cstr() ) );
						}
						else
							sendChat( client, "/Error, failed to unmute user..." );
					}
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find '%s'...", who.cstr()) );
			}
			else
				sendChat( client, "/Usage: /unmute [user name|@user id]" );
		}
		break;
	case CMD_CHECK:						// Quickcheck a player for Watchlist entrys
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );
				
				dword userId = 0;
				
				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
						userId = found[0];
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );
				
				if ( userId != 0 )
				{
					Database * pDB = getConnection();

					if ( who[0] == '@' )
					{
						ShortProfile targetProfile;
						if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
							who = targetProfile.name;
					}
					
					
					CharString result = "/";
					Database::Query userStatus( pDB->query( CharString().format("SELECT status, real_status FROM user_status WHERE user_id = %u", userId) ) );
					if ( userStatus.rows() > 0 )
					{
						CharString status = (const char *)userStatus[0]["status"];
						CharString sRealStatus = (const char *)userStatus[0]["real_status"];
						if( status.compare( sRealStatus ) != 0 )
							result += CharString().format( "Fake status: %s, real status: %s\n", status.cstr(), sRealStatus.cstr() );
					}

					Database::Query rows( pDB->query( CharString().format("SELECT username, addedtime, watch_type, reason FROM watchlist "
									"WHERE userid = %u AND is_active = 1", userId ) ) ); 

					if( rows.size() > 0 )
					{
						result += CharString().format("Found %u active watchlist entry%s for '%s' @%u:\n", 
							rows.size(),rows.size() > 1 ? "s":"", who.cstr(), userId );
						for(int i=0;i<rows.size();i++)
						{
							CharString sWType;
							switch( (dword)rows[i]["watch_type"] )
							{
								case 0: sWType = "WATCH"; break;
								case 1: sWType = "KICK"; break;
								case 2: sWType = "BAN"; break;
							}
							
							CharString userName			= (const char *)rows[i]["username"];
							dword addedTime			= rows[i]["addedtime"];
							CharString reason			= (const char *)rows[i]["reason"];
							
							result += CharString().format( "%s - '%s' [ %s ] reason: %s\n", sWType.cstr(), userName.cstr(), Time::format( addedTime, "%c" ).cstr(), reason.cstr() );
						}
						
						// search for possible alternative accounts
						Database::Query alternative( pDB->query( CharString().format("SELECT DISTINCT w1.username, w1.addedtime, w1.watch_type, w1.reason FROM watchlist AS w1, watchlist AS w2 "
									"WHERE w1.userid != %u AND w2.userid = %u AND ( ( w1.userip IS NOT NULL AND w1.userip = w2.userip ) "
									"OR ( w1.usermachine IS NOT NULL AND w1.usermachine = w2.usermachine ) )"
									, userId, userId ) ) ); 

						if( alternative.size() > 0 )
						{
							result += "Possible related entries:\n";
							for(int i=0;i<alternative.size();i++)
							{
								CharString sWType;
								switch( (dword)alternative[i]["watch_type"] )
								{
									case 0: sWType = "WATCH"; break;
									case 1: sWType = "KICK"; break;
									case 2: sWType = "BAN"; break;
								}
								
								CharString userName			= (const char *)alternative[i]["username"];
								dword addedTime			= alternative[i]["addedtime"];
								CharString reason			= (const char *)alternative[i]["reason"];
								
								result += CharString().format( "%s - '%s' [ %s ] reason: %s\n", sWType.cstr(), userName.cstr(), 
									Time::format( addedTime, "%c" ).cstr(), reason.cstr() );
							}
						}

					}
					else
						result = CharString().format("/User '%s' @%u has no active entry in the watchlist...", who.cstr(), userId );

					sendChat( client, result );	
					freeConnection( pDB );
				}
			}
			else	
				sendChat( client, "/Usage: /check [user name|@user id]" );
		}
		break;
	case CMD_WATCH:						// add user to the watchlist
		{
			if ( arguments.argumentCount() > 2 )
			{
				CharString who( arguments.argument( 1 ) );

				dword userId = 0;

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
						userId = found[0];
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );

				if ( userId != 0 )
				{
					if( addWatchList( userId, profile.userId, 0, withQuotes.restring( 2 ), 0 ) )
					{
						ShortProfile targetProfile;
						if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
							who = targetProfile.name;

						sendChat( client, CharString().format("/Added %s to watchlist...", who.cstr() ) );		
					}
					else
						sendChat( client, CharString().format("/Failed to add %s to watchlist...", who.cstr() ) );		
				}
			}
			else	
				sendChat( client, "/Usage: /watch [user name|@user id] [reason]" );
		}
		break;
	case CMD_BAN:						// ban a user
	case CMD_KICK:						// kick a user
	case CMD_PUSH:						// disconnect a user
		{
			dword duration;
			CharString actionName;
			CharString commandName;
			switch( command )
			{
				case CMD_PUSH: duration = 120;			actionName = "disconnected"; commandName = "push"; break;
				case CMD_KICK: duration = 60 * 10;		actionName = "kicked"; commandName = "kick"; break;
				case CMD_BAN:  duration = 60 * 60 * 24; actionName = "banned"; commandName = "ban"; break;
			}
	
			if ( arguments.argumentCount() > 2 )
			{
				CharString who( arguments.argument( 1 ) );
				CharString why( withQuotes.restring( 2 ) );
				CharString actionString;
				dword banId = 0;

				// Add information for the user - we don't want to show them internal reasons for the ban
				// Tell the user they get no infraction for a push, and a pending message for bans and kicks
				// Until we do the paper work later
				if ( command == CMD_PUSH )	
				{
					actionString = why;
					actionString += "/nThis minor infraction is unrecorded.";
				}
				else
					actionString = "Pending review...";

				if ( who.length() > 0 && who[0] == '$' )
				{
					CharString banIp = who.buffer() + 1;
					dword banStart = Time::seconds();
					dword banEnd = banStart + duration;

					Database * pDB = getConnection();
					// add the ban record
					pDB->execute( CharString().format("INSERT banlist(ban_ip,ban_start,ban_end,ban_time_type,ban_from,ban_why) "
						"VALUES('%s',%u,%u,4,%u,'%s')", banIp.cstr(), banStart, banEnd, profile.userId, actionString.cstr() ) );
					banId = pDB->insertId();
					freeConnection( pDB );

					sendChat( client, CharString().format("/%s IP address %s...%s", actionName.cstr(), banIp.cstr(), why.cstr()) );
				}
				else
				{
					Array< dword > found;
					if ( findUserExactFirst( who, found ) )
					{
						if ( found.size() == 1 )
						{
							ShortProfile kicked;
							if ( getShortProfile( 0, found[0], kicked ) )
							{
								banId = banUser( profile.userId, kicked.userId, duration, actionString );

								processMessage( client, 0, kicked.userId, 
									CharString().format("/You have been %s by %s.  Reason: %s.", actionName.cstr(), profile.name.cstr(), actionString.cstr() ) );

								sendChat( client, CharString().format("/%s %s... %s", actionName.cstr(), kicked.name.cstr(), why.cstr()) );

								CharString message;
								message.format("/<b>%s @%u</b> has been <b>%s</b> by %s... %s", 
									kicked.name.cstr(), kicked.userId, actionName.cstr(), profile.name.cstr(), why.cstr() );

								// notify mods
								dword gameId = getGameId( client );
								Array< dword > mods;

								bool bNotifyMods = false;
								
								// notify all Moderators in the room that the user has been kicked
								if ( ( roomId != 0 ) && ( command == CMD_KICK ) )	
									bNotifyMods = getModeratorsInRoom( roomId, gameId, mods );

								// notify all Moderators recently online that the user has been banned
								if ( command == CMD_BAN )	
									bNotifyMods = getModeratorsOnline( gameId, mods );

								if ( bNotifyMods )
									for(int i=0;i<mods.size();i++)
										if ( profile.userId != mods[i] )
											processMessage( client, 0, mods[i], message );

								// finally record this in the watchlist
								if( command != CMD_PUSH )
									addWatchList( kicked.userId, profile.userId, command == CMD_KICK ? 1 : 2, why, banId );

							}
							else
								sendChat( client, CharString().format( "/Error, failed to %s user...", actionName.cstr() ) );
						}
						else
							sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
					}
					else
						sendChat( client, CharString().format("/Error, failed to find '%s'...", who.cstr()) );
				}
			}
			else
				sendChat( client, CharString().format("/Usage: /%s [user name|@user id|$IP] [reason]",commandName.cstr()) );
		}
		break;
	case CMD_BANNED:
	case CMD_BANNEDLONG:
		{
			Database * pDB = getConnection();

			CharString message;
			CharString query = "SELECT * FROM banlist";
			if( command == CMD_BANNED )
				query += CharString().format(" WHERE ban_end <= %u AND ban_end != 0" , Time::seconds() + ( 60 * 60 * 24 ) );
			query += " ORDER BY ban_id";

			Database::Query banned( pDB->query( query ) );

			message += CharString().format("/%d ban records...\n", banned.size());
			for(int i=0;i<banned.size();i++)
			{
				dword banId = banned[i]["ban_id"];
				dword banUserId = banned[i]["ban_userid"];
				CharString banIP = (const char *)banned[i]["ban_ip"];
				dword banStart = banned[i]["ban_start"];
				dword banEnd = banned[i]["ban_end"];
				CharString banMID = (const char *)banned[i]["ban_machine"];
				dword banFrom = banned[i]["ban_from"];
				CharString banWhy = (const char *)banned[i]["ban_why" ];

				CharString query2 = CharString().format("SELECT reason FROM watchlist WHERE banid = %u", banId );
				Database::Query banned2( pDB->query( query ) );

				if ( banned2.size() > 0 )
					banWhy = (const char *)banned2[0]["reason"];

				message += CharString().format("- BanId: <b>%u</b>", banId);

				ShortProfile sp;
				if ( banUserId != 0 && getShortProfile( 0, banUserId, sp ) )
					message += CharString().format(", User: <b>%s @%u</b>", sp.name.cstr(), sp.userId);
				if ( banIP.length() > 0 )
					message += CharString().format(", IP: %s", banIP.cstr());
				if ( banMID.length() > 0 )
					message += CharString().format(", MID: %s", banMID.cstr() );
				if ( banFrom != 0 && getShortProfile( 0, banFrom, sp ) )
					message += CharString().format(", From: <b>%s @%u</b>", sp.name.cstr(), sp.userId );
				if ( banWhy.length() > 0 )
					message += CharString().format(", Reason: '<b>%s</b>'", banWhy.cstr() );

				message += CharString().format(", Start: %s, End: %s\n", 
					Time::format( banStart, "%c" ).cstr(), Time::format( banEnd, "%c").cstr() );
			}
			message += "End of list...";
			
			if( command == CMD_BANNED )
				message += "\nUse /bannedlong to see all bans";

			freeConnection( pDB );

			sendChat( client, message );
		}
		break;
	case CMD_UNBAN:
		{
			if ( arguments.argumentCount() > 1 )
			{
				dword banId = strtol( arguments.argument( 1 ), NULL, 10 );

				Database * pDB = getConnection();

				Database::Query ban( pDB->query( CharString().format("SELECT * from banlist WHERE ban_id=%u", banId) ) );
				if ( ban.size() > 0 )
				{
					dword bannedUserId = ban[0]["ban_userid"];
					dword userId = ban[0]["ban_from"];
					CharString banReason = (const char *)ban[0]["ban_why"];

					sendChat( client, CharString().format("/Removing ban %u on @%u by @%u, reason: \"%s\"...",
						banId, bannedUserId, userId, banReason.cstr() ) );
					pDB->execute( CharString().format("DELETE FROM banlist WHERE ban_id=%u", banId) );
				}
				else
					sendChat( client, "/Error, failed to remove ban..." );

				freeConnection( pDB );
			}
			else
				sendChat( client, "/Usage: /unban [banId]" );
		}
		break;
	case CMD_REBAN:
		{
			if ( arguments.argumentCount() > 2 )
			{
				Database * pDB = getConnection();
				dword banId = strtol( arguments.argument( 1 ), NULL, 10 );
				dword duration = strtol( arguments.argument( 2 ), NULL, 10 ) * 86400;

				Database::Query ban( pDB->query( CharString().format("SELECT * from banlist WHERE ban_id=%u", banId) ) );
				if ( ban.size() > 0 )
				{
					dword banEnd = ban[0]["ban_end"];
					banEnd += duration;

					pDB->execute( CharString().format("UPDATE banlist SET ban_end=%u WHERE ban_id=%u", banEnd, banId) );

					sendChat( client, CharString().format( "/Ban %u extended to %s", 
						banId, Time::format( banEnd, "%c").cstr()) );
				}
				else
					sendChat( client, "/Error, that ban was not found..." );

				freeConnection( pDB );
			}
			else
				sendChat( client, "/Usage: /reban [banId] [days]" );
		}
		break;
	case CMD_CLONES:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
					{
						Database * pDB = getConnection();

						Database::Query session( pDB->query( CharString().format("SELECT * FROM sessions WHERE user_id=%u", found[0]) ) );
						if ( session.rows() > 0 )
						{
							int nRecentSession = 0;
							if ( session.rows() > 1 )
								for(int i=1;i<session.rows();i++)
									if( (dword)session[i]["start_time"] >= (dword)session[nRecentSession]["start_time"] )
										nRecentSession = i;
						
							CharString remoteIP = (const char *)session[nRecentSession]["remote_ip"];
							CharString machineId = (const char *)session[nRecentSession]["mid"];
							if( machineId.length() == 0 ) machineId = "-NA-";	// prevent routine from showing up all forumusers as clones

							Database::Query clones( pDB->query( CharString().format("SELECT * FROM sessions WHERE mid='%s' OR remote_ip = '%s'", 
								machineId.cstr(), remoteIP.cstr()) ) );
							
							// check if there are different user_ids with same ip/mid
							int nClones = 0;
							for(int i=0;i<clones.rows();i++)
								if ( (dword)clones[i]["user_id"] != found[0] )
									nClones++;
			
							CharString info;
							info.format("/%d online clone(s) found for user '%s'...\n", nClones, who.cstr() );

							if ( clones.rows() > 1 )
								for(int i=0;i<clones.rows();i++)
								{
									dword userId = clones[i]["user_id"];
									dword startTime = clones[i]["start_time"];
									CharString remoteIP = (const char *)clones[i]["remote_ip"];
									CharString proxyIP = (const char *)clones[i]["proxy_ip"];
									CharString machineId = (const char *)clones[i]["mid"];
			
									ShortProfile targetProfile;
									CharString who = "";
									if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
										who = targetProfile.name;
																		
									info += CharString().format("%s - Name: '%s', UserId: %u, LoginTime: %s, Remote IP: %s, Proxy IP: %s, Machine ID: %s\n", 
										machineId.length() > 0 ? "GCQL":"Forum" ,who.cstr(), userId, Time::format( startTime, "%c" ).cstr(), remoteIP.cstr(), proxyIP.cstr(), machineId.cstr() );

									// prevent info string getting too big and crashing client on send...
									if (i % 20 == 0)
									{
										sendChat(client, info);
										info = "/";
									}
								}

							Database::Query offclones( pDB->query( CharString().format("SELECT DISTINCT us.user_id, us.username, us.last_login, us.last_ip, us.last_mid "
									"FROM users AS us LEFT JOIN sessions AS se USING( user_id ) WHERE ISNULL( se.user_id ) "
									"AND ( us.last_ip = '%s' OR us.last_mid = '%s' )", remoteIP.cstr(), machineId.cstr()) ) );
								
							if ( offclones.rows() > 0 )
							{
								info += "Possible additional offline matches:\n";
								for(int i=0;i<offclones.rows();i++)
								{
									dword userId = offclones[i]["user_id"];
									dword lastTime = offclones[i]["last_login"];
									CharString remoteIP = (const char *)offclones[i]["last_ip"];
									CharString machineId = (const char *)offclones[i]["last_mid"];
									CharString userName = (const char *)offclones[i]["username"];
									
									info += CharString().format("Name: '%s', UserId: %u, Last login: %s, IP: %s, Machine ID: %s\n", 
										userName.cstr(), userId, Time::format( lastTime, "%c" ).cstr(), remoteIP.cstr(), machineId.cstr() );

									// prevent info string getting too big and crashing client on send...
									if (i % 20 == 0)
									{
										sendChat(client, info);
										info = "/";
									}
								}
							}

							sendChat( client, info );
							
						}
						else	// There was no session for the user, so he hasn't been online recently
						{
							dword userId = found[0];
							Database::Query userinfo( pDB->query( CharString().format(
								"SELECT last_login, last_ip, last_mid FROM users WHERE user_id=%u AND NOT ISNULL( last_ip )", userId ) ) );
							if ( userinfo.rows() > 0 )
							{
								CharString remoteIP = (const char *)userinfo[0]["last_ip"];
								CharString machineId = (const char *)userinfo[0]["last_mid"];
								if( machineId.length() == 0 ) machineId = "-NA-";	// prevent routine from showing up all forumusers as clones
								
								Database::Query clones( pDB->query( CharString().format("SELECT * FROM sessions WHERE mid='%s' OR remote_ip = '%s'", machineId.cstr(), remoteIP.cstr()) ) );
								
								// due to a race condition the user could've just established a session
								// check if there are different user_ids with same ip/mid
								int nClones = 0;
								for(int i=0;i<clones.rows();i++)
									if ( (dword)clones[i]["user_id"] != userId )
										nClones++;
									
								CharString info;
								info = "/User is not online, result will be less reliable.\n";
								info += CharString().format("%d online clone%s found for user '%s'...\n", nClones, nClones != 1 ? "s":"", who.cstr() );
									
								if ( clones.rows() > 1 )
								{
									for(int i=0;i<clones.rows();i++)
									{
										dword userId = clones[i]["user_id"];
										dword startTime = clones[i]["start_time"];
										CharString remoteIP = (const char *)clones[i]["remote_ip"];
										CharString proxyIP = (const char *)clones[i]["proxy_ip"];
										CharString machineId = (const char *)clones[i]["mid"];
										
										ShortProfile targetProfile;
										CharString who = "";
										if ( getShortProfile( getGameId( client ), userId, targetProfile ) )
											who = targetProfile.name;
											
										info += CharString().format("%s - Name: '%s', UserId: %u, LoginTime: %s, Remote IP: %s, Proxy IP: %s, Machine ID: %s\n", 
											machineId.length() > 0 ? "GCQL":"Forum" ,who.cstr(), userId, Time::format( startTime, "%c" ).cstr(), remoteIP.cstr(), proxyIP.cstr(), machineId.cstr() );

										// prevent info string getting too big and crashing client on send...
										if (i % 20 == 0)
										{
											sendChat(client, info);
											info = "/";
										}
									}
								}
										
								Database::Query offclones( pDB->query( CharString().format("SELECT DISTINCT us.user_id, us.username, us.last_login, us.last_ip, us.last_mid "
									"FROM users AS us LEFT JOIN sessions AS se USING( user_id ) WHERE ISNULL( se.user_id ) "
									"AND ( us.last_ip = '%s' OR us.last_mid = '%s' ) AND us.user_id != %u", remoteIP.cstr(), machineId.cstr(), userId) ) );
										
								if ( offclones.rows() > 0 )
								{
									info += "Possible additional offline matches:\n";
									for(int i=0;i<offclones.rows();i++)
									{
										dword userId = offclones[i]["user_id"];
										dword lastTime = offclones[i]["last_login"];
										CharString remoteIP = (const char *)offclones[i]["last_ip"];
										CharString machineId = (const char *)offclones[i]["last_mid"];
										CharString userName = (const char *)offclones[i]["username"];
										
										info += CharString().format("Name: '%s', UserId: %u, Last login: %s, IP: %s, Machine ID: %s\n", 
											userName.cstr(), userId, Time::format( lastTime, "%c" ).cstr(), remoteIP.cstr(), machineId.cstr() );

										// prevent info string getting too big and crashing client on send...
										if (i % 20 == 0)
										{
											sendChat(client, info);
											info = "/";
										}
									}
								}
										
								sendChat( client, info );
										
							}
							else
								sendChat( client, CharString().format("/Error, no information found on user '%s'...", who.cstr()) );
						}

						freeConnection( pDB );
					}
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );
			}
			else
				sendChat( client, "/Usage: /clones [user name|@user id]" );
		}
		break;
	case CMD_SERVERS:
		{
			Database * pDB = getConnection();

			Database::Query info( pDB->query( CharString().format("SELECT DISTINCT servers.name, sessions.user_id, users.username FROM servers, sessions, users "
				"WHERE ( servers.type = %u ) AND ( servers.game_id = %u ) AND ( users.user_id = sessions.user_id ) AND ( servers.address = sessions.remote_ip OR "
				"servers.mid = sessions.mid ) AND users.username != \")DSS\"", MetaClient::GAME_SERVER, getGameId( client ) ) ) );
			if ( info.rows() > 0 )
			{
				CharString serverusers;
				serverusers.format( "/Found %d results:\n", info.rows() );

				for(int i=0;i<info.rows();i++)
				{
					dword userId = info[i]["user_id"];
					CharString userName = (const char *)info[i]["username"];
					CharString serverName = (const char *)info[i]["name"];

					serverusers += CharString().format( "User: %s @%d, Servername: %s\n", userName.cstr(), userId, serverName.cstr() );
				}
				
				sendChat( client, serverusers );
					
			}
			else
				sendChat( client, "/No results..." );

			freeConnection( pDB );
		}
		break;
	case CMD_LOGROOM:
		{
			if ( arguments.argumentCount() > 1 )
			{
				dword roomId = strtol( arguments.argument( 1 ) + 1, NULL, 10 );

				if( roomId > 0 )		// roomId must be numeric.
				{
					int limit = 100;
					int offset = 0;
					
					if ( arguments.argumentCount() > 2 )
						limit = strtol( arguments.argument( 2 ), NULL, 10 );
					if ( arguments.argumentCount() > 3 )
						offset = strtol( arguments.argument( 3 ), NULL, 10 );

					Database * pDB = getConnection();
					
					bool bRoomIdOK = true;
					if ( !(profile.flags & MetaClient::ADMINISTRATOR) )		// user is no admin, so may view only non-static rooms
					{
						Database::Query roomFlag( pDB->query( CharString().format("SELECT is_static FROM rooms WHERE room_id=%u", roomId) ) );
						if( roomFlag.rows() == 1 )
						{
							if( (int)roomFlag[0][0] != 1 )
								bRoomIdOK = false;
						}
						else
							bRoomIdOK = false;

					}
					
					if ( bRoomIdOK )
					{
						Database::Query messages( pDB->query( CharString().format("SELECT * FROM chat WHERE room_id=%u "
							"ORDER BY message_id DESC LIMIT %d,%d", roomId, offset, limit) ) );
					
						sendChat( client, CharString().format("/<font color=00ff00>Sending log, %d lines...</font>", messages.rows()) );
					
						// new messages, route the new messages to the clients
						for(int i=messages.rows()-1;i>=0;i--)
						{
							MetaClient::Chat chat;
							chat.author		= messages[i]["author"];
							chat.authorId	= messages[i]["author_id"];
							chat.time		= messages[i]["time"];
							chat.text		= messages[i]["text"];
							chat.recpId		= messages[i]["recp_id"];
							chat.roomId		= messages[i]["room_id"];
						
							send( client, MetaClient::CLIENT_RECV_CHAT) << chat;

						}
					
						sendChat( client, "/<font color=00ff00>Log end...</font>" );
					}
					else
						sendChat( client, "/Error, you may only view the chatlog from the main rooms" );

					freeConnection( pDB );
				}
				else
					sendChat( client, "/Error, RoomId must be numeric, not 0 and prefixed with @" );
			}
			else
				sendChat( client, "/Usage: /logroom [@roomId] [limit] [offset]" );
		}
		break;
	case CMD_LOGUSER:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );
				
				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
					{
						int limit = 100;
						int offset = 0;
						
						if ( arguments.argumentCount() > 2 )
							limit = strtol( arguments.argument( 2 ), NULL, 10 );
						if ( arguments.argumentCount() > 3 )
							offset = strtol( arguments.argument( 3 ), NULL, 10 );
						
						Database * pDB = getConnection();

						Database::Query messages( pDB->query( CharString().format("SELECT * FROM chat WHERE author_id=%u OR recp_id=%u "
							"ORDER BY message_id DESC LIMIT %d,%d", found[0],found[0], offset, limit) ) );
						sendChat( client, CharString().format("/<font color=00ff00>Sending log, %d lines...</font>", messages.rows()) );

						// new messages, route the new messages to the clients
						for(int i=messages.rows()-1;i>=0;i--)
						{
							MetaClient::Chat chat;
							chat.author		= messages[i]["author"];
							chat.authorId	= messages[i]["author_id"];
							chat.time		= messages[i]["time"];
							chat.text		= messages[i]["text"];
							chat.recpId		= messages[i]["recp_id"];
							chat.roomId		= messages[i]["room_id"];

							if( chat.recpId != 0 )
								chat.text = CharString().format( "/(To @%u) %s", chat.recpId, chat.text.cstr() );

							send( client, MetaClient::CLIENT_RECV_CHAT) << chat;
						}
						
						sendChat( client, "/<font color=00ff00>Log end...</font>" );
						freeConnection( pDB );
					}
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );
			}
			else
				sendChat( client, "/Usage: /loguser [user name|@user id] [limit] [offset] " );
		}
		break;
	case CMD_SESSION:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString who( arguments.argument( 1 ) );

				Array< dword > found;
				if ( findUserExactFirst( who, found ) )
				{
					if ( found.size() == 1 )
					{
						Database * pDB = getConnection();

						bool bFoundInfo = false;
						Database::Query session( pDB->query( CharString().format("SELECT * FROM sessions WHERE user_id=%u", found[0]) ) );
						if ( session.rows() > 0 )
						{
							CharString info;
							info.format( "/%d sessions found...\n", session.rows() );

							for(int i=0;i<session.rows();i++)
							{
								dword userId = session[i]["user_id"];
								dword sessionId = session[i]["sess_id"];
								dword startTime = session[i]["start_time"];
								CharString remoteIP = (const char *)session[i]["remote_ip"];
								CharString proxyIP = (const char *)session[i]["proxy_ip"];
								CharString machineId = (const char *)session[i]["mid"];

								info += CharString().format("- UserId: %u, SessionId: %u, Time: %s, Remote IP: %s, Proxy IP: %s, Machine ID: %s\n", 
									userId, sessionId, Time::format( startTime, "%c" ).cstr(), remoteIP.cstr(), proxyIP.cstr(), machineId.cstr());
							}

							sendChat( client, info );
							bFoundInfo = true;
						}
						else		// No session entry, try to get info from users table
						{
							dword userId = found[0];
							Database::Query data( pDB->query( CharString().format("SELECT last_login, last_ip, last_mid FROM users WHERE user_id=%u AND NOT ISNULL( last_ip )", userId) ) );
							if ( data.rows() > 0 )
							{
								CharString info;
								info.format( "/No session information available for user @%u, however the following info was found:\n", userId );
								
								dword lastLogin = data[0]["last_login"];
								CharString remoteIP = (const char *)data[0]["last_ip"];
								CharString machineId = (const char *)data[0]["last_mid"];
									
								info += CharString().format("Last login: %s, Last IP: %s, Last Machine ID: %s\n", 
										Time::format( lastLogin, "%c" ).cstr(), remoteIP.cstr(), machineId.cstr());
	
								sendChat( client, info );
								bFoundInfo = true;
							}
						}

						if( !bFoundInfo )
							sendChat( client, "/No session or offline information found for user..." );

						freeConnection( pDB );
					}
					else
						sendChat( client, CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
				}
				else
					sendChat( client, CharString().format("/Error, failed to find user '%s'...", who.cstr()) );
			}
			else
				sendChat( client, "/Usage: /session [user name|@user id]" );
		}
		break;
	case CMD_HIDE:
		{
			if ( (profile.flags & MetaClient::HIDDEN) == 0 )
			{
				profile.flags |= MetaClient::HIDDEN;

				Database * pDB = getConnection();
				pDB->execute( CharString().format("UPDATE user_status SET hidden=1 WHERE user_id=%u", profile.userId) );
				pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", profile.userId, Time::seconds()) );
				freeConnection( pDB );

				sendChat( client, "/You are hidden..." );
			}
		}
		break;
	case CMD_UNHIDE:
		{
			if ( (profile.flags & MetaClient::HIDDEN) != 0 )
			{
				profile.flags &= ~MetaClient::HIDDEN;

				Database * pDB = getConnection();
				pDB->execute( CharString().format("UPDATE user_status SET hidden=0 WHERE user_id=%u", profile.userId) );
				pDB->execute( CharString().format("INSERT user_reload(user_id,time) values(%u,%u)", profile.userId, Time::seconds()) );
				freeConnection( pDB );

				sendChat( client, "/You are visible..." );
			}
		}
		break;
	case CMD_NOTICE:					// send message to all clients
		{
			if ( arguments.argumentCount() > 1 )
				processMessage( client, 0, 0, withQuotes.restring( 1 ) );
			else
				sendChat( client, "/Usage: /notice [message]" );
		}
		break;
	case CMD_INVALID:
		sendChat( client, "/Invalid command, type '/?' for help..." );
		break;
	}
}

//----------------------------------------------------------------------------
//EOF
