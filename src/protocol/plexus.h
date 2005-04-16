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

#ifndef PLEXUS_H
#define PLEXUS_H

/* Messages/Tokens */
MODULEVAR const char MSG_EOB[] = "EOB";	/* end of burst */
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
MODULEVAR const char MSG_WHO[] = "WHO";	/* WHO  -> WHOC */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
MODULEVAR const char MSG_USER[] = "USER";	/* USER */
MODULEVAR const char MSG_NICK[] = "NICK";	/* NICK */
MODULEVAR const char MSG_SERVER[] = "SERVER";	/* SERV */
MODULEVAR const char MSG_LIST[] = "LIST";	/* LIST */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
MODULEVAR const char MSG_INVITE[] = "INVITE";	/* INVI */
MODULEVAR const char MSG_VERSION[] = "VERSION";	/* VERS */
MODULEVAR const char MSG_QUIT[] = "QUIT";	/* QUIT */
MODULEVAR const char MSG_SQUIT[] = "SQUIT";	/* SQUI */
MODULEVAR const char MSG_KILL[] = "KILL";	/* KILL */
MODULEVAR const char MSG_INFO[] = "INFO";	/* INFO */
MODULEVAR const char MSG_LINKS[] = "LINKS";	/* LINK */
MODULEVAR const char MSG_WATCH[] = "WATCH";	/* WATCH */
MODULEVAR const char MSG_STATS[] = "STATS";	/* STAT */
MODULEVAR const char MSG_HELP[] = "HELP";	/* HELP */
MODULEVAR const char MSG_HELPOP[] = "HELPOP";	/* HELP */
MODULEVAR const char MSG_ERROR[] = "ERROR";	/* ERRO */
MODULEVAR const char MSG_AWAY[] = "AWAY";	/* AWAY */
MODULEVAR const char MSG_CONNECT[] = "CONNECT";	/* CONN */
MODULEVAR const char MSG_PING[] = "PING";	/* PING */
MODULEVAR const char MSG_PONG[] = "PONG";	/* PONG */
MODULEVAR const char MSG_OPER[] = "OPER";	/* OPER */
MODULEVAR const char MSG_PASS[] = "PASS";	/* PASS */
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
MODULEVAR const char MSG_TIME[] = "TIME";	/* TIME */
MODULEVAR const char MSG_NAMES[] = "NAMES";	/* NAME */
MODULEVAR const char MSG_ADMIN[] = "ADMIN";	/* ADMI */
MODULEVAR const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
MODULEVAR const char MSG_JOIN[] = "JOIN";	/* JOIN */
MODULEVAR const char MSG_PART[] = "PART";	/* PART */
MODULEVAR const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
MODULEVAR const char MSG_MOTD[] = "MOTD";	/* MOTD */
MODULEVAR const char MSG_MODE[] = "MODE";	/* MODE */
MODULEVAR const char MSG_KICK[] = "KICK";	/* KICK */
MODULEVAR const char MSG_SERVICE[] = "SERVICE";	/* SERV -> SRVI */
MODULEVAR const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
MODULEVAR const char MSG_ISON[] = "ISON";	/* ISON */
MODULEVAR const char MSG_SQUERY[] = "SQUERY";	/* SQUE */
MODULEVAR const char MSG_SERVLIST[] = "SERVLIST";	/* SERV -> SLIS */
MODULEVAR const char MSG_SERVSET[] = "SERVSET";	/* SERV -> SSET */
MODULEVAR const char MSG_REHASH[] = "REHASH";	/* REHA */
MODULEVAR const char MSG_RESTART[] = "RESTART";	/* REST */
MODULEVAR const char MSG_CLOSE[] = "CLOSE";	/* CLOS */
MODULEVAR const char MSG_DIE[] = "DIE";	/* DIE */
MODULEVAR const char MSG_HASH[] = "HASH";	/* HASH */
MODULEVAR const char MSG_DNS[] = "DNS";	/* DNS  -> DNSS */
MODULEVAR const char MSG_SILENCE[] = "SILENCE";	/* SILE */
MODULEVAR const char MSG_AKILL[] = "AKILL";	/* AKILL */
MODULEVAR const char MSG_KLINE[] = "KLINE";	/* KLINE */
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
MODULEVAR const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
MODULEVAR const char MSG_GOPER[] = "GOPER";	/* GOPER */
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
MODULEVAR const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
MODULEVAR const char MSG_TRACE[] = "TRACE";	/* TRAC */
MODULEVAR const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
MODULEVAR const char MSG_NS[] = "NS";
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
MODULEVAR const char MSG_CS[] = "CS";
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
MODULEVAR const char MSG_OS[] = "OS";
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
MODULEVAR const char MSG_MS[] = "MS";
MODULEVAR const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
MODULEVAR const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
MODULEVAR const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
MODULEVAR const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
MODULEVAR const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
MODULEVAR const char MSG_RULES[] = "RULES";	/* RULES */
MODULEVAR const char MSG_MAP [] = "MAP";	/* MAP */
MODULEVAR const char MSG_NETG[] = "NETG";	/* NETG */
MODULEVAR const char MSG_ADCHAT[] = "ADCHAT";	/* Adchat */
MODULEVAR const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
MODULEVAR const char MSG_ADDHUB[] = "ADDHUB";	/* ADDHUB */
MODULEVAR const char MSG_DELHUB[] = "DELHUB";	/* DELHUB */
MODULEVAR const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
MODULEVAR const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
MODULEVAR const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
MODULEVAR const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
MODULEVAR const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
MODULEVAR const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
MODULEVAR const char MSG_GSOP[] = "GSOP";	/* GSOP */
MODULEVAR const char MSG_ISOPER[] = "ISOPER";	/* ISOPER */
MODULEVAR const char MSG_ADG[] = "ADG";	/* ADG */
MODULEVAR const char MSG_NMON[] = "NMON";	/* NMON */
MODULEVAR const char MSG_DALINFO[] = "DALINFO";	/* DALnet Credits */
MODULEVAR const char MSG_CREDITS[] = "CREDITS";	/* UltimateIRCd Credits and "Thanks To" */
MODULEVAR const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
MODULEVAR const char MSG_REMREHASH[] = "REMREHASH";	/* Remote Rehash */
MODULEVAR const char MSG_MONITOR[] = "MONITOR";	/* MONITOR */
MODULEVAR const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
MODULEVAR const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
MODULEVAR const char MSG_STATSERV[] = "STATSERV";	/* StatServ */
MODULEVAR const char MSG_RULESERV[] = "RULESERV";	/* RuleServ */
MODULEVAR const char MSG_SNETINFO[] = "SNETINFO";	/* SNetInfo */
MODULEVAR const char MSG_TSCTL[] = "TSCTL";	/* TSCTL */
MODULEVAR const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
MODULEVAR const char MSG_SAJOIN[] = "SAJOIN";	/* SAJOIN */
MODULEVAR const char MSG_SDESC[] = "SDESC";	/* SDESC */
MODULEVAR const char MSG_UNREALINFO[] = "UNREALINFO";	/* Unreal Info */
MODULEVAR const char MSG_SETHOST[] = "SETHOST";	/* sethost */
MODULEVAR const char MSG_SETIDENT[] = "SETIDENT";	/* set ident */
MODULEVAR const char MSG_SETNAME[] = "SETNAME";	/* set Realname */
MODULEVAR const char MSG_CHGHOST[] = "CHGHOST";	/* Changehost */
MODULEVAR const char MSG_CHGIDENT[] = "CHGIDENT";	/* Change Ident */
MODULEVAR const char MSG_RANDQUOTE[] = "RANDQUOTE";	/* Random Quote */
MODULEVAR const char MSG_ADDQUOTE[] = "ADDQUOTE";	/* Add Quote */
MODULEVAR const char MSG_ADDGQUOTE[] = "ADDGQUOTE";	/* Add Global Quote */
MODULEVAR const char MSG_ADDULINE[] = "ADDULINE";	/* Adds an U Line to ircd.conf file */
MODULEVAR const char MSG_DELULINE[] = "DELULINE";	/* Removes an U line from the ircd.conf */
MODULEVAR const char MSG_KNOCK[] = "KNOCK";	/* Knock Knock - Who's there? */
MODULEVAR const char MSG_SETTINGS[] = "SETTINGS";	/* Settings */
MODULEVAR const char MSG_IRCOPS[] = "IRCOPS";	/* Shows Online IRCOps */
MODULEVAR const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
MODULEVAR const char MSG_SAPART[] = "SAPART";	/* SAPART */
/*#define MSG_VCTRL	"VCTRL"	*//* VCTRL */
MODULEVAR const char MSG_GCLIENT[] = "GCLIENT";	/* GLIENT */
MODULEVAR const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
MODULEVAR const char MSG_UPTIME[] = "UPTIME";	/* UPTIME */
MODULEVAR const char MSG_FAILOPS[] = "FAILOPS";	/* FAILOPS */
MODULEVAR const char MSG_RPING[] = "RPING";	/* RPING */
MODULEVAR const char MSG_RPONG[] = "RPONG";	/* RPONG */
MODULEVAR const char MSG_UPING[] = "UPING";	/* UPING */
MODULEVAR const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* Copyright */
MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
MODULEVAR const char MSG_BS[] = "BS";
MODULEVAR const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
MODULEVAR const char MSG_SVINFO[] = "SVINFO";
MODULEVAR const char MSG_CAPAB[] = "CAPAB";
MODULEVAR const char MSG_BURST[] = "BURST";
MODULEVAR const char MSG_SJOIN[] = "SJOIN";
MODULEVAR const char MSG_TBURST[] = "TBURST";

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

#endif /* PLEXUS_H */
