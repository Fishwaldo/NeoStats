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

#ifndef UNREAL_H
#define UNREAL_H

/* Messages/Tokens */
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";	/* PRIV */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";	/* WHOI */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";	/* WHOW */
MODULEVAR const char MSG_USER[] = "USER";	/* USER */
MODULEVAR const char MSG_NICK[] = "NICK";	/* NICK */
MODULEVAR const char MSG_LIST[] = "LIST";	/* LIST */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";	/* TOPI */
MODULEVAR const char MSG_INVITE[] = "INVITE";	/* INVI */
MODULEVAR const char MSG_VERSION[] = "VERSION";	/* VERS */
MODULEVAR const char MSG_QUIT[] = "QUIT";	/* QUIT */
MODULEVAR const char MSG_KILL[] = "KILL";	/* KILL */
MODULEVAR const char MSG_INFO[] = "INFO";	/* INFO */
MODULEVAR const char MSG_LINKS[] = "LINKS";	/* LINK */
MODULEVAR const char MSG_SUMMON[] = "SUMMON";	/* SUMM */
MODULEVAR const char MSG_USERS[] = "USERS";	/* USER -> USRS */
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
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";	/* GLOBOPS */
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";	/* LOCOPS */
MODULEVAR const char MSG_PROTOCTL[] = "PROTOCTL";	/* PROTOCTL */
MODULEVAR const char MSG_WATCH[] = "WATCH";	/* WATCH */
MODULEVAR const char MSG_TRACE[] = "TRACE";	/* TRAC */
MODULEVAR const char MSG_SQLINE[] = "SQLINE";	/* SQLINE */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";	/* UNSQLINE */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";	/* IDENTIFY */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";	/* NICKSERV */
MODULEVAR const char MSG_NS[] = "NS";
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";	/* CHANSERV */
MODULEVAR const char MSG_CS[] = "CS";
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";	/* OPERSERV */
MODULEVAR const char MSG_OS[] = "OS";
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";	/* MEMOSERV */
MODULEVAR const char MSG_MS[] = "MS";
MODULEVAR const char MSG_SERVICES[] = "SERVICES";	/* SERVICES */
MODULEVAR const char MSG_SAMODE[] = "SAMODE";	/* SAMODE */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";	/* CHATOPS */
MODULEVAR const char MSG_ZLINE[] = "ZLINE";	/* ZLINE */
MODULEVAR const char MSG_UNZLINE[] = "UNZLINE";	/* UNZLINE */
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV";	/* HELPSERV */
MODULEVAR const char MSG_HS[] = "HS";
MODULEVAR const char MSG_RULES[] = "RULES";	/* RULES */
MODULEVAR const char MSG_MAP[] = "MAP";	/* MAP */
MODULEVAR const char MSG_DALINFO[] = "DALINFO";	/* dalinfo */
MODULEVAR const char MSG_ADMINCHAT[] = "ADCHAT";	/* Admin chat */
MODULEVAR const char MSG_MKPASSWD[] = "MKPASSWD";	/* MKPASSWD */
MODULEVAR const char MSG_ADDLINE[] = "ADDLINE";	/* ADDLINE */
MODULEVAR const char MSG_GLINE[] = "GLINE";	/* The awesome g-line */
MODULEVAR const char MSG_SJOIN[] = "SJOIN";
MODULEVAR const char MSG_SETHOST[] = "SETHOST";	/* sethost */
MODULEVAR const char MSG_NACHAT[] = "NACHAT";	/* netadmin chat */
MODULEVAR const char MSG_SETIDENT[] = "SETIDENT";
MODULEVAR const char MSG_SETNAME[] = "SETNAME";	/* set GECOS */
MODULEVAR const char MSG_LAG[] = "LAG";	/* Lag detect */
MODULEVAR const char MSG_STATSERV[] = "STATSERV";	/* alias */
MODULEVAR const char MSG_KNOCK[] = "KNOCK";
MODULEVAR const char MSG_CREDITS[] = "CREDITS";
MODULEVAR const char MSG_LICENSE[] = "LICENSE";
MODULEVAR const char MSG_CHGHOST[] = "CHGHOST";
MODULEVAR const char MSG_RPING[] = "RPING";
MODULEVAR const char MSG_RPONG[] = "RPONG";
MODULEVAR const char MSG_NETINFO[] = "NETINFO";
MODULEVAR const char MSG_SENDUMODE[] = "SENDUMODE";
MODULEVAR const char MSG_ADDMOTD[] = "ADDMOTD";
MODULEVAR const char MSG_ADDOMOTD[] = "ADDOMOTD";
MODULEVAR const char MSG_SMO[] = "SMO";
MODULEVAR const char MSG_OPERMOTD[] = "OPERMOTD";
MODULEVAR const char MSG_TSCTL[] = "TSCTL";
MODULEVAR const char MSG_SAJOIN[] = "SAJOIN";
MODULEVAR const char MSG_SAPART[] = "SAPART";
MODULEVAR const char MSG_CHGIDENT[] = "CHGIDENT";
MODULEVAR const char MSG_SWHOIS[] = "SWHOIS";
MODULEVAR const char MSG_SVSO[] = "SVSO";
MODULEVAR const char MSG_TKL[] = "TKL";
MODULEVAR const char MSG_VHOST[] = "VHOST";
MODULEVAR const char MSG_BOTMOTD[] = "BOTMOTD";
MODULEVAR const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
MODULEVAR const char MSG_UMODE2[] = "UMODE2";
MODULEVAR const char MSG_DCCDENY[] = "DCCDENY";
MODULEVAR const char MSG_UNDCCDENY[] = "UNDCCDENY";
MODULEVAR const char MSG_CHGNAME[] = "CHGNAME";
MODULEVAR const char MSG_SHUN[] = "SHUN";
MODULEVAR const char MSG_NEWJOIN[] = "NEWJOIN";	/* For CR Java Chat */
MODULEVAR const char MSG_POST[] = "POST";
MODULEVAR const char MSG_INFOSERV[] = "INFOSERV";
MODULEVAR const char MSG_IS[] = "IS";
MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";
MODULEVAR const char MSG_CYCLE[] = "CYCLE";
MODULEVAR const char MSG_MODULE[] = "MODULE";
MODULEVAR const char MSG_SENDSNO[] = "SENDSNO";

/* Umodes */
#define UMODE_FAILOP		0x00200000
#define UMODE_SERVNOTICE	0x00400000
#define UMODE_NOCTCP		0x00800000
#define UMODE_WEBTV			0x01000000
#define UMODE_WHOIS			0x02000000
#define UMODE_SECURE		0x04000000
#define UMODE_VICTIM		0x08000000
#define UMODE_HIDEOPER		0x10000000
#define UMODE_SETHOST		0x20000000
#define UMODE_STRIPBADWORDS	0x40000000
#define UMODE_HIDEWHOIS		0x80000000

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

#endif /* UNREAL_H Define */
