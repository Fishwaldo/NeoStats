/**********************************************************************t
*
 *   IRC - Internet Relay Chat, include/msg.h
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: Unreal.h,v 1.15 2002/03/28 05:29:36 fishwaldo Exp $
 */


#ifndef UNREAL_H

#define UNREAL_H








/* Shamelessly Stolen from Unreal2.1.7 for Token Support!  - Fish*/

/*
 * The tokens are in the ascii character range of 33-127, and we start
 * from 33 and just move up.  It would be nice to match then up so they
 * are slightly related to their string counterpart, but that makes it
 * too confusing when we want to add another one and need to make sure
 * we're not using one already used. -Cabal95
 *
 * As long as the #defines are kept statically placed, it will be fine.
 * We don't care/worry about the msgtab[] since it can be dynamic, but
 * the tokens it uses will still be static according to the messages
 * they represent.  In other words leave the #defines in order, if you're
 * going to add something, PUT IT AT THE END.  Do not even look for an
 * open spot somewhere, as that may lead to one type of message being
 * sent by server A to server B, but server B thinks its something else.
 * Remember, skip the : since its got a special use, and I skip the \ too
 * since it _may_ cause problems, but not sure.  -Cabal95
 * I'm skipping A and a as well, because some clients and scripts use
 * these to test if the server has already processed whole queue.
 * Since the client could request this protocol withhout the script
 * knowing it, I'm considering that reserved, and TRACE/A is now 'b'.
 * The normal msgtab should probably process this as special. -Donwulff
 */
 
/*	12/05/1999 - I was wrong - I didnt see the token[2] in struct Message
	okie 60*60 commands more :P - Sowwy!!! -sts
	
 */





