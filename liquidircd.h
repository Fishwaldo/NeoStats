/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
#ifndef LIQUIDIRCD_H
#define LIQUIDIRCD_H

/* we don't support server names as base 64 numerics */
#undef BASE64SERVERNAME
/* we don't support nick names as base 64 numerics */
#undef BASE64NICKNAME

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */

/* we don't support tokens */
#undef GOTTOKENSUPPORT
/* we don't support CLIENT */
#undef GOTCLIENTSUPPORT
/* we don't support svstime */
#undef GOTSVSTIME
/* we have vhost support */
#define GOTSVSHOST 
/* we don't have svsjoin support */
#undef GOTSVSJOIN 
/* we don't have svsmode */
#undef GOTSVSMODE
/* we don't have svspart */
#undef GOTSVSPART
/* we have svsnick */
#define GOTSVSNICK
/* we don't have smo */
#undef GOTSMO
/* we don't have swhois */
#undef GOTSWHOIS
/* we don't have bot mode support */
#undef GOTBOTMODE
/* we do have user smode support */
#define GOTUSERSMODES
/* we have svskill support */
#define GOTSVSKILL
/* we don't have automatic host cloaking support via Umode */
#undef GOTUMODECLOAKING

/* buffer sizes */
#define MAXHOST			(128 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(32 + 1)
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

/* Umode chars */
#define UMODE_CH_LOCOP 'O'
#define UMODE_CH_OPER 'o'
#define UMODE_CH_ADMIN 'A'
#define UMODE_CH_NETADMIN 'N'
#define UMODE_CH_TECHADMIN 'T'
#define UMODE_CH_SADMIN 'a'
#define UMODE_CH_BOT 'B'

/* Umodes */
#define UMODE_OPER     0x00001	/* umode +o - Oper */
#define UMODE_LOCOP     0x00002	/* umode +O - Local Oper */
#define UMODE_INVISIBLE     0x00004	/* umode +i - Invisible */
#define UMODE_WALLOP     0x00008	/* umode +w - Get wallops */
#define UMODE_SERVNOTICE     0x00010	/* umode +s - Server notices */
#define UMODE_CLIENT     0x00020	/* umode +c - Client connections/exits */
#define UMODE_REGNICK     0x00040	/* umode +r - registered nick */
#define UMODE_KILLS     0x00080	/* umode +k - Server kill messages */
#define UMODE_FLOOD     0x00100	/* umode +f - Server flood messages */
#define UMODE_SPY     0x00200	/* umode +y - Stats/links */
#define UMODE_DEBUG     0x00400	/* umode +d - Debug info */
#define UMODE_GLOBOPS     0x00800	/* umode +g - Globops */
#define UMODE_CHATOPS     0x01000	/* umode +b - Chatops */
#define UMODE_SADMIN     0x02000	/* umode +a - Services Admin */
#define UMODE_ADMIN     0x04000     /* umode +A - Server Admin */
#define UMODE_ROUTE     0x08000	/* umode +n - Routing Notices */
#define UMODE_HELPOP     0x10000     /* umode +h - Helper */
#define UMODE_SPAM     0x20000     /* umode +m - spambot notices */
#define UMODE_RGSTRONLY     0x40000     /* unmode +R - No non registered msgs */
#define UMODE_OPERNOTICE     0x80000    /* umode +e - oper notices for the above +D */
#define UMODE_SQUELCH     0x100000    /* umode +x - Squelch with notice */
#define UMODE_BOT     0x200000    /* umode +B - Bot Flag */
#define UMODE_HIDDENDCC     0x400000    /* umode +D - Hidden dccallow umode */
#define UMODE_THROTTLE     0x800000   /* umode +F - no cptr->since message rate throttle */
#define UMODE_REJ	    0x1000000   /* umode +j - client rejection notices */
#define UMODE_ULINEKILL     0x2000000   /* umode +K - U: lined server kill messages */
#define UMODE_HIDE    0x4000000	/* umode +z - Hostmasking */
#define UMODE_NETADMIN	    0x8000000   /* Network Administrator */
#define UMODE_TECHADMIN     0x10000000  /* Technical Administrator */
#define UMODE_CODER     0x20000000  /* */
#define UMODE_KIX	    0x40000000  /* Protection Flag */
#define UMODE_WHOIS	    0x80000000  /* whois */

/* Smodes */
#define SMODE_SSL              0x1  /* */

/* Cmode chars */
#define CMODE_CH_CHANOP 'o'
#define CMODE_CH_VOICE	'v'
#define CMODE_CH_HALFOP	'h'
#define CMODE_CH_CHANOWNER	'q'
#define CMODE_CH_CHANPROT	'a'
#define CMODE_CH_UOP	'u'

/* Cmode sjoin flags */
#define CMODE_FL_CHANOP '@'
#define CMODE_FL_VOICE	'+'
#define CMODE_FL_HALFOP	'%'
#define CMODE_FL_CHANOWNER	'!'
#define CMODE_FL_CHANPROT	'*'
#define CMODE_FL_UOP	'-'

/* Cmodes */
#define	CHFL_BAN		0x0200 /* ban channel flag */
#define CMODE_CHANOWNER	0x0080
#define CMODE_DECHANOWNER 0x0100
#define CMODE_CHANPROT	0x0020
#define CMODE_DECHANPROT	0x0040
#define	CMODE_CHANOP	0x0001
#define	CMODE_VOICE	0x0002
#define	CMODE_DEOPPED  	0x0004
#define CMODE_HALFOP	0x0008
#define CMODE_DEHALFOPPED 0x0010
#define CMODE_UOP	0x0400
#define CMODE_DEUOP	0x0800
#define	CMODE_PRIVATE  	0x00008
#define	CMODE_SECRET   	0x00010
#define	CMODE_MODERATED  0x00020
#define	CMODE_TOPICLIMIT 0x00040
#define	CMODE_INVITEONLY 0x00080
#define	CMODE_NOPRIVMSGS 0x00100
#define	CMODE_KEY	0x00200
#define	CMODE_BAN	0x00400
#define	CMODE_LIMIT	0x00800
#define CMODE_RGSTR	0x01000
#define CMODE_RGSTRONLY	0x02000
#define CMODE_NOCOLOR	0x04000
#define CMODE_OPERONLY   0x08000
#define CMODE_MODREG     0x10000
#define CMODE_LINK	0x20000
#define CMODE_STRIP		0x40000
#define CMODE_NONICKCHANGE		0x80000
#define CMODE_AUDITORIUM 0x100000


/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_OPERONLY)))
#define is_pub_chan(x)  ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))
#define is_priv_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))

/* Umode macros */
#define is_oper(x) ((x) && ((x->Umode & UMODE_OPER) || (x->Umode & UMODE_LOCOP)))
#define is_bot(x) ((x) && (x->Umode & UMODE_BOT))    

#endif
