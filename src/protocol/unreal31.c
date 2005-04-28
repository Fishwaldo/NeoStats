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
#include "protocol.h"

/* Messages/Tokens */
const char MSG_PRIVATE[] = "PRIVMSG";
const char TOK_PRIVATE[] = "!";
const char MSG_WHO[] = "WHO";
const char TOK_WHO[] = "\"";
const char MSG_WHOIS[] = "WHOIS";
const char TOK_WHOIS[] = "#";
const char MSG_WHOWAS[] = "WHOWAS";
const char TOK_WHOWAS[] = "$";
const char MSG_USER[] = "USER";
const char TOK_USER[] = "%";
const char MSG_NICK[] = "NICK";
const char TOK_NICK[] = "&";
const char MSG_SERVER[] = "SERVER";
const char TOK_SERVER[] = "'";
const char MSG_LIST[] = "LIST";
const char TOK_LIST[] = "(";
const char MSG_TOPIC[] = "TOPIC";
const char TOK_TOPIC[] = ")";
const char MSG_INVITE[] = "INVITE";
const char TOK_INVITE[] = "*";
const char MSG_VERSION[] = "VERSION";
const char TOK_VERSION[] = "+";
const char MSG_QUIT[] = "QUIT";
const char TOK_QUIT[] = ",";
const char MSG_SQUIT[] = "SQUIT";
const char TOK_SQUIT[] = "-";
const char MSG_KILL[] = "KILL";
const char TOK_KILL[] = ".";
const char MSG_INFO[] = "INFO";
const char TOK_INFO[] = "/";
const char MSG_LINKS[] = "LINKS";
const char TOK_LINKS[] = "0";
const char MSG_SUMMON[] = "SUMMON";
const char TOK_SUMMON[] = "1";
const char MSG_STATS[] = "STATS";
const char TOK_STATS[] = "2";
const char MSG_USERS[] = "USERS";
const char TOK_USERS[] = "3";
const char MSG_HELP[] = "HELP";
const char MSG_HELPOP[] = "HELPOP";
const char TOK_HELP[] = "4";
const char MSG_ERROR[] = "ERROR";
const char TOK_ERROR[] = "5";
const char MSG_AWAY[] = "AWAY";
const char TOK_AWAY[] = "6";
const char MSG_CONNECT[] = "CONNECT";
const char TOK_CONNECT[] = "7";
const char MSG_PING[] = "PING";
const char TOK_PING[] = "8";
const char MSG_PONG[] = "PONG";
const char TOK_PONG[] = "9";
const char MSG_OPER[] = "OPER";
const char TOK_OPER[] = ";";
const char MSG_PASS[] = "PASS";
const char TOK_PASS[] = "<";
const char MSG_WALLOPS[] = "WALLOPS";
const char TOK_WALLOPS[] = "=";
const char MSG_TIME[] = "TIME";
const char TOK_TIME[] = ">";
const char MSG_NAMES[] = "NAMES";
const char TOK_NAMES[] = "?";
const char MSG_ADMIN[] = "ADMIN";
const char TOK_ADMIN[] = "@";
const char MSG_NOTICE[] = "NOTICE";
const char TOK_NOTICE[] = "B";
const char MSG_JOIN[] = "JOIN";
const char TOK_JOIN[] = "C";
const char MSG_PART[] = "PART";
const char TOK_PART[] = "D";
const char MSG_LUSERS[] = "LUSERS";
const char TOK_LUSERS[] = "E";
const char MSG_MOTD[] = "MOTD";
const char TOK_MOTD[] = "F";
const char MSG_MODE[] = "MODE";
const char TOK_MODE[] = "G";
const char MSG_KICK[] = "KICK";
const char TOK_KICK[] = "H";
const char MSG_SERVICE[] = "SERVICE";
const char TOK_SERVICE[] = "I";
const char MSG_USERHOST[] = "USERHOST";
const char TOK_USERHOST[] = "J";
const char MSG_ISON[] = "ISON";
const char TOK_ISON[] = "K";
const char MSG_REHASH[] = "REHASH";
const char TOK_REHASH[] = "O";
const char MSG_RESTART[] = "RESTART";
const char TOK_RESTART[] = "P";
const char MSG_CLOSE[] = "CLOSE";
const char TOK_CLOSE[] = "Q";
const char MSG_DIE[] = "DIE";
const char TOK_DIE[] = "R";
const char MSG_HASH[] = "HASH";
const char TOK_HASH[] = "S";
const char MSG_DNS[] = "DNS";
const char TOK_DNS[] = "T";
const char MSG_SILENCE[] = "SILENCE";
const char TOK_SILENCE[] = "U";
const char MSG_AKILL[] = "AKILL";
const char TOK_AKILL[] = "V";
const char MSG_KLINE[] = "KLINE";
const char TOK_KLINE[] = "W";
const char MSG_UNKLINE[] = "UNKLINE";
const char TOK_UNKLINE[] = "X";
const char MSG_RAKILL[] = "RAKILL";
const char TOK_RAKILL[] = "Y";
const char MSG_GNOTICE[] = "GNOTICE";
const char TOK_GNOTICE[] = "Z";
const char MSG_GOPER[] = "GOPER";
const char TOK_GOPER[] = "[";
const char MSG_GLOBOPS[] = "GLOBOPS";
const char TOK_GLOBOPS[] = "]";
const char MSG_LOCOPS[] = "LOCOPS";
const char TOK_LOCOPS[] = "^";
const char MSG_PROTOCTL[] = "PROTOCTL";
const char TOK_PROTOCTL[] = "_";
const char MSG_WATCH[] = "WATCH";
const char TOK_WATCH[] = "`";
const char MSG_TRACE[] = "TRACE";
const char TOK_TRACE[] = "b";
const char MSG_SQLINE[] = "SQLINE";
const char TOK_SQLINE[] = "c";
const char MSG_UNSQLINE[] = "UNSQLINE";
const char TOK_UNSQLINE[] = "d";
const char MSG_SVSNICK[] = "SVSNICK";
const char TOK_SVSNICK[] = "e";
const char MSG_SVSNOOP[] = "SVSNOOP";
const char TOK_SVSNOOP[] = "f";
const char MSG_IDENTIFY[] = "IDENTIFY";
const char TOK_IDENTIFY[] = "g";
const char MSG_SVSKILL[] = "SVSKILL";
const char TOK_SVSKILL[] = "h";
const char MSG_NICKSERV[] = "NICKSERV";
const char MSG_NS[] = "NS";
const char TOK_NICKSERV[] = "i";
const char MSG_CHANSERV[] = "CHANSERV";
const char MSG_CS[] = "CS";
const char TOK_CHANSERV[] = "j";
const char MSG_OPERSERV[] = "OPERSERV";
const char MSG_OS[] = "OS";
const char TOK_OPERSERV[] = "k";
const char MSG_MEMOSERV[] = "MEMOSERV";
const char MSG_MS[] = "MS";
const char TOK_MEMOSERV[] = "l";
const char MSG_SERVICES[] = "SERVICES";
const char TOK_SERVICES[] = "m";
const char MSG_SVSMODE[] = "SVSMODE";
const char TOK_SVSMODE[] = "n";
const char MSG_SAMODE[] = "SAMODE";
const char TOK_SAMODE[] = "o";
const char MSG_CHATOPS[] = "CHATOPS";
const char TOK_CHATOPS[] = "p";
const char MSG_ZLINE[] = "ZLINE";
const char TOK_ZLINE[] = "q";
const char MSG_UNZLINE[] = "UNZLINE";
const char TOK_UNZLINE[] = "r";
const char MSG_HELPSERV[] = "HELPSERV";
const char MSG_HS[] = "HS";
const char TOK_HELPSERV[] = "s";
const char MSG_RULES[] = "RULES";
const char TOK_RULES[] = "t";
const char MSG_MAP[] = "MAP";
const char TOK_MAP[] = "u";
const char MSG_SVS2MODE[] = "SVS2MODE";
const char TOK_SVS2MODE[] = "v";
const char MSG_DALINFO[] = "DALINFO";
const char TOK_DALINFO[] = "w";
const char MSG_ADMINCHAT[] = "ADCHAT";
const char TOK_ADMINCHAT[] = "x";
const char MSG_MKPASSWD[] = "MKPASSWD";
const char TOK_MKPASSWD[] = "y";
const char MSG_ADDLINE[] = "ADDLINE";
const char TOK_ADDLINE[] = "z";
const char MSG_GLINE[] = "GLINE";
const char TOK_GLINE[] = "}";
const char MSG_GZLINE[] = "GZLINE";
const char TOK_GZLINE[] = "{";
const char MSG_SJOIN[] = "SJOIN";
const char TOK_SJOIN[] = "~";
const char MSG_SETHOST[] = "SETHOST";
const char TOK_SETHOST[] = "AA";
const char MSG_NACHAT[] = "NACHAT";
const char TOK_NACHAT[] = "AC";
const char MSG_SETIDENT[] = "SETIDENT";
const char TOK_SETIDENT[] = "AD";
const char MSG_SETNAME[] = "SETNAME";
const char TOK_SETNAME[] = "AE";
const char MSG_LAG[] = "LAG";
const char TOK_LAG[] = "AF";
const char MSG_SDESC[] = "SDESC";
const char TOK_SDESC[] = "AG";
const char MSG_STATSERV[] = "STATSERV";
const char TOK_STATSERV[] = "AH";
const char MSG_KNOCK[] = "KNOCK";
const char TOK_KNOCK[] = "AI";
const char MSG_CREDITS[] = "CREDITS";
const char TOK_CREDITS[] = "AJ";
const char MSG_LICENSE[] = "LICENSE";
const char TOK_LICENSE[] = "AK";
const char MSG_CHGHOST[] = "CHGHOST";
const char TOK_CHGHOST[] = "AL";
const char MSG_RPING[] = "RPING";
const char TOK_RPING[] = "AM";
const char MSG_RPONG[] = "RPONG";
const char TOK_RPONG[] = "AN";
const char MSG_NETINFO[] = "NETINFO";
const char TOK_NETINFO[] = "AO";
const char MSG_SENDUMODE[] = "SENDUMODE";
const char TOK_SENDUMODE[] = "AP";
const char MSG_ADDMOTD[] = "ADDMOTD";
const char TOK_ADDMOTD[] = "AQ";
const char MSG_ADDOMOTD[] = "ADDOMOTD";
const char TOK_ADDOMOTD[] = "AR";
const char MSG_SVSMOTD[] = "SVSMOTD";
const char TOK_SVSMOTD[] = "AS";
const char MSG_SMO[] = "SMO";
const char TOK_SMO[] = "AU";
const char MSG_OPERMOTD[] = "OPERMOTD";
const char TOK_OPERMOTD[] = "AV";
const char MSG_TSCTL[] = "TSCTL";
const char TOK_TSCTL[] = "AW";
const char MSG_SVSJOIN[] = "SVSJOIN";
const char TOK_SVSJOIN[] = "AX";
const char MSG_SAJOIN[] = "SAJOIN";
const char TOK_SAJOIN[] = "AY";
const char MSG_SVSPART[] = "SVSPART";
const char TOK_SVSPART[] = "AX";
const char MSG_SAPART[] = "SAPART";
const char TOK_SAPART[] = "AY";
const char MSG_CHGIDENT[] = "CHGIDENT";
const char TOK_CHGIDENT[] = "AZ";
const char MSG_SWHOIS[] = "SWHOIS";
const char TOK_SWHOIS[] = "BA";
const char MSG_SVSO[] = "SVSO";
const char TOK_SVSO[] = "BB";
const char MSG_SVSFLINE[] = "SVSFLINE";
const char TOK_SVSFLINE[] = "BC";
const char MSG_TKL[] = "TKL";
const char TOK_TKL[] = "BD";
const char MSG_VHOST[] = "VHOST";
const char TOK_VHOST[] = "BE";
const char MSG_BOTMOTD[] = "BOTMOTD";
const char TOK_BOTMOTD[] = "BF";
const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
const char TOK_REMGLINE[] = "BG";
const char MSG_REMGZLINE[] = "REMGZLINE";	/* remove global z-line */
const char TOK_REMGZLINE[] = "BP";
const char MSG_HTM[] = "HTM";
const char TOK_HTM[] = "BH";
const char MSG_UMODE2[] = "UMODE2";
const char TOK_UMODE2[] = "|";
const char MSG_DCCDENY[] = "DCCDENY";
const char TOK_DCCDENY[] = "BI";
const char MSG_UNDCCDENY[] = "UNDCCDENY";
const char TOK_UNDCCDENY[] = "BJ";
const char MSG_CHGNAME[] = "CHGNAME";
const char MSG_SVSNAME[] = "SVSNAME";
const char TOK_CHGNAME[] = "BK";
const char MSG_SHUN[] = "SHUN";
const char TOK_SHUN[] = "BL";
const char MSG_NEWJOIN[] = "NEWJOIN";	/* For CR Java Chat */
const char MSG_POST[] = "POST";
const char TOK_POST[] = "BN";
const char MSG_INFOSERV[] = "INFOSERV";
const char MSG_IS[] = "IS";
const char TOK_INFOSERV[] = "BO";
const char MSG_BOTSERV[] = "BOTSERV";
const char TOK_BOTSERV[] = "BS";

