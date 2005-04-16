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
MODULEVAR const char MSG_PRIVATE[] = "PRIVMSG";
MODULEVAR const char TOK_PRIVATE[] = "!";
MODULEVAR const char MSG_WHO[] = "WHO";
MODULEVAR const char TOK_WHO[] = "\"";
MODULEVAR const char MSG_WHOIS[] = "WHOIS";
MODULEVAR const char TOK_WHOIS[] = "#";
MODULEVAR const char MSG_WHOWAS[] = "WHOWAS";
MODULEVAR const char TOK_WHOWAS[] = "$";
MODULEVAR const char MSG_USER[] = "USER";
MODULEVAR const char TOK_USER[] = "%";
MODULEVAR const char MSG_NICK[] = "NICK";
MODULEVAR const char TOK_NICK[] = "&";
MODULEVAR const char MSG_SERVER[] = "SERVER";
MODULEVAR const char TOK_SERVER[] = "'";
MODULEVAR const char MSG_LIST[] = "LIST";
MODULEVAR const char TOK_LIST[] = "(";
MODULEVAR const char MSG_TOPIC[] = "TOPIC";
MODULEVAR const char TOK_TOPIC[] = ")";
MODULEVAR const char MSG_INVITE[] = "INVITE";
MODULEVAR const char TOK_INVITE[] = "*";
MODULEVAR const char MSG_VERSION[] = "VERSION";
MODULEVAR const char TOK_VERSION[] = "+";
MODULEVAR const char MSG_QUIT[] = "QUIT";
MODULEVAR const char TOK_QUIT[] = ",";
MODULEVAR const char MSG_SQUIT[] = "SQUIT";
MODULEVAR const char TOK_SQUIT[] = "-";
MODULEVAR const char MSG_KILL[] = "KILL";
MODULEVAR const char TOK_KILL[] = ".";
MODULEVAR const char MSG_INFO[] = "INFO";
MODULEVAR const char TOK_INFO[] = "/";
MODULEVAR const char MSG_LINKS[] = "LINKS";
MODULEVAR const char TOK_LINKS[] = "0";
MODULEVAR const char MSG_SUMMON[] = "SUMMON";
MODULEVAR const char TOK_SUMMON[] = "1";
MODULEVAR const char MSG_STATS[] = "STATS";
MODULEVAR const char TOK_STATS[] = "2";
MODULEVAR const char MSG_USERS[] = "USERS";
MODULEVAR const char TOK_USERS[] = "3";
MODULEVAR const char MSG_HELP[] = "HELP";
MODULEVAR const char MSG_HELPOP[] = "HELPOP";
MODULEVAR const char TOK_HELP[] = "4";
MODULEVAR const char MSG_ERROR[] = "ERROR";
MODULEVAR const char TOK_ERROR[] = "5";
MODULEVAR const char MSG_AWAY[] = "AWAY";
MODULEVAR const char TOK_AWAY[] = "6";
MODULEVAR const char MSG_CONNECT[] = "CONNECT";
MODULEVAR const char TOK_CONNECT[] = "7";
MODULEVAR const char MSG_PING[] = "PING";
MODULEVAR const char TOK_PING[] = "8";
MODULEVAR const char MSG_PONG[] = "PONG";
MODULEVAR const char TOK_PONG[] = "9";
MODULEVAR const char MSG_OPER[] = "OPER";
MODULEVAR const char TOK_OPER[] = ";";
MODULEVAR const char MSG_PASS[] = "PASS";
MODULEVAR const char TOK_PASS[] = "<";
MODULEVAR const char MSG_WALLOPS[] = "WALLOPS";
MODULEVAR const char TOK_WALLOPS[] = "=";
MODULEVAR const char MSG_TIME[] = "TIME";
MODULEVAR const char TOK_TIME[] = ">";
MODULEVAR const char MSG_NAMES[] = "NAMES";
MODULEVAR const char TOK_NAMES[] = "?";
MODULEVAR const char MSG_ADMIN[] = "ADMIN";
MODULEVAR const char TOK_ADMIN[] = "@";
MODULEVAR const char MSG_NOTICE[] = "NOTICE";
MODULEVAR const char TOK_NOTICE[] = "B";
MODULEVAR const char MSG_JOIN[] = "JOIN";
MODULEVAR const char TOK_JOIN[] = "C";
MODULEVAR const char MSG_PART[] = "PART";
MODULEVAR const char TOK_PART[] = "D";
MODULEVAR const char MSG_LUSERS[] = "LUSERS";
MODULEVAR const char TOK_LUSERS[] = "E";
MODULEVAR const char MSG_MOTD[] = "MOTD";
MODULEVAR const char TOK_MOTD[] = "F";
MODULEVAR const char MSG_MODE[] = "MODE";
MODULEVAR const char TOK_MODE[] = "G";
MODULEVAR const char MSG_KICK[] = "KICK";
MODULEVAR const char TOK_KICK[] = "H";
MODULEVAR const char MSG_SERVICE[] = "SERVICE";
MODULEVAR const char TOK_SERVICE[] = "I";
MODULEVAR const char MSG_USERHOST[] = "USERHOST";
MODULEVAR const char TOK_USERHOST[] = "J";
MODULEVAR const char MSG_ISON[] = "ISON";
MODULEVAR const char TOK_ISON[] = "K";
MODULEVAR const char MSG_REHASH[] = "REHASH";
MODULEVAR const char TOK_REHASH[] = "O";
MODULEVAR const char MSG_RESTART[] = "RESTART";
MODULEVAR const char TOK_RESTART[] = "P";
MODULEVAR const char MSG_CLOSE[] = "CLOSE";
MODULEVAR const char TOK_CLOSE[] = "Q";
MODULEVAR const char MSG_DIE[] = "DIE";
MODULEVAR const char TOK_DIE[] = "R";
MODULEVAR const char MSG_HASH[] = "HASH";
MODULEVAR const char TOK_HASH[] = "S";
MODULEVAR const char MSG_DNS[] = "DNS";
MODULEVAR const char TOK_DNS[] = "T";
MODULEVAR const char MSG_SILENCE[] = "SILENCE";
MODULEVAR const char TOK_SILENCE[] = "U";
MODULEVAR const char MSG_AKILL[] = "AKILL";
MODULEVAR const char TOK_AKILL[] = "V";
MODULEVAR const char MSG_KLINE[] = "KLINE";
MODULEVAR const char TOK_KLINE[] = "W";
MODULEVAR const char MSG_UNKLINE[] = "UNKLINE";
MODULEVAR const char TOK_UNKLINE[] = "X";
MODULEVAR const char MSG_RAKILL[] = "RAKILL";
MODULEVAR const char TOK_RAKILL[] = "Y";
MODULEVAR const char MSG_GNOTICE[] = "GNOTICE";
MODULEVAR const char TOK_GNOTICE[] = "Z";
MODULEVAR const char MSG_GOPER[] = "GOPER";
MODULEVAR const char TOK_GOPER[] = "[";
MODULEVAR const char MSG_GLOBOPS[] = "GLOBOPS";
MODULEVAR const char TOK_GLOBOPS[] = "]";
MODULEVAR const char MSG_LOCOPS[] = "LOCOPS";
MODULEVAR const char TOK_LOCOPS[] = "^";
MODULEVAR const char MSG_PROTOCTL[] = "PROTOCTL";
MODULEVAR const char TOK_PROTOCTL[] = "_";
MODULEVAR const char MSG_WATCH[] = "WATCH";
MODULEVAR const char TOK_WATCH[] = "`";
MODULEVAR const char MSG_TRACE[] = "TRACE";
MODULEVAR const char TOK_TRACE[] = "b";
MODULEVAR const char MSG_SQLINE[] = "SQLINE";
MODULEVAR const char TOK_SQLINE[] = "c";
MODULEVAR const char MSG_UNSQLINE[] = "UNSQLINE";
MODULEVAR const char TOK_UNSQLINE[] = "d";
MODULEVAR const char MSG_SVSNICK[] = "SVSNICK";
MODULEVAR const char TOK_SVSNICK[] = "e";
MODULEVAR const char MSG_SVSNOOP[] = "SVSNOOP";
MODULEVAR const char TOK_SVSNOOP[] = "f";
MODULEVAR const char MSG_IDENTIFY[] = "IDENTIFY";
MODULEVAR const char TOK_IDENTIFY[] = "g";
MODULEVAR const char MSG_SVSKILL[] = "SVSKILL";
MODULEVAR const char TOK_SVSKILL[] = "h";
MODULEVAR const char MSG_NICKSERV[] = "NICKSERV";
MODULEVAR const char MSG_NS[] = "NS";
MODULEVAR const char TOK_NICKSERV[] = "i";
MODULEVAR const char MSG_CHANSERV[] = "CHANSERV";
MODULEVAR const char MSG_CS[] = "CS";
MODULEVAR const char TOK_CHANSERV[] = "j";
MODULEVAR const char MSG_OPERSERV[] = "OPERSERV";
MODULEVAR const char MSG_OS[] = "OS";
MODULEVAR const char TOK_OPERSERV[] = "k";
MODULEVAR const char MSG_MEMOSERV[] = "MEMOSERV";
MODULEVAR const char MSG_MS[] = "MS";
MODULEVAR const char TOK_MEMOSERV[] = "l";
MODULEVAR const char MSG_SERVICES[] = "SERVICES";
MODULEVAR const char TOK_SERVICES[] = "m";
MODULEVAR const char MSG_SVSMODE[] = "SVSMODE";
MODULEVAR const char TOK_SVSMODE[] = "n";
MODULEVAR const char MSG_SAMODE[] = "SAMODE";
MODULEVAR const char TOK_SAMODE[] = "o";
MODULEVAR const char MSG_CHATOPS[] = "CHATOPS";
MODULEVAR const char TOK_CHATOPS[] = "p";
MODULEVAR const char MSG_ZLINE[] = "ZLINE";
MODULEVAR const char TOK_ZLINE[] = "q";
MODULEVAR const char MSG_UNZLINE[] = "UNZLINE";
MODULEVAR const char TOK_UNZLINE[] = "r";
MODULEVAR const char MSG_HELPSERV[] = "HELPSERV";
MODULEVAR const char MSG_HS[] = "HS";
MODULEVAR const char TOK_HELPSERV[] = "s";
MODULEVAR const char MSG_RULES[] = "RULES";
MODULEVAR const char TOK_RULES[] = "t";
MODULEVAR const char MSG_MAP[] = "MAP";
MODULEVAR const char TOK_MAP[] = "u";
MODULEVAR const char MSG_SVS2MODE[] = "SVS2MODE";
MODULEVAR const char TOK_SVS2MODE[] = "v";
MODULEVAR const char MSG_DALINFO[] = "DALINFO";
MODULEVAR const char TOK_DALINFO[] = "w";
MODULEVAR const char MSG_ADMINCHAT[] = "ADCHAT";
MODULEVAR const char TOK_ADMINCHAT[] = "x";
MODULEVAR const char MSG_MKPASSWD[] = "MKPASSWD";
MODULEVAR const char TOK_MKPASSWD[] = "y";
MODULEVAR const char MSG_ADDLINE[] = "ADDLINE";
MODULEVAR const char TOK_ADDLINE[] = "z";
MODULEVAR const char MSG_GLINE[] = "GLINE";
MODULEVAR const char TOK_GLINE[] = "}";
MODULEVAR const char MSG_GZLINE[] = "GZLINE";
MODULEVAR const char TOK_GZLINE[] = "{";
MODULEVAR const char MSG_SJOIN[] = "SJOIN";
MODULEVAR const char TOK_SJOIN[] = "~";
MODULEVAR const char MSG_SETHOST[] = "SETHOST";
MODULEVAR const char TOK_SETHOST[] = "AA";
MODULEVAR const char MSG_NACHAT[] = "NACHAT";
MODULEVAR const char TOK_NACHAT[] = "AC";
MODULEVAR const char MSG_SETIDENT[] = "SETIDENT";
MODULEVAR const char TOK_SETIDENT[] = "AD";
MODULEVAR const char MSG_SETNAME[] = "SETNAME";
MODULEVAR const char TOK_SETNAME[] = "AE";
MODULEVAR const char MSG_LAG[] = "LAG";
MODULEVAR const char TOK_LAG[] = "AF";
MODULEVAR const char MSG_SDESC[] = "SDESC";
MODULEVAR const char TOK_SDESC[] = "AG";
MODULEVAR const char MSG_STATSERV[] = "STATSERV";
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
MODULEVAR const char TOK_SVSJOIN[] = "AX";
MODULEVAR const char MSG_SAJOIN[] = "SAJOIN";
MODULEVAR const char TOK_SAJOIN[] = "AY";
MODULEVAR const char MSG_SVSPART[] = "SVSPART";
MODULEVAR const char TOK_SVSPART[] = "AX";
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
MODULEVAR const char MSG_REMGLINE[] = "REMGLINE";	/* remove g-line */
MODULEVAR const char TOK_REMGLINE[] = "BG";
MODULEVAR const char MSG_REMGZLINE[] = "REMGZLINE";	/* remove global z-line */
MODULEVAR const char TOK_REMGZLINE[] = "BP";
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
MODULEVAR const char MSG_NEWJOIN[] = "NEWJOIN";	/* For CR Java Chat */
MODULEVAR const char MSG_POST[] = "POST";
MODULEVAR const char TOK_POST[] = "BN";
MODULEVAR const char MSG_INFOSERV[] = "INFOSERV";
MODULEVAR const char MSG_IS[] = "IS";
MODULEVAR const char TOK_INFOSERV[] = "BO";
MODULEVAR const char MSG_BOTSERV[] = "BOTSERV";
MODULEVAR const char TOK_BOTSERV[] = "BS";

/* Umodes */
#define UMODE_FAILOP		0x00100000	/* Shows some global messages */
#define UMODE_SERVNOTICE	0x00200000	/* server notices such as kill */
#define UMODE_KILLS			0x00400000	/* Show server-kills... */
#define UMODE_FLOOD			0x00800000	/* Receive flood warnings */
#define UMODE_JUNK			0x01000000	/* can junk */
#define UMODE_EYES			0x02000000	/* Mode to see server stuff */
#define UMODE_WHOIS			0x04000000	/* gets notice on /whois */
#define UMODE_SECURE		0x08000000	/* User is a secure connect */
#define UMODE_VICTIM		0x10000000	/* Intentional Victim */
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

#endif /* UNREAL_H Define */
