/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#ifndef VIAGRA_H
#define VIAGRA_H

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */

/* we don't support tokens */
#undef GOTTOKENSUPPORT
/* we don't support CLIENT */
#undef GOTCLIENTSUPPORT
/* we don't support svstime */
#undef GOTSVSTIME
/* we have svshost support */
#define GOTSVSHOST 
/* we have svsjoin support */
#define GOTSVSJOIN 
/* we have svsmode */
#define GOTSVSMODE
/* we have svspart */
#define GOTSVSPART
/* we have svsnick */
#define GOTSVSNICK
/* we don't have smo */
#undef GOTSMO
/* we don't have swhois */
#undef GOTSWHOIS
/* we have bot mode support */
#define GOTBOTMODE
/* we don't have user smode support */
#undef GOTUSERSMODES
/* we have svskill support */
#define GOTSVSKILL
/* we have automatic host cloaking support via Umode */
#define GOTUMODECLOAKING

/* buffer sizes */
#define MAXHOST			(128 + 1)
#define MAXPASS			(63 + 1)
#define MAXNICK			(30 + 1)
#define MAXUSER			(10 + 1)
#define MAXREALNAME		(50 + 1)
#define CHANLEN			(32 + 1)
#define TOPICLEN		(307 + 1)

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
#define MSG_SS	     "SS"            	/* StatServ commands */
#define MSG_HS	     "HS"            	/* StatServ commands */
#define MSG_SVSJOIN  "SVSJOIN"	/* SVSJOIN */
#define MSG_SAJOIN 	"SAJOIN"	/* SAJOIN */
#define MSG_SETHOST 	"SETHOST"	/* sethost */
#define MSG_SETIDENT 	"SETIDENT"	/* set ident */
#define MSG_SETNAME 	"SETNAME"	/* set Realname */
#define MSG_CHGHOST 	"CHGHOST"	/* Changehost */
#define MSG_CHGIDENT 	"CHGIDENT"	/* Change Ident */
#define MSG_SVSPART	"SVSPART"	/* SVSPART */
#define MSG_SAPART	"SAPART"	/* SAPART */
#define MSG_GLINE	"GLINE"	/* The awesome g-line */
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define MSG_OPERMOTD    "OPERMOTD"	/* OPERMOTD */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */


/* Umode chars */
#define UMODE_CH_LOCOP 'O'
#define UMODE_CH_OPER 'o'
#define UMODE_CH_ADMIN 'A'
#define UMODE_CH_COADMIN 'C'
#define UMODE_CH_NETADMIN 'N'
#define UMODE_CH_TECHADMIN 'T'
#define UMODE_CH_SADMIN 'a'
#define UMODE_CH_SERVICES 'S'
#define UMODE_CH_BOT 'B'

/* Umodes */
#define	UMODE_INVISIBLE  	0x0001	/* makes user invisible */
#define	UMODE_OPER       	0x0002	/* Operator */
#define UMODE_REGONLY		0x0010	/* only registered nicks may PM */
#define UMODE_REGNICK	 	0x0020	/* Nick set by services as registered */
#define UMODE_SERVADMIN		0x0040	/* server admin */
#define UMODE_SERVICESADMIN	0x0080	/* Marks the client as a Services Administrator */
#define UMODE_HIDE              0x8000  /* cloak host*/
#define	UMODE_WALLOP     	0x0004	/* send wallops to them */
#define UMODE_HELPOP	 	0x0010	/* Help system operator */
#define UMODE_ADMIN	 	0x0080	/* Admin */
#define UMODE_NETADMIN  	0x10000	/* Network Admin */
#define UMODE_TECHADMIN		0x80000000	/* Marks the client as a Technical Admin */
#define UMODE_DEAF		0x100000000	/* client is deaf on channels */
#define	UMODE_LOCOP      	0x0200	/* Local operator -- SRB */
#define UMODE_WHOIS    		0x200000	/* Lets Opers see when people do a /WhoIs on them */
#define UMODE_CLIENT	 	0x0800	/* Show client information */
#define UMODE_GLOBCON		0x800000	/* Global Connect notices */

/* Cmode chars */
#define CMODE_CH_CHANOP 'o'
#define CMODE_CH_VOICE	'v'
#define CMODE_CH_HALFOP	'h'
#define CMODE_CH_CHANOWNER	'q'
#define CMODE_CH_CHANADMIN	'a'

/* Cmode sjoin flags */
#define CMODE_FL_CHANOP '@'
#define CMODE_FL_VOICE	'+'
#define CMODE_FL_HALFOP	'%'
#define CMODE_FL_CHANOWNER	'!'
#define CMODE_FL_CHANADMIN	'*'

/* Cmodes */
#define CMODE_CHANOP	0x0001
#define CMODE_HALFOP	0x0002
#define	CMODE_VOICE	0x0004
#define	CMODE_PRIVATE	0x0008
#define	CMODE_SECRET	0x0010
#define	CMODE_MODERATED  0x0020
#define	CMODE_TOPICLIMIT 0x0040
#define	CMODE_INVITEONLY 0x0080
#define	CMODE_NOPRIVMSGS 0x0100
#define	CMODE_KEY	0x0200
#define CMODE_CHANOWNER 0x0400
#define	CMODE_BAN	0x0800
#define CMODE_LIMIT	0x1000
#define CMODE_RGSTR	0x2000
#define CMODE_RGSTRONLY  0x4000
#define CMODE_OPERONLY   0x8000
#define CMODE_ADMONLY   	0x10000
#define CMODE_LINK	0x20000
#define CMODE_NOCOLOR	0x40000
#define CMODE_CHANADMIN  0x800000

/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_OPERONLY)))
#define is_pub_chan(x)  ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))
#define is_priv_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))

/* Umode macros */
#define is_oper(x) ((x) && (x->Umode & UMODE_OPER))
#define is_bot(x) (0)

#endif