/* Umodes */
#define UMODE_FAILOP		0x00100000	/* Shows some global messages */
#define UMODE_SERVNOTICE	0x00200000	/* server notices such as kill */
#define UMODE_KILLS			0x00400000	/* Show server-kills... */
#define UMODE_FLOOD			0x00800000	/* Receive flood warnings */
#define UMODE_JUNK			0x01000000	/* can junk */
#define UMODE_EYES			0x02000000	/* Mode to see server stuff */
#define UMODE_WHOIS			0x04000000	/* gets notice on /whois */
#define UMODE_SECURE		0x08000000	/* User is a secure connect */
#define UMODE_VICTIM		0x10000000	/* Intentional Victim */
#define UMODE_HIDEOPER		0x20000000	/* Hide oper mode */
#define UMODE_SETHOST		0x40000000	/* used sethost */
#define UMODE_STRIPBADWORDS 0x80000000	/* */

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_umode2( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );
static void m_smo( char *origin, char **argv, int argc, int srv );
static void m_swhois( char *origin, char **argv, int argc, int srv );
static void m_tkl( char *origin, char **argv, int argc, int srv );

#define NICKV2	

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	30,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	32,
	/* Max topic length */
	307,
	/* Default operator modes for NeoStats service bots */
	"+oSq",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

