/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/

#include "neostats.h"
#include "ircd.h"

/* Messages/Tokens */
const char MSG_PRIVATE[] = "PRIVMSG";		/* PRIV */
const char MSG_WHO[] = "WHO";	      	/* WHO  -> WHOC */
const char MSG_WHOIS[] = "WHOIS";	   	/* WHOI */
const char MSG_WHOWAS[] = "WHOWAS";	   	/* WHOW */
const char MSG_USER[] = "USER";	   	/* USER */
const char MSG_NICK[] = "NICK";	   	/* NICK */
const char MSG_SERVER[] = "SERVER";	   	/* SERV */
const char MSG_LIST[] = "LIST";	   	/* LIST */
const char MSG_TOPIC[] = "TOPIC";	   	/* TOPI */
const char MSG_INVITE[] = "INVITE";	   	/* INVI */
const char MSG_VERSION[] = "VERSION";		/* VERS */
const char MSG_QUIT[] = "QUIT";	   	/* QUIT */
const char MSG_SQUIT[] = "SQUIT";	   	/* SQUI */
const char MSG_KILL[] = "KILL";	   	/* KILL */
const char MSG_INFO[] = "INFO";	   	/* INFO */
const char MSG_LINKS[] = "LINKS";	   	/* LINK */
const char MSG_STATS[] = "STATS";	   	/* STAT */
const char MSG_USERS[] = "USERS";	   	/* USER -> USRS */
const char MSG_HELP[] = "HELP";	   	/* HELP */
const char MSG_ERROR[] = "ERROR";	   	/* ERRO */
const char MSG_AWAY[] = "AWAY";	   	/* AWAY */
const char MSG_CONNECT[] = "CONNECT";		/* CONN */
const char MSG_PING[] = "PING";	   	/* PING */
const char MSG_PONG[] = "PONG";	   	/* PONG */
const char MSG_OPER[] = "OPER";	   	/* OPER */
const char MSG_PASS[] = "PASS";	   	/* PASS */
const char MSG_WALLOPS[] = "WALLOPS";		/* WALL */
const char MSG_TIME[] = "TIME";	   	/* TIME */
const char MSG_NAMES[] = "NAMES";	   	/* NAME */
const char MSG_ADMIN[] = "ADMIN";	   	/* ADMI */
const char MSG_TRACE[] = "TRACE";	   	/* TRAC */
const char MSG_NOTICE[] = "NOTICE";	   	/* NOTI */
const char MSG_JOIN[] = "JOIN";	   	/* JOIN */
const char MSG_PART[] = "PART";	   	/* PART */
const char MSG_LUSERS[] = "LUSERS";	   	/* LUSE */
const char MSG_MOTD[] = "MOTD";	   	/* MOTD */
const char MSG_MODE[] = "MODE";	   	/* MODE */
const char MSG_KICK[] = "KICK";	   	/* KICK */
const char MSG_USERHOST[] = "USERHOST";		/* USER -> USRH */
const char MSG_ISON[] = "ISON";	   	/* ISON */
const char MSG_REHASH[] = "REHASH";	   	/* REHA */
const char MSG_RESTART[] = "RESTART";		/* REST */
const char MSG_CLOSE[] = "CLOSE";	   	/* CLOS */
const char MSG_SVINFO[] = "SVINFO";	   	/* SVINFO */
const char MSG_SJOIN[] = "SJOIN";	   	/* SJOIN */
const char MSG_DIE[] = "DIE"; 		/* DIE */
const char MSG_HASH[] = "HASH";	   	/* HASH */
const char MSG_DNS[] = "DNS";   	   	/* DNS  -> DNSS */
const char MSG_OPERWALL[] = "OPERWALL";		/* OPERWALL */
const char MSG_GLOBOPS[] = "GLOBOPS";		/* GLOBOPS */
const char MSG_CHATOPS[] = "CHATOPS";		/* CHATOPS */
const char MSG_GOPER[] = "GOPER";	   	/* GOPER */
const char MSG_GNOTICE[] = "GNOTICE";		/* GNOTICE */
const char MSG_KLINE[] = "KLINE";	   	/* KLINE */
const char MSG_UNKLINE[] = "UNKLINE";		/* UNKLINE */
const char MSG_HTM[] = "HTM";	      	/* HTM */
const char MSG_SET[] = "SET";	      	/* SET */
const char MSG_SAMODE[] = "SAMODE";    	/* SAMODE */
const char MSG_CHANSERV[] = "CHANSERV";		/* CHANSERV */
const char MSG_NICKSERV[] = "NICKSERV";		/* NICKSERV */
const char MSG_MEMOSERV[] = "MEMOSERV";		/* MEMOSERV */
const char MSG_ROOTSERV[] = "ROOTSERV";		/* MEMOSERV */
const char MSG_OPERSERV[] = "OPERSERV";		/* OPERSERV */
const char MSG_STATSERV[] = "STATSERV"; 	/* STATSERV */
const char MSG_HELPSERV[] = "HELPSERV"; 	/* HELPSERV */
const char MSG_SERVICES[] = "SERVICES";		/* SERVICES */
const char MSG_IDENTIFY[] = "IDENTIFY";		/* IDENTIFY */
const char MSG_CAPAB[] = "CAPAB";	   	/* CAPAB */ 
const char MSG_LOCOPS[] = "LOCOPS";	   	/* LOCOPS */
const char MSG_SVSNICK[] = "SVSNICK";   	/* SVSNICK */
const char MSG_SVSNOOP[] = "SVSNOOP";   	/* SVSNOOP */
const char MSG_SVSKILL[] = "SVSKILL";   	/* SVSKILL */
const char MSG_SVSMODE[] = "SVSMODE";   	/* SVSMODE */
const char MSG_AKILL[] = "AKILL";     	/* AKILL */
const char MSG_RAKILL[] = "RAKILL";    	/* RAKILL */
const char MSG_SILENCE[] = "SILENCE";   	/* SILENCE */
const char MSG_WATCH[] = "WATCH";     	/* WATCH */
const char MSG_SQLINE[] = "SQLINE"; 		/* SQLINE */
const char MSG_UNSQLINE[] = "UNSQLINE"; 	/* UNSQLINE */
const char MSG_BURST[] = "BURST";     	/* BURST */
const char MSG_DCCALLOW[] = "DCCALLOW";		/* dccallow */
const char MSG_SGLINE[] = "SGLINE";           /* sgline */
const char MSG_UNSGLINE[] = "UNSGLINE";         /* unsgline */
const char MSG_DKEY[] = "DKEY";		/* diffie-hellman negotiation */
const char MSG_NS[] = "NS";            	/* NickServ commands */
const char MSG_CS[] = "CS";            	/* ChanServ commands */
const char MSG_MS[] = "MS";            	/* MemoServ commands */
const char MSG_RS[] = "RS";            	/* RootServ commands */
const char MSG_OS[] = "OS";            	/* OperServ commands */
const char MSG_SS[] = "SS";            	/* StatServ commands */
const char MSG_HS[] = "HS";            	/* StatServ commands */

