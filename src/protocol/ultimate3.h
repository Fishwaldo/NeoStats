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
#ifndef ULTIMATE_H
#define ULTIMATE_H

/* Messages/Tokens */
#define MSG_PRIVATE  	"PRIVMSG"	/* PRIV */
#define MSG_WHO      	"WHO"		/* WHO  -> WHOC */
#define MSG_WHOIS    	"WHOIS"		/* WHOI */
#define MSG_WHOWAS   	"WHOWAS"	/* WHOW */
#define MSG_USER     	"USER"		/* USER */
#define MSG_NICK     	"NICK"		/* NICK */
#define MSG_SERVER   	"SERVER"	/* SERV */
#define MSG_LIST     	"LIST"		/* LIST */
#define MSG_TOPIC    	"TOPIC"		/* TOPI */
#define MSG_INVITE   	"INVITE"	/* INVI */
#define MSG_VERSION  	"VERSION"	/* VERS */
#define MSG_QUIT     	"QUIT"		/* QUIT */
#define MSG_SQUIT    	"SQUIT"		/* SQUI */
#define MSG_KILL     	"KILL"		/* KILL */
#define MSG_INFO     	"INFO"		/* INFO */
#define MSG_LINKS    	"LINKS"		/* LINK */
#define MSG_STATS    	"STATS"		/* STAT */
#define MSG_USERS    	"USERS"		/* USER -> USRS */
#define MSG_HELP     	"HELP"		/* HELP */
#define MSG_ERROR    	"ERROR"		/* ERRO */
#define MSG_AWAY     	"AWAY"		/* AWAY */
#define MSG_CONNECT  	"CONNECT"	/* CONN */
#define MSG_PING     	"PING"		/* PING */
#define MSG_PONG     	"PONG"		/* PONG */
#define MSG_OPER     	"OPER"		/* OPER */
#define MSG_PASS     	"PASS"		/* PASS */
#define MSG_WALLOPS  	"WALLOPS"	/* WALL */
#define MSG_TIME     	"TIME"		/* TIME */
#define MSG_NAMES    	"NAMES"		/* NAME */
#define MSG_ADMIN    	"ADMIN"		/* ADMI */
#define MSG_TRACE    	"TRACE"		/* TRAC */
#define MSG_NOTICE   	"NOTICE"	/* NOTI */
#define MSG_JOIN     	"JOIN"		/* JOIN */
#define MSG_PART     	"PART"		/* PART */
#define MSG_LUSERS   	"LUSERS"	/* LUSE */
#define MSG_MOTD     	"MOTD"		/* MOTD */
#define MSG_MODE     	"MODE"		/* MODE */
#define MSG_KICK     	"KICK"		/* KICK */
#define MSG_USERHOST 	"USERHOST"	/* USER -> USRH */
#define MSG_ISON     	"ISON"		/* ISON */
#define MSG_REHASH   	"REHASH"	/* REHA */
#define MSG_RESTART  	"RESTART"	/* REST */
#define MSG_CLOSE    	"CLOSE"		/* CLOS */
#define MSG_SVINFO   	"SVINFO"	/* SVINFO */
#define MSG_SJOIN    	"SJOIN"		/* SJOIN */
#define MSG_DIE	     	"DIE"		/* DIE */
#define MSG_HASH     	"HASH"		/* HASH */
#define MSG_DNS      	"DNS"		/* DNS  -> DNSS */
#define MSG_OPERWALL 	"OPERWALL"	/* OPERWALL */
#define MSG_GLOBOPS  	"GLOBOPS"	/* GLOBOPS */
#define MSG_CHATOPS  	"CHATOPS"	/* CHATOPS */
#define MSG_GOPER    	"GOPER"		/* GOPER */
#define MSG_GNOTICE  	"GNOTICE"	/* GNOTICE */
#define MSG_KLINE    	"KLINE"		/* KLINE */
#define MSG_UNKLINE  	"UNKLINE"	/* UNKLINE */
#define MSG_HTM      	"HTM"		/* HTM */
#define MSG_SET      	"SET"		/* SET */
#define MSG_CAPAB    	"CAPAB"		/* CAPAB */
#define MSG_LOCOPS   	"LOCOPS"	/* LOCOPS */
#define MSG_SVSNICK  	"SVSNICK"	/* SVSNICK */
#define MSG_SVSNOOP  	"SVSNOOP"	/* SVSNOOP */
#define MSG_SVSKILL  	"SVSKILL"	/* SVSKILL */
#define MSG_SVSMODE  	"SVSMODE"	/* SVSMODE */
#define MSG_AKILL    	"AKILL"		/* AKILL */
#define MSG_RAKILL   	"RAKILL"	/* RAKILL */
#define MSG_SILENCE  	"SILENCE"	/* SILENCE */
#define MSG_WATCH    	"WATCH"		/* WATCH */
#define MSG_SQLINE   	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE 	"UNSQLINE"	/* UNSQLINE */
#define MSG_BURST    	"BURST"		/* BURST */
#define MSG_DCCALLOW 	"DCCALLOW"	/* dccallow */
#define MSG_SGLINE   	"SGLINE"	/* sgline */
#define MSG_UNSGLINE 	"UNSGLINE"	/* unsgline */
#define MSG_SETTINGS	"SETTINGS"	/* SETTINGS */
#define MSG_RULES	"RULES"		/* RULES */
#define MSG_OPERMOTD	"OPERMOTD"	/* OPERMOTD */
#define MSG_NETINFO	"NETINFO"	/* NETINFO */
#define MSG_NETGLOBAL	"NETGLOBAL"	/* NETGLOBAL */
#define MSG_SETHOST	"SETHOST"	/* SETHOST */
#define MSG_VHOST	"VHOST"		/* VHOST */
#define MSG_CREDITS	"CREDITS"	/* CREDITS */
#define MSG_COPYRIGHT	"COPYRIGHT"	/* COPYRIGHT */
#define MSG_ADCHAT	"ADCHAT"	/* ADCHAT */
#define MSG_GCONNECT	"GCONNECT"	/* GCONNECT */
#define MSG_IRCOPS	"IRCOPS"	/* IRCOPS */
#define MSG_KNOCK	"KNOCK"		/* KNOCK */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define MSG_VCTRL	"VCTRL"		/* VCTRL */
#define MSG_CSCHAT	"CSCHAT"	/* CSCHAT */
#define MSG_MAP		"MAP"		/* MAP */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */
#define MSG_DKEY     	"DKEY"		/* diffie-hellman negotiation */
#define MSG_FJOIN	"FJOIN"		/* Forced Join's */
#define MSG_FMODE	"FMODE"		/* Forced Mode's */
#define MSG_IRCDHELP	"IRCDHELP"	/* IRCDHELP */
#define MSG_ADDOPER	"ADDOPER"	/* ADDOPER */
#define MSG_DELOPER	"DELOPER"	/* DELOPER */
#define MSG_ADDCNLINE	"ADDCNLINE"	/* ADDCNLINE */
#define MSG_DELCNLINE	"DELCNLINE"	/* DELCNLINE */
#define MSG_ADDQLINE	"ADDQLINE"	/* ADDQLINE */
#define MSG_DELQLINE	"DELQLINE"	/* DELQLINE */
#define MSG_ADDHLINE	"ADDHLINE"	/* ADDHLINE */
#define MSG_DELHLINE	"DELHLINE"	/* DELHLINE */
#define MSG_ADDULINE	"ADDULINE"	/* ADDULINE */
#define MSG_DELULINE	"DELULINE"	/* DELULINE */
#define MSG_CLIENT	"CLIENT"	/* CLIENT */
#define MSG_NETCTRL	"NETCTRL"	/* NETCTRL */
#define MSG_SMODE	"SMODE"		/* SMODE */
#define MSG_RESYNCH	"RESYNCH"	/* RESYNCH */
#define MSG_EOBURST	"EOBURST"	/* EOBURST */
#define MSG_CS		"CS"		/* CS */
#define MSG_CHANSERV 	"CHANSERV"	/* CHANSERV */
#define MSG_NS		"NS"		/* NS */
#define MSG_NICKSERV 	"NICKSERV"	/* NICKSERV */
#define MSG_MS		"MS"		/* MS */
#define MSG_MEMOSERV 	"MEMOSERV"	/* MEMOSERV */
#define MSG_OS		"OS"		/* OS */
#define MSG_OPERSERV 	"OPERSERV"	/* OPERSERV */
#define MSG_SS		"SS"		/* SS */
#define MSG_STATSERV 	"STATSERV"	/* STATSERV */
#define MSG_BS		"BS"		/* BS */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_RS		"RS"		/* RS */
#define MSG_HS		"HS"		/* HS aka HeadShot :) */
#define MSG_HOSTSERV 	"HOSTSERV"	/* HOSTSERV */
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_SERVICES 	"SERVICES"	/* SERVICES */
#define MSG_IDENTIFY 	"IDENTIFY"	/* IDENTIFY */
#define MSG_NMODE	"NMODE"		/* NMODE */
#define MSG_SVSJOIN	"SVSJOIN"	/* SVSJOIN */
#define MSG_CHANFIX	"CHANFIX"	/* CHANFIX */
#define MSG_SVSPART	"SVSPART"	/* SVSPART */
#define MSG_USERIP	"USERIP"	/* USERIP */