irc_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_UMODE2, TOK_UMODE2, m_umode2, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_SVS2MODE, TOK_SVS2MODE, m_svsmode, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_SJOIN, TOK_SJOIN, m_sjoin, 0},
	{MSG_SWHOIS, TOK_SWHOIS, m_swhois, 0},
	{MSG_SMO, TOK_SMO, m_smo, 0},
	{MSG_TKL, TOK_TKL, m_tkl, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'c', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'A', CMODE_ADMONLY, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'Q', CMODE_NOKICKS, 0},
	{'S', CMODE_STRIP, 0},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'K', CMODE_NOKNOCK, 0},
	{'V', CMODE_NOINVITE, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'M', CMODE_MODREG, 0},
	{'G', CMODE_STRIPBADWORDS, 0},
	{'C', CMODE_NOCTCP, 0},
	{'u', CMODE_AUDITORIUM, 0},
	{'z', CMODE_ONLYSECURE, 0},
	{'N', CMODE_NONICKCHANGE, 0},
	{0, 0},
};

mode_init user_umodes[] = 
{
	{'S', UMODE_SERVICES},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'C', UMODE_COADMIN},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'q', UMODE_KIX},
	{'B', UMODE_BOT},
 	{'d', UMODE_DEAF},
	{'k', UMODE_KILLS},
	{'e', UMODE_EYES},
	{'F', UMODE_FCLIENT},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FLOOD},
	{'j', UMODE_JUNK},
	{'G', UMODE_STRIPBADWORDS},
	{'t', UMODE_SETHOST},
	{'x', UMODE_HIDE},
	/*{'b', UMODE_CHATOP},*/
	{'W', UMODE_WHOIS},
	{'z', UMODE_SECURE},
	{'v', UMODE_VICTIM},	
	{0, 0},
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 */
	send_cmd( "%s TOKEN NICKv2 VHP SJOIN SJOIN2 SJ3 UMODE2", MSGTOK( PROTOCTL ) );
	send_cmd( "%s %s", MSGTOK( PASS ), pass );
	send_cmd( "%s %s %d :%s", MSGTOK( SERVER ), name, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSGTOK( SJOIN ), ts, chan, target );
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 */
void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s 0 %s * :%s", MSGTOK( NICK ), nick, ts, ident, host, server, newmode, realname );
}

