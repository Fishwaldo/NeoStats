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
#define OVERRIDECOREMESSAGESUPPORT
#include "ircd.h"
#include "base64.h"
#include "numerics.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

/* QuakeNet Asuka - http://dev.quakenet.org */

/* Messages/Tokens */

const char MSG_PRIVATE[] = "PRIVMSG";
const char TOK_PRIVATE[] = "P";
const char MSG_WHO[] = "WHO";
const char TOK_WHO[] = "H";
const char MSG_WHOIS[] = "WHOIS";
const char TOK_WHOIS[] = "W";
const char MSG_WHOWAS[] = "WHOWAS";
const char TOK_WHOWAS[] = "X";
const char MSG_USER[] = "USER";
const char TOK_USER[] = "USER";
const char MSG_NICK[] = "NICK";
const char TOK_NICK[] = "N";
const char MSG_SERVER[] = "SERVER";
const char TOK_SERVER[] = "S";
const char MSG_LIST[] = "LIST";
const char TOK_LIST[] = "LIST";
const char MSG_TOPIC[] = "TOPIC";
const char TOK_TOPIC[] = "T";
const char MSG_INVITE[] = "INVITE";
const char TOK_INVITE[] = "I";
const char MSG_VERSION[] = "VERSION";
const char TOK_VERSION[] = "V";
const char MSG_QUIT[] = "QUIT";
const char TOK_QUIT[] = "Q";
const char MSG_SQUIT[] = "SQUIT";
const char TOK_SQUIT[] = "SQ";
const char MSG_KILL[] = "KILL";
const char TOK_KILL[] = "D";
const char MSG_INFO[] = "INFO";
const char TOK_INFO[] = "F";
const char MSG_LINKS[] = "LINKS";
const char TOK_LINKS[] = "LI";
const char MSG_STATS[] = "STATS";
const char TOK_STATS[] = "R";
const char MSG_HELP[] = "HELP";
const char TOK_HELP[] = "HELP";
const char MSG_ERROR[] = "ERROR";
const char TOK_ERROR[] = "Y";
const char MSG_AWAY[] = "AWAY";
const char TOK_AWAY[] = "A";
const char MSG_CONNECT[] = "CONNECT";
const char TOK_CONNECT[] = "CO";
const char MSG_MAP[] = "MAP";
const char TOK_MAP[] = "MAP";
const char MSG_PING[] = "PING";
const char TOK_PING[] = "G";
const char MSG_PONG[] = "PONG";
const char TOK_PONG[] = "Z";
const char MSG_OPER[] = "OPER";
const char TOK_OPER[] = "OPER";
const char MSG_PASS[] = "PASS";
const char TOK_PASS[] = "PA";
const char MSG_WALLOPS[] = "WALLOPS";
const char TOK_WALLOPS[] = "WA";
const char MSG_WALLUSERS[] = "WALLUSERS";
const char TOK_WALLUSERS[] = "WU";
const char MSG_DESYNCH[] = "DESYNCH";
const char TOK_DESYNCH[] = "DS";
const char MSG_TIME[] = "TIME";
const char TOK_TIME[] = "TI";
const char MSG_SETTIME[] = "SETTIME";
const char TOK_SETTIME[] = "SE";
const char MSG_RPING[] = "RPING";
const char TOK_RPING[] = "RI";
const char MSG_RPONG[] = "RPONG";
const char TOK_RPONG[] = "RO";
const char MSG_NAMES[] = "NAMES";
const char TOK_NAMES[] = "E";
const char MSG_ADMIN[] = "ADMIN";
const char TOK_ADMIN[] = "AD";
const char MSG_TRACE[] = "TRACE";
const char TOK_TRACE[] = "TR";
const char MSG_NOTICE[] = "NOTICE";
const char TOK_NOTICE[] = "O";
const char MSG_WALLCHOPS[] = "WALLCHOPS";
const char TOK_WALLCHOPS[] = "WC";
const char MSG_WALLVOICES[] = "WALLVOICES";
const char TOK_WALLVOICES[] = "WV";
const char MSG_CPRIVMSG[] = "CPRIVMSG";
const char TOK_CPRIVMSG[] = "CP";
const char MSG_CNOTICE[] = "CNOTICE";
const char TOK_CNOTICE[] = "CN";
const char MSG_JOIN[] = "JOIN";
const char TOK_JOIN[] = "J";
const char MSG_PART[] = "PART";
const char TOK_PART[] = "L";
const char MSG_LUSERS[] = "LUSERS";
const char TOK_LUSERS[] = "LU";
const char MSG_MOTD[] = "MOTD";
const char TOK_MOTD[] = "MO";
const char MSG_MODE[] = "MODE";
const char TOK_MODE[] = "M";
const char MSG_KICK[] = "KICK";
const char TOK_KICK[] = "K";
const char MSG_USERHOST[] = "USERHOST";
const char TOK_USERHOST[] = "USERHOST";
const char MSG_USERIP[] = "USERIP";
const char TOK_USERIP[] = "USERIP";
const char MSG_ISON[] = "ISON";
const char TOK_ISON[] = "ISON";
const char MSG_SQUERY[] = "SQUERY";
const char TOK_SQUERY[] = "SQUERY";
const char MSG_SERVLIST[] = "SERVLIST";
const char TOK_SERVLIST[] = "SERVSET";
const char MSG_SERVSET[] = "SERVSET";
const char TOK_SERVSET[] = "SERVSET";
const char MSG_CHECK[] = "CHECK";
const char TOK_CHECK[] = "CC";
const char MSG_REHASH[] = "REHASH";
const char TOK_REHASH[] = "REHASH";
const char MSG_RESTART[] = "RESTART";
const char TOK_RESTART[] = "RESTART";
const char MSG_CLOSE[] = "CLOSE";
const char TOK_CLOSE[] = "CLOSE";
const char MSG_DIE[] = "DIE";
const char TOK_DIE[] = "DIE";
const char MSG_HASH[] = "HASH";
const char TOK_HASH[] = "HASH";
const char MSG_DNS[] = "DNS";
const char TOK_DNS[] = "DNS";
const char MSG_SILENCE[] = "SILENCE";
const char TOK_SILENCE[] = "U";
const char MSG_GLINE[] = "GLINE";
const char TOK_GLINE[] = "GL";
const char MSG_BURST[] = "BURST";
const char TOK_BURST[] = "B";
const char MSG_UPING[] = "UPING";
const char TOK_UPING[] = "UP";
const char MSG_CREATE[] = "CREATE";
const char TOK_CREATE[] = "C";
const char MSG_DESTRUCT[] = "DESTRUCT";
const char TOK_DESTRUCT[] = "DE";
const char MSG_END_OF_BURST[] = "END_OF_BURST";
const char TOK_END_OF_BURST[] = "EB";
const char MSG_END_OF_BURST_ACK[] = "EOB_ACK";
const char TOK_END_OF_BURST_ACK[] = "EA";
const char MSG_PROTO[] = "PROTO";
const char TOK_PROTO[] = "PROTO";
const char MSG_JUPE[] = "JUPE";
const char TOK_JUPE[] = "JU";
const char MSG_OPMODE[] = "OPMODE";
const char TOK_OPMODE[] = "OM";
const char MSG_CLEARMODE[] = "CLEARMODE";
const char TOK_CLEARMODE[] = "CM";
const char MSG_ACCOUNT[] = "ACCOUNT";
const char TOK_ACCOUNT[] = "AC";
const char MSG_ASLL[] = "ASLL";
const char TOK_ASLL[] = "LL";
const char MSG_POST[] = "POST";
const char TOK_POST[] = "POST";
const char MSG_SET[] = "SET";
const char TOK_SET[] = "SET";
const char MSG_RESET[] = "RESET";
const char TOK_RESET[] = "RESET";
const char MSG_GET[] = "GET";
const char TOK_GET[] = "GET";
const char MSG_PRIVS[] = "PRIVS";
const char TOK_PRIVS[] = "PRIVS";
const char MSG_SETHOST[] = "SETHOST";
const char TOK_SETHOST[] = "SH";

 /* User modes: */
