/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
*/

#ifndef UNREAL_H
#define UNREAL_H

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */

#define FEATURES FEATURE_SVSHOST \
	| FEATURE_SMO \
	| FEATURE_SWHOIS \
	| FEATURE_SVSTIME \
	| FEATURE_SVSJOIN \
	| FEATURE_SVSPART \
	| FEATURE_SVSMODE \
	| FEATURE_SVSNICK \
	| FEATURE_SVSKILL \
	| FEATURE_BOTMODES \
	| FEATURE_UMODECLOAK


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
#define TOK_PRIVATE	"!"	/* 33 */
#define MSG_WHO		"WHO"	/* WHO  -> WHOC */
#define TOK_WHO		"\""	/* 34 */
#define MSG_WHOIS	"WHOIS"	/* WHOI */
#define TOK_WHOIS	"#"	/* 35 */
#define MSG_WHOWAS	"WHOWAS"	/* WHOW */
#define TOK_WHOWAS	"$"	/* 36 */
#define MSG_USER	"USER"	/* USER */
#define TOK_USER	"%"	/* 37 */
#define MSG_NICK	"NICK"	/* NICK */
#define TOK_NICK	"&"	/* 38 */
#define MSG_SERVER	"SERVER"	/* SERV */
#define TOK_SERVER	"'"	/* 39 */
#define MSG_LIST	"LIST"	/* LIST */
#define TOK_LIST	"("	/* 40 */
#define MSG_TOPIC	"TOPIC"	/* TOPI */
#define TOK_TOPIC	")"	/* 41 */
#define MSG_INVITE	"INVITE"	/* INVI */
#define TOK_INVITE	"*"	/* 42 */
#define MSG_VERSION	"VERSION"	/* VERS */
#define TOK_VERSION	"+"	/* 43 */
#define MSG_QUIT	"QUIT"	/* QUIT */
#define TOK_QUIT	","	/* 44 */
#define MSG_SQUIT	"SQUIT"	/* SQUI */
#define TOK_SQUIT	"-"	/* 45 */
#define MSG_KILL	"KILL"	/* KILL */
#define TOK_KILL	"."	/* 46 */
#define MSG_INFO	"INFO"	/* INFO */
#define TOK_INFO	"/"	/* 47 */
#define MSG_LINKS	"LINKS"	/* LINK */
#define TOK_LINKS	"0"	/* 48 */
#define MSG_SUMMON	"SUMMON"	/* SUMM */
#define TOK_SUMMON	"1"	/* 49 */
#define MSG_STATS	"STATS"	/* STAT */
#define TOK_STATS	"2"	/* 50 */
#define MSG_USERS	"USERS"	/* USER -> USRS */
#define TOK_USERS	"3"	/* 51 */
#define MSG_HELP	"HELP"	/* HELP */
#define MSG_HELPOP	"HELPOP"	/* HELP */
#define TOK_HELP	"4"	/* 52 */
#define MSG_ERROR	"ERROR"	/* ERRO */
#define TOK_ERROR	"5"	/* 53 */
#define MSG_AWAY	"AWAY"	/* AWAY */
#define TOK_AWAY	"6"	/* 54 */
#define MSG_CONNECT	"CONNECT"	/* CONN */
#define TOK_CONNECT	"7"	/* 55 */
#define MSG_PING	"PING"	/* PING */
#define TOK_PING	"8"	/* 56 */
#define MSG_PONG	"PONG"	/* PONG */
#define TOK_PONG	"9"	/* 57 */
#define MSG_OPER	"OPER"	/* OPER */
#define TOK_OPER	";"	/* 59 */
#define MSG_PASS	"PASS"	/* PASS */
#define TOK_PASS	"<"	/* 60 */
#define MSG_WALLOPS	"WALLOPS"	/* WALL */
#define TOK_WALLOPS	"="	/* 61 */
#define MSG_TIME	"TIME"	/* TIME */
#define TOK_TIME	">"	/* 62 */
#define MSG_NAMES	"NAMES"	/* NAME */
#define TOK_NAMES	"?"	/* 63 */
#define MSG_ADMIN	"ADMIN"	/* ADMI */
#define TOK_ADMIN	"@"	/* 64 */
#define MSG_NOTICE	"NOTICE"	/* NOTI */
#define TOK_NOTICE	"B"	/* 66 */
#define MSG_JOIN	"JOIN"	/* JOIN */
#define TOK_JOIN	"C"	/* 67 */
#define MSG_PART	"PART"	/* PART */
#define TOK_PART	"D"	/* 68 */
#define MSG_LUSERS	"LUSERS"	/* LUSE */
#define TOK_LUSERS	"E"	/* 69 */
#define MSG_MOTD	"MOTD"	/* MOTD */
#define TOK_MOTD	"F"	/* 70 */
#define MSG_MODE	"MODE"	/* MODE */
#define TOK_MODE	"G"	/* 71 */
#define MSG_KICK	"KICK"	/* KICK */
#define TOK_KICK	"H"	/* 72 */
#define MSG_SERVICE	"SERVICE"	/* SERV -> SRVI */
#define TOK_SERVICE	"I"	/* 73 */
#define MSG_USERHOST	"USERHOST"	/* USER -> USRH */
#define TOK_USERHOST	"J"	/* 74 */
#define MSG_ISON	"ISON"	/* ISON */
#define TOK_ISON	"K"	/* 75 */
#define MSG_REHASH	"REHASH"	/* REHA */
#define TOK_REHASH	"O"	/* 79 */
#define MSG_RESTART	"RESTART"	/* REST */
#define TOK_RESTART	"P"	/* 80 */
#define MSG_CLOSE	"CLOSE"	/* CLOS */
#define TOK_CLOSE	"Q"	/* 81 */
#define MSG_DIE		"DIE"	/* DIE */
#define TOK_DIE		"R"	/* 82 */
#define MSG_HASH	"HASH"	/* HASH */
#define TOK_HASH	"S"	/* 83 */
#define MSG_DNS		"DNS"	/* DNS  -> DNSS */
#define TOK_DNS		"T"	/* 84 */
#define MSG_SILENCE	"SILENCE"	/* SILE */
#define TOK_SILENCE	"U"	/* 85 */
#define MSG_AKILL	"AKILL"	/* AKILL */
#define TOK_AKILL	"V"	/* 86 */
#define MSG_KLINE	"KLINE"	/* KLINE */
#define TOK_KLINE	"W"	/* 87 */
#define MSG_UNKLINE     "UNKLINE"	/* UNKLINE */
#define TOK_UNKLINE	"X"	/* 88 */
#define MSG_RAKILL	"RAKILL"	/* RAKILL */
#define TOK_RAKILL	"Y"	/* 89 */
#define MSG_GNOTICE	"GNOTICE"	/* GNOTICE */
#define TOK_GNOTICE	"Z"	/* 90 */
#define MSG_GOPER	"GOPER"	/* GOPER */
#define TOK_GOPER	"["	/* 91 */
#define MSG_GLOBOPS	"GLOBOPS"	/* GLOBOPS */
#define TOK_GLOBOPS	"]"	/* 93 */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define TOK_LOCOPS	"^"	/* 94 */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define TOK_PROTOCTL	"_"	/* 95 */
#define MSG_WATCH	"WATCH"	/* WATCH */
#define TOK_WATCH	"`"	/* 96 */
#define MSG_TRACE	"TRACE"	/* TRAC */
#define TOK_TRACE	"b"	/* 97 */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define TOK_SQLINE	"c"	/* 98 */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define TOK_UNSQLINE	"d"	/* 99 */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define TOK_SVSNICK	"e"	/* 100 */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define TOK_SVSNOOP	"f"	/* 101 */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define TOK_IDENTIFY	"g"	/* 102 */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
#define TOK_SVSKILL	"h"	/* 103 */
#define MSG_NICKSERV	"NICKSERV"	/* NICKSERV */
#define MSG_NS		"NS"
#define TOK_NICKSERV	"i"	/* 104 */
#define MSG_CHANSERV	"CHANSERV"	/* CHANSERV */
#define MSG_CS		"CS"
#define TOK_CHANSERV	"j"	/* 105 */
#define MSG_OPERSERV	"OPERSERV"	/* OPERSERV */
#define MSG_OS		"OS"
#define TOK_OPERSERV	"k"	/* 106 */
#define MSG_MEMOSERV	"MEMOSERV"	/* MEMOSERV */
#define MSG_MS		"MS"
#define TOK_MEMOSERV	"l"	/* 107 */
#define MSG_SERVICES	"SERVICES"	/* SERVICES */
#define TOK_SERVICES	"m"	/* 108 */
#define MSG_SVSMODE	"SVSMODE"	/* SVSMODE */
#define TOK_SVSMODE	"n"	/* 109 */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define TOK_SAMODE	"o"	/* 110 */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define TOK_CHATOPS	"p"	/* 111 */
#define MSG_ZLINE    	"ZLINE"	/* ZLINE */
#define TOK_ZLINE	"q"	/* 112 */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */
#define TOK_UNZLINE	"r"	/* 113 */
#define MSG_HELPSERV    "HELPSERV"	/* HELPSERV */
#define MSG_HS		"HS"
#define TOK_HELPSERV    "s"	/* 114 */
#define MSG_RULES       "RULES"	/* RULES */
#define TOK_RULES       "t"	/* 115 */
#define MSG_MAP         "MAP"	/* MAP */
#define TOK_MAP         "u"	/* 117 */
#define MSG_SVS2MODE    "SVS2MODE"	/* SVS2MODE */
#define TOK_SVS2MODE	"v"	/* 118 */
#define MSG_DALINFO     "DALINFO"	/* dalinfo */
#define TOK_DALINFO     "w"	/* 119 */
#define MSG_ADMINCHAT   "ADCHAT"	/* Admin chat */
#define TOK_ADMINCHAT   "x"	/* 120 */
#define MSG_MKPASSWD	"MKPASSWD"	/* MKPASSWD */
#define TOK_MKPASSWD	"y"	/* 121 */
#define MSG_ADDLINE     "ADDLINE"	/* ADDLINE */
#define TOK_ADDLINE     "z"	/* 122 */
#define MSG_GLINE	"GLINE"	/* The awesome g-line */
#define TOK_GLINE	"}"	/* 125 */
#define MSG_GZLINE	"GZLINE" /* Teh awesome global z-line */
#define TOK_GZLINE	"{"	/* ahem? */
#define MSG_SJOIN	"SJOIN"
#define TOK_SJOIN	"~"
#define MSG_SETHOST 	"SETHOST"	/* sethost */
#define TOK_SETHOST 	"AA"	/* 127 4ever !;) */
#define MSG_NACHAT  	"NACHAT"	/* netadmin chat */
#define TOK_NACHAT  	"AC"	/* *beep* */
#define MSG_SETIDENT 	"SETIDENT"	/* set ident */
#define TOK_SETIDENT	"AD"	/* good old BASIC ;P */
#define MSG_SETNAME	"SETNAME"	/* set GECOS */
#define TOK_SETNAME	"AE"	/* its almost unreeaaall... */
#define MSG_LAG		"LAG"	/* Lag detect */
#define TOK_LAG		"AF"	/* a or ? */
#define MSG_SDESC       "SDESC"	/* set description */
#define TOK_SDESC       "AG"
#define MSG_STATSERV	"STATSERV"	/* alias */
#define TOK_STATSERV	"AH"
#define MSG_KNOCK	"KNOCK"
#define TOK_KNOCK	"AI"
#define MSG_CREDITS 	"CREDITS"
#define TOK_CREDITS 	"AJ"
#define MSG_LICENSE 	"LICENSE"
#define TOK_LICENSE 	"AK"
#define MSG_CHGHOST 	"CHGHOST"
#define TOK_CHGHOST 	"AL"
#define MSG_RPING   	"RPING"
#define TOK_RPING	"AM"
#define MSG_RPONG   	"RPONG"
#define TOK_RPONG	"AN"
#define MSG_NETINFO 	"NETINFO"
#define TOK_NETINFO 	"AO"
#define MSG_SENDUMODE 	"SENDUMODE"
#define TOK_SENDUMODE 	"AP"
#define MSG_ADDMOTD 	"ADDMOTD"
#define TOK_ADDMOTD	"AQ"
#define MSG_ADDOMOTD	"ADDOMOTD"
#define TOK_ADDOMOTD	"AR"
#define MSG_SVSMOTD	"SVSMOTD"
#define TOK_SVSMOTD	"AS"
#define MSG_SMO 	"SMO"
#define TOK_SMO 	"AU"
#define MSG_OPERMOTD 	"OPERMOTD"
#define TOK_OPERMOTD 	"AV"
#define MSG_TSCTL 	"TSCTL"
#define TOK_TSCTL 	"AW"
#define MSG_SVSJOIN 	"SVSJOIN"
#define TOK_SVSJOIN 	"AX"
#define MSG_SAJOIN 	"SAJOIN"
#define TOK_SAJOIN 	"AY"
#define MSG_SVSPART 	"SVSPART"
#define TOK_SVSPART 	"AX"
#define MSG_SAPART 	"SAPART"
#define TOK_SAPART 	"AY"
#define MSG_CHGIDENT 	"CHGIDENT"
#define TOK_CHGIDENT 	"AZ"
#define MSG_SWHOIS 	"SWHOIS"
#define TOK_SWHOIS 	"BA"
#define MSG_SVSO 	"SVSO"
#define TOK_SVSO 	"BB"
#define MSG_SVSFLINE 	"SVSFLINE"
#define TOK_SVSFLINE 	"BC"
#define MSG_TKL		"TKL"
#define TOK_TKL 	"BD"
#define MSG_VHOST 	"VHOST"
#define TOK_VHOST 	"BE"
#define MSG_BOTMOTD 	"BOTMOTD"
#define TOK_BOTMOTD 	"BF"
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define TOK_REMGLINE	"BG"
#define MSG_REMGZLINE	"REMGZLINE"	/* remove global z-line */
#define TOK_REMGZLINE	"BP"
#define MSG_HTM		"HTM"
#define TOK_HTM		"BH"
#define MSG_UMODE2	"UMODE2"
#define TOK_UMODE2	"|"
#define MSG_DCCDENY	"DCCDENY"
#define TOK_DCCDENY	"BI"
#define MSG_UNDCCDENY   "UNDCCDENY"
#define TOK_UNDCCDENY   "BJ"
#define MSG_CHGNAME	"CHGNAME"
#define MSG_SVSNAME	"SVSNAME"
#define TOK_CHGNAME	"BK"
#define MSG_SHUN	"SHUN"
#define TOK_SHUN	"BL"
#define MSG_NEWJOIN 	"NEWJOIN"	/* For CR Java Chat */
#define MSG_POST	"POST"
#define TOK_POST	"BN"
#define MSG_INFOSERV 	"INFOSERV"
#define MSG_IS		"IS"
#define TOK_INFOSERV	"BO"

