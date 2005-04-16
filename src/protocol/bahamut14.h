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
#ifndef BAHAMUT14_H
#define BAHAMUT14_H

/* Messages/Tokens */
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";		/* PRIV */
MODULEVAR const char MSG_WHO[] = "WHO";	      	/* WHO  -> WHOC */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";	   	/* WHOI */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";	   	/* WHOW */
MODULEVAR const char MSG_USER[] = "USER";	   	/* USER */
MODULEVAR const char MSG_NICK[] = "NICK";	   	/* NICK */
MODULEVAR const char MSG_SERVER[] = "SERVER";	   	/* SERV */
MODULEVAR const char MSG_LIST[] = "LIST";	   	/* LIST */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";	   	/* TOPI */
MODULEVAR const char MSG_INVITE[] = "INVITE";	   	/* INVI */
MODULEVAR const char MSG_VERSION[] = "VERSION";		/* VERS */
MODULEVAR const char MSG_QUIT[] = "QUIT";	   	/* QUIT */
MODULEVAR const char MSG_SQUIT[] = "SQUIT";	   	/* SQUI */
MODULEVAR const char MSG_KILL[] = "KILL";	   	/* KILL */
MODULEVAR const char MSG_INFO[] = "INFO";	   	/* INFO */
MODULEVAR const char MSG_LINKS[] = "LINKS";	   	/* LINK */
MODULEVAR const char MSG_STATS[] = "STATS";	   	/* STAT */
MODULEVAR const char MSG_USERS[] = "USERS";	   	/* USER -> USRS */
MODULEVAR const char MSG_HELP[] = "HELP";	   	/* HELP */
MODULEVAR const char MSG_ERROR[] = "ERROR";	   	/* ERRO */
MODULEVAR const char MSG_AWAY[] = "AWAY";	   	/* AWAY */
MODULEVAR const char MSG_CONNECT[] = "CONNECT";		/* CONN */
MODULEVAR const char MSG_PING[] = "PING";	   	/* PING */
MODULEVAR const char MSG_PONG[] = "PONG";	   	/* PONG */
MODULEVAR const char MSG_OPER[] = "OPER";	   	/* OPER */
MODULEVAR const char MSG_PASS[] = "PASS";	   	/* PASS */
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";		/* WALL */
MODULEVAR const char MSG_TIME[] = "TIME";	   	/* TIME */
MODULEVAR const char MSG_NAMES[] = "NAMES";	   	/* NAME */
MODULEVAR const char MSG_ADMIN[] = "ADMIN";	   	/* ADMI */
MODULEVAR const char MSG_TRACE[] = "TRACE";	   	/* TRAC */
MODULEVAR const char MSG_NOTICE[] = "NOTICE";	   	/* NOTI */
MODULEVAR const char MSG_JOIN[] = "JOIN";	   	/* JOIN */
MODULEVAR const char MSG_PART[] = "PART";	   	/* PART */
MODULEVAR const char MSG_LUSERS[] = "LUSERS";	   	/* LUSE */
MODULEVAR const char MSG_MOTD[] = "MOTD";	   	/* MOTD */
MODULEVAR const char MSG_MODE[] = "MODE";	   	/* MODE */
MODULEVAR const char MSG_KICK[] = "KICK";	   	/* KICK */
MODULEVAR const char MSG_USERHOST[] = "USERHOST";		/* USER -> USRH */
MODULEVAR const char MSG_ISON[] = "ISON";	   	/* ISON */
MODULEVAR const char MSG_REHASH[] = "REHASH";	   	/* REHA */
MODULEVAR const char MSG_RESTART[] = "RESTART";		/* REST */
MODULEVAR const char MSG_CLOSE[] = "CLOSE";	   	/* CLOS */
MODULEVAR const char MSG_SVINFO[] = "SVINFO";	   	/* SVINFO */
MODULEVAR const char MSG_SJOIN[] = "SJOIN";	   	/* SJOIN */
MODULEVAR const char MSG_DIE[] = "DIE"; 		/* DIE */
MODULEVAR const char MSG_HASH[] = "HASH";	   	/* HASH */
MODULEVAR const char MSG_DNS[] = "DNS";   	   	/* DNS  -> DNSS */
MODULEVAR const char MSG_OPERWALL[] = "OPERWALL";		/* OPERWALL */
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";		/* GLOBOPS */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";		/* CHATOPS */
MODULEVAR const char MSG_GOPER[] = "GOPER";	   	/* GOPER */
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";		/* GNOTICE */
MODULEVAR const char MSG_KLINE[] = "KLINE";	   	/* KLINE */
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";		/* UNKLINE */
MODULEVAR const char MSG_HTM[] = "HTM";	      	/* HTM */
MODULEVAR const char MSG_SET[] = "SET";	      	/* SET */
MODULEVAR const char MSG_SAMODE[] = "SAMODE";    	/* SAMODE */
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";		/* CHANSERV */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";		/* NICKSERV */
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";		/* MEMOSERV */
MODULEVAR const char MSG_ROOTSERV[] = "ROOTSERV";		/* MEMOSERV */
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";		/* OPERSERV */
MODULEVAR const char MSG_STATSERV[] = "STATSERV"; 	/* STATSERV */
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV"; 	/* HELPSERV */
MODULEVAR const char MSG_SERVICES[] = "SERVICES";		/* SERVICES */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";		/* IDENTIFY */
MODULEVAR const char MSG_CAPAB[] = "CAPAB";	   	/* CAPAB */ 
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";	   	/* LOCOPS */
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";   	/* SVSNICK */
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";   	/* SVSNOOP */
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";   	/* SVSKILL */
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";   	/* SVSMODE */
MODULEVAR const char MSG_AKILL[] = "AKILL";     	/* AKILL */
MODULEVAR const char MSG_RAKILL[] = "RAKILL";    	/* RAKILL */
MODULEVAR const char MSG_SILENCE[] = "SILENCE";   	/* SILENCE */
MODULEVAR const char MSG_WATCH[] = "WATCH";     	/* WATCH */
MODULEVAR const char MSG_SQLINE[] = "SQLINE"; 		/* SQLINE */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE"; 	/* UNSQLINE */
MODULEVAR const char MSG_BURST[] = "BURST";     	/* BURST */
MODULEVAR const char MSG_DCCALLOW[] = "DCCALLOW";		/* dccallow */
MODULEVAR const char MSG_SGLINE[] = "SGLINE";           /* sgline */
MODULEVAR const char MSG_UNSGLINE[] = "UNSGLINE";         /* unsgline */
MODULEVAR const char MSG_DKEY[] = "DKEY";		/* diffie-hellman negotiation */
MODULEVAR const char MSG_NS[] = "NS";            	/* NickServ commands */
MODULEVAR const char MSG_CS[] = "CS";            	/* ChanServ commands */
MODULEVAR const char MSG_MS[] = "MS";            	/* MemoServ commands */
MODULEVAR const char MSG_RS[] = "RS";            	/* RootServ commands */
MODULEVAR const char MSG_OS[] = "OS";            	/* OperServ commands */
MODULEVAR const char MSG_SS[] = "SS";            	/* StatServ commands */
MODULEVAR const char MSG_HS[] = "HS";            	/* StatServ commands */

