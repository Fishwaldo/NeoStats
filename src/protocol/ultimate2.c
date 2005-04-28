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
#include "services.h"

/* Messages/Tokens */
const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
const char TOK_PRIVATE[] = "!";	/* 33 */
const char MSG_WHO[] = "WHO";	/* WHO -> WHOC */
const char TOK_WHO[] = "\"";	/* 34 */
const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
const char TOK_WHOIS[] = "#";	/* 35 */
const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
const char TOK_WHOWAS[] = "$";	/* 36 */
const char MSG_USER[] = "USER";	/* USER */
const char TOK_USER[] = "%";	/* 37 */
const char MSG_NICK[] = "NICK";	/* NICK */
const char TOK_NICK[] = "&";	/* 38 */
const char MSG_SERVER[] = "SERVER";	/* SERV */
const char TOK_SERVER[] = "'";	/* 39 */
const char MSG_LIST[] = "LIST";	/* LIST */
const char TOK_LIST[] = "(";	/* 40 */
const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
const char TOK_TOPIC[] = ")";	/* 41 */
const char MSG_INVITE[] = "INVITE";	/* INVI */
const char TOK_INVITE[] = "*";	/* 42 */
const char MSG_VERSION[] = "VERSION";	/* VERS */
const char TOK_VERSION[] = "+";	/* 43 */
const char MSG_QUIT[] = "QUIT";	/* QUIT */
const char TOK_QUIT[] = ",";	/* 44 */
const char MSG_SQUIT[] = "SQUIT";	/* SQUI */
const char TOK_SQUIT[] = "-";	/* 45 */
const char MSG_KILL[] = "KILL";	/* KILL */
const char TOK_KILL[] = ".";	/* 46 */
const char MSG_INFO[] = "INFO";	/* INFO */
const char TOK_INFO[] = "/";	/* 47 */
const char MSG_LINKS[] = "LINKS";	/* LINK */
const char TOK_LINKS[] = "0";	/* 48 */
const char MSG_WATCH[] = "WATCH";	/* WATCH */
const char TOK_WATCH[] = "1";	/* 49 */
const char MSG_STATS[] = "STATS";	/* STAT */
const char TOK_STATS[] = "2";	/* 50 */
const char MSG_HELP[] = "HELP";	/* HELP */
const char MSG_HELPOP[] = "HELPOP";	/* HELP */
const char TOK_HELP[] = "4";	/* 52 */
const char MSG_ERROR[] = "ERROR";	/* ERRO */
const char TOK_ERROR[] = "5";	/* 53 */
const char MSG_AWAY[] = "AWAY";	/* AWAY */
const char TOK_AWAY[] = "6";	/* 54 */
const char MSG_CONNECT[] = "CONNECT";	/* CONN */
const char TOK_CONNECT[] = "7";	/* 55 */
const char MSG_PING[] = "PING";	/* PING */
const char TOK_PING[] = "8";	/* 56 */
const char MSG_PONG[] = "PONG";	/* PONG */
const char TOK_PONG[] = "9";	/* 57 */
const char MSG_OPER[] = "OPER";	/* OPER */
const char TOK_OPER[] = ";";	/* 59 */
const char MSG_PASS[] = "PASS";	/* PASS */
const char TOK_PASS[] = "<";	/* 60 */
const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
const char TOK_WALLOPS[] = "=";	/* 61 */
const char MSG_TIME[] = "TIME";	/* TIME */
const char TOK_TIME[] = ">";	/* 62 */
const char MSG_NAMES[] = "NAMES";	/* NAME */
const char TOK_NAMES[] = "?";	/* 63 */
const char MSG_ADMIN[] = "ADMIN";	/* ADMI */
const char TOK_ADMIN[] = "@";	/* 64 */
const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
const char TOK_NOTICE[] = "B";	/* 66 */
const char MSG_JOIN[] = "JOIN";	/* JOIN */
const char TOK_JOIN[] = "C";	/* 67 */
const char MSG_PART[] = "PART";	/* PART */
const char TOK_PART[] = "D";	/* 68 */
const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
const char TOK_LUSERS[] = "E";	/* 69 */
const char MSG_MOTD[] = "MOTD";	/* MOTD */
const char TOK_MOTD[] = "F";	/* 70 */
const char MSG_MODE[] = "MODE";	/* MODE */
const char TOK_MODE[] = "G";	/* 71 */
const char MSG_KICK[] = "KICK";	/* KICK */
const char TOK_KICK[] = "H";	/* 72 */
const char MSG_SERVICE[] = "SERVICE";	/* SERV -> SRVI */
const char TOK_SERVICE[] = "I";	/* 73 */
const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
const char TOK_USERHOST[] = "J";	/* 74 */
const char MSG_ISON[] = "ISON";	/* ISON */
const char TOK_ISON[] = "K";	/* 75 */
const char MSG_SQUERY[] = "SQUERY";	/* SQUE */
const char TOK_SQUERY[] = "L";	/* 76 */
const char MSG_SERVLIST[] = "SERVLIST";	/* SERV -> SLIS */
const char TOK_SERVLIST[] = "M";	/* 77 */
const char MSG_SERVSET[] = "SERVSET";	/* SERV -> SSET */
const char TOK_SERVSET[] = "N";	/* 78 */
const char MSG_REHASH[] = "REHASH";	/* REHA */
const char TOK_REHASH[] = "O";	/* 79 */
const char MSG_RESTART[] = "RESTART";	/* REST */
const char TOK_RESTART[] = "P";	/* 80 */
const char MSG_CLOSE[] = "CLOSE";	/* CLOS */
const char TOK_CLOSE[] = "Q";	/* 81 */
const char MSG_DIE[] = "DIE";	/* DIE */
const char TOK_DIE[] = "R";	/* 82 */
const char MSG_HASH[] = "HASH";	/* HASH */
const char TOK_HASH[] = "S";	/* 83 */
const char MSG_DNS[] = "DNS";	/* DNS -> DNSS */
const char TOK_DNS[] = "T";	/* 84 */
const char MSG_SILENCE[] = "SILENCE";	/* SILE */
const char TOK_SILENCE[] = "U";	/* 85 */
const char MSG_AKILL[] = "AKILL";	/* AKILL */
const char TOK_AKILL[] = "V";	/* 86 */
const char MSG_KLINE[] = "KLINE";	/* KLINE */
const char TOK_KLINE[] = "W";	/* 87 */
const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
const char TOK_UNKLINE[] = "X";	/* 88 */
const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
const char TOK_RAKILL[] = "Y";	/* 89 */
const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
const char TOK_GNOTICE[] = "Z";	/* 90 */
const char MSG_GOPER[] = "GOPER";	/* GOPER */
const char TOK_GOPER[] = "[";	/* 91 */
const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
const char TOK_GLOBOPS[] = "]";	/* 93 */
const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
const char TOK_LOCOPS[] = "^";	/* 94 */
const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
const char TOK_PROTOCTL[] = "_";	/* 95 */
const char MSG_TRACE[] = "TRACE";	/* TRAC */
const char TOK_TRACE[] = "b";	/* 98 */
const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
const char TOK_SQLINE[] = "c";	/* 99 */
const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
const char TOK_UNSQLINE[] = "d";	/* 100 */
const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
const char TOK_SVSNICK[] = "e";	/* 101 */
const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
const char TOK_SVSNOOP[] = "f";	/* 101 */
const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
const char TOK_IDENTIFY[] = "g";	/* 103 */
const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
const char TOK_SVSKILL[] = "h";	/* 104 */
const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
const char MSG_NS[] = "NS";
const char TOK_NICKSERV[] = "i";	/* 105 */
const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
const char MSG_CS[] = "CS";
const char TOK_CHANSERV[] = "j";	/* 106 */
const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
const char MSG_OS[] = "OS";
const char TOK_OPERSERV[] = "k";	/* 107 */
const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
const char MSG_MS[] = "MS";
const char TOK_MEMOSERV[] = "l";	/* 108 */
const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
const char TOK_SERVICES[] = "m";	/* 109 */
const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
const char TOK_SVSMODE[] = "n";	/* 110 */
const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
const char TOK_SAMODE[] = "o";	/* 111 */
const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
const char TOK_CHATOPS[] = "p";	/* 112 */
const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
const char TOK_HELPSERV[] = "r";	/* 114 */
const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
const char TOK_ZLINE[] = "s";	/* 115 */
const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
const char TOK_UNZLINE[] = "t";	/* 116 */
const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
const char TOK_NETINFO[] = "u";	/* 117 */
const char MSG_RULES[] = "RULES";	/* RULES */
const char TOK_RULES[] = "v";	/* 118 */
const char MSG_MAP[] = "MAP";	/* MAP */
const char TOK_MAP[] = "w";	/* 119 */
const char MSG_NETG[] = "NETG";	/* NETG */
const char TOK_NETG[] = "x";	/* 120 */
const char MSG_ADCHAT[] = "ADCHAT";	/* Adchat */
const char TOK_ADCHAT[] = "y";	/* 121 */
const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
const char TOK_MAKEPASS[] = "z";	/* 122 */
const char MSG_ADDHUB[] = "ADDHUB";	/* ADDHUB */
const char TOK_ADDHUB[] = "{";	/* 123 */
const char MSG_DELHUB[] = "DELHUB";	/* DELHUB */
const char TOK_DELHUB[] = "|";	/* 124 */
const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
const char TOK_ADDCNLINE[] = "}";	/* 125 */
const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
const char TOK_DELCNLINE[] = "~";	/* 126 */
const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
const char TOK_ADDOPER[] = "";	/* 127 */
const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
const char TOK_DELOPER[] = "!!";	/* 33 + 33 */
const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
const char TOK_ADDQLINE[] = "!\"";	/* 33 + 34 */
const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
const char TOK_DELQLINE[] = "!#";	/* 33 + 35 */
const char MSG_GSOP[] = "GSOP";	/* GSOP */
const char TOK_GSOP[] = "!$";	/* 33 + 36 */
const char MSG_ISOPER[] = "ISOPER";	/* ISOPER */
const char TOK_ISOPER[] = "!%";	/* 33 + 37 */
const char MSG_ADG[] = "ADG";	/* ADG */
const char TOK_ADG[] = "!&";	/* 33 + 38 */
const char MSG_NMON[] = "NMON";	/* NMON */
const char TOK_NMON[] = "!'";	/* 33 + 39 */
const char MSG_DALINFO[] = "DALINFO";	/* DALnet Credits */
const char TOK_DALINFO[] = "!(";	/* 33 + 40 */
const char MSG_CREDITS[] = "CREDITS";	/* UltimateIRCd Credits and "Thanks To" */
const char TOK_CREDITS[] = "!)";	/* 33 + 41 */
const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
const char TOK_OPERMOTD[] = "!*";	/* 33 + 42 */
const char MSG_REMREHASH[] = "REMREHASH";	/* Remote Rehash */
const char TOK_REMREHASH[] = "!+";	/* 33 + 43 */
const char MSG_MONITOR[] = "MONITOR";	/* MONITOR */
const char TOK_MONITOR[] = "!,";	/* 33 + 44 */
const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
const char TOK_GLINE[] = "!-";	/* 33 + 45 */
const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
const char TOK_REMGLINE[] = "!.";	/* 33 + 46 */
const char MSG_STATSERV[] = "STATSERV";	/* StatServ */
const char TOK_STATSERV[] = "!/";	/* 33 + 47 */
const char MSG_RULESERV[] = "RULESERV";	/* RuleServ */
const char TOK_RULESERV[] = "!0";	/* 33 + 48 */
const char MSG_SNETINFO[] = "SNETINFO";	/* SNetInfo */
const char TOK_SNETINFO[] = "!1";	/* 33 + 49 */
const char MSG_TSCTL[] = "TSCTL";	/* TSCTL */
const char TOK_TSCTL[] = "!3";	/* 33 + 51 */
const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
const char TOK_SVSJOIN[] = "!4";	/* 33 + 52 */
const char MSG_SAJOIN[] = "SAJOIN";	/* SAJOIN */
const char TOK_SAJOIN[] = "!5";	/* 33 + 53 */
const char MSG_SDESC[] = "SDESC";	/* SDESC */
const char TOK_SDESC[] = "!6";	/* 33 + 54 */
const char MSG_UNREALINFO[] = "UNREALINFO";	/* Unreal Info */
const char TOK_UNREALINFO[] = "!7";	/* 33 + 55 */
const char MSG_SETHOST[] = "SETHOST";	/* sethost */
const char TOK_SETHOST[] = "!8";	/* 33 + 56 */
const char MSG_SETIDENT[] = "SETIDENT";	/* set ident */
const char TOK_SETIDENT[] = "!9";	/* 33 + 57 */
const char MSG_SETNAME[] = "SETNAME";	/* set Realname */
const char TOK_SETNAME[] = "!;";	/* 33 + 59 */
const char MSG_CHGHOST[] = "CHGHOST";	/* Changehost */
const char TOK_CHGHOST[] = "!<";	/* 33 + 60 */
const char MSG_CHGIDENT[] = "CHGIDENT";	/* Change Ident */
const char TOK_CHGIDENT[] = "!=";	/* 33 + 61 */
const char MSG_RANDQUOTE[] = "RANDQUOTE";	/* Random Quote */
const char TOK_RANDQUOTE[] = "!>";	/* 33 + 62 */
const char MSG_ADDQUOTE[] = "ADDQUOTE";	/* Add Quote */
const char TOK_ADDQUOTE[] = "!?";	/* 33 + 63 */
const char MSG_ADDGQUOTE[] = "ADDGQUOTE";	/* Add Global Quote */
const char TOK_ADDGQUOTE[] = "!@";	/* 33 + 64 */
const char MSG_ADDULINE[] = "ADDULINE";	/* Adds an U Line to ircd.conf file */
const char TOK_ADDULINE[] = "!B";	/* 33 + 66 */
const char MSG_DELULINE[] = "DELULINE";	/* Removes an U line from the ircd.conf */
const char TOK_DELULINE[] = "!C";	/* 33 + 67 */
const char MSG_KNOCK[] = "KNOCK";	/* Knock Knock - Who's there? */
const char TOK_KNOCK[] = "!D";	/* 33 + 68 */
const char MSG_SETTINGS[] = "SETTINGS";	/* Settings */
const char TOK_SETTINGS[] = "!E";	/* 33 + 69 */
const char MSG_IRCOPS[] = "IRCOPS";	/* Shows Online IRCOps */
const char TOK_IRCOPS[] = "!F";	/* 33 + 70 */
const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
const char TOK_SVSPART[] = "!G";	/* 33 + 71 */
const char MSG_SAPART[] = "SAPART";	/* SAPART */
const char TOK_SAPART[] = "!H";	/* 33 + 72 */
const char MSG_VCTRL[] = "VCTRL";	/* VCTRL */
const char TOK_VCTRL[] = "!I";	/* 33 + 73 */
const char MSG_GCLIENT[] = "GCLIENT";	/* GLIENT */
const char TOK_GCLIENT[] = "!J";	/* 33 + 74 */
const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
const char TOK_CHANNEL[] = "!K";	/* 33 + 75 */
const char MSG_UPTIME[] = "UPTIME";	/* UPTIME */
const char TOK_UPTIME[] = "!L";	/* 33 + 76 */
const char MSG_FAILOPS[] = "FAILOPS";	/* FAILOPS */
const char TOK_FAILOPS[] = "!M";	/* 33 + 77 */

