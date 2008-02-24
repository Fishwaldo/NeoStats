/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
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


#ifndef NEOIRCD_H
#define NEOIRCD_H

/* Messages/Tokens */
#define MSG_EOB		"EOB"	/* end of burst */
#define MSG_PRIVATE	"PRIVMSG"	/* PRIV */
#define MSG_WHO		"WHO"	/* WHO  -> WHOC */
#define MSG_WHOIS	"WHOIS"	/* WHOI */
#define MSG_WHOWAS	"WHOWAS"	/* WHOW */
#define MSG_USER	"USER"	/* USER */
#define MSG_NICK	"NICK"	/* NICK */
#define MSG_SERVER	"SERVER"	/* SERV */
#define MSG_LIST	"LIST"	/* LIST */
#define MSG_TOPIC	"TOPIC"	/* TOPI */
#define MSG_INVITE	"INVITE"	/* INVI */
#define MSG_VERSION	"VERSION"	/* VERS */
#define MSG_QUIT	"QUIT"	/* QUIT */
#define MSG_SQUIT	"SQUIT"	/* SQUI */
#define MSG_KILL	"KILL"	/* KILL */
#define MSG_INFO	"INFO"	/* INFO */
#define MSG_LINKS	"LINKS"	/* LINK */
#define MSG_WATCH	"WATCH"	/* WATCH */
#define MSG_STATS	"STATS"	/* STAT */
#define MSG_HELP	"HELP"	/* HELP */
#define MSG_HELPOP	"HELPOP"	/* HELP */
#define MSG_ERROR	"ERROR"	/* ERRO */
#define MSG_AWAY	"AWAY"	/* AWAY */
#define MSG_CONNECT	"CONNECT"	/* CONN */
#define MSG_PING	"PING"	/* PING */
#define MSG_PONG	"PONG"	/* PONG */
#define MSG_OPER	"OPER"	/* OPER */
#define MSG_PASS	"PASS"	/* PASS */
#define MSG_WALLOPS	"WALLOPS"	/* WALL */
#define MSG_TIME	"TIME"	/* TIME */
#define MSG_NAMES	"NAMES"	/* NAME */
#define MSG_ADMIN	"ADMIN"	/* ADMI */
#define MSG_NOTICE	"NOTICE"	/* NOTI */
#define MSG_JOIN	"JOIN"	/* JOIN */
#define MSG_PART	"PART"	/* PART */
#define MSG_LUSERS	"LUSERS"	/* LUSE */
#define MSG_MOTD	"MOTD"	/* MOTD */
#define MSG_MODE	"MODE"	/* MODE */
#define MSG_KICK	"KICK"	/* KICK */
#define MSG_SERVICE	"SERVICE"	/* SERV -> SRVI */
#define MSG_USERHOST	"USERHOST"	/* USER -> USRH */
#define MSG_ISON	"ISON"	/* ISON */
#define MSG_SQUERY	"SQUERY"	/* SQUE */
#define MSG_SERVLIST	"SERVLIST"	/* SERV -> SLIS */
#define MSG_SERVSET	"SERVSET"	/* SERV -> SSET */
#define MSG_REHASH	"REHASH"	/* REHA */
#define MSG_RESTART	"RESTART"	/* REST */
#define MSG_CLOSE	"CLOSE"	/* CLOS */
#define MSG_DIE		"DIE"	/* DIE */
#define MSG_HASH	"HASH"	/* HASH */
#define MSG_DNS		"DNS"	/* DNS  -> DNSS */
#define MSG_SILENCE	"SILENCE"	/* SILE */
#define MSG_AKILL	"AKILL"	/* AKILL */
#define MSG_KLINE	"KLINE"	/* KLINE */
#define MSG_UNKLINE     "UNKLINE"	/* UNKLINE */
#define MSG_RAKILL	"RAKILL"	/* RAKILL */
#define MSG_GNOTICE	"GNOTICE"	/* GNOTICE */
#define MSG_GOPER	"GOPER"	/* GOPER */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define MSG_TRACE	"TRACE"	/* TRAC */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
#define MSG_NICKSERV	"NICKSERV"	/* NICKSERV */
#define MSG_NS		"NS"
#define MSG_CHANSERV	"CHANSERV"	/* CHANSERV */
#define MSG_CS		"CS"
#define MSG_OPERSERV	"OPERSERV"	/* OPERSERV */
#define MSG_OS		"OS"
#define MSG_MEMOSERV	"MEMOSERV"	/* MEMOSERV */
#define MSG_MS		"MS"
#define MSG_SERVICES	"SERVICES"	/* SERVICES */
#define MSG_SVSMODE	"SVSMODE"	/* SVSMODE */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define MSG_HELPSERV    "HELPSERV"	/* HELPSERV */
#define MSG_ZLINE    	"ZLINE"	/* ZLINE */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */
#define MSG_NETINFO	"NETINFO"	/* NETINFO */
#define MSG_RULES       "RULES"	/* RULES */
#define MSG_MAP         "MAP"	/* MAP */
#define MSG_NETG	"NETG"	/* NETG */
#define MSG_ADCHAT   	"ADCHAT"	/* Adchat */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */
#define MSG_ADDHUB   	"ADDHUB"	/* ADDHUB */
#define MSG_DELHUB   	"DELHUB"	/* DELHUB */
#define MSG_ADDCNLINE  	"ADDCNLINE"	/* ADDCNLINE */
#define MSG_DELCNLINE  	"DELCNLINE"	/* DELCNLINE */
#define MSG_ADDOPER   	"ADDOPER"	/* ADDOPER */
#define MSG_DELOPER   	"DELOPER"	/* DELOPER */
#define MSG_ADDQLINE   	"ADDQLINE"	/* ADDQLINE */
#define MSG_DELQLINE   	"DELQLINE"	/* DELQLINE */
#define MSG_GSOP    	"GSOP"	/* GSOP */
#define MSG_ISOPER	"ISOPER"	/* ISOPER */
#define MSG_ADG	    	"ADG"	/* ADG */
#define MSG_NMON	"NMON"	/* NMON */
#define MSG_DALINFO	"DALINFO"	/* DALnet Credits */
#define MSG_CREDITS	"CREDITS"	/* UltimateIRCd Credits and "Thanks To" */
#define MSG_OPERMOTD    "OPERMOTD"	/* OPERMOTD */
#define MSG_REMREHASH	"REMREHASH"	/* Remote Rehash */
#define MSG_MONITOR	"MONITOR"	/* MONITOR */
#define MSG_GLINE	"GLINE"	/* The awesome g-line */
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define MSG_STATSERV	"STATSERV"	/* StatServ */
#define MSG_RULESERV	"RULESERV"	/* RuleServ */
#define MSG_SNETINFO	"SNETINFO"	/* SNetInfo */
#define MSG_TSCTL 	"TSCTL"	/* TSCTL */
#define MSG_SVSJOIN 	"SVSJOIN"	/* SVSJOIN */
#define MSG_SAJOIN 	"SAJOIN"	/* SAJOIN */
#define MSG_SDESC       "SDESC"	/* SDESC */
#define MSG_UNREALINFO	"UNREALINFO"	/* Unreal Info */
#define MSG_SETHOST 	"SETHOST"	/* sethost */
#define MSG_SETIDENT 	"SETIDENT"	/* set ident */
#define MSG_SETNAME 	"SETNAME"	/* set Realname */
#define MSG_CHGHOST 	"CHGHOST"	/* Changehost */
#define MSG_CHGIDENT 	"CHGIDENT"	/* Change Ident */
#define MSG_RANDQUOTE	"RANDQUOTE"	/* Random Quote */
#define MSG_ADDQUOTE	"ADDQUOTE"	/* Add Quote */
#define MSG_ADDGQUOTE	"ADDGQUOTE"	/* Add Global Quote */
#define MSG_ADDULINE	"ADDULINE"	/* Adds an U Line to ircd.conf file */
#define MSG_DELULINE	"DELULINE"	/* Removes an U line from the ircd.conf */
#define MSG_KNOCK	"KNOCK"	/* Knock Knock - Who's there? */
#define MSG_SETTINGS	"SETTINGS"	/* Settings */
#define MSG_IRCOPS	"IRCOPS"	/* Shows Online IRCOps */
#define MSG_SVSPART	"SVSPART"	/* SVSPART */
#define MSG_SAPART	"SAPART"	/* SAPART */
/*#define MSG_VCTRL	"VCTRL"	*//* VCTRL */
#define MSG_GCLIENT	"GCLIENT"	/* GLIENT */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define MSG_UPTIME	"UPTIME"	/* UPTIME */
#define MSG_FAILOPS	"FAILOPS"	/* FAILOPS */
#define MSG_RPING	"RPING"	/* RPING */
#define MSG_RPONG       "RPONG"	/* RPONG */
#define MSG_UPING       "UPING"	/* UPING */
#define MSG_COPYRIGHT	"COPYRIGHT"	/* Copyright */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_BS		"BS"
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_SVINFO	"SVINFO"
#define MSG_CAPAB	"CAPAB"
#define MSG_BURST	"BURST"
#define MSG_SJOIN	"SJOIN"
#define MSG_TBURST	"TBURST"

/* Umodes */
#define UMODE_BOTS			0x00100000	/* shows bots */
#define UMODE_DEBUG			0x00200000	/* show debug info */
#define UMODE_FULL			0x00400000	/* show full messages */
#define UMODE_CALLERID		0x00800000	/* client has callerid enabled */
#define UMODE_SKILL			0x01000000	/* client see's server kills */
#define UMODE_NCHANGE		0x02000000	/* client can see nick change notices */
#define UMODE_REJ			0x04000000	/* client is registered */
#define UMODE_SERVNOTICE	0x08000000	/* client can see server notices */
#define UMODE_UNAUTH		0x10000000	/* client can see unauthd connections */
#define UMODE_EXTERNAL		0x20000000	/* client can see server joins/splits */
#define UMODE_SPY			0x40000000	/* client can spy on user commands */
#define UMODE_OPERWALL		0x80000000	/* client gets operwalls */
/* Cmodes */
#define CMODE_HIDEOPS	0x02000000
#define CMODE_INVEX		0x04000000

#endif
