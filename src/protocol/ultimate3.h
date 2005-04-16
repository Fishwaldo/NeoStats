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
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
MODULEVAR const char MSG_WHO[] = "WHO";		/* WHO  -> WHOC */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";		/* WHOI */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
MODULEVAR const char MSG_USER[] = "USER";		/* USER */
MODULEVAR const char MSG_NICK[] = "NICK";		/* NICK */
MODULEVAR const char MSG_SERVER[] = "SERVER";	/* SERV */
MODULEVAR const char MSG_LIST[] = "LIST";		/* LIST */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";		/* TOPI */
MODULEVAR const char MSG_INVITE[] = "INVITE";	/* INVI */
MODULEVAR const char MSG_VERSION[] = "VERSION";	/* VERS */
MODULEVAR const char MSG_QUIT[] = "QUIT";		/* QUIT */
MODULEVAR const char MSG_SQUIT[] = "SQUIT";		/* SQUI */
MODULEVAR const char MSG_KILL[] = "KILL";		/* KILL */
MODULEVAR const char MSG_INFO[] = "INFO";		/* INFO */
MODULEVAR const char MSG_LINKS[] = "LINKS";		/* LINK */
MODULEVAR const char MSG_STATS[] = "STATS";		/* STAT */
MODULEVAR const char MSG_USERS[] = "USERS";		/* USER -> USRS */
MODULEVAR const char MSG_HELP[] = "HELP";		/* HELP */
MODULEVAR const char MSG_ERROR[] = "ERROR";		/* ERRO */
MODULEVAR const char MSG_AWAY[] = "AWAY";		/* AWAY */
MODULEVAR const char MSG_CONNECT[] = "CONNECT";	/* CONN */
MODULEVAR const char MSG_PING[] = "PING";		/* PING */
MODULEVAR const char MSG_PONG[] = "PONG";		/* PONG */
MODULEVAR const char MSG_OPER[] = "OPER";		/* OPER */
MODULEVAR const char MSG_PASS[] = "PASS";		/* PASS */
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
MODULEVAR const char MSG_TIME[] = "TIME";		/* TIME */
MODULEVAR const char MSG_NAMES[] = "NAMES";		/* NAME */
MODULEVAR const char MSG_ADMIN[] = "ADMIN";		/* ADMI */
MODULEVAR const char MSG_TRACE[] = "TRACE";		/* TRAC */
MODULEVAR const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
MODULEVAR const char MSG_JOIN[] = "JOIN";		/* JOIN */
MODULEVAR const char MSG_PART[] = "PART";		/* PART */
MODULEVAR const char MSG_LUSERS[] = "LUSERS";	/* LUSE */
MODULEVAR const char MSG_MOTD[] = "MOTD";		/* MOTD */
MODULEVAR const char MSG_MODE[] = "MODE";		/* MODE */
MODULEVAR const char MSG_KICK[] = "KICK";		/* KICK */
MODULEVAR const char MSG_USERHOST[] = "USERHOST";	/* USER -> USRH */
MODULEVAR const char MSG_ISON[] = "ISON";		/* ISON */
MODULEVAR const char MSG_REHASH[] = "REHASH";	/* REHA */
MODULEVAR const char MSG_RESTART[] = "RESTART";	/* REST */
MODULEVAR const char MSG_CLOSE[] = "CLOSE";		/* CLOS */
MODULEVAR const char MSG_SVINFO[] = "SVINFO";	/* SVINFO */
MODULEVAR const char MSG_SJOIN[] = "SJOIN";		/* SJOIN */
MODULEVAR const char MSG_DIE[] = "DIE";		/* DIE */
MODULEVAR const char MSG_HASH[] = "HASH";		/* HASH */
MODULEVAR const char MSG_DNS[] = "DNS";		/* DNS  -> DNSS */
MODULEVAR const char MSG_OPERWALL[] = "OPERWALL";	/* OPERWALL */
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
MODULEVAR const char MSG_GOPER[] = "GOPER";		/* GOPER */
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";	/* GNOTICE */
MODULEVAR const char MSG_KLINE[] = "KLINE";		/* KLINE */
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
MODULEVAR const char MSG_HTM[] = "HTM";		/* HTM */
MODULEVAR const char MSG_SET[] = "SET";		/* SET */
MODULEVAR const char MSG_CAPAB[] = "CAPAB";		/* CAPAB */
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";	/* SVSNICK */
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";	/* SVSNOOP */
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";	/* SVSKILL */
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";	/* SVSMODE */
MODULEVAR const char MSG_AKILL[] = "AKILL";		/* AKILL */
MODULEVAR const char MSG_RAKILL[] = "RAKILL";	/* RAKILL */
MODULEVAR const char MSG_SILENCE[] = "SILENCE";	/* SILENCE */
MODULEVAR const char MSG_WATCH[] = "WATCH";		/* WATCH */
MODULEVAR const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
MODULEVAR const char MSG_BURST[] = "BURST";		/* BURST */
MODULEVAR const char MSG_DCCALLOW[] = "DCCALLOW";	/* dccallow */
MODULEVAR const char MSG_SGLINE[] = "SGLINE";	/* sgline */
MODULEVAR const char MSG_UNSGLINE[] = "UNSGLINE";	/* unsgline */
MODULEVAR const char MSG_SETTINGS[] = "SETTINGS";	/* SETTINGS */
MODULEVAR const char MSG_RULES[] = "RULES";		/* RULES */
MODULEVAR const char MSG_OPERMOTD[] = "OPERMOTD";	/* OPERMOTD */
MODULEVAR const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
MODULEVAR const char MSG_NETGLOBAL[] = "NETGLOBAL";	/* NETGLOBAL */
MODULEVAR const char MSG_SETHOST[] = "SETHOST";	/* SETHOST */
MODULEVAR const char MSG_VHOST[] = "VHOST";		/* VHOST */
MODULEVAR const char MSG_CREDITS[] = "CREDITS";	/* CREDITS */
MODULEVAR const char MSG_COPYRIGHT[] = "COPYRIGHT";	/* COPYRIGHT */
MODULEVAR const char MSG_ADCHAT[] = "ADCHAT";	/* ADCHAT */
MODULEVAR const char MSG_GCONNECT[] = "GCONNECT";	/* GCONNECT */
MODULEVAR const char MSG_IRCOPS[] = "IRCOPS";	/* IRCOPS */
MODULEVAR const char MSG_KNOCK[] = "KNOCK";		/* KNOCK */
MODULEVAR const char MSG_CHANNEL[] = "CHANNEL";	/* CHANNEL */
MODULEVAR const char MSG_VCTRL[] = "VCTRL";		/* VCTRL */
MODULEVAR const char MSG_CSCHAT[] = "CSCHAT";	/* CSCHAT */
MODULEVAR const char MSG_MAP[] = "MAP";		/* MAP */
MODULEVAR const char MSG_MAKEPASS[] = "MAKEPASS";	/* MAKEPASS */
MODULEVAR const char MSG_DKEY[] = "DKEY";		/* diffie-hellman negotiation */
MODULEVAR const char MSG_FJOIN[] = "FJOIN";		/* Forced Join's */
MODULEVAR const char MSG_FMODE[] = "FMODE";		/* Forced Mode's */
MODULEVAR const char MSG_IRCDHELP[] = "IRCDHELP";	/* IRCDHELP */
MODULEVAR const char MSG_ADDOPER[] = "ADDOPER";	/* ADDOPER */
MODULEVAR const char MSG_DELOPER[] = "DELOPER";	/* DELOPER */
MODULEVAR const char MSG_ADDCNLINE[] = "ADDCNLINE";	/* ADDCNLINE */
MODULEVAR const char MSG_DELCNLINE[] = "DELCNLINE";	/* DELCNLINE */
MODULEVAR const char MSG_ADDQLINE[] = "ADDQLINE";	/* ADDQLINE */
MODULEVAR const char MSG_DELQLINE[] = "DELQLINE";	/* DELQLINE */
MODULEVAR const char MSG_ADDHLINE[] = "ADDHLINE";	/* ADDHLINE */
MODULEVAR const char MSG_DELHLINE[] = "DELHLINE";	/* DELHLINE */
MODULEVAR const char MSG_ADDULINE[] = "ADDULINE";	/* ADDULINE */
MODULEVAR const char MSG_DELULINE[] = "DELULINE";	/* DELULINE */
MODULEVAR const char MSG_CLIENT[] = "CLIENT";	/* CLIENT */
MODULEVAR const char MSG_NETCTRL[] = "NETCTRL";	/* NETCTRL */
MODULEVAR const char MSG_SMODE[] = "SMODE";		/* SMODE */
MODULEVAR const char MSG_RESYNCH[] = "RESYNCH";	/* RESYNCH */
MODULEVAR const char MSG_EOBURST[] = "EOBURST";	/* EOBURST */
MODULEVAR const char MSG_CS[] = "CS";		/* CS */
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
MODULEVAR const char MSG_NS[] = "NS";		/* NS */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
MODULEVAR const char MSG_MS[] = "MS";		/* MS */
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
MODULEVAR const char MSG_OS[] = "OS";		/* OS */
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
MODULEVAR const char MSG_SS[] = "SS";		/* SS */
MODULEVAR const char MSG_STATSERV[] = "STATSERV";	/* STATSERV */
MODULEVAR const char MSG_BS[] = "BS";		/* BS */
MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";	/* BOTSERV */
MODULEVAR const char MSG_RS[] = "RS";		/* RS */
MODULEVAR const char MSG_HS[] = "HS";		/* HS aka HeadShot :) */
MODULEVAR const char MSG_HOSTSERV[] = "HOSTSERV";	/* HOSTSERV */
MODULEVAR const char MSG_ROOTSERV[] = "ROOTSERV";	/* ROOTSERV */
MODULEVAR const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
MODULEVAR const char MSG_NMODE[] = "NMODE";		/* NMODE */
MODULEVAR const char MSG_SVSJOIN[] = "SVSJOIN";	/* SVSJOIN */
MODULEVAR const char MSG_CHANFIX[] = "CHANFIX";	/* CHANFIX */
MODULEVAR const char MSG_SVSPART[] = "SVSPART";	/* SVSPART */
MODULEVAR const char MSG_USERIP[] = "USERIP";	/* USERIP */

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