const char MSG_RPING[] = "RPING";	/* RPING */
const char TOK_RPING[] = "!P";	/* 33 + 80 */
const char MSG_RPONG[] = "RPONG";	/* RPONG */
const char TOK_RPONG[] = "!Q";	/* 33 + 81 */
const char MSG_UPING[] = "UPING";	/* UPING */
const char TOK_UPING[] = "!R";	/* 33 + 82 */
const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* Copyright */
const char TOK_COPYRIGHT[] = "!S";	/* 33 + 83 */
const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
const char MSG_BS[] = "BS";
const char TOK_BOTSERV[] = "!T";	/* 33 + 84 */
const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
const char MSG_RS[] = "RS";
const char TOK_ROOTSERV[] = "!U";	/* 33 + 85 */
const char MSG_SVINFO[] = "SVINFO";
const char MSG_CAPAB[] = "CAPAB";
const char MSG_BURST[] = "BURST";
const char MSG_SJOIN[] = "SJOIN";
const char MSG_CLIENT[] = "CLIENT";
const char MSG_SMODE[] = "SMODE";

/* Umodes */
#define UMODE_FAILOP	 	0x00020000	/* Shows some global messages */
#define UMODE_SERVICESOPER	0x00040000	/* Services Oper */
#define UMODE_ALTADMIN	 	0x00080000	/* Admin */
#define UMODE_SERVNOTICE 	0x00100000	/* server notices such as kill */
#define UMODE_KILLS	 		0x00200000	/* Show server-kills... */
#define UMODE_FLOOD	 		0x00400000	/* Receive flood warnings */
#define UMODE_CHATOP	 	0x00800000	/* can receive chatops */
#define UMODE_SUPER			0x01000000	/* Oper Is Protected from Kick's and Kill's */
#define UMODE_NGLOBAL 		0x02000000	/* See Network Globals */
#define UMODE_WHOIS 		0x04000000	/* Lets Opers see when people do a /WhoIs on them */
#define UMODE_NETINFO 		0x08000000	/* Server link, Delink Notces etc. */
#define UMODE_MAGICK 		0x10000000	/* Allows Opers To See +s and +p Channels */
#define UMODE_IRCADMIN 		0x20000000	/* Marks the client as an IRC Administrator */
#define UMODE_WATCHER		0x40000000	/* Recive Monitor Globals */
#define UMODE_NETMON		0x80000000	/* Marks the client as an Network Monitor */

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_vctrl( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	15,
	/* Max real name length */
	50,
	/* Max channel name length */
	50,
	/* Max topic length */
	512,
	/* Default operator modes for NeoStats service bots */
	"+oS",
	/* Default channel mode for NeoStats service bots */
	"+a",
};