void send_smo( const char *source, const char *umodetarget, const char *msg )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SMO ), umodetarget, msg );
}

void send_swhois( const char *source, const char *target, const char *swhois )
{
	send_cmd( "%s %s :%s", MSGTOK( SWHOIS ), target, swhois );
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s + G %s %s %s %lu %lu :%s", source, MSGTOK( TKL ), ident, host, setby,( ts + length ), ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s - G %s %s %s", source, MSGTOK( TKL ), ident, host, source );
}

void send_svstime( const char *source, const unsigned long ts )
{
	send_cmd( ":%s %s SVSTIME %lu", source, MSGTOK( TSCTL ), ts );
}

/* m_server
 *	argv[0] = servername
 *  argv[1] = hopcount
 *  argv[2] = numeric
 *  argv[3] = serverinfo
 * on old protocols, serverinfo is argv[2], and numeric is left out
 */
/*SERVER servername hopcount :U<protocol>-flags-numeric serverdesc*/
static void m_server( char *origin, char **argv, int argc, int srv )
{
	char* s = argv[argc-1];
	if( *origin== 0 ) {
		/* server desc from uplink includes extra info so we need to 
		   strip protocol, flags and numeric. We can use the first
		   space to do this*/
		while( *s != ' ' )
			s++;
		/* Strip the now leading space */
		s++;
	}
	if( argc > 3 ) {
		do_server( argv[0], origin, argv[1], argv[2], s, srv );
	} else {
		do_server( argv[0], origin, argv[1], NULL, s, srv );
	}
	
}

