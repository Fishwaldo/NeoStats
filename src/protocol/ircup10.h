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


#ifndef IRCU_H
#define IRCU_H

/* IRCu modification support 
 * Currently we only support modifications for Nefarious and Asuka
 * To enable support uncomment the appropriate line below.
 */
/* Nefarious IRCu - http://evilnet.sourceforge.net */
/* #define NEFARIOUS */
/* ...... */
/* Nefarious IRCu with F:HOST_HIDING_STYLE:2 */
/* #define NEFARIOUS_CLOAKHOST */
/* ...... */
/* QuakeNet Asuka - http://dev.quakenet.org */
/* #define ASUKA */

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
#define MSG_SVSNICK             "SVSNICK"       /* SVNI */
#define TOK_SVSNICK             "SN"
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
#define MSG_WALLVOICES          "WALLVOICES"    /* WV */
#define TOK_WALLVOICES          "WV"
#define MSG_WALLHOPS            "WALLHOPS"      /* WH */
#define TOK_WALLHOPS            "WH"
#define MSG_CPRIVMSG            "CPRIVMSG"      /* CPRI */
#define TOK_CPRIVMSG            "CP"
#define MSG_CNOTICE             "CNOTICE"       /* CNOT */
#define TOK_CNOTICE             "CN"
#define MSG_JOIN                "JOIN"          /* JOIN */
#define TOK_JOIN                "J"
#define MSG_SVSJOIN             "SVSJOIN"       /* SVJO */
#define TOK_SVSJOIN             "SJ"
#define MSG_PART                "PART"          /* PART */
#define TOK_PART                "L"
#define MSG_SVSPART             "SVSPART"       /* SVPA */
#define TOK_SVSPART             "SP"
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
#define MSG_CHECK               "CHECK"         /* CHEC */
#define TOK_CHECK               "CC"
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
#define MSG_EXEMPT              "EXEMPT"       /* EXEM */
#define TOK_EXEMPT              "EX"
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
#define MSG_MKPASSWD            "MKPASSWD"      /* MKPA */
#define TOK_MKPASSWD            "MKPASSWD"
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
#define MSG_SETHOST             "SETHOST"       /* SETH */
#define TOK_SETHOST             "SH"
#define MSG_FAKEHOST            "FAKE"          /* FAKE */
#define TOK_FAKEHOST            "FA"
#define MSG_OPERMOTD            "OPERMOTD"      /* OPMO */
#define TOK_OPERMOTD            "OPM"
#define MSG_RULES               "RULES"         /* RULE */
#define TOK_RULES               "RL"
#define MSG_SVSNOOP             "SVSNOOP"       /* SVNO */
#define TOK_SVSNOOP             "SO"
#define MSG_SWHOIS              "SWHOIS"        /* SWHO */
#define TOK_SWHOIS              "SW"
#define MSG_MARK                "MARK"          /* MARK */
#define TOK_MARK                "MK"

 /* User modes: */
#define UMODE_SERVNOTICE        0x00800000	/* See server notices */
#define UMODE_DEBUG             0x01000000	/* See hack notices */
#ifdef NEFARIOUS
#define UMODE_FAKEHOST		0x02000000	/* */
#define UMODE_WHOIS		0x04000000	/* */
#endif
#if ( defined NEFARIOUS ) || (defined ASUKA )
#define UMODE_SETHOST		0x08000000	/* */
#define UMODE_NOCHAN		0x10000000	/* */
#define UMODE_NOIDLE		0x20000000	/* */
#define UMODE_XTRAOP		0x40000000	/* */
#endif

/* Cmodes */
#if ( defined NEFARIOUS ) || (defined ASUKA )
#define CMODE_NOCTCP		0x02000000
#define CMODE_NONOTICE		0x04000000
#define CMODE_NOQUITPARTS	0x08000000
#endif
#ifdef ASUKA
#define CMODE_DELJOINS		0x10000000
#endif
#ifdef NEFARIOUS
#define CMODE_PERSIST		0x20000000
#define CMODE_NOLISTMODES	0x40000000
#define CMODE_MODREG		0x80000000
#define CMODE_NOAMSG		0x100000000
#define CMODE_ONLYSECURE	0x200000000
#endif

#endif
