/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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


#ifndef IRCU_H
#define IRCU_H

/* we support server names as base 64 numerics */
#define BASE64SERVERNAME
/* we support nick names as base 64 numerics */
#define BASE64NICKNAME

/* Feature support for use by modules to determine whether
 * certain functionality is available
 */
/* The following defines might not be correct for IRCu but are used to 
 * ensure NeoStats compiles correctly until we get updated files */

/* we support tokens */
#define GOTTOKENSUPPORT
/* we don't support CLIENT */
#undef GOTCLIENTSUPPORT
/* we don't support svstime */
#undef GOTSVSTIME
/* we don't have vhost support */
#undef GOTSVSHOST 
/* we don't have svsjoin */
#undef GOTSVSJOIN
/* we don't have svsmode */
#undef GOTSVSMODE
/* we don't have svspart */
#undef GOTSVSPART
/* we don't have svsnick */
#undef GOTSVSNICK
/* we don't have smo */
#undef GOTSMO
/* we don't have swhois */
#undef GOTSWHOIS
/* we don't have bot mode support */
#undef GOTBOTMODE
/* we don't have user smode support */
#undef GOTUSERSMODES
/* we don't have svskill support */
#undef GOTSVSKILL
/* we don't have automatic host cloaking support via Umode */
#undef GOTUMODECLOAKING

/* Override NeoStats core splitbuf function */
/* #define IRCD_SPLITBUF */
/* Override NeoStats core parse function */
#define IRCD_PARSE

/* buffer sizes */
#define MAXHOST			(63 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(32 + 1)
#define MAXUSER			(10 + 1)
#define MAXREALNAME		(50 + 1)
#define CHANLEN			(50 + 1)
#define TOPICLEN		(250 + 1)

/* Messages/Tokens */