/* Umodes */
#define UMODE_SERVNOTICE    0x00020000	/* umode +s - Server notices */
#define UMODE_KILLS     	0x00040000	/* umode +k - Server kill messages */
#define UMODE_FLOOD     	0x00080000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00100000	/* umode +y - Stats/links */
#define UMODE_DCC     		0x00200000	/* umode +D - pseudo/hidden, has seen dcc warning message */
#define UMODE_GLOBOPS     	0x00400000	/* umode +g - Globops */
#define UMODE_CHATOPS     	0x00800000	/* umode +C - Chatops */
#define UMODE_SERVICESOPER  0x01000000	/* umode +a - Services Operator - Should be moved to smode */
#define UMODE_REJ			0x02000000	/* umode +j - Reject notices */
#define UMODE_ROUTE     	0x04000000	/* umode +n - Routing Notices */
#define UMODE_SPAM     		0x08000000	/* umode +m - spambot notices */
#define UMODE_SRA			0x10000000	/* umode +Z - Services Root Admin - Should be moved to smode */
#define UMODE_DEBUG			0x20000000	/* umode +d - Debug Info */
#define UMODE_DCCWARN		0x40000000	/* umode +e - See DCC send warnings */
#define UMODE_WHOIS			0x80000000	/* umode +W - Opers can see when a user /whois's them */

/* Smodes */

/* Cmodes */

#endif
