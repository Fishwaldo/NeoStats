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
*/

#ifndef UNREAL_H
#define UNREAL_H

/* Messages/Tokens */
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";/* PRIV */
MODULEVAR const char TOK_PRIVATE[] = "!";/* 33 */
MODULEVAR const char MSG_WHOIS[] = "WHOIS";/* WHOI */
MODULEVAR const char TOK_WHOIS[] = "#";/* 35 */
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";/* WHOW */
MODULEVAR const char TOK_WHOWAS[] = "$";/* 36 */
MODULEVAR const char MSG_USER[] = "USER";/* USER */
MODULEVAR const char TOK_USER[] = "%";/* 37 */
MODULEVAR const char MSG_NICK[] = "NICK";/* NICK */
MODULEVAR const char TOK_NICK[] = "&";/* 38 */
MODULEVAR const char MSG_SERVER[] = "SERVER";/* SERV */
MODULEVAR const char TOK_SERVER[] = "'";/* 39 */
MODULEVAR const char MSG_LIST[] = "LIST";/* LIST */
MODULEVAR const char TOK_LIST[] = "(";/* 40 */
MODULEVAR const char MSG_TOPIC[] = "TOPIC";/* TOPI */
MODULEVAR const char TOK_TOPIC[] = ")";/* 41 */
MODULEVAR const char MSG_INVITE[] = "INVITE";/* INVI */
MODULEVAR const char TOK_INVITE[] = "*";/* 42 */
MODULEVAR const char MSG_VERSION[] = "VERSION";/* VERS */
MODULEVAR const char TOK_VERSION[] = "+";/* 43 */
MODULEVAR const char MSG_QUIT[] = "QUIT";/* QUIT */
MODULEVAR const char TOK_QUIT[] = ",";/* 44 */
MODULEVAR const char MSG_SQUIT[] = "SQUIT";/* SQUI */
MODULEVAR const char TOK_SQUIT[] = "-";/* 45 */
MODULEVAR const char MSG_KILL[] = "KILL";/* KILL */
MODULEVAR const char TOK_KILL[] = ".";/* 46 */
MODULEVAR const char MSG_INFO[] = "INFO";/* INFO */
MODULEVAR const char TOK_INFO[] = "/";/* 47 */
MODULEVAR const char MSG_LINKS[] = "LINKS";/* LINK */
MODULEVAR const char TOK_LINKS[] = "0";/* 48 */
MODULEVAR const char MSG_SUMMON[] = "SUMMON";/* SUMM */
MODULEVAR const char TOK_SUMMON[] = "1";/* 49 */
MODULEVAR const char MSG_STATS[] = "STATS";/* STAT */
MODULEVAR const char TOK_STATS[] = "2";/* 50 */
MODULEVAR const char MSG_USERS[] = "USERS";/* USER -> USRS */
MODULEVAR const char TOK_USERS[] = "3";/* 51 */
MODULEVAR const char MSG_HELP[] = "HELP";/* HELP */
MODULEVAR const char MSG_HELPOP[] = "HELPOP";/* HELP */
MODULEVAR const char TOK_HELP[] = "4";/* 52 */
MODULEVAR const char MSG_ERROR[] = "ERROR";/* ERRO */
MODULEVAR const char TOK_ERROR[] = "5";/* 53 */
MODULEVAR const char MSG_AWAY[] = "AWAY";/* AWAY */
MODULEVAR const char TOK_AWAY[] = "6";/* 54 */
MODULEVAR const char MSG_CONNECT[] = "CONNECT";/* CONN */
MODULEVAR const char TOK_CONNECT[] = "7";/* 55 */
MODULEVAR const char MSG_PING[] = "PING";/* PING */
MODULEVAR const char TOK_PING[] = "8";/* 56 */
MODULEVAR const char MSG_PONG[] = "PONG";/* PONG */
MODULEVAR const char TOK_PONG[] = "9";/* 57 */
MODULEVAR const char MSG_OPER[] = "OPER";/* OPER */
MODULEVAR const char TOK_OPER[] = ";";/* 59 */
MODULEVAR const char MSG_PASS[] = "PASS";/* PASS */
MODULEVAR const char TOK_PASS[] = "<";/* 60 */
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";/* WALL */
MODULEVAR const char TOK_WALLOPS[] = "=";/* 61 */
MODULEVAR const char MSG_TIME[] = "TIME";/* TIME */
MODULEVAR const char TOK_TIME[] = ">";/* 62 */
MODULEVAR const char MSG_NAMES[] = "NAMES";/* NAME */
MODULEVAR const char TOK_NAMES[] = "?";/* 63 */
MODULEVAR const char MSG_ADMIN[] = "ADMIN";/* ADMI */
MODULEVAR const char TOK_ADMIN[] = "@";/* 64 */
MODULEVAR const char MSG_NOTICE[] = "NOTICE";/* NOTI */
MODULEVAR const char TOK_NOTICE[] = "B";/* 66 */
MODULEVAR const char MSG_JOIN[] = "JOIN";/* JOIN */
MODULEVAR const char TOK_JOIN[] = "C";/* 67 */
MODULEVAR const char MSG_PART[] = "PART";/* PART */
MODULEVAR const char TOK_PART[] = "D";/* 68 */
MODULEVAR const char MSG_LUSERS[] = "LUSERS";/* LUSE */
MODULEVAR const char TOK_LUSERS[] = "E";/* 69 */
MODULEVAR const char MSG_MOTD[] = "MOTD";/* MOTD */
MODULEVAR const char TOK_MOTD[] = "F";/* 70 */
MODULEVAR const char MSG_MODE[] = "MODE";/* MODE */
MODULEVAR const char TOK_MODE[] = "G";/* 71 */
MODULEVAR const char MSG_KICK[] = "KICK";/* KICK */
MODULEVAR const char TOK_KICK[] = "H";/* 72 */
MODULEVAR const char MSG_SERVICE[] = "SERVICE";/* SERV -> SRVI */
MODULEVAR const char TOK_SERVICE[] = "I";/* 73 */
MODULEVAR const char MSG_USERHOST[] = "USERHOST";/* USER -> USRH */
MODULEVAR const char TOK_USERHOST[] = "J";/* 74 */
MODULEVAR const char MSG_ISON[] = "ISON";/* ISON */
MODULEVAR const char TOK_ISON[] = "K";/* 75 */
MODULEVAR const char MSG_REHASH[] = "REHASH";/* REHA */
MODULEVAR const char TOK_REHASH[] = "O";/* 79 */
MODULEVAR const char MSG_RESTART[] = "RESTART";/* REST */
MODULEVAR const char TOK_RESTART[] = "P";/* 80 */
MODULEVAR const char MSG_CLOSE[] = "CLOSE";/* CLOS */
MODULEVAR const char TOK_CLOSE[] = "Q";/* 81 */
MODULEVAR const char MSG_DIE[] = "DIE";/* DIE */
MODULEVAR const char TOK_DIE[] = "R";/* 82 */
MODULEVAR const char MSG_HASH[] = "HASH";/* HASH */
MODULEVAR const char TOK_HASH[] = "S";/* 83 */
MODULEVAR const char MSG_DNS[] = "DNS";/* DNS -> DNSS */
MODULEVAR const char TOK_DNS[] = "T";/* 84 */
MODULEVAR const char MSG_SILENCE[] = "SILENCE";/* SILE */
MODULEVAR const char TOK_SILENCE[] = "U";/* 85 */
MODULEVAR const char MSG_AKILL[] = "AKILL";/* AKILL */
MODULEVAR const char TOK_AKILL[] = "V";/* 86 */
MODULEVAR const char MSG_KLINE[] = "KLINE";/* KLINE */
MODULEVAR const char TOK_KLINE[] = "W";/* 87 */
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";/* UNKLINE */
MODULEVAR const char TOK_UNKLINE[] = "X";/* 88 */
MODULEVAR const char MSG_RAKILL[] = "RAKILL";/* RAKILL */
MODULEVAR const char TOK_RAKILL[] = "Y";/* 89 */
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";/* GNOTICE */
MODULEVAR const char TOK_GNOTICE[] = "Z";/* 90 */
MODULEVAR const char MSG_GOPER[] = "GOPER";/* GOPER */
MODULEVAR const char TOK_GOPER[] = "[";/* 91 */
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";/* GLOBOPS */
MODULEVAR const char TOK_GLOBOPS[] = "]";/* 93 */
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";/* LOCOPS */
MODULEVAR const char TOK_LOCOPS[] = "^";/* 94 */
MODULEVAR const char MSG_PROTOCTL[] = "PROTOCTL";/* PROTOCTL */
MODULEVAR const char TOK_PROTOCTL[] = "_";/* 95 */
MODULEVAR const char MSG_WATCH[] = "WATCH";/* WATCH */
MODULEVAR const char TOK_WATCH[] = "`";/* 96 */
MODULEVAR const char MSG_TRACE[] = "TRACE";/* TRAC */
MODULEVAR const char TOK_TRACE[] = "b";/* 97 */
MODULEVAR const char MSG_SQLINE[] = "SQLINE";/* SQLINE */
MODULEVAR const char TOK_SQLINE[] = "c";/* 98 */
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";/* UNSQLINE */
MODULEVAR const char TOK_UNSQLINE[] = "d";/* 99 */
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";/* SVSNICK */
MODULEVAR const char TOK_SVSNICK[] = "e";/* 100 */
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";/* SVSNOOP */
MODULEVAR const char TOK_SVSNOOP[] = "f";/* 101 */
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";/* IDENTIFY */
MODULEVAR const char TOK_IDENTIFY[] = "g";/* 102 */
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";/* SVSKILL */
MODULEVAR const char TOK_SVSKILL[] = "h";/* 103 */
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";/* NICKSERV */
MODULEVAR const char MSG_NS[] = "NS";
MODULEVAR const char TOK_NICKSERV[] = "i";/* 104 */
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";/* CHANSERV */
MODULEVAR const char MSG_CS[] = "CS";
MODULEVAR const char TOK_CHANSERV[] = "j";/* 105 */
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";/* OPERSERV */
MODULEVAR const char MSG_OS[] = "OS";
MODULEVAR const char TOK_OPERSERV[] = "k";/* 106 */
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";/* MEMOSERV */
MODULEVAR const char MSG_MS[] = "MS";
MODULEVAR const char TOK_MEMOSERV[] = "l";/* 107 */
MODULEVAR const char MSG_SERVICES[] = "SERVICES";/* SERVICES */
MODULEVAR const char TOK_SERVICES[] = "m";/* 108 */
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";/* SVSMODE */
MODULEVAR const char TOK_SVSMODE[] = "n";/* 109 */
MODULEVAR const char MSG_SAMODE[] = "SAMODE";/* SAMODE */
MODULEVAR const char TOK_SAMODE[] = "o";/* 110 */
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";/* CHATOPS */
MODULEVAR const char TOK_CHATOPS[] = "p";/* 111 */
MODULEVAR const char MSG_ZLINE[] = "ZLINE";/* ZLINE */
MODULEVAR const char TOK_ZLINE[] = "q";/* 112 */
MODULEVAR const char MSG_UNZLINE[] = "UNZLINE";/* UNZLINE */
MODULEVAR const char TOK_UNZLINE[] = "r";/* 113 */
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV";/* HELPSERV */
MODULEVAR const char MSG_HS[] = "HS";
MODULEVAR const char TOK_HELPSERV[] = "s";/* 114 */
MODULEVAR const char MSG_RULES[] = "RULES";/* RULES */
MODULEVAR const char TOK_RULES[] = "t";/* 115 */
MODULEVAR const char MSG_MAP[] = "MAP";/* MAP */
MODULEVAR const char TOK_MAP[] = "u";/* 117 */
MODULEVAR const char MSG_SVS2MODE[] = "SVS2MODE";/* SVS2MODE */
MODULEVAR const char TOK_SVS2MODE[] = "v";/* 118 */
MODULEVAR const char MSG_DALINFO[] = "DALINFO";/* dalinfo */
MODULEVAR const char TOK_DALINFO[] = "w";/* 119 */
MODULEVAR const char MSG_ADMINCHAT[] = "ADCHAT";/* Admin chat */
MODULEVAR const char TOK_ADMINCHAT[] = "x";/* 120 */
MODULEVAR const char MSG_MKPASSWD[] = "MKPASSWD";/* MKPASSWD */
MODULEVAR const char TOK_MKPASSWD[] = "y";/* 121 */
MODULEVAR const char MSG_ADDLINE[] = "ADDLINE";/* ADDLINE */
MODULEVAR const char TOK_ADDLINE[] = "z";/* 122 */
MODULEVAR const char MSG_GLINE[] = "GLINE";/* The awesome g-line */
MODULEVAR const char TOK_GLINE[] = "}";/* 125 */
MODULEVAR const char MSG_SJOIN[] = "SJOIN";
MODULEVAR const char TOK_SJOIN[] = "~";
MODULEVAR const char MSG_SETHOST[] = "SETHOST";/* sethost */
MODULEVAR const char TOK_SETHOST[] = "AA";/* 127 4ever !;) */
MODULEVAR const char MSG_NACHAT[] = "NACHAT";/* netadmin chat */
MODULEVAR const char TOK_NACHAT[] = "AC";/* *beep* */
MODULEVAR const char MSG_SETIDENT[] = "SETIDENT";
MODULEVAR const char TOK_SETIDENT[] = "AD";
MODULEVAR const char MSG_SETNAME[] = "SETNAME";/* set GECOS */
MODULEVAR const char TOK_SETNAME[] = "AE";/* its almost unreeaaall... */
MODULEVAR const char MSG_LAG[] = "LAG";/* Lag detect */
MODULEVAR const char TOK_LAG[] = "AF";/* a or ? */
MODULEVAR const char MSG_STATSERV[] = "STATSERV";/* alias */
MODULEVAR const char TOK_STATSERV[] = "AH";
MODULEVAR const char MSG_KNOCK[] = "KNOCK";
MODULEVAR const char TOK_KNOCK[] = "AI";
MODULEVAR const char MSG_CREDITS[] = "CREDITS";
MODULEVAR const char TOK_CREDITS[] = "AJ";
MODULEVAR const char MSG_LICENSE[] = "LICENSE";
MODULEVAR const char TOK_LICENSE[] = "AK";
MODULEVAR const char MSG_CHGHOST[] = "CHGHOST";
MODULEVAR const char TOK_CHGHOST[] = "AL";
MODULEVAR const char MSG_RPING[] = "RPING";
MODULEVAR const char TOK_RPING[] = "AM";
MODULEVAR const char MSG_RPONG[] = "RPONG";
MODULEVAR const char TOK_RPONG[] = "AN";
MODULEVAR const char MSG_NETINFO[] = "NETINFO";
MODULEVAR const char TOK_NETINFO[] = "AO";
MODULEVAR const char MSG_SENDUMODE[] = "SENDUMODE";
MODULEVAR const char TOK_SENDUMODE[] = "AP";
MODULEVAR const char MSG_ADDMOTD[] = "ADDMOTD";
MODULEVAR const char TOK_ADDMOTD[] = "AQ";
MODULEVAR const char MSG_ADDOMOTD[] = "ADDOMOTD";
MODULEVAR const char TOK_ADDOMOTD[] = "AR";
MODULEVAR const char MSG_SVSMOTD[] = "SVSMOTD";
MODULEVAR const char TOK_SVSMOTD[] = "AS";
MODULEVAR const char MSG_SMO[] = "SMO";
MODULEVAR const char TOK_SMO[] = "AU";
MODULEVAR const char MSG_OPERMOTD[] = "OPERMOTD";
MODULEVAR const char TOK_OPERMOTD[] = "AV";
MODULEVAR const char MSG_TSCTL[] = "TSCTL";
MODULEVAR const char TOK_TSCTL[] = "AW";
MODULEVAR const char MSG_SVSJOIN[] = "SVSJOIN";
MODULEVAR const char TOK_SVSJOIN[] = "BR";
MODULEVAR const char MSG_SAJOIN[] = "SAJOIN";
MODULEVAR const char TOK_SAJOIN[] = "AX";
MODULEVAR const char MSG_SVSPART[] = "SVSPART";
MODULEVAR const char TOK_SVSPART[] = "BT";
MODULEVAR const char MSG_SAPART[] = "SAPART";
MODULEVAR const char TOK_SAPART[] = "AY";
MODULEVAR const char MSG_CHGIDENT[] = "CHGIDENT";
MODULEVAR const char TOK_CHGIDENT[] = "AZ";
MODULEVAR const char MSG_SWHOIS[] = "SWHOIS";
MODULEVAR const char TOK_SWHOIS[] = "BA";
MODULEVAR const char MSG_SVSO[] = "SVSO";
MODULEVAR const char TOK_SVSO[] = "BB";
MODULEVAR const char MSG_SVSFLINE[] = "SVSFLINE";
MODULEVAR const char TOK_SVSFLINE[] = "BC";
MODULEVAR const char MSG_TKL[] = "TKL";
MODULEVAR const char TOK_TKL[] = "BD";
MODULEVAR const char MSG_VHOST[] = "VHOST";
MODULEVAR const char TOK_VHOST[] = "BE";
MODULEVAR const char MSG_BOTMOTD[] = "BOTMOTD";
MODULEVAR const char TOK_BOTMOTD[] = "BF";
MODULEVAR const char MSG_REMGLINE[] = "REMGLINE";/* remove g-line */
MODULEVAR const char TOK_REMGLINE[] = "BG";
MODULEVAR const char MSG_HTM[] = "HTM";
MODULEVAR const char TOK_HTM[] = "BH";
MODULEVAR const char MSG_UMODE2[] = "UMODE2";
MODULEVAR const char TOK_UMODE2[] = "|";
MODULEVAR const char MSG_DCCDENY[] = "DCCDENY";
MODULEVAR const char TOK_DCCDENY[] = "BI";
MODULEVAR const char MSG_UNDCCDENY[] = "UNDCCDENY";
MODULEVAR const char TOK_UNDCCDENY[] = "BJ";
MODULEVAR const char MSG_CHGNAME[] = "CHGNAME";
MODULEVAR const char MSG_SVSNAME[] = "SVSNAME";
MODULEVAR const char TOK_CHGNAME[] = "BK";
MODULEVAR const char MSG_SHUN[] = "SHUN";
MODULEVAR const char TOK_SHUN[] = "BL";
MODULEVAR const char MSG_NEWJOIN[] = "NEWJOIN";/* For CR Java Chat */
MODULEVAR const char MSG_POST[] = "POST";
MODULEVAR const char TOK_POST[] = "BN";
MODULEVAR const char MSG_INFOSERV[] = "INFOSERV";
MODULEVAR const char MSG_IS[] = "IS";
MODULEVAR const char TOK_INFOSERV[] = "BO";

MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";
MODULEVAR const char TOK_BOTSERV[] = "BS";

MODULEVAR const char MSG_CYCLE[] = "CYCLE";
MODULEVAR const char TOK_CYCLE[] = "BP";

MODULEVAR const char MSG_MODULE[] = "MODULE";
MODULEVAR const char TOK_MODULE[] = "BQ";
/* BR and BT are in use */

MODULEVAR const char MSG_SENDSNO[] = "SENDSNO";
MODULEVAR const char TOK_SENDSNO[] = "Ss";

MODULEVAR const char MSG_EOS[] = "EOS";
MODULEVAR const char TOK_EOS[] = "ES";

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