#define MSG_PRIVATE	"PRIVMSG"	/* PRIV */
#define TOK_PRIVATE	"!"		/* 33 */
#define MSG_WHO		"WHO"		/* WHO  -> WHOC */
#define TOK_WHO		"\""		/* 34 */
#define MSG_WHOIS	"WHOIS"		/* WHOI */
#define TOK_WHOIS	"#"		/* 35 */
#define MSG_WHOWAS	"WHOWAS"	/* WHOW */
#define TOK_WHOWAS	"$"		/* 36 */
#define MSG_USER	"USER"		/* USER */
#define TOK_USER	"%"		/* 37 */
#define MSG_NICK	"NICK"		/* NICK */
#define TOK_NICK	"&"		/* 38 */
#define MSG_SERVER	"SERVER"	/* SERV */
#define TOK_SERVER	"'"		/* 39 */
#define MSG_LIST	"LIST"		/* LIST */
#define TOK_LIST	"("		/* 40 */
#define MSG_TOPIC	"TOPIC"		/* TOPI */
#define TOK_TOPIC	")"		/* 41 */
#define MSG_INVITE	"INVITE"	/* INVI */
#define TOK_INVITE	"*"		/* 42 */
#define MSG_VERSION	"VERSION"	/* VERS */
#define TOK_VERSION	"+"		/* 43 */
#define MSG_QUIT	"QUIT"		/* QUIT */
#define TOK_QUIT	","		/* 44 */
#define MSG_SQUIT	"SQUIT"		/* SQUI */
#define TOK_SQUIT	"-"		/* 45 */
#define MSG_KILL	"KILL"		/* KILL */
#define TOK_KILL	"."		/* 46 */
#define MSG_INFO	"INFO"		/* INFO */
#define TOK_INFO	"/"		/* 47 */
#define MSG_LINKS	"LINKS"		/* LINK */
#define TOK_LINKS	"0"		/* 48 */
#define MSG_SUMMON	"SUMMON"	/* SUMM */
#define TOK_SUMMON	"1"		/* 49 */
#define MSG_STATS	"STATS"		/* STAT */
#define TOK_STATS	"2"		/* 50 */
#define MSG_USERS	"USERS"		/* USER -> USRS */
#define TOK_USERS	"3"		/* 51 */
#define MSG_HELP	"HELP"		/* HELP */
#define MSG_HELPOP	"HELPOP"	/* HELP */
#define MSG_IRCDHELP "IRCDHELP" /* HELP */
#define TOK_HELP	"4"		/* 52 */
#define MSG_ERROR	"ERROR"		/* ERRO */
#define TOK_ERROR	"5"		/* 53 */
#define MSG_AWAY	"AWAY"		/* AWAY */
#define TOK_AWAY	"6"		/* 54 */
#define MSG_CONNECT	"CONNECT"	/* CONN */
#define TOK_CONNECT	"7"		/* 55 */
#define MSG_PING	"PING"		/* PING */
#define TOK_PING	"8"		/* 56 */
#define MSG_PONG	"PONG"		/* PONG */
#define TOK_PONG	"9"		/* 57 */
#define MSG_OPER	"OPER"		/* OPER */
#define TOK_OPER	";"		/* 59 */
#define MSG_PASS	"PASS"		/* PASS */
#define TOK_PASS	"<"		/* 60 */
#define MSG_WALLOPS	"WALLOPS"	/* WALL */
#define TOK_WALLOPS	"="		/* 61 */
#define MSG_TIME	"TIME"		/* TIME */
#define TOK_TIME	">"		/* 62 */
#define MSG_NAMES	"NAMES"		/* NAME */
#define TOK_NAMES	"?"		/* 63 */
#define MSG_ADMIN	"ADMIN"		/* ADMI */
#define TOK_ADMIN	"@"		/* 64 */
#define MSG_NOTICE	"NOTICE"	/* NOTI */
#define TOK_NOTICE	"B"		/* 66 */
#define MSG_JOIN	"JOIN"		/* JOIN */
#define TOK_JOIN	"C"		/* 67 */
#define MSG_PART	"PART"		/* PART */
#define TOK_PART	"D"		/* 68 */
#define MSG_LUSERS	"LUSERS"	/* LUSE */
#define TOK_LUSERS	"E"		/* 69 */
#define MSG_MOTD	"MOTD"		/* MOTD */
#define TOK_MOTD	"F"		/* 70 */
#define MSG_MODE	"MODE"		/* MODE */
#define TOK_MODE	"G"		/* 71 */
#define MSG_KICK	"KICK"		/* KICK */
#define TOK_KICK	"H"		/* 72 */
#define MSG_SERVICE	"SERVICE"	/* SERV -> SRVI */
#define TOK_SERVICE	"I"		/* 73 */
#define MSG_USERHOST	"USERHOST"	/* USER -> USRH */
#define TOK_USERHOST	"J"		/* 74 */
#define MSG_ISON	"ISON"		/* ISON */
#define TOK_ISON	"K"		/* 75 */
#define MSG_SQUERY	"SQUERY"	/* SQUE */
#define TOK_SQUERY	"L"		/* 76 */
#define MSG_SERVLIST	"SERVLIST"	/* SERV -> SLIS */
#define TOK_SERVLIST	"M"		/* 77 */
#define MSG_SERVSET	"SERVSET"	/* SERV -> SSET */
#define TOK_SERVSET	"N"		/* 78 */
#define	MSG_REHASH	"REHASH"	/* REHA */
#define TOK_REHASH	"O"		/* 79 */
#define	MSG_RESTART	"RESTART"	/* REST */
#define TOK_RESTART	"P"		/* 80 */
#define	MSG_CLOSE	"CLOSE"		/* CLOS */
#define TOK_CLOSE	"Q"		/* 81 */
#define	MSG_DIE		"DIE"		/* DIE */
#define TOK_DIE		"R"		/* 82 */
#define	MSG_HASH	"HASH"		/* HASH */
#define TOK_HASH	"S"		/* 83 */
#define	MSG_DNS		"DNS"		/* DNS  -> DNSS */
#define TOK_DNS		"T"		/* 84 */
#define MSG_SILENCE	"SILENCE"	/* SILE */
#define TOK_SILENCE	"U"		/* 85 */
#define MSG_AKILL	"AKILL"		/* AKILL */
#define TOK_AKILL	"V"		/* 86 */
#define MSG_KLINE	"KLINE"		/* KLINE */
#define TOK_KLINE	"W"		/* 87 */
#define MSG_UNKLINE     "UNKLINE"       /* UNKLINE */
#define TOK_UNKLINE	"X"		/* 88 */
#define MSG_RAKILL	"RAKILL"	/* RAKILL */
#define TOK_RAKILL	"Y"		/* 89 */
#define MSG_GNOTICE	"GNOTICE"	/* GNOTICE */
#define TOK_GNOTICE	"Z"		/* 90 */
#define MSG_GOPER	"GOPER"		/* GOPER */
#define TOK_GOPER	"["		/* 91 */
#define MSG_GLOBOPS	"GLOBOPS"	/* GLOBOPS */
#define TOK_GLOBOPS	"]"		/* 93 */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define TOK_LOCOPS	"^"		/* 94 */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define TOK_PROTOCTL	"_"		/* 95 */
#define MSG_WATCH	"WATCH"		/* WATCH */
#define TOK_WATCH	"`"		/* 96 */
#define MSG_TRACE	"TRACE"		/* TRAC */
#define TOK_TRACE	"b"		/* 97 */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define TOK_SQLINE	"c"		/* 98 */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define TOK_UNSQLINE	"d"		/* 99 */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define TOK_SVSNICK	"e"		/* 100 */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define TOK_SVSNOOP	"f"		/* 101 */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define TOK_IDENTIFY	"g"		/* 102 */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
#define TOK_SVSKILL	"h"		/* 103 */
#define MSG_NICKSERV	"NICKSERV"	/* NICKSERV */
#define MSG_NS		"NS"
#define TOK_NICKSERV	"i"		/* 104 */
#define MSG_CHANSERV	"CHANSERV"	/* CHANSERV */
#define MSG_CS		"CS"
#define TOK_CHANSERV	"j"		/* 105 */
#define MSG_OPERSERV	"OPERSERV"	/* OPERSERV */
#define MSG_OS		"OS"
#define TOK_OPERSERV	"k"		/* 106 */
#define MSG_MEMOSERV	"MEMOSERV"	/* MEMOSERV */
#define MSG_MS		"MS"
#define TOK_MEMOSERV	"l"		/* 107 */
#define MSG_SERVICES	"SERVICES"	/* SERVICES */
#define TOK_SERVICES	"m"		/* 108 */
#define MSG_SVSMODE	"SVSMODE"	/* SVSMODE */
#define TOK_SVSMODE	"n"		/* 109 */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define TOK_SAMODE	"o"		/* 110 */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define TOK_CHATOPS	"p"		/* 111 */
#define MSG_ZLINE    	"ZLINE"		/* ZLINE */
#define TOK_ZLINE	"q"		/* 112 */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */                           
#define TOK_UNZLINE	"r"		/* 113 */
#define MSG_HELPSERV    "HELPSERV"      /* HELPSERV */
#define MSG_HS		"HS"
#define TOK_HELPSERV    "s"             /* 114 */
#define MSG_RULES       "RULES"         /* RULES */
#define TOK_RULES       "t"             /* 115 */
#define MSG_MAP         "MAP"           /* MAP */
#define TOK_MAP         "u"             /* 117 */
#define MSG_SVS2MODE    "SVS2MODE"      /* SVS2MODE */
#define TOK_SVS2MODE	"v"		/* 118 */
#define MSG_DALINFO     "DALINFO"       /* dalinfo */
#define TOK_DALINFO     "w"             /* 119 */
#define MSG_ADMINCHAT   "ADCHAT"        /* Admin chat */
#define TOK_ADMINCHAT   "x"             /* 120 */
#define MSG_MKPASSWD	"MKPASSWD"	/* MKPASSWD */
#define TOK_MKPASSWD	"y"		/* 121 */
#define MSG_ADDLINE     "ADDLINE"       /* ADDLINE */
#define TOK_ADDLINE     "z"             /* 122 */
#define MSG_SNOTES      "SNOTES"        /* Read server notes */
#define TOK_SNOTES      "{"             /* 123 */
#define MSG_SNOTE       "SNOTE"         /* Write server note */
#define TOK_SNOTE       "|"		/* 124 */
#define MSG_GLINE	"GLINE"		/* The awesome g-line */
#define TOK_GLINE	"}"		/* 125 */
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define TOK_REMGLINE	"~"		/* 126 */
#define MSG_SETHOST "SETHOST"   /* sethost */
#define TOK_SETHOST "AA"         /* 127 4ever !;) */
#define MSG_TECHAT  "TECHAT"    /* techadmin chat */
#define TOK_TECHAT  "AB"         /* questionmark? */
#define MSG_NACHAT  "NACHAT"    /* netadmin chat */
#define TOK_NACHAT  "AC"         /* *beep* */
#define MSG_SETIDENT "SETIDENT" /* set ident */
#define	TOK_SETIDENT "AD"        /* good old BASIC ;P */
#define MSG_SETNAME "SETNAME"   /* set GECOS */
#define TOK_SETNAME "AE"		    /* its almost unreeaaall... */
#define MSG_LAG		"LAG"		/* Lag detect */
#define TOK_LAG		"AF"		/* a or ? */
#define MSG_SDESC       "SDESC"     /* set description */
#define TOK_SDESC       "AG"
#define MSG_STATSERV "STATSERV" /* alias */
#define TOK_STATSERV "AH" 
#define MSG_KNOCK "KNOCK"
#define TOK_KNOCK "AI"
#define MSG_CREDITS "CREDITS"
#define TOK_CREDITS "AJ"
#define MSG_LICENSE "LICENSE"
#define TOK_LICENSE "AK"
#define MSG_CHGHOST "CHGHOST"
#define TOK_CHGHOST "AL"
#define MSG_RPING   "RPING"
#define TOK_RPING	"AM"
#define MSG_RPONG   "RPONG"
#define TOK_RPONG   "AN"
#define MSG_NETINFO "NETINFO"
#define TOK_NETINFO "AO"
#define MSG_SENDUMODE "SENDUMODE"
#define TOK_SENDUMODE "AP"
#define MSG_ADDMOTD "ADDMOTD"
#define TOK_ADDMOTD "AQ"
#define MSG_ADDOMOTD "ADDOMOTD"
#define TOK_ADDOMOTD "AR"
#define MSG_SVSMOTD "SVSMOTD"
#define TOK_SVSMOTD "AS"
#define MSG_DUSERS "DUSERS"
#define TOK_DUSERS "AT"
#define MSG_SMO "SMO"
#define TOK_SMO "AU"
#define MSG_OPERMOTD "OPERMOTD"
#define TOK_OPERMOTD "AV"
#define MSG_TSCTL "TSCTL"
#define TOK_TSCTL "AW"
#define MSG_SVSJOIN "SVSJOIN"
#define TOK_SVSJOIN "AX"
#define MSG_SAJOIN "SAJOIN"
#define TOK_SAJOIN "AY"
#define MSG_SVSPART "SVSPART"
#define TOK_SVSPART "AX"
#define MSG_SAPART "SAPART"
#define TOK_SAPART "AY"
#define MSG_CHGIDENT "CHGIDENT"
#define TOK_CHGIDENT "AZ"
#define MSG_SWHOIS "SWHOIS"
#define TOK_SWHOIS "BA"
#define MSG_SVSO "SVSO"
#define TOK_SVSO "BB"