#define MSG_PRIVATE             "PRIVMSG"       /* PRIV */
#define TOK_PRIVATE             "P"
#define MSG_WHO                 "WHO"           /* WHO  -> WHOC */
#define TOK_WHO                 "H"
#define MSG_WHOIS               "WHOIS"         /* WHOI */
#define TOK_WHOIS               "W"
#define MSG_WHOWAS              "WHOWAS"        /* WHOW */
#define TOK_WHOWAS              "X"
#define MSG_USER                "USER"          /* USER */
#define TOK_USER                "USER"
#define MSG_NICK                "NICK"          /* NICK */
#define TOK_NICK                "N"
#define MSG_SERVER              "SERVER"        /* SERV */
#define TOK_SERVER              "S"
#define MSG_LIST                "LIST"          /* LIST */
#define TOK_LIST                "LIST"
#define MSG_TOPIC               "TOPIC"         /* TOPI */
#define TOK_TOPIC               "T"
#define MSG_INVITE              "INVITE"        /* INVI */
#define TOK_INVITE              "I"
#define MSG_VERSION             "VERSION"       /* VERS */
#define TOK_VERSION             "V"
#define MSG_QUIT                "QUIT"          /* QUIT */
#define TOK_QUIT                "Q"
#define MSG_SQUIT               "SQUIT"         /* SQUI */
#define TOK_SQUIT               "SQ"
#define MSG_KILL                "KILL"          /* KILL */
#define TOK_KILL                "D"
#define MSG_INFO                "INFO"          /* INFO */
#define TOK_INFO                "F"
#define MSG_LINKS               "LINKS"         /* LINK */
#define TOK_LINKS               "LI"
#define MSG_STATS               "STATS"         /* STAT */
#define TOK_STATS               "R"
#define MSG_HELP                "HELP"          /* HELP */
#define TOK_HELP                "HELP"
#define MSG_ERROR               "ERROR"         /* ERRO */
#define TOK_ERROR               "Y"
#define MSG_AWAY                "AWAY"          /* AWAY */
#define TOK_AWAY                "A"
#define MSG_CONNECT             "CONNECT"       /* CONN */
#define TOK_CONNECT             "CO"
#define MSG_MAP                 "MAP"           /* MAP  */
#define TOK_MAP                 "MAP"
#define MSG_PING                "PING"          /* PING */
#define TOK_PING                "G"
#define MSG_PONG                "PONG"          /* PONG */
#define TOK_PONG                "Z"
#define MSG_OPER                "OPER"          /* OPER */
#define TOK_OPER                "OPER"
#define MSG_PASS                "PASS"          /* PASS */
#define TOK_PASS                "PA"
#define MSG_WALLOPS             "WALLOPS"       /* WALL */
#define TOK_WALLOPS             "WA"
#define MSG_WALLUSERS           "WALLUSERS"     /* WALL */
#define TOK_WALLUSERS           "WU"
#define MSG_DESYNCH             "DESYNCH"       /* DESY */
#define TOK_DESYNCH             "DS"
#define MSG_TIME                "TIME"          /* TIME */
#define TOK_TIME                "TI"
#define MSG_SETTIME             "SETTIME"       /* SETT */
#define TOK_SETTIME             "SE"
#define MSG_RPING               "RPING"         /* RPIN */
#define TOK_RPING               "RI"
#define MSG_RPONG               "RPONG"         /* RPON */
#define TOK_RPONG               "RO"
#define MSG_NAMES               "NAMES"         /* NAME */
#define TOK_NAMES               "E"
#define MSG_ADMIN               "ADMIN"         /* ADMI */
#define TOK_ADMIN               "AD"
#define MSG_TRACE               "TRACE"         /* TRAC */
#define TOK_TRACE               "TR"
#define MSG_NOTICE              "NOTICE"        /* NOTI */
#define TOK_NOTICE              "O"
#define MSG_WALLCHOPS           "WALLCHOPS"     /* WC */
#define TOK_WALLCHOPS           "WC"
#define MSG_WALLVOICES           "WALLVOICES"     /* WV */
#define TOK_WALLVOICES           "WV"
#define MSG_CPRIVMSG            "CPRIVMSG"      /* CPRI */
#define TOK_CPRIVMSG            "CP"
#define MSG_CNOTICE             "CNOTICE"       /* CNOT */
#define TOK_CNOTICE             "CN"
#define MSG_JOIN                "JOIN"          /* JOIN */
#define TOK_JOIN                "J"
#define MSG_PART                "PART"          /* PART */
#define TOK_PART                "L"
#define MSG_LUSERS              "LUSERS"        /* LUSE */
#define TOK_LUSERS              "LU"
#define MSG_MOTD                "MOTD"          /* MOTD */
#define TOK_MOTD                "MO"
#define MSG_MODE                "MODE"          /* MODE */
#define TOK_MODE                "M"
#define MSG_KICK                "KICK"          /* KICK */
#define TOK_KICK                "K"
#define MSG_USERHOST            "USERHOST"      /* USER -> USRH */
#define TOK_USERHOST            "USERHOST"
#define MSG_USERIP              "USERIP"        /* USER -> USIP */
#define TOK_USERIP              "USERIP"
#define MSG_ISON                "ISON"          /* ISON */
#define TOK_ISON                "ISON"
#define MSG_SQUERY              "SQUERY"        /* SQUE */
#define TOK_SQUERY              "SQUERY"
#define MSG_SERVLIST            "SERVLIST"      /* SERV -> SLIS */
#define TOK_SERVLIST            "SERVSET"
#define MSG_SERVSET             "SERVSET"       /* SERV -> SSET */
#define TOK_SERVSET             "SERVSET"
#define MSG_REHASH              "REHASH"        /* REHA */
#define TOK_REHASH              "REHASH"
#define MSG_RESTART             "RESTART"       /* REST */
#define TOK_RESTART             "RESTART"
#define MSG_CLOSE               "CLOSE"         /* CLOS */
#define TOK_CLOSE               "CLOSE"
#define MSG_DIE                 "DIE"           /* DIE  */
#define TOK_DIE                 "DIE"
#define MSG_HASH                "HASH"          /* HASH */
#define TOK_HASH                "HASH"
#define MSG_DNS                 "DNS"           /* DNS  -> DNSS */
#define TOK_DNS                 "DNS"
#define MSG_SILENCE             "SILENCE"       /* SILE */
#define TOK_SILENCE             "U"
#define MSG_GLINE               "GLINE"         /* GLIN */
#define TOK_GLINE               "GL"
#define MSG_BURST               "BURST"         /* BURS */
#define TOK_BURST               "B"
#define MSG_UPING               "UPING"         /* UPIN */
#define TOK_UPING               "UP"
#define MSG_CREATE              "CREATE"        /* CREA */
#define TOK_CREATE              "C"
#define MSG_DESTRUCT            "DESTRUCT"      /* DEST */
#define TOK_DESTRUCT            "DE"
#define MSG_END_OF_BURST        "END_OF_BURST"  /* END_ */
#define TOK_END_OF_BURST        "EB"
#define MSG_END_OF_BURST_ACK    "EOB_ACK"       /* EOB_ */
#define TOK_END_OF_BURST_ACK    "EA"
#define MSG_PROTO               "PROTO"         /* PROTO */
#define TOK_PROTO               "PROTO"         /* PROTO */
#define MSG_JUPE                "JUPE"          /* JUPE */
#define TOK_JUPE                "JU"
#define MSG_OPMODE              "OPMODE"        /* OPMO */
#define TOK_OPMODE              "OM"
#define MSG_CLEARMODE           "CLEARMODE"     /* CLMO */
#define TOK_CLEARMODE           "CM"
#define MSG_ACCOUNT		"ACCOUNT"	/* ACCO */
#define TOK_ACCOUNT		"AC"
#define MSG_ASLL		"ASLL"		/* ASLL */
#define TOK_ASLL		"LL"
#define MSG_POST                "POST"          /* POST */
#define TOK_POST                "POST"
#define MSG_SET			"SET"		/* SET */
#define TOK_SET			"SET"
#define MSG_RESET		"RESET"		/* RESE */
#define TOK_RESET		"RESET"
#define MSG_GET			"GET"		/* GET */
#define TOK_GET			"GET"
#define MSG_PRIVS		"PRIVS"		/* PRIV */
#define TOK_PRIVS		"PRIVS"

