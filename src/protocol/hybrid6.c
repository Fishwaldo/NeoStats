/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
#include "protocol.h"

const char MSG_PRIVATE[] = "PRIVMSG";
const char MSG_WHO[] = "WHO";
const char MSG_WHOIS[] = "WHOIS";
const char MSG_WHOWAS[] = "WHOWAS";
const char MSG_USER[] = "USER";
const char MSG_NICK[] = "NICK";
const char MSG_SERVER[] = "SERVER";
const char MSG_LIST[] = "LIST";
const char MSG_TOPIC[] = "TOPIC";
const char MSG_INVITE[] = "INVITE";
const char MSG_VERSION[] = "VERSION";
const char MSG_QUIT[] = "QUIT";
const char MSG_SQUIT[] = "SQUIT";
const char MSG_KILL[] = "KILL";
const char MSG_INFO[] = "INFO";
const char MSG_LINKS[] = "LINKS";
const char MSG_STATS[] = "STATS";
const char MSG_USERS[] = "USERS";
const char MSG_HELP[] = "HELP";
const char MSG_ERROR[] = "ERROR";
const char MSG_AWAY[] = "AWAY";
const char MSG_CONNECT[] = "CONNECT";
const char MSG_PING[] = "PING";
const char MSG_PONG[] = "PONG";
const char MSG_OPER[] = "OPER";
const char MSG_PASS[] = "PASS";
const char MSG_WALLOPS[] = "WALLOPS";
const char MSG_TIME[] = "TIME";
const char MSG_NAMES[] = "NAMES";
const char MSG_ADMIN[] = "ADMIN";
const char MSG_TRACE[] = "TRACE";
const char MSG_LTRACE[] = "LTRACE";
const char MSG_NOTICE[] = "NOTICE";
const char MSG_JOIN[] = "JOIN";
const char MSG_PART[] = "PART";
const char MSG_LUSERS[] = "LUSERS";
const char MSG_MOTD[] = "MOTD";
const char MSG_MODE[] = "MODE";
const char MSG_KICK[] = "KICK";
const char MSG_USERHOST[] = "USERHOST";
const char MSG_ISON[] = "ISON";
const char MSG_REHASH[] = "REHASH";
const char MSG_RESTART[] = "RESTART";
const char MSG_CLOSE[] = "CLOSE";
const char MSG_SVINFO[] = "SVINFO";
const char MSG_SJOIN[] = "SJOIN";
const char MSG_CAPAB[] = "CAPAB";
const char MSG_DIE[] = "DIE";
const char MSG_HASH[] = "HASH";
const char MSG_DNS[] = "DNS";
const char MSG_OPERWALL[] = "OPERWALL";
const char MSG_KLINE[] = "KLINE";
const char MSG_UNKLINE[] = "UNKLINE";
const char MSG_DLINE[] = "DLINE";
const char MSG_UNDLINE[] = "UNDLINE";
const char MSG_HTM[] = "HTM";
const char MSG_SET[] = "SET";
const char MSG_GLINE[] = "GLINE";
const char MSG_UNGLINE[] = "UNGLINE";
const char MSG_LOCOPS[] = "LOCOPS";
const char MSG_LWALLOPS[] = "LWALLOPS";
const char MSG_KNOCK[] = "KNOCK";
const char MSG_MAP[] = "MAP";
const char MSG_ETRACE[] = "ETRACE";
const char MSG_SINFO[] = "SINFO";
const char MSG_TESTLINE[] = "TESTLINE";
const char MSG_OPERSPY[] = "OPERSPY";
const char MSG_ENCAP[] = "ENCAP";
const char MSG_XLINE[] = "XLINE";
const char MSG_UNXLINE[] = "UNXLINE";
const char MSG_RESV[] = "RESV";
const char MSG_UNRESV[] = "UNRESV";

/* Umodes */
#define UMODE_SERVNOTICE   0x00000000 /* server notices such as kill */
#define UMODE_REJ          0x00000000 /* Bot Rejections */
#define UMODE_SKILL        0x00000000 /* Server Killed */
#define UMODE_FULL         0x00080000 /* Full messages */
#define UMODE_SPY          0x00100000 /* see STATS / LINKS */
#define UMODE_DEBUG        0x00200000 /* 'debugging' info */
#define UMODE_NCHANGE      0x00400000 /* Nick change notice */
#define UMODE_OPERWALL     0x00800000 /* Operwalls */
#define UMODE_BOTS         0x01000000 /* shows bots */
#define UMODE_EXTERNAL     0x02000000 /* show servers introduced and splitting */
#define UMODE_CALLERID     0x04000000 /* block unless caller id's */
#define UMODE_UNAUTH       0x08000000 /* show unauth connects here */
#define UMODE_STATSPHIDE   0x10000000 /* Oper hides from stats P */
#define UMODE_OSPYLOG      0x20000000 /* show Oper Spy being used */
#define UMODE_UNIDLE       0x40000000 /* Hide idle time with umode +u */
#define UMODE_LOCOPS       0x80000000 /* Oper see's LOCOPS */
 
/* Cmodes */
#define CMODE_EXCEPTION  0x0800
#define CMODE_DENY       0x1000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	0,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	63,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	9,
	/* Max real name length */
	50,
	/* Max channel name length */
	200,
	/* Max topic length */
	120,
	/* Default operator modes for NeoStats service bots */
	"+o",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_SERVER, 0, m_server, 0},
	{MSG_NICK, 0, m_nick, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{0, 0},
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	/* CAP QS ZIP EX CHW KNOCK KLN UNKLN CLUSTER ENCAP IE */
	send_cmd( "CAPAB :EX CHW IE KLN KNOCK" );
	send_cmd( "%s %s :%s", MSG_SERVER, name, infoline );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname );
}

/*  m_nick
 *   RX: SERVER servername 1 :hybrid6server
 *   argv[0] = servername
 *   argv[1] = hopcount
 *   argv[2] = serverinfo
 */

static void m_server( char *origin, char **argv, int argc, int srv )
{
	do_server( argv[0], origin, argv[1], NULL, argv[2], srv );
	if( !srv )
		do_synch_neostats();
}

/*  m_nick
 *   argv[0] = nickname
 *   argv[1] = optional hopcount when new user; TS when nick change
 *   argv[2] = optional TS
 *   argv[3] = optional umode
 *   argv[4] = optional username
 *   argv[5] = optional hostname
 *   argv[6] = optional server
 *   argv[7] = optional ircname
 */

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			NULL, NULL, argv[3], NULL, argv[7], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], argv[1] );
	}
}