irc_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_SERVER,    0,		m_server,	0},
	{MSG_SVSMODE,   0,   m_svsmode,   0},
	{MSG_NICK,      TOK_NICK,      m_nick,      0},
	{MSG_VCTRL,     0,     m_vctrl,     0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANADMIN, 0, '!'},
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'r', CMODE_RGSTR, 0},
	{'x', CMODE_NOCOLOR, 0},
	{'A', CMODE_ADMONLY, 0},
	{'I', CMODE_NOINVITE, 0},
	{'K', CMODE_NOKNOCK, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'O', CMODE_OPERONLY, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'S', CMODE_STRIP, 0},	
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{'S', UMODE_SERVICES},
	{'P', UMODE_SADMIN},
	{'T', UMODE_TECHADMIN},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SERVICESOPER},
	{'Z', UMODE_IRCADMIN},
	{'z', UMODE_ADMIN},
	{'i', UMODE_ALTADMIN},
	{'p', UMODE_SUPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'k', UMODE_KILLS},
	{'B', UMODE_RBOT},
	{'b', UMODE_SBOT},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FLOOD},
	{'x', UMODE_HIDE},
	{'W', UMODE_WATCHER},
	{0, 0},
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s", MSG_PASS, pass );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
	send_cmd( "%s TOKEN CLIENT", MSG_PROTOCTL );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSGTOK( MODE ), chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s 0 :%s", MSGTOK( NICK ), nick, ts, ident, host, server, realname );
	send_cmd( ":%s %s %s :%s", nick, MSGTOK( MODE ), nick, newmode );
}

