/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */

#define FEATURES 0

/* buffer sizes */
#define MAXHOST			(128 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(30 + 1)
#define MAXUSER			(10 + 1)
#define MAXREALNAME		(50 + 1)
#define CHANLEN			(32 + 1)
#define TOPICLEN		(307 + 1)

/* Messages/Tokens */
#define MSG_PRIVATE	"PRIVMSG"	/* PRIV */
#define MSG_WHOIS	"WHOIS"	/* WHOI */
#define MSG_WHOWAS	"WHOWAS"	/* WHOW */
#define MSG_USER	"USER"	/* USER */
#define MSG_NICK	"NICK"	/* NICK */
#define MSG_LIST	"LIST"	/* LIST */
#define MSG_TOPIC	"TOPIC"	/* TOPI */
#define MSG_INVITE	"INVITE"	/* INVI */
#define MSG_VERSION	"VERSION"	/* VERS */
#define MSG_QUIT	"QUIT"	/* QUIT */
#define MSG_KILL	"KILL"	/* KILL */
#define MSG_INFO	"INFO"	/* INFO */
#define MSG_LINKS	"LINKS"	/* LINK */
#define MSG_SUMMON	"SUMMON"	/* SUMM */
#define MSG_USERS	"USERS"	/* USER -> USRS */
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
#define MSG_GLOBOPS	"GLOBOPS"	/* GLOBOPS */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define MSG_WATCH	"WATCH"	/* WATCH */
#define MSG_TRACE	"TRACE"	/* TRAC */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define MSG_NICKSERV	"NICKSERV"	/* NICKSERV */
#define MSG_NS		"NS"
#define MSG_CHANSERV	"CHANSERV"	/* CHANSERV */
#define MSG_CS		"CS"
#define MSG_OPERSERV	"OPERSERV"	/* OPERSERV */
#define MSG_OS		"OS"
#define MSG_MEMOSERV	"MEMOSERV"	/* MEMOSERV */
#define MSG_MS		"MS"
#define MSG_SERVICES	"SERVICES"	/* SERVICES */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define MSG_ZLINE    	"ZLINE"	/* ZLINE */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */
#define MSG_HELPSERV    "HELPSERV"	/* HELPSERV */
#define MSG_HS		"HS"
#define MSG_RULES       "RULES"	/* RULES */
#define MSG_MAP         "MAP"	/* MAP */
#define MSG_DALINFO     "DALINFO"	/* dalinfo */
#define MSG_ADMINCHAT   "ADCHAT"	/* Admin chat */
#define MSG_MKPASSWD	"MKPASSWD"	/* MKPASSWD */
#define MSG_ADDLINE     "ADDLINE"	/* ADDLINE */
#define MSG_GLINE	"GLINE"	/* The awesome g-line */
#define MSG_SJOIN	"SJOIN"
#define MSG_SETHOST 	"SETHOST"	/* sethost */
#define MSG_NACHAT  	"NACHAT"	/* netadmin chat */
#define MSG_SETIDENT    "SETIDENT"
#define MSG_SETNAME	"SETNAME"	/* set GECOS */
#define MSG_LAG		"LAG"	/* Lag detect */
#define MSG_STATSERV	"STATSERV"	/* alias */
#define MSG_KNOCK	"KNOCK"
#define MSG_CREDITS 	"CREDITS"
#define MSG_LICENSE 	"LICENSE"
#define MSG_CHGHOST 	"CHGHOST"
#define MSG_RPING   	"RPING"
#define MSG_RPONG   	"RPONG"
#define MSG_NETINFO 	"NETINFO"
#define MSG_SENDUMODE 	"SENDUMODE"
#define MSG_ADDMOTD 	"ADDMOTD"
#define MSG_ADDOMOTD	"ADDOMOTD"
#define MSG_SMO 	"SMO"
#define MSG_OPERMOTD 	"OPERMOTD"
#define MSG_TSCTL 	"TSCTL"
#define MSG_SAJOIN 	"SAJOIN"
#define MSG_SAPART 	"SAPART"
#define MSG_CHGIDENT 	"CHGIDENT"
#define MSG_SWHOIS 	"SWHOIS"
#define MSG_SVSO 	"SVSO"
#define MSG_TKL		"TKL"
#define MSG_VHOST 	"VHOST"
#define MSG_BOTMOTD 	"BOTMOTD"
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define MSG_UMODE2	"UMODE2"
#define MSG_DCCDENY	"DCCDENY"
#define MSG_UNDCCDENY   "UNDCCDENY"
#define MSG_CHGNAME	"CHGNAME"
#define MSG_SHUN	"SHUN"
#define MSG_NEWJOIN 	"NEWJOIN"	/* For CR Java Chat */
#define MSG_POST	"POST"
#define MSG_INFOSERV 	"INFOSERV"
#define MSG_IS		"IS"
#define MSG_BOTSERV	"BOTSERV"
#define MSG_CYCLE	"CYCLE"
#define MSG_MODULE	"MODULE"
#define MSG_SENDSNO	"SENDSNO"

/* Umode chars */
#define UMODE_CH_LOCOP 'O'
#define UMODE_CH_OPER 'o'
#define UMODE_CH_COADMIN 'C'
#define UMODE_CH_ADMIN 'A'
#define UMODE_CH_NETADMIN 'N'
#define UMODE_CH_SADMIN 'a'
#define UMODE_CH_SERVICES 'S'
#define UMODE_CH_BOT 'B'

/* Umodes */
#define UMODE_WALLOP		0x00000004	
#define UMODE_FAILOP		0x00000008	
#define UMODE_HELPOP		0x00000010	
#define UMODE_REGNICK		0x00000020	
#define UMODE_SADMIN		0x00000040	
#define UMODE_ADMIN			0x00000080	
#define UMODE_SERVNOTICE	0x00000100	
#define UMODE_LOCOP			0x00000200	
#define UMODE_RGSTRONLY		0x00000400	
#define UMODE_NOCTCP		0x00000800	
#define UMODE_WEBTV			0x00001000	
#define UMODE_SERVICES		0x00002000	
#define UMODE_HIDE			0x00004000	
#define UMODE_NETADMIN		0x00008000	
#define UMODE_COADMIN		0x00010000	
#define UMODE_WHOIS			0x00020000	
#define UMODE_KIX			0x00040000
#define UMODE_BOT			0x00080000	
#define UMODE_SECURE		0x00100000
#define UMODE_VICTIM		0x00200000
#define UMODE_DEAF			0x00400000
#define UMODE_HIDEOPER		0x00800000
#define UMODE_SETHOST		0x01000000
#define UMODE_STRIPBADWORDS	0x02000000
#define UMODE_HIDEWHOIS		0x04000000

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

#endif /* UNREAL_H Define */