#define UMODE_SERVNOTICE        0x00800000	/* See server notices */
#define UMODE_DEBUG             0x01000000	/* See hack notices */
#define UMODE_SETHOST		0x08000000	/* */
#define UMODE_NOCHAN		0x10000000	/* */
#define UMODE_NOIDLE		0x20000000	/* */
#define UMODE_XTRAOP		0x40000000	/* */

/* Cmodes */
#define CMODE_NOCTCP		0x02000000
#define CMODE_NONOTICE		0x04000000
#define CMODE_NOQUITPARTS	0x08000000
#define CMODE_DELJOINS		0x10000000

/* I really hate including .c files but this is a temporary measure
 * while core/IRCu interaction is improved.
 */
#include "ircup10base.c"

static void m_nick( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_TOKEN|PROTOCOL_NOQUIT|PROTOCOL_B64SERVER|PROTOCOL_B64NICK|PROTOCOL_NICKIP|PROTOCOL_KICKPART,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	FEATURE_SVSTIME,
	/* Max host length */
	63 ,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	200,
	/* Max topic length */
	250,
	/* Default operator modes for NeoStats service bots */
	"+o",
	/* Default channel mode for NeoStats service bots */
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_PRIVATE, TOK_PRIVATE, m_private, 0},
	{MSG_CPRIVMSG, TOK_CPRIVMSG, m_private, 0},
	{MSG_NOTICE, TOK_NOTICE, m_notice, 0},
	{MSG_CNOTICE, TOK_CNOTICE, m_notice, 0},
	{MSG_STATS, TOK_STATS, m_stats, 0},
	{MSG_VERSION, TOK_VERSION, m_version, 0},
	{MSG_MOTD, TOK_MOTD, m_motd, 0},
	{MSG_ADMIN, TOK_ADMIN, m_admin, 0},
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_SQUIT, TOK_SQUIT, m_squit, 0},
	{MSG_QUIT, TOK_QUIT, m_quit, 0},
	{MSG_MODE, TOK_MODE, m_mode, 0},
	{MSG_KILL, TOK_KILL, m_kill, 0},
	{MSG_PONG, TOK_PONG, m_pong, 0},
	{MSG_AWAY, TOK_AWAY, m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, m_topic, 0},
	{MSG_KICK, TOK_KICK, m_kick, 0},
	{MSG_CREATE, TOK_CREATE, m_create, 0},
	{MSG_JOIN, TOK_JOIN, m_join, 0},
	{MSG_PART, TOK_PART, m_part, 0},
	{MSG_PING, TOK_PING, m_ping, 0},
	{MSG_PASS, TOK_PASS, _m_pass, 0},
	{MSG_BURST, TOK_BURST, m_burst, 0},
	{MSG_END_OF_BURST, TOK_END_OF_BURST, m_end_of_burst, 0},
	{MSG_END_OF_BURST_ACK, TOK_END_OF_BURST_ACK, _m_ignorecommand, 0},
	{MSG_WALLOPS, TOK_WALLOPS, m_wallops, 0},
	{MSG_WALLUSERS, TOK_WALLUSERS, m_wallusers, 0},
	{MSG_ERROR, TOK_ERROR, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'c', CMODE_NOCOLOR, 0},
	{'C', CMODE_NOCTCP, 0},
	{'N', CMODE_NONOTICE, 0},
	{'D', CMODE_DELJOINS, 0},
	{'u', CMODE_NOQUITPARTS, 0},
	{'r', CMODE_RGSTRONLY, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{'O', UMODE_LOCOP},
	{'g', UMODE_DEBUG},
	{'w', UMODE_WALLOP},
	{'s', UMODE_SERVNOTICE},
	{'d', UMODE_DEAF},
	{'k', UMODE_SERVICES},
	{'r', UMODE_REGNICK},
	{'x', UMODE_HIDE},
	{'h', UMODE_SETHOST},
	{'n', UMODE_NOCHAN},
	{'I', UMODE_NOIDLE},
	{'R', UMODE_RGSTRONLY},
	{'X', UMODE_XTRAOP},
	{0, 0},
};