void send_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char* netname )
{
	send_cmd( "%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname );
}

void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s %s@%s %lu %lu %s :%s", source, MSG_GLINE, ident, host,( ts + length ), ts, setby, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	/* ultimate2 needs an oper to remove */
	send_cmd( ":%s %s :%s@%s", ns_botptr->name, MSG_REMGLINE, host, ident );
}

void send_burst( int b )
{
	if( b == 0 ) {
		send_cmd( "BURST 0" );
	} else {
		send_cmd( "BURST" );
	}
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
		do_svsmode_user( argv[0], argv[1], argv[2] );
	}
}

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv )
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, NULL, NULL, NULL, argv[7], NULL, NULL );
	else
		do_nickchange( origin, argv[0], NULL );
}

/*
 *  argv[0] = ultimate protocol
 *  argv[1] = nickname length
 *  argv[2] = Global Connect Notices
 *  argv[3] = Reserved for future extentions
 *  argv[4] = Reserved for future extentions
 *  argv[5] = Reserved for future extentions
 *  argv[6] = Reserved for future extentions
 *  argv[7] = Reserved for future extentions
 *  argv[8] = Reserved for future extentions
 *  argv[9] = Reserved for future extentions
 *  argv[10] = Reserved for future extentions
 *  argv[11] = Reserved for future extentions
 *  argv[12] = Reserved for future extentions
 *  argv[13] = Reserved for future extentions
 *  argv[14] = ircnet
 */
static void m_vctrl( char *origin, char **argv, int argc, int srv )
{
	do_vctrl( argv[0], argv[1], argv[2], argv[3], argv[14] );
}