/* Umodes */
#define UMODE_SERVNOTICE	0x00100000	/* umode +s - Server notices */
#define UMODE_KILLS			0x00200000	/* umode +k - Server kill messages */
#define UMODE_FLOOD			0x00400000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00800000	/* umode +y - Stats/links */
#define UMODE_DEBUG			0x00100000	/* umode +d - Debug info */
#define UMODE_GLOBOPS		0x00200000	/* umode +g - Globops */
#define UMODE_CHATOPS		0x00400000	/* umode +b - Chatops */
#define UMODE_ROUTE			0x00800000	/* umode +n - Routing Notices */
#define UMODE_SPAM			0x01000000	/* umode +m - spambot notices */
#define UMODE_OPERNOTICE	0x02000000	/* umode +e - oper notices for the above +D */
#define UMODE_SQUELCH		0x04000000	/* umode +x - Squelch with notice */
#define UMODE_SQUELCHN		0x08000000	/* umode +X - Squelch without notice */
#define UMODE_HIDDENDCC     0x10000000	/* umode +D - Hidden dccallow umode */
#define UMODE_THROTTLE		0x20000000	/* umode +F - no cptr->since message rate throttle */
#define UMODE_REJ			0x40000000	/* umode +j - client rejection notices */
#define UMODE_ULINEKILL     0x80000000	/* umode +K - U: lined server kill messages */

/* Cmodes */
#define CMODE_MODREG	0x02000000
#define CMODE_LISTED	0x04000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_svsnick( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );

/* buffer sizes */
const int proto_maxhost		=( 128 + 1 );
const int proto_maxpass		=( 63 + 1 );
const int proto_maxnick		=( 30 + 1 );
const int proto_maxuser		=( 10 + 1 );
const int proto_maxrealname	=( 50 + 1 );
const int proto_chanlen		=( 32 + 1 );
const int proto_topiclen	=( 307 + 1 );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_NICKIP,
	/* Features supported by this IRCd */
	0,
	"+oS",
	"+o",
};

ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_SERVER, 0, m_server, 0},
	{MSG_SVSMODE, 0, m_svsmode, 0},
	{MSG_AWAY, 0, _m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, _m_topic, 0},
	{MSG_CAPAB, 0, _m_capab, 0},
	{MSG_BURST, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_PASS, 0, _m_pass, 0},
	{MSG_SVSNICK, 0, m_svsnick, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{MSG_ERROR, 0, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'x', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'M', CMODE_MODREG, 0},
	{'L', CMODE_LISTED, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'w', UMODE_WALLOP},
	{'s', UMODE_SERVNOTICE},
	{'c', UMODE_CLIENT},
	{'k', UMODE_KILLS},
	{'f', UMODE_FLOOD},
	{'y', UMODE_SPY},
	{'d', UMODE_DEBUG},
	{'g', UMODE_GLOBOPS},
	{'b', UMODE_CHATOPS},
	{'n', UMODE_ROUTE},
	{'h', UMODE_HELPOP},
	{'m', UMODE_SPAM},
	{'R', UMODE_RGSTRONLY},
	{'e', UMODE_OPERNOTICE},
	{'x', UMODE_SQUELCH},
	{'X', UMODE_SQUELCHN},
	{'D', UMODE_HIDDENDCC},
	{'F', UMODE_THROTTLE},
	{'j', UMODE_REJ},
	{'K', UMODE_ULINEKILL},
	{0, 0},
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB TS3 SSJOIN BURST NICKIP" );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, ts, newmode, ident, host, server, ts, realname );
}

void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %lu %s %lu :%s", source, MSG_AKILL, host, ident, length, setby, ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s %s %s", source, MSG_RAKILL, host, ident );
}

void send_burst( int b )
{
	if( b == 0 )
		send_cmd( "BURST 0" );
	else
		send_cmd( "BURST" );
}

/* from SJOIN TS TS chan modebuf parabuf :nicks */
/* from SJOIN TS TS chan modebuf :nicks */
/* from SJOIN TS TS chan modebuf parabuf : */
/* from SJOIN TS TS chan + :@nicks */
/* from SJOIN TS TS chan 0 : */
/* from SJOIN TS chan modebuf parabuf :nicks */
/* from SJOIN TS chan modebuf :nicks */
/* from SJOIN TS chan modebuf parabuf : */
/* from SJOIN TS chan + :@nicks */
/* from SJOIN TS chan 0 : */
/* from SJOIN TS chan */

static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1],(( argc <= 2 ) ? argv[1] : argv[2] ), origin, argv, argc );
}

static void m_burst( char *origin, char **argv, int argc, int srv )
{
	do_burst( origin, argv, argc );
}

static void m_server( char *origin, char **argv, int argc, int srv )
{
	if( argc > 2 ) {
		do_server( argv[0], origin, argv[1], argv[2], NULL, srv );
	} else {
		do_server( argv[0], origin, NULL, argv[1], NULL, srv );
	}
}

static void m_svsmode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' ) {
		do_svsmode_channel( origin, argv, argc );
	} else {
		do_svsmode_user( argv[0], argv[2], NULL );
	}
}

/* m_nick 
 * argv[0] = nickname 
 * argv[1] = hopcount when new user; TS when nick change 
 * argv[2] = TS
 * ---- new user only below ---- 
 * argv[3] = umode 
 * argv[4] = username 
 * argv[5] = hostname 
 * argv[6] = server 
 * argv[7] = serviceid
 * -- If NICKIP
 * argv[8] = IP
 * argv[9] = ircname
 * -- else
 * argv[8] = ircname
 * -- endif
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], argv[1] );
	}
}

static void m_svsnick( char *origin, char **argv, int argc, int srv )
{
	do_nickchange( argv[0], argv[1], NULL );
}