/*
1 <nickname>
2 <hops>
3 <TS>
4 <userid>
5 <host>
6 [<+modes>]
7+ [<mode parameters>]
-3 <base64 IP>
-2 <numeric>
-1 <fullname>
*/
/* R: AB N Mark 1 1076011621 a xxx.xxx.xxx.xxx DAqO4N ABAAB :M */
/* R: AB N TheEggMan 1 1076104492 ~eggy 64.XX.XXX.XXX +oiwg BAFtnj ABAAA :eggy */
/* R: ABAAH N m2 1076077934 */
/*
<reed> in a generated burst message, the users must be sorted by the modes: first users w/o modes, then users with voice, then with op, then with op+voice: num,num:v,num:o,num:ov
*/

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( argc > 2 ) {
		char IPAddress[32];
		unsigned long IP;
		const char *modes;
		const char *modeptr;
		const char *account = NULL;
		const char *sethost = NULL;
		int param;

		modes =( argv[5][0] == '+' ) ? argv[5]: NULL;
		if( modes ) {
			param = 6;
			for( modeptr = modes; *modeptr; ++modeptr ) {
				switch( *modeptr ) {
				case 'r':
					account = argv[param++];
					break;
				case 'h':
					sethost = argv[param++];
					break;
				default:
					break;
				} /* switch( *modeptr ) */
			} /* for( ) */
		} /* if( modes ) */

		IP = htonl( base64toIP( argv[argc-3] ) );
		ircsnprintf( IPAddress, 32, "%lu", IP );

		/*       nick,    hopcount, TS,     user,    host, */       
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], 
			/* server, ip, servicestamp, modes, */
			base64_to_server( origin ), IPAddress, NULL, modes,
			/* vhost, realname, numeric, smodes */ 
			NULL, argv[argc-1], argv[argc-2], NULL );
	} else {
		do_nickchange( base64_to_nick( origin ), argv[0], argv[1] );
	}
}

