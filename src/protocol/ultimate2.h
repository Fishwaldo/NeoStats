/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
** Portions Copyright (c) 2000-2001 ^Enigma^
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
** USA
**
** NeoStats CVS Identification
** $Id$
*/
#ifndef ULTIMATE_H
#define ULTIMATE_H

/* Messages/Tokens */
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
MODULEVAR const char TOK_PRIVATE[] = "!";	/* 33 */
MODULEVAR const char MSG_WHO[] = "WHO";	/* WHO -> WHOC */
MODULEVAR const char TOK_WHO[] = "\"";	/* 34 */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
MODULEVAR const char TOK_WHOIS[] = "#";	/* 35 */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
MODULEVAR const char TOK_WHOWAS[] = "$";	/* 36 */
MODULEVAR const char MSG_USER[] = "USER";	/* USER */
MODULEVAR const char TOK_USER[] = "%";	/* 37 */
MODULEVAR const char MSG_NICK[] = "NICK";	/* NICK */
MODULEVAR const char TOK_NICK[] = "&";	/* 38 */
MODULEVAR const char MSG_SERVER[] = "SERVER";	/* SERV */
MODULEVAR const char TOK_SERVER[] = "'";	/* 39 */
MODULEVAR const char MSG_LIST[] = "LIST";	/* LIST */
MODULEVAR const char TOK_LIST[] = "(";	/* 40 */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
MODULEVAR const char TOK_TOPIC[] = ")";	/* 41 */
MODULEVAR const char MSG_INVITE[] = "INVITE";	/* INVI */
MODULEVAR const char TOK_INVITE[] = "*";	/* 42 */
MODULEVAR const char MSG_VERSION[] = "VERSION";	/* VERS */
MODULEVAR const char TOK_VERSION[] = "+";	/* 43 */
MODULEVAR const char MSG_QUIT[] = "QUIT";	/* QUIT */
MODULEVAR const char TOK_QUIT[] = ",";	/* 44 */
MODULEVAR const char MSG_SQUIT[] = "SQUIT";	/* SQUI */
MODULEVAR const char TOK_SQUIT[] = "-";	/* 45 */
MODULEVAR const char MSG_KILL[] = "KILL";	/* KILL */
MODULEVAR const char TOK_KILL[] = ".";	/* 46 */
MODULEVAR const char MSG_INFO[] = "INFO";	/* INFO */
MODULEVAR const char TOK_INFO[] = "/";	/* 47 */
MODULEVAR const char MSG_LINKS[] = "LINKS";	/* LINK */
MODULEVAR const char TOK_LINKS[] = "0";	/* 48 */
MODULEVAR const char MSG_WATCH[] = "WATCH";	/* WATCH */
MODULEVAR const char TOK_WATCH[] = "1";	/* 49 */
MODULEVAR const char MSG_STATS[] = "STATS";	/* STAT */
MODULEVAR const char TOK_STATS[] = "2";	/* 50 */
MODULEVAR const char MSG_HELP[] = "HELP";	/* HELP */
MODULEVAR const char MSG_HELPOP[] = "HELPOP";	/* HELP */
MODULEVAR const char TOK_HELP[] = "4";	/* 52 */
MODULEVAR const char MSG_ERROR[] = "ERROR";	/* ERRO */
MODULEVAR const char TOK_ERROR[] = "5";	/* 53 */
MODULEVAR const char MSG_AWAY[] = "AWAY";	/* AWAY */
MODULEVAR const char TOK_AWAY[] = "6";	/* 54 */
MODULEVAR const char MSG_CONNECT[] = "CONNECT";	/* CONN */
MODULEVAR const char TOK_CONNECT[] = "7";	/* 55 */
MODULEVAR const char MSG_PING[] = "PING";	/* PING */
MODULEVAR const char TOK_PING[] = "8";	/* 56 */
MODULEVAR const char MSG_PONG[] = "PONG";	/* PONG */
MODULEVAR const char TOK_PONG[] = "9";	/* 57 */
MODULEVAR const char MSG_OPER[] = "OPER";	/* OPER */
MODULEVAR const char TOK_OPER[] = ";";	/* 59 */
MODULEVAR const char MSG_PASS[] = "PASS";	/* PASS */
MODULEVAR const char TOK_PASS[] = "<";	/* 60 */
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
MODULEVAR const char TOK_WALLOPS[] = "=";	/* 61 */
MODULEVAR const char MSG_TIME[] = "TIME";	/* TIME */
MODULEVAR const char TOK_TIME[] = ">";	/* 62 */
MODULEVAR const char MSG_NAMES[] = "NAMES";	/* NAME */
MODULEVAR const char TOK_NAMES[] = "?";	/* 63 */
MODULEVAR const char MSG_ADMIN[] = "ADMIN";	/* ADMI */
MODULEVAR const char TOK_ADMIN[] = "@";	/* 64 */
MODULEVAR const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
MODULEVAR const char TOK_NOTICE[] = "B";	/* 66 */
MODULEVAR const char MSG_JOIN[] = "JOIN";	/* JOIN */
MODULEVAR const char TOK_JOIN[] = "C";	/* 67 */
MODULEVAR const char MSG_PART[] = "PART";	/* PART */
MODULEVAR const char TOK_PART[] = "D";	/* 68 */
MODULEVAR const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
MODULEVAR const char TOK_LUSERS[] = "E";	/* 69 */
MODULEVAR const char MSG_MOTD[] = "MOTD";	/* MOTD */
MODULEVAR const char TOK_MOTD[] = "F";	/* 70 */
MODULEVAR const char MSG_MODE[] = "MODE";	/* MODE */
MODULEVAR const char TOK_MODE[] = "G";	/* 71 */
MODULEVAR const char MSG_KICK[] = "KICK";	/* KICK */
MODULEVAR const char TOK_KICK[] = "H";	/* 72 */
MODULEVAR const char MSG_SERVICE[] = "SERVICE";	/* SERV -> SRVI */
MODULEVAR const char TOK_SERVICE[] = "I";	/* 73 */
MODULEVAR const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
MODULEVAR const char TOK_USERHOST[] = "J";	/* 74 */
MODULEVAR const char MSG_ISON[] = "ISON";	/* ISON */
MODULEVAR const char TOK_ISON[] = "K";	/* 75 */
MODULEVAR const char MSG_SQUERY[] = "SQUERY";	/* SQUE */
MODULEVAR const char TOK_SQUERY[] = "L";	/* 76 */
MODULEVAR const char MSG_SERVLIST[] = "SERVLIST";	/* SERV -> SLIS */
MODULEVAR const char TOK_SERVLIST[] = "M";	/* 77 */
MODULEVAR const char MSG_SERVSET[] = "SERVSET";	/* SERV -> SSET */
MODULEVAR const char TOK_SERVSET[] = "N";	/* 78 */
MODULEVAR const char MSG_REHASH[] = "REHASH";	/* REHA */
MODULEVAR const char TOK_REHASH[] = "O";	/* 79 */
MODULEVAR const char MSG_RESTART[] = "RESTART";	/* REST */
MODULEVAR const char TOK_RESTART[] = "P";	/* 80 */
MODULEVAR const char MSG_CLOSE[] = "CLOSE";	/* CLOS */
MODULEVAR const char TOK_CLOSE[] = "Q";	/* 81 */
MODULEVAR const char MSG_DIE[] = "DIE";	/* DIE */
MODULEVAR const char TOK_DIE[] = "R";	/* 82 */
MODULEVAR const char MSG_HASH[] = "HASH";	/* HASH */
MODULEVAR const char TOK_HASH[] = "S";	/* 83 */
MODULEVAR const char MSG_DNS[] = "DNS";	/* DNS -> DNSS */
MODULEVAR const char TOK_DNS[] = "T";	/* 84 */
MODULEVAR const char MSG_SILENCE[] = "SILENCE";	/* SILE */
MODULEVAR const char TOK_SILENCE[] = "U";	/* 85 */
MODULEVAR const char MSG_AKILL[] = "AKILL";	/* AKILL */
MODULEVAR const char TOK_AKILL[] = "V";	/* 86 */
MODULEVAR const char MSG_KLINE[] = "KLINE";	/* KLINE */
MODULEVAR const char TOK_KLINE[] = "W";	/* 87 */
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
MODULEVAR const char TOK_UNKLINE[] = "X";	/* 88 */
MODULEVAR const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
MODULEVAR const char TOK_RAKILL[] = "Y";	/* 89 */
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
MODULEVAR const char TOK_GNOTICE[] = "Z";	/* 90 */
MODULEVAR const char MSG_GOPER[] = "GOPER";	/* GOPER */
MODULEVAR const char TOK_GOPER[] = "[";	/* 91 */
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
MODULEVAR const char TOK_GLOBOPS[] = "]";	/* 93 */
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
MODULEVAR const char TOK_LOCOPS[] = "^";	/* 94 */
MODULEVAR const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
MODULEVAR const char TOK_PROTOCTL[] = "_";	/* 95 */
MODULEVAR const char MSG_TRACE[] = "TRACE";	/* TRAC */
MODULEVAR const char TOK_TRACE[] = "b";	/* 98 */
MODULEVAR const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
MODULEVAR const char TOK_SQLINE[] = "c";	/* 99 */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
MODULEVAR const char TOK_UNSQLINE[] = "d";	/* 100 */
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
MODULEVAR const char TOK_SVSNICK[] = "e";	/* 101 */
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
MODULEVAR const char TOK_SVSNOOP[] = "f";	/* 101 */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
MODULEVAR const char TOK_IDENTIFY[] = "g";	/* 103 */
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
MODULEVAR const char TOK_SVSKILL[] = "h";	/* 104 */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
MODULEVAR const char MSG_NS[] = "NS";
MODULEVAR const char TOK_NICKSERV[] = "i";	/* 105 */
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
MODULEVAR const char MSG_CS[] = "CS";
MODULEVAR const char TOK_CHANSERV[] = "j";	/* 106 */
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
MODULEVAR const char MSG_OS[] = "OS";
MODULEVAR const char TOK_OPERSERV[] = "k";	/* 107 */
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
MODULEVAR const char MSG_MS[] = "MS";
MODULEVAR const char TOK_MEMOSERV[] = "l";	/* 108 */
MODULEVAR const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
MODULEVAR const char TOK_SERVICES[] = "m";	/* 109 */
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
MODULEVAR const char TOK_SVSMODE[] = "n";	/* 110 */
MODULEVAR const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
MODULEVAR const char TOK_SAMODE[] = "o";	/* 111 */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
MODULEVAR const char TOK_CHATOPS[] = "p";	/* 112 */
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
MODULEVAR const char TOK_HELPSERV[] = "r";	/* 114 */
MODULEVAR const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
MODULEVAR const char TOK_ZLINE[] = "s";	/* 115 */
MODULEVAR const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
MODULEVAR const char TOK_UNZLINE[] = "t";	/* 116 */
MODULEVAR const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
MODULEVAR const char TOK_NETINFO[] = "u";	/* 117 */
MODULEVAR const char MSG_RULES[] = "RULES";	/* RULES */
MODULEVAR const char TOK_RULES[] = "v";	/* 118 */
MODULEVAR const char MSG_MAP[] = "MAP";	/* MAP */
MODULEVAR const char TOK_MAP[] = "w";	/* 119 */
MODULEVAR const char MSG_NETG[] = "NETG";	/* NETG */
MODULEVAR const char TOK_NETG[] = "x";	/* 120 */
MODULEVAR const char MSG_ADCHAT[] = "ADCHAT";	/* Adchat */
MODULEVAR const char TOK_ADCHAT[] = "y";	/* 121 */
MODULEVAR const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
MODULEVAR const char TOK_MAKEPASS[] = "z";	/* 122 */
MODULEVAR const char MSG_ADDHUB[] = "ADDHUB";	/* ADDHUB */
MODULEVAR const char TOK_ADDHUB[] = "{";	/* 123 */
MODULEVAR const char MSG_DELHUB[] = "DELHUB";	/* DELHUB */
MODULEVAR const char TOK_DELHUB[] = "|";	/* 124 */
MODULEVAR const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
MODULEVAR const char TOK_ADDCNLINE[] = "}";	/* 125 */
MODULEVAR const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
MODULEVAR const char TOK_DELCNLINE[] = "~";	/* 126 */
MODULEVAR const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
MODULEVAR const char TOK_ADDOPER[] = "";	/* 127 */
MODULEVAR const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
MODULEVAR const char TOK_DELOPER[] = "!!";	/* 33 + 33 */
MODULEVAR const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
MODULEVAR const char TOK_ADDQLINE[] = "!\"";	/* 33 + 34 */
MODULEVAR const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
MODULEVAR const char TOK_DELQLINE[] = "!#";	/* 33 + 35 */
MODULEVAR const char MSG_GSOP[] = "GSOP";	/* GSOP */
MODULEVAR const char TOK_GSOP[] = "!$";	/* 33 + 36 */
MODULEVAR const char MSG_ISOPER[] = "ISOPER";	/* ISOPER */
MODULEVAR const char TOK_ISOPER[] = "!%";	/* 33 + 37 */
MODULEVAR const char MSG_ADG[] = "ADG";	/* ADG */
MODULEVAR const char TOK_ADG[] = "!&";	/* 33 + 38 */
MODULEVAR const char MSG_NMON[] = "NMON";	/* NMON */
MODULEVAR const char TOK_NMON[] = "!'";	/* 33 + 39 */
MODULEVAR const char MSG_DALINFO[] = "DALINFO";	/* DALnet Credits */
MODULEVAR const char TOK_DALINFO[] = "!(";	/* 33 + 40 */
MODULEVAR const char MSG_CREDITS[] = "CREDITS";	/* UltimateIRCd Credits and "Thanks To" */
MODULEVAR const char TOK_CREDITS[] = "!)";	/* 33 + 41 */
MODULEVAR const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
MODULEVAR const char TOK_OPERMOTD[] = "!*";	/* 33 + 42 */
MODULEVAR const char MSG_REMREHASH[] = "REMREHASH";	/* Remote Rehash */
MODULEVAR const char TOK_REMREHASH[] = "!+";	/* 33 + 43 */
MODULEVAR const char MSG_MONITOR[] = "MONITOR";	/* MONITOR */
MODULEVAR const char TOK_MONITOR[] = "!,";	/* 33 + 44 */
MODULEVAR const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
MODULEVAR const char TOK_GLINE[] = "!-";	/* 33 + 45 */
MODULEVAR const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
MODULEVAR const char TOK_REMGLINE[] = "!.";	/* 33 + 46 */
MODULEVAR const char MSG_STATSERV[] = "STATSERV";	/* StatServ */
MODULEVAR const char TOK_STATSERV[] = "!/";	/* 33 + 47 */
MODULEVAR const char MSG_RULESERV[] = "RULESERV";	/* RuleServ */
MODULEVAR const char TOK_RULESERV[] = "!0";	/* 33 + 48 */
MODULEVAR const char MSG_SNETINFO[] = "SNETINFO";	/* SNetInfo */
MODULEVAR const char TOK_SNETINFO[] = "!1";	/* 33 + 49 */
MODULEVAR const char MSG_TSCTL[] = "TSCTL";	/* TSCTL */
MODULEVAR const char TOK_TSCTL[] = "!3";	/* 33 + 51 */
MODULEVAR const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
MODULEVAR const char TOK_SVSJOIN[] = "!4";	/* 33 + 52 */
MODULEVAR const char MSG_SAJOIN[] = "SAJOIN";	/* SAJOIN */
MODULEVAR const char TOK_SAJOIN[] = "!5";	/* 33 + 53 */
MODULEVAR const char MSG_SDESC[] = "SDESC";	/* SDESC */
MODULEVAR const char TOK_SDESC[] = "!6";	/* 33 + 54 */
MODULEVAR const char MSG_UNREALINFO[] = "UNREALINFO";	/* Unreal Info */
MODULEVAR const char TOK_UNREALINFO[] = "!7";	/* 33 + 55 */
MODULEVAR const char MSG_SETHOST[] = "SETHOST";	/* sethost */
MODULEVAR const char TOK_SETHOST[] = "!8";	/* 33 + 56 */
MODULEVAR const char MSG_SETIDENT[] = "SETIDENT";	/* set ident */
MODULEVAR const char TOK_SETIDENT[] = "!9";	/* 33 + 57 */
MODULEVAR const char MSG_SETNAME[] = "SETNAME";	/* set Realname */
MODULEVAR const char TOK_SETNAME[] = "!;";	/* 33 + 59 */
MODULEVAR const char MSG_CHGHOST[] = "CHGHOST";	/* Changehost */
MODULEVAR const char TOK_CHGHOST[] = "!<";	/* 33 + 60 */
MODULEVAR const char MSG_CHGIDENT[] = "CHGIDENT";	/* Change Ident */
MODULEVAR const char TOK_CHGIDENT[] = "!=";	/* 33 + 61 */
MODULEVAR const char MSG_RANDQUOTE[] = "RANDQUOTE";	/* Random Quote */
MODULEVAR const char TOK_RANDQUOTE[] = "!>";	/* 33 + 62 */
MODULEVAR const char MSG_ADDQUOTE[] = "ADDQUOTE";	/* Add Quote */
MODULEVAR const char TOK_ADDQUOTE[] = "!?";	/* 33 + 63 */
MODULEVAR const char MSG_ADDGQUOTE[] = "ADDGQUOTE";	/* Add Global Quote */
MODULEVAR const char TOK_ADDGQUOTE[] = "!@";	/* 33 + 64 */
MODULEVAR const char MSG_ADDULINE[] = "ADDULINE";	/* Adds an U Line to ircd.conf file */
MODULEVAR const char TOK_ADDULINE[] = "!B";	/* 33 + 66 */
MODULEVAR const char MSG_DELULINE[] = "DELULINE";	/* Removes an U line from the ircd.conf */
MODULEVAR const char TOK_DELULINE[] = "!C";	/* 33 + 67 */
MODULEVAR const char MSG_KNOCK[] = "KNOCK";	/* Knock Knock - Who's there? */
MODULEVAR const char TOK_KNOCK[] = "!D";	/* 33 + 68 */
MODULEVAR const char MSG_SETTINGS[] = "SETTINGS";	/* Settings */
MODULEVAR const char TOK_SETTINGS[] = "!E";	/* 33 + 69 */
MODULEVAR const char MSG_IRCOPS[] = "IRCOPS";	/* Shows Online IRCOps */
MODULEVAR const char TOK_IRCOPS[] = "!F";	/* 33 + 70 */
MODULEVAR const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
MODULEVAR const char TOK_SVSPART[] = "!G";	/* 33 + 71 */
MODULEVAR const char MSG_SAPART[] = "SAPART";	/* SAPART */
MODULEVAR const char TOK_SAPART[] = "!H";	/* 33 + 72 */
MODULEVAR const char MSG_VCTRL[] = "VCTRL";	/* VCTRL */
MODULEVAR const char TOK_VCTRL[] = "!I";	/* 33 + 73 */
MODULEVAR const char MSG_GCLIENT[] = "GCLIENT";	/* GLIENT */
MODULEVAR const char TOK_GCLIENT[] = "!J";	/* 33 + 74 */
MODULEVAR const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
MODULEVAR const char TOK_CHANNEL[] = "!K";	/* 33 + 75 */
MODULEVAR const char MSG_UPTIME[] = "UPTIME";	/* UPTIME */
MODULEVAR const char TOK_UPTIME[] = "!L";	/* 33 + 76 */
MODULEVAR const char MSG_FAILOPS[] = "FAILOPS";	/* FAILOPS */
MODULEVAR const char TOK_FAILOPS[] = "!M";	/* 33 + 77 */