#define	UMODE_INVISIBLE  0x0001 /* makes user invisible */
#define	UMODE_OPER       0x0002	/* Operator */
#define	UMODE_WALLOP     0x0004 /* send wallops to them */
#define UMODE_FAILOP	 0x0008 /* Shows some global messages */
#define UMODE_HELPOP	 0x0010 /* Help system operator */
#define UMODE_REGNICK	 0x0020 /* Nick set by services as registered */
#define UMODE_SADMIN	 0x0040 /* Services Admin */
#define UMODE_ADMIN	 0x0080 /* Admin */

#define	UMODE_SERVNOTICE 0x0100 /* server notices such as kill */
#define	UMODE_LOCOP      0x0200 /* Local operator -- SRB */
#define UMODE_KILLS	 0x0400 /* Show server-kills... */
#define UMODE_CLIENT	 0x0800 /* Show client information */
#define UMODE_FLOOD	 0x1000 /* Receive flood warnings */
#define UMODE_CHATOP	 0x2000 /* can receive chatops */
#define UMODE_SERVICES   0x4000 /* services */
#define UMODE_HIDE	 0x8000 /* Hide from Nukes */
#define UMODE_NETADMIN  0x10000 /* Network Admin */
#define UMODE_EYES      0x20000 /* Mode to see server stuff */
#define UMODE_TECHADMIN 0x40000 /* Tech Admin */
#define UMODE_COADMIN   0x80000 /* Co Admin */
#define UMODE_WHOIS    0x100000 /* gets notice on /whois */
#define UMODE_KIX      0x200000 /* usermode +q 
                                   cannot be kicked from any channel 
                                   except by U:Lines
                                */
