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
#include "services.h"

/* Messages/Tokens */
const char MSG_EOB[] = "EOB";	/* end of burst */
const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
const char MSG_WHO[] = "WHO";	/* WHO  -> WHOC */
const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
const char MSG_USER[] = "USER";	/* USER */
const char MSG_NICK[] = "NICK";	/* NICK */
const char MSG_SERVER[] = "SERVER";	/* SERV */
const char MSG_LIST[] = "LIST";	/* LIST */
const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
const char MSG_INVITE[] = "INVITE";	/* INVI */
const char MSG_VERSION[] = "VERSION";	/* VERS */
const char MSG_QUIT[] = "QUIT";	/* QUIT */
const char MSG_SQUIT[] = "SQUIT";	/* SQUI */
const char MSG_KILL[] = "KILL";	/* KILL */
const char MSG_INFO[] = "INFO";	/* INFO */
const char MSG_LINKS[] = "LINKS";	/* LINK */
const char MSG_WATCH[] = "WATCH";	/* WATCH */
const char MSG_STATS[] = "STATS";	/* STAT */
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
const char MSG_SQUERY[] = "SQUERY";	/* SQUE */
const char MSG_SERVLIST[] = "SERVLIST";	/* SERV -> SLIS */
const char MSG_SERVSET[] = "SERVSET";	/* SERV -> SSET */
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
const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
const char MSG_TRACE[] = "TRACE";	/* TRAC */
const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
const char MSG_NS[] = "NS";
const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
const char MSG_CS[] = "CS";
const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
const char MSG_OS[] = "OS";
const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
const char MSG_MS[] = "MS";
const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
const char MSG_RULES[] = "RULES";	/* RULES */
const char MSG_MAP [] = "MAP";	/* MAP */
const char MSG_NETG[] = "NETG";	/* NETG */
const char MSG_ADCHAT[] = "ADCHAT";	/* Adchat */
const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
const char MSG_ADDHUB[] = "ADDHUB";	/* ADDHUB */
const char MSG_DELHUB[] = "DELHUB";	/* DELHUB */
const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
const char MSG_GSOP[] = "GSOP";	/* GSOP */
const char MSG_ISOPER[] = "ISOPER";	/* ISOPER */
const char MSG_ADG[] = "ADG";	/* ADG */
const char MSG_NMON[] = "NMON";	/* NMON */
const char MSG_DALINFO[] = "DALINFO";	/* DALnet Credits */
const char MSG_CREDITS[] = "CREDITS";	/* UltimateIRCd Credits and "Thanks To" */
const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
const char MSG_REMREHASH[] = "REMREHASH";	/* Remote Rehash */
const char MSG_MONITOR[] = "MONITOR";	/* MONITOR */
const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
const char MSG_STATSERV[] = "STATSERV";	/* StatServ */
const char MSG_RULESERV[] = "RULESERV";	/* RuleServ */
const char MSG_SNETINFO[] = "SNETINFO";	/* SNetInfo */
const char MSG_TSCTL[] = "TSCTL";	/* TSCTL */
const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
const char MSG_SAJOIN[] = "SAJOIN";	/* SAJOIN */
const char MSG_SDESC[] = "SDESC";	/* SDESC */
const char MSG_UNREALINFO[] = "UNREALINFO";	/* Unreal Info */
const char MSG_SETHOST[] = "SETHOST";	/* sethost */
const char MSG_SETIDENT[] = "SETIDENT";	/* set ident */
const char MSG_SETNAME[] = "SETNAME";	/* set Realname */
const char MSG_CHGHOST[] = "CHGHOST";	/* Changehost */
const char MSG_CHGIDENT[] = "CHGIDENT";	/* Change Ident */
const char MSG_RANDQUOTE[] = "RANDQUOTE";	/* Random Quote */
const char MSG_ADDQUOTE[] = "ADDQUOTE";	/* Add Quote */
const char MSG_ADDGQUOTE[] = "ADDGQUOTE";	/* Add Global Quote */
const char MSG_ADDULINE[] = "ADDULINE";	/* Adds an U Line to ircd.conf file */
const char MSG_DELULINE[] = "DELULINE";	/* Removes an U line from the ircd.conf */
const char MSG_KNOCK[] = "KNOCK";	/* Knock Knock - Who's there? */
const char MSG_SETTINGS[] = "SETTINGS";	/* Settings */
const char MSG_IRCOPS[] = "IRCOPS";	/* Shows Online IRCOps */
const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
const char MSG_SAPART[] = "SAPART";	/* SAPART */
/*#define MSG_VCTRL	"VCTRL"	*//* VCTRL */
const char MSG_GCLIENT[] = "GCLIENT";	/* GLIENT */
const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
const char MSG_UPTIME[] = "UPTIME";	/* UPTIME */
const char MSG_FAILOPS[] = "FAILOPS";	/* FAILOPS */
const char MSG_RPING[] = "RPING";	/* RPING */
const char MSG_RPONG[] = "RPONG";	/* RPONG */
const char MSG_UPING[] = "UPING";	/* UPING */
const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* Copyright */
const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
const char MSG_BS[] = "BS";
const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
const char MSG_SVINFO[] = "SVINFO";
const char MSG_CAPAB[] = "CAPAB";
const char MSG_BURST[] = "BURST";
const char MSG_SJOIN[] = "SJOIN";
const char MSG_TBURST[] = "TBURST";

