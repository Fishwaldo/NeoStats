/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2008 ^Enigma^
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
#ifndef LIQUIDIRCD_H
#define LIQUIDIRCD_H

/* Messages/Tokens */
#define MSG_PRIVATE  "PRIVMSG"		/* PRIV */
#define MSG_WHO      "WHO"	      	/* WHO  -> WHOC */
#define MSG_WHOIS    "WHOIS"	   	/* WHOI */
#define MSG_WHOWAS   "WHOWAS"	   	/* WHOW */
#define MSG_USER     "USER"	   	/* USER */
#define MSG_NICK     "NICK"	   	/* NICK */
#define MSG_SERVER   "SERVER"	   	/* SERV */
#define MSG_LIST     "LIST"	   	/* LIST */
#define MSG_TOPIC    "TOPIC"	   	/* TOPI */
#define MSG_INVITE   "INVITE"	   	/* INVI */
#define MSG_VERSION  "VERSION"		/* VERS */
#define MSG_QUIT     "QUIT"	   	/* QUIT */
#define MSG_SQUIT    "SQUIT"	   	/* SQUI */
#define MSG_KILL     "KILL"	   	/* KILL */
#define MSG_INFO     "INFO"	   	/* INFO */
#define MSG_LINKS    "LINKS"	   	/* LINK */
#define MSG_STATS    "STATS"	   	/* STAT */
#define MSG_USERS    "USERS"	   	/* USER -> USRS */
#define MSG_HELP     "HELP"	   	/* HELP */
#define MSG_ERROR    "ERROR"	   	/* ERRO */
#define MSG_AWAY     "AWAY"	   	/* AWAY */
#define MSG_CONNECT  "CONNECT"		/* CONN */
#define MSG_PING     "PING"	   	/* PING */
#define MSG_PONG     "PONG"	   	/* PONG */
#define MSG_OPER     "OPER"	   	/* OPER */
#define MSG_PASS     "PASS"	   	/* PASS */
#define MSG_WALLOPS  "WALLOPS"		/* WALL */
#define MSG_TIME     "TIME"	   	/* TIME */
#define MSG_NAMES    "NAMES"	   	/* NAME */
#define MSG_ADMIN    "ADMIN"	   	/* ADMI */
#define MSG_TRACE    "TRACE"	   	/* TRAC */
#define MSG_NOTICE   "NOTICE"	   	/* NOTI */
#define MSG_JOIN     "JOIN"	   	/* JOIN */
#define MSG_PART     "PART"	   	/* PART */
#define MSG_LUSERS   "LUSERS"	   	/* LUSE */
#define MSG_MOTD     "MOTD"	   	/* MOTD */
#define MSG_MODE     "MODE"	   	/* MODE */
#define MSG_KICK     "KICK"	   	/* KICK */
#define MSG_USERHOST "USERHOST"		/* USER -> USRH */
#define MSG_ISON     "ISON"	   	/* ISON */
#define MSG_REHASH   "REHASH"	   	/* REHA */
#define MSG_RESTART  "RESTART"		/* REST */
#define MSG_CLOSE    "CLOSE"	   	/* CLOS */
#define MSG_SVINFO   "SVINFO"	   	/* SVINFO */
#define MSG_SJOIN    "SJOIN"	   	/* SJOIN */
#define MSG_DIE	     "DIE" 		/* DIE */
#define MSG_HASH     "HASH"	   	/* HASH */
#define MSG_DNS      "DNS"   	   	/* DNS  -> DNSS */
#define MSG_OPERWALL "OPERWALL"		/* OPERWALL */
#define MSG_GLOBOPS  "GLOBOPS"		/* GLOBOPS */
#define MSG_CHATOPS  "CHATOPS"		/* CHATOPS */
#define MSG_GOPER    "GOPER"	   	/* GOPER */
#define MSG_GNOTICE  "GNOTICE"		/* GNOTICE */
#define MSG_KLINE    "KLINE"	   	/* KLINE */
#define MSG_UNKLINE  "UNKLINE"		/* UNKLINE */
#define MSG_HTM      "HTM"	      	/* HTM */
#define MSG_SET      "SET"	      	/* SET */
#define MSG_SAMODE   "SAMODE"    	/* SAMODE */
#define MSG_CHANSERV "CHANSERV"		/* CHANSERV */
#define MSG_NICKSERV "NICKSERV"		/* NICKSERV */
#define MSG_MEMOSERV "MEMOSERV"		/* MEMOSERV */
#define MSG_ROOTSERV "ROOTSERV"		/* MEMOSERV */
#define MSG_OPERSERV "OPERSERV"		/* OPERSERV */
#define MSG_BOTSERV  "BOTSERV"		/* BOTSERV */
#define MSG_STATSERV "STATSERV" 	/* STATSERV */
#define MSG_HELPSERV "HELPSERV" 	/* HELPSERV */
#define MSG_SERVICES "SERVICES"		/* SERVICES */
#define MSG_IDENTIFY "IDENTIFY"		/* IDENTIFY */
#define MSG_CAPAB    "CAPAB"	   	/* CAPAB */ 
#define MSG_LOCOPS   "LOCOPS"	   	/* LOCOPS */
#define MSG_SVSNICK  "SVSNICK"   	/* SVSNICK */
#define MSG_SVSNOOP  "SVSNOOP"   	/* SVSNOOP */
#define MSG_SVSKILL  "SVSKILL"   	/* SVSKILL */
#define MSG_SVSMODE  "SVSMODE"   	/* SVSMODE */
#define MSG_AKILL    "AKILL"     	/* AKILL */
#define MSG_RAKILL   "RAKILL"    	/* RAKILL */
#define MSG_SILENCE  "SILENCE"   	/* SILENCE */
#define MSG_WATCH    "WATCH"     	/* WATCH */
#define MSG_SQLINE   "SQLINE" 		/* SQLINE */
#define MSG_UNSQLINE "UNSQLINE" 	/* UNSQLINE */
#define MSG_BURST    "BURST"     	/* BURST */
#define MSG_DCCALLOW "DCCALLOW"		/* dccallow */
#define MSG_SGLINE   "SGLINE"           /* sgline */
#define MSG_UNSGLINE "UNSGLINE"         /* unsgline */
#define MSG_DKEY     "DKEY"		/* diffie-hellman negotiation */
#define MSG_NS	     "NS"            	/* NickServ commands */
#define MSG_CS	     "CS"            	/* ChanServ commands */
#define MSG_MS	     "MS"            	/* MemoServ commands */
#define MSG_RS	     "RS"            	/* RootServ commands */
#define MSG_OS	     "OS"            	/* OperServ commands */
#define MSG_BS       "BS"               /* BotServ commands */
#define MSG_SS	     "SS"            	/* StatServ commands */
#define MSG_HS	     "HS"            	/* StatServ commands */
#define MSG_SETHOST     "SETHOST"       /* sethost */
#define MSG_CHGHOST     "CHGHOST"       /* chghost */
#define MSG_VS          "VS"            /* VS */
#define MSG_SVSCHGHOST  "SVSCHGHOST"    /* SVSCHGHOST */
#define MSG_CHGIDENT    "CHGIDENT"      /* CHGIDENT */
#define MSG_SETIDENT    "SETIDENT"      /* SETIDENT */
#define MSG_MAP         "MAP"           /* MAP */
#define MSG_SLINE       "SLINE"         /* SLINE */
#define MSG_GLINE       "GLINE"         /* GLINE */
#define MSG_SAJOIN      "SAJOIN"        /* SAJOIN */
#define MSG_SAPART      "SAPART"        /* SAPART */
#define MSG_IRCOPS      "IRCOPS"        /* IRCOPS */
#define MSG_SMODE	"SMODE"		/* SMODE */
#define MSG_UMODE	"UMODE"		/* UMODE */
#define MSG_GREHASH	"GREHASH"	/* GREHASH */
#define MSG_LAG         "LAG"           /* LAG */
#define MSG_LAGTEST     "LAGTEST"       /* LAGTEST */