#define UMODE_BOT       0x400000 /* User is a bot */
#define UMODE_CODER	0x800000 /* User is a network coder */
#define UMODE_FCLIENT  0x1000000 /* recieve client on far connects.. */
#define UMODE_HIDING   0x2000000 /* Totally invisible .. */
#define UMODE_AGENT    0x4000000 /* Is an IRCd Agent local only */
#define UMODE_DEAF     0x8000000 /* User can't here anything in channel */



#define	MODE_CHANOP		0x0001
#define	MODE_VOICE		0x0002
#define	MODE_PRIVATE		0x0004
#define	MODE_SECRET			0x0008
#define	MODE_MODERATED  	0x0010
#define	MODE_TOPICLIMIT 	0x0020
#define MODE_CHANOWNER		0x0040
#define MODE_CHANPROT		0x0080
#define	MODE_HALFOP			0x0100
#define MODE_EXCEPT			0x0200
#define	MODE_BAN			0x0400
#define	MODE_INVITEONLY 	0x0800
#define	MODE_NOPRIVMSGS 	0x1000
#define	MODE_KEY			0x2000
#define	MODE_LIMIT			0x4000
#define MODE_RGSTR			0x8000
#define MODE_RGSTRONLY 		 	0x10000
#define MODE_LINK			0x20000
#define MODE_NOCOLOR		0x40000
#define MODE_OPERONLY   	0x80000
#define MODE_ADMONLY   		0x100000
#define MODE_NOKICKS   		0x200000
#define MODE_STRIP	   	0x400000
#define MODE_NOKNOCK		0x800000
#define MODE_NOINVITE  		0x1000000
#define MODE_FLOODLIMIT		0x2000000
#define MODE_NOHIDING		0x4000000
#define MODE_STRIPBADWORDS	0x8000000
#define MODE_NOCTCP		0x10000000
#define MODE_AUDITORIUM		0x20000000
#define MODE_ONLYSECURE		0x40000000
#define MODE_NONICKCHANGE	0x80000000