/* Umode chars */
#define UMODE_CH_LOCOP 'O'
#define UMODE_CH_OPER 'o'

 /* User modes: */
#define UMODE_OPER		0x0001	/* Operator */
#define UMODE_LOCOP		0x0002	/* Local oper */
#define UMODE_INVISIBLE         0x0004	/* Invisible */
#define UMODE_WALLOP            0x0008	/* see wallops */
#define UMODE_SERVNOTICE        0x0010	/* See server notices */
#define UMODE_DEAF              0x0020	/* Dont see chan msgs */
#define UMODE_CHSERV            0x0040	/* Unkickable/-o able */
#define UMODE_DEBUG             0x0080	/* See hack notices */
#define UMODE_HELPER            0x0100	/* Afternets +h cs override mode */
#define UMODE_ACCOUNT			0x1000	/* */
#define UMODE_HIDE				0x2000	/* */

/* Cmodes */
#define CMODE_CHANOP	0x0001
#define CMODE_VOICE		0x0002
#define CMODE_PRIVATE	0x0004
#define CMODE_SECRET	0x0008
#define CMODE_MODERATED	0x0010
#define CMODE_TOPICLIMIT 0x0020
#define CMODE_INVITEONLY 0x0040
#define CMODE_NOPRIVMSGS 0x0080
#define CMODE_KEY		0x0100
#define CMODE_BAN		0x0200
#define CMODE_LIMIT		0x0400
#define CMODE_SENDTS	0x0800	
#define CMODE_DELAYJOINS 0x1000
#define CMODE_LISTED	0x10000

/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & CMODE_SECRET))
#define is_pub_chan(x)  ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))
#define is_priv_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))

/* Umode macros */
#define is_oper(x) ((x) && (x->Umode & UMODE_OPER))
#define is_bot(x) (0)           

#endif
