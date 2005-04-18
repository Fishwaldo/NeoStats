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
#include "ircd.h"
#include "bots.h"
#include "users.h"

/* Messages/Tokens */
const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
const char MSG_USER[] = "USER";	/* USER */
const char MSG_NICK[] = "NICK";	/* NICK */
const char MSG_LIST[] = "LIST";	/* LIST */
const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
const char MSG_INVITE[] = "INVITE";	/* INVI */
const char MSG_VERSION[] = "VERSION";	/* VERS */
const char MSG_QUIT[] = "QUIT";	/* QUIT */
const char MSG_KILL[] = "KILL";	/* KILL */
const char MSG_INFO[] = "INFO";	/* INFO */
const char MSG_LINKS[] = "LINKS";	/* LINK */
const char MSG_SUMMON[] = "SUMMON";	/* SUMM */
const char MSG_USERS[] = "USERS";	/* USER -> USRS */
const char MSG_HELP[] = "HELP";	/* HELP */
const char MSG_HELPOP[] = "HELPOP";	/* HELP */
const char MSG_ERROR[] = "ERROR";	/* ERRO */
const char MSG_AWAY[] = "AWAY";	/* AWAY */
const char MSG_CONNECT[] = "CONNECT";	/* CONN */
const char MSG_PING[] = "PING";	/* PING */
const char MSG_PONG[] = "PONG";	/* PONG */
const char MSG_OPER[] = "OPER";	/* OPER */
const char MSG_PASS[] = "PASS";	/* PASS */
const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
const char MSG_TIME[] = "TIME";	/* TIME */
const char MSG_NAMES[] = "NAMES";	/* NAME */
const char MSG_ADMIN[] = "ADMIN";	/* ADMI */
const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
const char MSG_JOIN[] = "JOIN";	/* JOIN */
const char MSG_PART[] = "PART";	/* PART */
const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
const char MSG_MOTD[] = "MOTD";	/* MOTD */
const char MSG_MODE[] = "MODE";	/* MODE */
const char MSG_KICK[] = "KICK";	/* KICK */
const char MSG_SERVICE[] = "SERVICE";	/* SERV -> SRVI */
const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
const char MSG_ISON[] = "ISON";	/* ISON */
const char MSG_REHASH[] = "REHASH";	/* REHA */
const char MSG_RESTART[] = "RESTART";	/* REST */
const char MSG_CLOSE[] = "CLOSE";	/* CLOS */
const char MSG_DIE[] = "DIE";	/* DIE */
const char MSG_HASH[] = "HASH";	/* HASH */
const char MSG_DNS[] = "DNS";	/* DNS  -> DNSS */
const char MSG_SILENCE[] = "SILENCE";	/* SILE */
const char MSG_AKILL[] = "AKILL";	/* AKILL */
const char MSG_KLINE[] = "KLINE";	/* KLINE */
const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
const char MSG_GOPER[] = "GOPER";	/* GOPER */
const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
const char MSG_WATCH[] = "WATCH";	/* WATCH */
const char MSG_TRACE[] = "TRACE";	/* TRAC */
const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
const char MSG_NS[] = "NS";
const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
const char MSG_CS[] = "CS";
const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
const char MSG_OS[] = "OS";
const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
const char MSG_MS[] = "MS";
const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
const char MSG_HS[] = "HS";
const char MSG_RULES[] = "RULES";	/* RULES */
const char MSG_MAP[] = "MAP";	/* MAP */
const char MSG_DALINFO[] = "DALINFO";	/* dalinfo */
const char MSG_ADMINCHAT[] = "ADCHAT";	/* Admin chat */
const char MSG_MKPASSWD[] = "MKPASSWD";	/* MKPASSWD */
const char MSG_ADDLINE[] = "ADDLINE";	/* ADDLINE */
const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
const char MSG_SJOIN[] = "SJOIN";
const char MSG_SETHOST[] = "SETHOST";	/* sethost */
const char MSG_NACHAT[] = "NACHAT";	/* netadmin chat */
const char MSG_SETIDENT[] = "SETIDENT";
const char MSG_SETNAME[] = "SETNAME";	/* set GECOS */
const char MSG_LAG[] = "LAG";	/* Lag detect */
const char MSG_STATSERV[] = "STATSERV";	/* alias */
const char MSG_KNOCK[] = "KNOCK";
const char MSG_CREDITS[] = "CREDITS";
const char MSG_LICENSE[] = "LICENSE";
const char MSG_CHGHOST[] = "CHGHOST";
const char MSG_RPING[] = "RPING";
const char MSG_RPONG[] = "RPONG";
const char MSG_NETINFO[] = "NETINFO";
const char MSG_SENDUMODE[] = "SENDUMODE";
const char MSG_ADDMOTD[] = "ADDMOTD";
const char MSG_ADDOMOTD[] = "ADDOMOTD";
const char MSG_SMO[] = "SMO";
const char MSG_OPERMOTD[] = "OPERMOTD";
const char MSG_TSCTL[] = "TSCTL";
const char MSG_SAJOIN[] = "SAJOIN";
const char MSG_SAPART[] = "SAPART";
const char MSG_CHGIDENT[] = "CHGIDENT";
const char MSG_SWHOIS[] = "SWHOIS";
const char MSG_SVSO[] = "SVSO";
const char MSG_TKL[] = "TKL";
const char MSG_VHOST[] = "VHOST";
const char MSG_BOTMOTD[] = "BOTMOTD";
const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
const char MSG_UMODE2[] = "UMODE2";
const char MSG_DCCDENY[] = "DCCDENY";
const char MSG_UNDCCDENY[] = "UNDCCDENY";
const char MSG_CHGNAME[] = "CHGNAME";
const char MSG_SHUN[] = "SHUN";
const char MSG_NEWJOIN[] = "NEWJOIN";	/* For CR Java Chat */
const char MSG_POST[] = "POST";
const char MSG_INFOSERV[] = "INFOSERV";
const char MSG_IS[] = "IS";
const char MSG_BOTSERV[] = "BOTSERV";
const char MSG_CYCLE[] = "CYCLE";
const char MSG_MODULE[] = "MODULE";
const char MSG_SENDSNO[] = "SENDSNO";