#define MSG_BOTSERV	"BOTSERV"
#define TOK_BOTSERV	"BS"

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
#define UMODE_INVISIBLE		0x00000001	/* makes user invisible */
#define UMODE_OPER			0x00000002	/* Operator */
#define UMODE_WALLOP		0x00000004	/* send wallops to them */
#define UMODE_FAILOP		0x00000008	/* Shows some global messages */
#define UMODE_HELPOP		0x00000010	/* Help system operator */
#define UMODE_REGNICK		0x00000020	/* Nick set by services as registered */
#define UMODE_SADMIN		0x00000040	/* Services Admin */
#define UMODE_ADMIN			0x00000080	/* Admin */
#define UMODE_SERVNOTICE	0x00000100	/* server notices such as kill */
#define UMODE_LOCOP			0x00000200	/* Local operator -- SRB */
#define UMODE_KILLS			0x00000400	/* Show server-kills... */
#define UMODE_CLIENT		0x00000800	/* Show client information */
#define UMODE_FLOOD			0x00001000	/* Receive flood warnings */
#define UMODE_JUNK			0x00002000	/* can junk */
#define UMODE_SERVICES		0x00004000	/* services */
#define UMODE_HIDE			0x00008000	/* Hide from Nukes */
#define UMODE_NETADMIN		0x00010000	/* Network Admin */
#define UMODE_EYES			0x00020000	/* Mode to see server stuff */
#define UMODE_COADMIN		0x00080000	/* Co Admin */
#define UMODE_WHOIS			0x00100000	/* gets notice on /whois */
#define UMODE_KIX			0x00200000	/* usermode +q 
										cannot be kicked from any channel 
										except by U:Lines */
