/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#ifndef LIQUIDIRCD_H
#define LIQUIDIRCD_H

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */

/* we don't support tokens */
#undef GOTTOKENSUPPORT
/* we don't support CLIENT */
#undef GOTCLIENTSUPPORT
/* we have vhost support */
#define GOTSVSHOST 
/* we don't have svsjoin support */
#undef GOTSVSJOIN 
/* we don't have svsmode */
#undef GOTSVSMODE
/* we don't have svspart */
#undef GOTSVSPART
/* we have svsnick */
#define GOTSVSNICK
/* we don't have smo */
#undef GOTSMO
/* we don't have swhois */
#undef GOTSWHOIS
/* we don't have bot mode support */
#undef GOTBOTMODE
/* we do have user smode support */
#define GOTUSERSMODES
/* we have svskill support */
#define GOTSVSKILL
/* we don't have automatic host cloaking support via Umode */
#undef GOTUMODECLOAKING

/* buffer sizes */
#define MAXHOST			128
#define MAXPASS			32
#define MAXNICK			32
#define MAXUSER			15
#define MAXREALNAME		50
#define CHANLEN			50
#define TOPICLEN		512

/* Messages/Tokens */
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
#define	MSG_REHASH	"REHASH"	/* REHA */
#define	MSG_RESTART	"RESTART"	/* REST */
#define	MSG_CLOSE	"CLOSE"	/* CLOS */
#define	MSG_DIE		"DIE"	/* DIE */
#define	MSG_HASH	"HASH"	/* HASH */
#define	MSG_DNS		"DNS"	/* DNS  -> DNSS */
#define MSG_SILENCE	"SILENCE"	/* SILE */
#define MSG_AKILL	"AKILL"	/* AKILL */
#define MSG_KLINE	"KLINE"	/* KLINE */
#define MSG_UNKLINE     "UNKLINE"	/* UNKLINE */
#define MSG_RAKILL	"RAKILL"	/* RAKILL */
#define MSG_GNOTICE	"GNOTICE"	/* GNOTICE */
#define MSG_GOPER	"GOPER"	/* GOPER */
#define MSG_GLOBOPS	"GLOBOPS"	/* GLOBOPS */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define MSG_TRACE	"TRACE"	/* TRAC */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
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
#define MSG_VCTRL	"VCTRL"	/* VCTRL */
#define MSG_GCLIENT	"GCLIENT"	/* GLIENT */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define MSG_UPTIME	"UPTIME"	/* UPTIME */
#define MSG_FAILOPS	"FAILOPS"	/* FAILOPS */
#define MSG_RPING	"RPING"	/* RPING */
#define MSG_RPONG       "RPONG"	/* RPONG */
#define MSG_UPING       "UPING"	/* UPING */
#define MSG_COPYRIGHT	"COPYRIGHT"	/* Copyright */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_RS		"RS"
#define MSG_SVINFO	"SVINFO"
#define MSG_CAPAB	"CAPAB"
#define MSG_BURST	"BURST"
#define MSG_SJOIN	"SJOIN"
#define MSG_CLIENT	"CLIENT"
#define MSG_SMODE	"SMODE"

/* Umode chars */
#define UMODE_CH_LOCOP 'O'
#define UMODE_CH_OPER 'o'
#define UMODE_CH_ADMIN 'A'
#define UMODE_CH_NETADMIN 'N'
#define UMODE_CH_TECHADMIN 'T'
#define UMODE_CH_SADMIN 'a'
#define UMODE_CH_SERVICES 'S'
#define UMODE_CH_BOT 'B'

/* Umodes */
#define	UMODE_INVISIBLE  	0x0001	/* makes user invisible */
#define	UMODE_OPER       	0x0002	/* Operator */
#define UMODE_LOCOP             0x0004  /* Local Operator */
#define UMODE_WALLOP            0x0008  /* */
#define UMODE_REGONLY		0x0010	/* only registered nicks may PM */
#define UMODE_REGNICK	 	0x0020	/* Nick set by services as registered */
#define UMODE_SERVADMIN		0x0040	/* server admin */
#define UMODE_SERVICESADMIN	0x0080	/* Marks the client as a Services Administrator */
#define UMODE_FAILOP            0x0100  /* */
#define UMODE_HELPOP            0x0200  /* */
#define UMODE_SERVNOTICE        0x0400  /* */
#define UMODE_KILLS             0x0800  /* */
#define UMODE_CLIENT            0x1000  /* */
#define UMODE_FLOOD             0x2000  /* */
#define UMODE_NETADMIN          0x4000  /* */
#define UMODE_TECHADMIN         0x8000  /* */
#define UMODE_KIX               0x10000  /* */
#define UMODE_BOT               0x20000  /* */
#define UMODE_WHOIS             0x40000  /* */
#define UMODE_HIDE              0x80000  /* */

/* Smodes */
#define SMODE_SSL              0x1  /* */

/* Cmodes */
#define CMODE_CHANOP	0x0001
#define CMODE_CHANOWNER  0x0002
#define	CMODE_VOICE	0x0004
#define	CMODE_PRIVATE	0x0008
#define	CMODE_SECRET	0x0010
#define	CMODE_MODERATED  0x0020
#define	CMODE_TOPICLIMIT 0x0040
#define	CMODE_INVITEONLY 0x0080
#define	CMODE_NOPRIVMSGS 0x0100
#define	CMODE_KEY	0x0200
#define CMODE_NONICKCHANGE 0x0400
#define	CMODE_BAN	0x0800
#define CMODE_LIMIT	0x1000
#define CMODE_RGSTR	0x2000
#define CMODE_RGSTRONLY  0x4000
#define CMODE_OPERONLY   0x8000
#define CMODE_STRIP      0x10000
#define CMODE_LINK	0x20000
#define CMODE_NOCOLOR	0x40000
#define CMODE_CHANPROT   0x80000
#define CMODE_VIP        0x100000
#define CMODE_UOP        0x200000
#define CMODE_HALFOP     0x400000

/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_OPERONLY)))
#define is_pub_chan(x) ((x) && (CheckChanMode(x, CMODE_PRIVATE) || CheckChanMode(x, CMODE_SECRET) || CheckChanMode(x, CMODE_RGSTRONLY) || CheckChanMode(x, CMODE_OPERONLY) || CheckChanMode(x, CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))
#define is_priv_chan(x) ((x) && (CheckChanMode(x, CMODE_PRIVATE) || CheckChanMode(x, CMODE_SECRET) || CheckChanMode(x, CMODE_RGSTRONLY) || CheckChanMode(x, CMODE_OPERONLY) || CheckChanMode(x, CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))

/* Umode macros */
#define is_oper(x) ((x) && ((x->Umode & UMODE_OPER) || (x->Umode & UMODE_LOCOP)))
#define is_bot(x) ((x) && (x->Umode & UMODE_BOT))    

#endif