MODULEVAR const char MSG_RPING[] = "RPING";	/* RPING */
MODULEVAR const char TOK_RPING[] = "!P";	/* 33 + 80 */
MODULEVAR const char MSG_RPONG[] = "RPONG";	/* RPONG */
MODULEVAR const char TOK_RPONG[] = "!Q";	/* 33 + 81 */
MODULEVAR const char MSG_UPING[] = "UPING";	/* UPING */
MODULEVAR const char TOK_UPING[] = "!R";	/* 33 + 82 */
MODULEVAR const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* Copyright */
MODULEVAR const char TOK_COPYRIGHT[] = "!S";	/* 33 + 83 */
MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
MODULEVAR const char MSG_BS[] = "BS";
MODULEVAR const char TOK_BOTSERV[] = "!T";	/* 33 + 84 */
MODULEVAR const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
MODULEVAR const char MSG_RS[] = "RS";
MODULEVAR const char TOK_ROOTSERV[] = "!U";	/* 33 + 85 */
MODULEVAR const char MSG_SVINFO[] = "SVINFO";
MODULEVAR const char MSG_CAPAB[] = "CAPAB";
MODULEVAR const char MSG_BURST[] = "BURST";
MODULEVAR const char MSG_SJOIN[] = "SJOIN";
MODULEVAR const char MSG_CLIENT[] = "CLIENT";
MODULEVAR const char MSG_SMODE[] = "SMODE";

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

/* Smodes */

/* Cmodes */

#endif