#define UMODE_BOT			0x00400000	/* User is a bot */
#define UMODE_SECURE		0x00800000	/* User is a secure connect */
#define UMODE_FCLIENT		0x01000000	/* recieve client on far connects.. */

#define UMODE_VICTIM		0x08000000	/* Intentional Victim */
#define UMODE_DEAF			0x10000000
#define UMODE_HIDEOPER		0x20000000	/* Hide oper mode */
#define UMODE_SETHOST		0x40000000	/* used sethost */
#define UMODE_STRIPBADWORDS 0x80000000	/* */

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_ADMONLY|CMODE_OPERONLY)))
#define is_pub_chan(x)  ((x) && !(x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_ADMONLY|CMODE_OPERONLY|CMODE_INVITEONLY|CMODE_RGSTRONLY) || CheckChanMode(x, CMODE_KEY) ))
#define is_priv_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_ADMONLY|CMODE_OPERONLY|CMODE_INVITEONLY|CMODE_RGSTRONLY) || CheckChanMode(x, CMODE_KEY) ))

/* Umode macros */
#define is_oper(x) ((x) && ((x->Umode & UMODE_OPER) || (x->Umode & UMODE_LOCOP)))
#define is_bot(x) ((x) && (x->Umode & UMODE_BOT))

#endif /* UNREAL_H Define */
