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
const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
const char MSG_WHO[] = "WHO";		/* WHO  -> WHOC */
const char MSG_WHOIS[] = "WHOIS";		/* WHOI */
const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
const char MSG_USER[] = "USER";		/* USER */
const char MSG_NICK[] = "NICK";		/* NICK */
const char MSG_SERVER[] = "SERVER";	/* SERV */
const char MSG_LIST[] = "LIST";		/* LIST */
const char MSG_TOPIC[] = "TOPIC";		/* TOPI */
const char MSG_INVITE[] = "INVITE";	/* INVI */
const char MSG_VERSION[] = "VERSION";	/* VERS */
const char MSG_QUIT[] = "QUIT";		/* QUIT */
const char MSG_SQUIT[] = "SQUIT";		/* SQUI */
const char MSG_KILL[] = "KILL";		/* KILL */
const char MSG_INFO[] = "INFO";		/* INFO */
const char MSG_LINKS[] = "LINKS";		/* LINK */
const char MSG_STATS[] = "STATS";		/* STAT */
const char MSG_USERS[] = "USERS";		/* USER -> USRS */
const char MSG_HELP[] = "HELP";		/* HELP */
const char MSG_ERROR[] = "ERROR";		/* ERRO */
const char MSG_AWAY[] = "AWAY";		/* AWAY */
const char MSG_CONNECT[] = "CONNECT";	/* CONN */
const char MSG_PING[] = "PING";		/* PING */
const char MSG_PONG[] = "PONG";		/* PONG */
const char MSG_OPER[] = "OPER";		/* OPER */
const char MSG_PASS[] = "PASS";		/* PASS */
const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
const char MSG_TIME[] = "TIME";		/* TIME */
const char MSG_NAMES[] = "NAMES";		/* NAME */
const char MSG_ADMIN[] = "ADMIN";		/* ADMI */
const char MSG_TRACE[] = "TRACE";		/* TRAC */
const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
const char MSG_JOIN[] = "JOIN";		/* JOIN */
const char MSG_PART[] = "PART";		/* PART */
const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
const char MSG_MOTD[] = "MOTD";		/* MOTD */
const char MSG_MODE[] = "MODE";		/* MODE */
const char MSG_KICK[] = "KICK";		/* KICK */
const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
const char MSG_ISON[] = "ISON";		/* ISON */
const char MSG_REHASH[] = "REHASH";	/* REHA */
const char MSG_RESTART[] = "RESTART";	/* REST */
const char MSG_CLOSE[] = "CLOSE";		/* CLOS */
const char MSG_SVINFO[] = "SVINFO";	/* SVINFO */
const char MSG_SJOIN[] = "SJOIN";		/* SJOIN */
const char MSG_DIE[] = "DIE";		/* DIE */
const char MSG_HASH[] = "HASH";		/* HASH */
const char MSG_DNS[] = "DNS";		/* DNS  -> DNSS */
const char MSG_OPERWALL[] = "OPERWALL";	/* OPERWALL */
const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
const char MSG_GOPER[] = "GOPER";		/* GOPER */
const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
const char MSG_KLINE[] = "KLINE";		/* KLINE */
const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
const char MSG_HTM[] = "HTM";		/* HTM */
const char MSG_SET[] = "SET";		/* SET */
const char MSG_CAPAB[] = "CAPAB";		/* CAPAB */
const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
const char MSG_AKILL[] = "AKILL";		/* AKILL */
const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
const char MSG_SILENCE[] = "SILENCE";	/* SILENCE */
const char MSG_WATCH[] = "WATCH";		/* WATCH */
const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
const char MSG_BURST[] = "BURST";		/* BURST */
const char MSG_DCCALLOW[] = "DCCALLOW";	/* dccallow */
const char MSG_SGLINE[] = "SGLINE";	/* sgline */
const char MSG_UNSGLINE[] = "UNSGLINE";	/* unsgline */
const char MSG_SETTINGS[] = "SETTINGS";	/* SETTINGS */
const char MSG_RULES[] = "RULES";		/* RULES */
const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
const char MSG_NETGLOBAL[] = "NETGLOBAL";	/* NETGLOBAL */
const char MSG_SETHOST[] = "SETHOST";	/* SETHOST */
const char MSG_VHOST[] = "VHOST";		/* VHOST */
const char MSG_CREDITS[] = "CREDITS";	/* CREDITS */
const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* COPYRIGHT */
const char MSG_ADCHAT[] = "ADCHAT";	/* ADCHAT */
const char MSG_GCONNECT[] = "GCONNECT";	/* GCONNECT */
const char MSG_IRCOPS[] = "IRCOPS";	/* IRCOPS */
const char MSG_KNOCK[] = "KNOCK";		/* KNOCK */
const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
const char MSG_VCTRL[] = "VCTRL";		/* VCTRL */
const char MSG_CSCHAT[] = "CSCHAT";	/* CSCHAT */
const char MSG_MAP[] = "MAP";		/* MAP */
const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
const char MSG_DKEY[] = "DKEY";		/* diffie-hellman negotiation */
const char MSG_FJOIN[] = "FJOIN";		/* Forced Join's */
const char MSG_FMODE[] = "FMODE";		/* Forced Mode's */
const char MSG_IRCDHELP[] = "IRCDHELP";	/* IRCDHELP */
const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
const char MSG_ADDHLINE[] = "ADDHLINE";	/* ADDHLINE */
const char MSG_DELHLINE[] = "DELHLINE";	/* DELHLINE */
const char MSG_ADDULINE[] = "ADDULINE";	/* ADDULINE */
const char MSG_DELULINE[] = "DELULINE";	/* DELULINE */
const char MSG_CLIENT[] = "CLIENT";	/* CLIENT */
const char MSG_NETCTRL[] = "NETCTRL";	/* NETCTRL */
const char MSG_SMODE[] = "SMODE";		/* SMODE */
const char MSG_RESYNCH[] = "RESYNCH";	/* RESYNCH */
const char MSG_EOBURST[] = "EOBURST";	/* EOBURST */
const char MSG_CS[] = "CS";		/* CS */
const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
const char MSG_NS[] = "NS";		/* NS */
const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
const char MSG_MS[] = "MS";		/* MS */
const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
const char MSG_OS[] = "OS";		/* OS */
const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
const char MSG_SS[] = "SS";		/* SS */
const char MSG_STATSERV[] = "STATSERV";	/* STATSERV */
const char MSG_BS[] = "BS";		/* BS */
const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
const char MSG_RS[] = "RS";		/* RS */
const char MSG_HS[] = "HS";		/* HS aka HeadShot :) */
const char MSG_HOSTSERV[] = "HOSTSERV";	/* HOSTSERV */
const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
const char MSG_NMODE[] = "NMODE";		/* NMODE */
const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
const char MSG_CHANFIX[] = "CHANFIX";	/* CHANFIX */
const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
const char MSG_USERIP[] = "USERIP";	/* USERIP */

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