/* Umodes */
#define UMODE_FAILOP		0x00200000
#define UMODE_SERVNOTICE	0x00400000
#define UMODE_NOCTCP		0x00800000
#define UMODE_WEBTV			0x01000000
#define UMODE_WHOIS			0x02000000
#define UMODE_SECURE		0x04000000
#define UMODE_VICTIM		0x08000000
#define UMODE_HIDEOPER		0x10000000
#define UMODE_SETHOST		0x20000000
#define UMODE_STRIPBADWORDS	0x40000000
#define UMODE_HIDEWHOIS		0x80000000

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

static void m_private( char* origin, char **av, int ac, int cmdptr );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_kick( char *origin, char **argv, int argc, int srv );
static void m_join( char *origin, char **argv, int argc, int srv );
static void m_part( char *origin, char **argv, int argc, int srv );
static void m_emotd( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_CLIENTMODE,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
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

ircd_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{MSG_PRIVATE, 0, m_private, 0},
	{MSG_NOTICE, 0, _m_notice, 0},
	{"376", 0, m_emotd, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, m_topic, 0},
	{MSG_KICK, 0, m_kick, 0},
	{MSG_JOIN, 0, m_join, 0},
	{MSG_PART, 0, m_part, 0},
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
	{'R', UMODE_RGSTRONLY},
 	{'T', UMODE_NOCTCP},
	{'V', UMODE_WEBTV},
	{'p', UMODE_HIDEWHOIS},
	{'H', UMODE_HIDEOPER},
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
	send_cmd( "%s %s", MSG_PASS, pass );
	send_cmd( "%s %s", MSG_NICK, "NeoStats" );
	send_cmd( "%s %s %d %d :%s", MSG_USER, "user",( int ) me.now,( int ) me.now, "real name" );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSG_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s", MSG_NICK, nick );
}

void send_swhois( const char *source, const char *target, const char *swhois )
{
	send_cmd( "%s %s :%s", MSG_SWHOIS, target, swhois );
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s + G %s %s %s %lu %lu :%s", source, MSG_TKL, ident, host, setby,( ts + length ), ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s - G %s %s %s", source, MSG_TKL, ident, host, source );
}

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	do_nickchange( origin, argv[0], NULL );
}

/* m_topic
 *  argv[0] = channel name
 *  argv[1] = topic text
 */
/* TOPIC #channel :topic */
static void m_topic( char *origin, char **argv, int argc, int srv )
{
	do_topic( argv[0], NULL, NULL, argv[1] );
}

/* m_kick
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 */
static void m_kick( char *origin, char **argv, int argc, int srv )
{
	do_kick( origin, argv[0], argv[1], argv[2] );
}

/* m_join
 *	argv[0] = channel
 *	argv[1] = channel password( key )
 */
static void m_join( char *origin, char **argv, int argc, int srv )
{
	do_join( origin, argv[0], argv[1] );
}

/* m_part
 *	argv[0] = channel
 *	argv[1] = comment
 */
static void m_part( char *origin, char **argv, int argc, int srv )
{
	do_part( origin, argv[0], argv[1] );
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */

static void m_private( char* origin, char **av, int ac, int cmdptr )
{
	char *p;
	char nick[MAXNICK];
	
	strlcpy( nick, origin, MAXNICK );
	p = strchr( nick, '!' );
	*p = 0;
	AddFakeUser( origin );
	_m_private( nick, av, ac, cmdptr );
	DelFakeUser( origin );
}

static void m_emotd( char *origin, char **argv, int argc, int srv )
{
	send_cmd( "%s mark mark", MSG_OPER );
	do_synch_neostats(  );
}
