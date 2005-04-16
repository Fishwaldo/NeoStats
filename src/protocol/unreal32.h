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
const char MSG_PRIVATE[] = "PRIVMSG";/* PRIV */
const char TOK_PRIVATE[] = "!";/* 33 */
const char MSG_WHOIS[] = "WHOIS";/* WHOI */
const char TOK_WHOIS[] = "#";/* 35 */
const char MSG_WHOWAS[] = "WHOWAS";/* WHOW */
const char TOK_WHOWAS[] = "$";/* 36 */
const char MSG_USER[] = "USER";/* USER */
const char TOK_USER[] = "%";/* 37 */
const char MSG_NICK[] = "NICK";/* NICK */
const char TOK_NICK[] = "&";/* 38 */
const char MSG_SERVER[] = "SERVER";/* SERV */
const char TOK_SERVER[] = "'";/* 39 */
const char MSG_LIST[] = "LIST";/* LIST */
const char TOK_LIST[] = "(";/* 40 */
const char MSG_TOPIC[] = "TOPIC";/* TOPI */
const char TOK_TOPIC[] = ")";/* 41 */
const char MSG_INVITE[] = "INVITE";/* INVI */
const char TOK_INVITE[] = "*";/* 42 */
const char MSG_VERSION[] = "VERSION";/* VERS */
const char TOK_VERSION[] = "+";/* 43 */
const char MSG_QUIT[] = "QUIT";/* QUIT */
const char TOK_QUIT[] = ",";/* 44 */
const char MSG_SQUIT[] = "SQUIT";/* SQUI */
const char TOK_SQUIT[] = "-";/* 45 */
const char MSG_KILL[] = "KILL";/* KILL */
const char TOK_KILL[] = ".";/* 46 */
const char MSG_INFO[] = "INFO";/* INFO */
const char TOK_INFO[] = "/";/* 47 */
const char MSG_LINKS[] = "LINKS";/* LINK */
const char TOK_LINKS[] = "0";/* 48 */
const char MSG_SUMMON[] = "SUMMON";/* SUMM */
const char TOK_SUMMON[] = "1";/* 49 */
const char MSG_STATS[] = "STATS";/* STAT */
const char TOK_STATS[] = "2";/* 50 */
const char MSG_USERS[] = "USERS";/* USER -> USRS */
const char TOK_USERS[] = "3";/* 51 */
const char MSG_HELP[] = "HELP";/* HELP */
const char MSG_HELPOP[] = "HELPOP";/* HELP */
const char TOK_HELP[] = "4";/* 52 */
const char MSG_ERROR[] = "ERROR";/* ERRO */
const char TOK_ERROR[] = "5";/* 53 */
const char MSG_AWAY[] = "AWAY";/* AWAY */
const char TOK_AWAY[] = "6";/* 54 */
const char MSG_CONNECT[] = "CONNECT";/* CONN */
const char TOK_CONNECT[] = "7";/* 55 */
const char MSG_PING[] = "PING";/* PING */
const char TOK_PING[] = "8";/* 56 */
const char MSG_PONG[] = "PONG";/* PONG */
const char TOK_PONG[] = "9";/* 57 */
const char MSG_OPER[] = "OPER";/* OPER */
const char TOK_OPER[] = ";";/* 59 */
const char MSG_PASS[] = "PASS";/* PASS */
const char TOK_PASS[] = "<";/* 60 */
const char MSG_WALLOPS[] = "WALLOPS";/* WALL */
const char TOK_WALLOPS[] = "=";/* 61 */
const char MSG_TIME[] = "TIME";/* TIME */
const char TOK_TIME[] = ">";/* 62 */
const char MSG_NAMES[] = "NAMES";/* NAME */
const char TOK_NAMES[] = "?";/* 63 */
const char MSG_ADMIN[] = "ADMIN";/* ADMI */
const char TOK_ADMIN[] = "@";/* 64 */
const char MSG_NOTICE[] = "NOTICE";/* NOTI */
const char TOK_NOTICE[] = "B";/* 66 */
const char MSG_JOIN[] = "JOIN";/* JOIN */
const char TOK_JOIN[] = "C";/* 67 */
const char MSG_PART[] = "PART";/* PART */
const char TOK_PART[] = "D";/* 68 */
const char MSG_LUSERS[] = "LUSERS";/* LUSE */
const char TOK_LUSERS[] = "E";/* 69 */
const char MSG_MOTD[] = "MOTD";/* MOTD */
const char TOK_MOTD[] = "F";/* 70 */
const char MSG_MODE[] = "MODE";/* MODE */
const char TOK_MODE[] = "G";/* 71 */
const char MSG_KICK[] = "KICK";/* KICK */
const char TOK_KICK[] = "H";/* 72 */
const char MSG_SERVICE[] = "SERVICE";/* SERV -> SRVI */
const char TOK_SERVICE[] = "I";/* 73 */
const char MSG_USERHOST[] = "USERHOST";/* USER -> USRH */
const char TOK_USERHOST[] = "J";/* 74 */
const char MSG_ISON[] = "ISON";/* ISON */
const char TOK_ISON[] = "K";/* 75 */
const char MSG_REHASH[] = "REHASH";/* REHA */
const char TOK_REHASH[] = "O";/* 79 */
const char MSG_RESTART[] = "RESTART";/* REST */
const char TOK_RESTART[] = "P";/* 80 */
const char MSG_CLOSE[] = "CLOSE";/* CLOS */
const char TOK_CLOSE[] = "Q";/* 81 */
const char MSG_DIE[] = "DIE";/* DIE */
const char TOK_DIE[] = "R";/* 82 */
const char MSG_HASH[] = "HASH";/* HASH */
const char TOK_HASH[] = "S";/* 83 */
const char MSG_DNS[] = "DNS";/* DNS -> DNSS */
const char TOK_DNS[] = "T";/* 84 */
const char MSG_SILENCE[] = "SILENCE";/* SILE */
const char TOK_SILENCE[] = "U";/* 85 */
const char MSG_AKILL[] = "AKILL";/* AKILL */
const char TOK_AKILL[] = "V";/* 86 */
const char MSG_KLINE[] = "KLINE";/* KLINE */
const char TOK_KLINE[] = "W";/* 87 */
const char MSG_UNKLINE[] = "UNKLINE";/* UNKLINE */
const char TOK_UNKLINE[] = "X";/* 88 */
const char MSG_RAKILL[] = "RAKILL";/* RAKILL */
const char TOK_RAKILL[] = "Y";/* 89 */
const char MSG_GNOTICE[] = "GNOTICE";/* GNOTICE */
const char TOK_GNOTICE[] = "Z";/* 90 */
const char MSG_GOPER[] = "GOPER";/* GOPER */
const char TOK_GOPER[] = "[";/* 91 */
const char MSG_GLOBOPS[] = "GLOBOPS";/* GLOBOPS */
const char TOK_GLOBOPS[] = "]";/* 93 */
const char MSG_LOCOPS[] = "LOCOPS";/* LOCOPS */
const char TOK_LOCOPS[] = "^";/* 94 */
const char MSG_PROTOCTL[] = "PROTOCTL";/* PROTOCTL */
const char TOK_PROTOCTL[] = "_";/* 95 */
const char MSG_WATCH[] = "WATCH";/* WATCH */
const char TOK_WATCH[] = "`";/* 96 */
const char MSG_TRACE[] = "TRACE";/* TRAC */
const char TOK_TRACE[] = "b";/* 97 */
const char MSG_SQLINE[] = "SQLINE";/* SQLINE */
const char TOK_SQLINE[] = "c";/* 98 */
const char MSG_UNSQLINE[] = "UNSQLINE";/* UNSQLINE */
const char TOK_UNSQLINE[] = "d";/* 99 */
const char MSG_SVSNICK[] = "SVSNICK";/* SVSNICK */
const char TOK_SVSNICK[] = "e";/* 100 */
const char MSG_SVSNOOP[] = "SVSNOOP";/* SVSNOOP */
const char TOK_SVSNOOP[] = "f";/* 101 */
const char MSG_IDENTIFY[] = "IDENTIFY";/* IDENTIFY */
const char TOK_IDENTIFY[] = "g";/* 102 */
const char MSG_SVSKILL[] = "SVSKILL";/* SVSKILL */
const char TOK_SVSKILL[] = "h";/* 103 */
const char MSG_NICKSERV[] = "NICKSERV";/* NICKSERV */
const char MSG_NS[] = "NS";
const char TOK_NICKSERV[] = "i";/* 104 */
const char MSG_CHANSERV[] = "CHANSERV";/* CHANSERV */
const char MSG_CS[] = "CS";
const char TOK_CHANSERV[] = "j";/* 105 */
const char MSG_OPERSERV[] = "OPERSERV";/* OPERSERV */
const char MSG_OS[] = "OS";
const char TOK_OPERSERV[] = "k";/* 106 */
const char MSG_MEMOSERV[] = "MEMOSERV";/* MEMOSERV */
const char MSG_MS[] = "MS";
const char TOK_MEMOSERV[] = "l";/* 107 */
const char MSG_SERVICES[] = "SERVICES";/* SERVICES */
const char TOK_SERVICES[] = "m";/* 108 */
const char MSG_SVSMODE[] = "SVSMODE";/* SVSMODE */
const char TOK_SVSMODE[] = "n";/* 109 */
const char MSG_SAMODE[] = "SAMODE";/* SAMODE */
const char TOK_SAMODE[] = "o";/* 110 */
const char MSG_CHATOPS[] = "CHATOPS";/* CHATOPS */
const char TOK_CHATOPS[] = "p";/* 111 */
const char MSG_ZLINE[] = "ZLINE";/* ZLINE */
const char TOK_ZLINE[] = "q";/* 112 */
const char MSG_UNZLINE[] = "UNZLINE";/* UNZLINE */
const char TOK_UNZLINE[] = "r";/* 113 */
const char MSG_HELPSERV[] = "HELPSERV";/* HELPSERV */
const char MSG_HS[] = "HS";
const char TOK_HELPSERV[] = "s";/* 114 */
const char MSG_RULES[] = "RULES";/* RULES */
const char TOK_RULES[] = "t";/* 115 */
const char MSG_MAP[] = "MAP";/* MAP */
const char TOK_MAP[] = "u";/* 117 */
const char MSG_SVS2MODE[] = "SVS2MODE";/* SVS2MODE */
const char TOK_SVS2MODE[] = "v";/* 118 */
const char MSG_DALINFO[] = "DALINFO";/* dalinfo */
const char TOK_DALINFO[] = "w";/* 119 */
const char MSG_ADMINCHAT[] = "ADCHAT";/* Admin chat */
const char TOK_ADMINCHAT[] = "x";/* 120 */
const char MSG_MKPASSWD[] = "MKPASSWD";/* MKPASSWD */
const char TOK_MKPASSWD[] = "y";/* 121 */
const char MSG_ADDLINE[] = "ADDLINE";/* ADDLINE */
const char TOK_ADDLINE[] = "z";/* 122 */
const char MSG_GLINE[] = "GLINE";/* The awesome g-line */
const char TOK_GLINE[] = "}";/* 125 */
const char MSG_SJOIN[] = "SJOIN";
const char TOK_SJOIN[] = "~";
const char MSG_SETHOST[] = "SETHOST";/* sethost */
const char TOK_SETHOST[] = "AA";/* 127 4ever !;) */
const char MSG_NACHAT[] = "NACHAT";/* netadmin chat */
const char TOK_NACHAT[] = "AC";/* *beep* */
const char MSG_SETIDENT[] = "SETIDENT";
const char TOK_SETIDENT[] = "AD";
const char MSG_SETNAME[] = "SETNAME";/* set GECOS */
const char TOK_SETNAME[] = "AE";/* its almost unreeaaall... */
const char MSG_LAG[] = "LAG";/* Lag detect */
const char TOK_LAG[] = "AF";/* a or ? */
const char MSG_STATSERV[] = "STATSERV";/* alias */
const char TOK_STATSERV[] = "AH";
const char MSG_KNOCK[] = "KNOCK";
const char TOK_KNOCK[] = "AI";
const char MSG_CREDITS[] = "CREDITS";
const char TOK_CREDITS[] = "AJ";
const char MSG_LICENSE[] = "LICENSE";
const char TOK_LICENSE[] = "AK";
const char MSG_CHGHOST[] = "CHGHOST";
const char TOK_CHGHOST[] = "AL";
const char MSG_RPING[] = "RPING";
const char TOK_RPING[] = "AM";
const char MSG_RPONG[] = "RPONG";
const char TOK_RPONG[] = "AN";
const char MSG_NETINFO[] = "NETINFO";
const char TOK_NETINFO[] = "AO";
const char MSG_SENDUMODE[] = "SENDUMODE";
const char TOK_SENDUMODE[] = "AP";
const char MSG_ADDMOTD[] = "ADDMOTD";
const char TOK_ADDMOTD[] = "AQ";
const char MSG_ADDOMOTD[] = "ADDOMOTD";
const char TOK_ADDOMOTD[] = "AR";
const char MSG_SVSMOTD[] = "SVSMOTD";
const char TOK_SVSMOTD[] = "AS";
const char MSG_SMO[] = "SMO";
const char TOK_SMO[] = "AU";
const char MSG_OPERMOTD[] = "OPERMOTD";
const char TOK_OPERMOTD[] = "AV";
const char MSG_TSCTL[] = "TSCTL";
const char TOK_TSCTL[] = "AW";
const char MSG_SVSJOIN[] = "SVSJOIN";
const char TOK_SVSJOIN[] = "BR";
const char MSG_SAJOIN[] = "SAJOIN";
const char TOK_SAJOIN[] = "AX";
const char MSG_SVSPART[] = "SVSPART";
const char TOK_SVSPART[] = "BT";
const char MSG_SAPART[] = "SAPART";
const char TOK_SAPART[] = "AY";
const char MSG_CHGIDENT[] = "CHGIDENT";
const char TOK_CHGIDENT[] = "AZ";
const char MSG_SWHOIS[] = "SWHOIS";
const char TOK_SWHOIS[] = "BA";
const char MSG_SVSO[] = "SVSO";
const char TOK_SVSO[] = "BB";
const char MSG_SVSFLINE[] = "SVSFLINE";
const char TOK_SVSFLINE[] = "BC";
const char MSG_TKL[] = "TKL";
const char TOK_TKL[] = "BD";
const char MSG_VHOST[] = "VHOST";
const char TOK_VHOST[] = "BE";
const char MSG_BOTMOTD[] = "BOTMOTD";
const char TOK_BOTMOTD[] = "BF";
const char MSG_REMGLINE[] = "REMGLINE";/* remove g-line */
const char TOK_REMGLINE[] = "BG";
const char MSG_HTM[] = "HTM";
const char TOK_HTM[] = "BH";
const char MSG_UMODE2[] = "UMODE2";
const char TOK_UMODE2[] = "|";
const char MSG_DCCDENY[] = "DCCDENY";
const char TOK_DCCDENY[] = "BI";
const char MSG_UNDCCDENY[] = "UNDCCDENY";
const char TOK_UNDCCDENY[] = "BJ";
const char MSG_CHGNAME[] = "CHGNAME";
const char MSG_SVSNAME[] = "SVSNAME";
const char TOK_CHGNAME[] = "BK";
const char MSG_SHUN[] = "SHUN";
const char TOK_SHUN[] = "BL";
const char MSG_NEWJOIN[] = "NEWJOIN";/* For CR Java Chat */
const char MSG_POST[] = "POST";
const char TOK_POST[] = "BN";
const char MSG_INFOSERV[] = "INFOSERV";
const char MSG_IS[] = "IS";
const char TOK_INFOSERV[] = "BO";

const char MSG_BOTSERV[] = "BOTSERV";
const char TOK_BOTSERV[] = "BS";

const char MSG_CYCLE[] = "CYCLE";
const char TOK_CYCLE[] = "BP";

const char MSG_MODULE[] = "MODULE";
const char TOK_MODULE[] = "BQ";
/* BR and BT are in use */

const char MSG_SENDSNO[] = "SENDSNO";
const char TOK_SENDSNO[] = "Ss";

const char MSG_EOS[] = "EOS";
const char TOK_EOS[] = "ES";

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