/* Umodes */
#define UMODE_SERVNOTICE   0x00100000 /* server notices such as kill */
#define UMODE_REJ          0x00200000 /* Bot Rejections */
#define UMODE_SKILL        0x00400000 /* Server Killed */
#define UMODE_FULL         0x00800000 /* Full messages */
#define UMODE_SPY          0x01000000 /* see STATS / LINKS */
#define UMODE_DEBUG        0x02000000 /* 'debugging' info */
#define UMODE_NCHANGE      0x04000000 /* Nick change notice */
#define UMODE_OPERWALL     0x08000000 /* Operwalls */
#define UMODE_BOTS         0x10000000 /* shows bots */
#define UMODE_EXTERNAL     0x20000000 /* show servers introduced and splitting */
#define UMODE_CALLERID     0x40000000 /* block unless caller id's */
#define UMODE_UNAUTH       0x80000000 /* show unauth connects here */
 
/* Channel Visibility macros */
#define CMODE_INVEX		0x02000000
#define CMODE_HIDEOPS	0x04000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );

/* buffer sizes */
const int proto_maxhost		=( 128 + 1 );
const int proto_maxpass		=( 32 + 1 );
const int proto_maxnick		=( 32 + 1 );
const int proto_maxuser		=( 10 + 1 );
const int proto_maxrealname	=( 50 + 1 );
const int proto_chanlen		=( 200 + 1 );
const int proto_topiclen	=( 512 + 1 );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_UNKLN,
	/* Features supported by this IRCd */
	0,
	"+o",
	"+o",
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_SERVER, 0, m_server, 0},
	{MSG_AWAY, 0, _m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, m_topic, 0},
	{MSG_PASS, 0, _m_pass, 0},
	{MSG_EOB, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_CAPAB, 0, _m_capab, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{MSG_ERROR, 0, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'I', CMODE_INVEX, MODEPARAM},
	{'a', CMODE_HIDEOPS, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{'d', UMODE_DEBUG},
	{'a', UMODE_ADMIN},
	{'l', UMODE_LOCOP},
	{'b', UMODE_BOTS},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FULL},
	{'g', UMODE_CALLERID},
	{'k', UMODE_SKILL},
	{'n', UMODE_NCHANGE},
	{'r', UMODE_REJ},
	{'s', UMODE_SERVNOTICE},
	{'u', UMODE_UNAUTH},
	{'w', UMODE_WALLOP},
	{'x', UMODE_EXTERNAL},
	{'y', UMODE_SPY},
	{'z', UMODE_OPERWALL},
	{0, 0},
};

void send_eob( const char *server )
{
	send_cmd( ":%s %s", server, MSG_EOB );
}

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB :TS EX CHW IE EOB KLN GLN KNOCK HOPS HUB AOPS MX" );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
}

void send_burst( int b )
{
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSG_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname );
}

void send_snetinfo( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_SNETINFO, ts, prot, cloak, netname );
}

void send_netinfo( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_NETINFO, ts, prot, cloak, netname );
}

/* there isn't an akill on Hybrid, so we send a kline to all servers! */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s * %lu %s %s :%s", setby, MSG_KLINE, length, ident, host, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	if( ircd_srv.protocol&PROTOCOL_UNKLN ) {
		send_cmd( ":%s %s %s %s", source, MSG_UNKLINE, ident, host );
	} else {
		irc_chanalert( ns_botptr, "Please Manually remove KLINES using /unkline on each server" );
	}
}

/* source SJOIN unsigned long chan modes param:" */
static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1],( ( argc <= 2 ) ? argv[1] : argv[2] ), argv[4], argv, argc );
}

static void m_burst( char *origin, char **argv, int argc, int srv )
{
	send_eob( me.name );
	do_synch_neostats();
}

static void m_server( char *origin, char **argv, int argc, int srv )
{
	do_server( argv[0], origin, argv[1], NULL, argv[2], srv );
}

/*  m_nick
 *    argv[0] = nickname
 *    argv[1] = hop count
 *    argv[2] = TS
 *    argv[3] = umode
 *    argv[4] = username
 *    argv[5] = hostname
 *    argv[6] = server
 *    argv[7] = ircname
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			NULL, NULL, argv[3], NULL, argv[7], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

static void m_topic( char *origin, char **argv, int argc, int srv )
{
	/*
	** Hybrid uses two different formats for the topic change protocol... 
	** :user TOPIC channel :topic 
	** and 
	** :server TOPIC channel author topicts :topic 
	** Both forms must be accepted.
	** - Hwy
	*/	
	if( FindUser( origin ) ) {
		do_topic( argv[0], origin, NULL, argv[1] );
	} else if( FindServer( origin ) ) {
		do_topic( argv[0], argv[1], argv[2], argv[3] );
	} else {
		nlog( LOG_WARNING, "m_topic: can't find topic setter %s for topic %s", origin, argv[1] ); 
	}
}