/* m_svsmode
 *  argv[0] - username to change mode for
 *  argv[1] - modes to change
 *  argv[2] - Service Stamp( if mode == d )
 */
static void m_svsmode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' ) {
		do_svsmode_channel( origin, argv, argc );
	} else {
		do_svsmode_user( argv[0], argv[1], argv[2] );
	}
}

/* m_umode2
 * argv[0] - modes to change
 */
static void m_umode2( char *origin, char **argv, int argc, int srv )
{
	do_mode_user( origin, argv[0] );
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
#ifdef NICKV2	
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, argv[6], argv[7], argv[8], argv[9], NULL, NULL );
#else
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, argv[6], NULL, NULL, argv[9], NULL, NULL );
#endif
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

/* m_sjoin  
 *  argv[0] = channel timestamp
 *    char *argv[], pvar[MAXMODEPARAMS][MODEBUFLEN + 3];
 *  argv[1] = channel name
 *  "ts chname :"
 * if( argc == 3 ) 
 *  argv[2] = nick names + modes - all in one parameter
 *  "ts chname modebuf :"
 *  "ts chname :"@/"""name"	OPT_SJ3
 * if( argc == 4 )
 *  argv[2] = channel modes
 *  argv[3] = nick names + modes - all in one parameter
 *  "ts chname modebuf parabuf :"
 * if( argc > 4 )
 *  argv[2] = channel modes
 *  argv[3 to argc - 2] = mode parameters
 *  argv[argc - 1] = nick names + modes
 *  "ts parabuf :parv[parc - 1]"	OPT_SJOIN | OPT_SJ3 
 */
/*    MSG_SJOIN creationtime chname    modebuf parabuf :member list */
/* R: ~         1073861298   #services +       <none>  :Mark */
static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1],( ( argc >= 4 ) ? argv[2] : "" ), origin, argv, argc );
}

/* m_swhois
 *  argv[0] = nickname
 *  argv[1] = new swhois
 */
static void m_swhois( char *origin, char **argv, int argc, int srv )
{
	do_swhois( argv[0], argv[1] );
}

static void m_smo( char *origin, char **argv, int argc, int srv )
{
	/* TODO */
}

/*
 *  argv[0]  +|- 
 *  argv[1]  G   
 *  argv[2]  user 
 *  argv[3]  host 
 *  argv[4]  setby 
 *  argv[5]  expire_at 
 *  argv[6]  set_at 
 *  argv[7]  reason 

R: :server BD + G * mask setter 1074811259 1074206459 :reason
R: :server BD + Z * mask setter 0 1070062390 :reason
R: :server c dos_bot* :Reserved nickname: Dosbot
*/

static void m_tkl( char *origin, char **argv, int argc, int srv )
{
	do_tkl( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7] );
}