/* Umodes */
#define UMODE_SERVNOTICE    0x00008000	/* umode +s - Server notices */
#define UMODE_KILLS			0x00010000	/* umode +k - Server kill messages */
#define UMODE_FLOOD			0x00020000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00040000	/* umode +y - Stats/links */
#define UMODE_DEBUG			0x00080000	/* umode +d - Debug info */
#define UMODE_GLOBOPS		0x00100000	/* umode +g - Globops */
#define UMODE_CHATOPS		0x00200000	/* umode +b - Chatops */
#define UMODE_ROUTE			0x00400000	/* umode +n - Routing Notices */
#define UMODE_SPAM			0x00800000	/* umode +m - spambot notices */
#define UMODE_OPERNOTICE    0x01000000	/* umode +e - oper notices for the above +D */
#define UMODE_SQUELCH		0x02000000	/* umode +x - Squelch with notice */
#define UMODE_HIDDENDCC     0x04000000	/* umode +D - Hidden dccallow umode */
#define UMODE_THROTTLE		0x08000000	/* umode +F - no cptr->since message rate throttle */
#define UMODE_REJ			0x10000000	/* umode +j - client rejection notices */
#define UMODE_ULINEKILL     0x20000000	/* umode +K - U: lined server kill messages */
#define UMODE_CODER			0x40000000	/* */
#define UMODE_WHOIS			0x80000000	/* whois */

/* Cmodes */
#define CMODE_UOP			0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_NONICKCHANGE	0x08000000
#define CMODE_AUDITORIUM	0x10000000

#endif