#define is_hidden_chan(x) ((x) && (x->modes & (MODE_PRIVATE|MODE_SECRET|MODE_ADMONLY|MODE_OPERONLY)))



struct ircd_srv_ {
	int uprot;
	char cloak[10];
} ircd_srv;

typedef struct {
	long mode;
	char flag;
	unsigned  nickparam : 1;		/* 1 = yes 0 = no */
	unsigned  parameters : 1; 
} aCtab;





typedef struct {
	long umodes;
	char mode;
	int level;
} Oper_Modes;


aCtab cFlagTab[33];
Oper_Modes usr_mds[27];




/* function declarations */
extern void init_ircd();
extern void notice(char *,char *, ...);
extern int sserver_cmd(const char *, const int numeric, const char *);
extern int slogin_cmd(const char *, const int numeric, const char *, const char *);
extern int ssquit_cmd(const char *);
extern int sprotocol_cmd(const char *);
extern int squit_cmd(const char *, const char *);
extern int spart_cmd(const char *, const char *);
extern int sjoin_cmd(const char *, const char *);
extern int schmode_cmd(const char *, const char *, const char *, const char *);
extern int snewnick_cmd(const char *, const char *, const char *, const char *);
extern int sping_cmd(const char *from, const char *reply, const char *to);
extern int sumode_cmd(const char *who, const char *target, long mode);
extern int snumeric_cmd(const int numeric, const char *target, const char *data,...);
extern int spong_cmd(const char *reply);
extern int snetinfo_cmd();
extern int skill_cmd(const char *from, const char *target, const char *reason,...);
extern int ssmo_cmd(const char *from, const char *umodetarget, const char *msg);
extern int snick_cmd(const char *oldnick, const char *newnick);
extern int sswhois_cmd(const char *target, const char *swhois);
extern int ssvsnick_cmd(const char *target, const char *newnick);
extern int ssvsjoin_cmd(const char *target, const char *chan);
extern int ssvspart_cmd(const char *target, const char *chan);
extern int skick_cmd(const char *who, const char *target, const char *chan, const char *reason);
extern int swallops_cmd(const char *who, const char *msg,...);
#endif  /* UNREAL_H Define */

