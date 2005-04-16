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


#ifndef HYBRID7_H
#define HYBRID7_H

/* Messages/Tokens */
const char MSG_EOB[] = "EOB";	/* end of burst */
const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
const char MSG_NICK[] = "NICK";	/* NICK */
const char MSG_SERVER[] = "SERVER";	/* SERV */
const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
const char MSG_INVITE[] = "INVITE";	/* INVI */
const char MSG_VERSION[] = "VERSION";	/* VERS */
const char MSG_QUIT[] = "QUIT";	/* QUIT */
const char MSG_SQUIT[] = "SQUIT";	/* SQUI */
const char MSG_KILL[] = "KILL";	/* KILL */
const char MSG_STATS[] = "STATS";	/* STAT */
const char MSG_ERROR[] = "ERROR";	/* ERRO */
const char MSG_AWAY[] = "AWAY";	/* AWAY */
const char MSG_PING[] = "PING";	/* PING */
const char MSG_PONG[] = "PONG";	/* PONG */
const char MSG_PASS[] = "PASS";	/* PASS */
const char MSG_WALLOPS[] = "WALLOPS";	/* WALL */
const char MSG_ADMIN[] = "ADMIN";	/* ADMI */
const char MSG_NOTICE[] = "NOTICE";	/* NOTI */
const char MSG_JOIN[] = "JOIN";	/* JOIN */
const char MSG_PART[] = "PART";	/* PART */
const char MSG_MOTD[] = "MOTD";	/* MOTD */
const char MSG_MODE[] = "MODE";	/* MODE */
const char MSG_KICK[] = "KICK";	/* KICK */
const char MSG_KLINE[] = "KLINE";	/* KLINE */
const char MSG_UNKLINE[] = "UNKLINE";	/* UNKLINE */
const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
const char MSG_NETINFO[] = "NETINFO";	/* NETINFO */
const char MSG_CREDITS[] = "CREDITS";
const char MSG_SNETINFO[] = "SNETINFO";	/* SNetInfo */
const char MSG_SVINFO[] = "SVINFO";
const char MSG_CAPAB[] = "CAPAB";
const char MSG_SJOIN[] = "SJOIN";

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

#endif