/* Umodes */
#define UMODE_SERVNOTICE	0x00100000	/* umode +s - Server notices */
#define UMODE_KILLS			0x00200000	/* umode +k - Server kill messages */
#define UMODE_FLOOD			0x00400000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00800000	/* umode +y - Stats/links */
#define UMODE_DEBUG			0x00100000	/* umode +d - Debug info */
#define UMODE_GLOBOPS		0x00200000	/* umode +g - Globops */
#define UMODE_CHATOPS		0x00400000	/* umode +b - Chatops */
#define UMODE_ROUTE			0x00800000	/* umode +n - Routing Notices */
#define UMODE_SPAM			0x01000000	/* umode +m - spambot notices */
#define UMODE_OPERNOTICE	0x02000000	/* umode +e - oper notices for the above +D */
#define UMODE_SQUELCH		0x04000000	/* umode +x - Squelch with notice */
#define UMODE_SQUELCHN		0x08000000	/* umode +X - Squelch without notice */
#define UMODE_HIDDENDCC     0x10000000	/* umode +D - Hidden dccallow umode */
#define UMODE_THROTTLE		0x20000000	/* umode +F - no cptr->since message rate throttle */
#define UMODE_REJ			0x40000000	/* umode +j - client rejection notices */
#define UMODE_ULINEKILL     0x80000000	/* umode +K - U: lined server kill messages */

/* Cmodes */
#define CMODE_MODREG	0x02000000
#define CMODE_LISTED	0x04000000

#endif /* BAHAMUT14_H */
