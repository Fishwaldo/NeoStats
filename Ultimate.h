#ifndef ULTIMATE_H
#define ULTIMATE_H


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
#define MSG_WATCH	"WATCH"		/* WATCH */
#define TOK_WATCH	"1"		/* 49 */
#define MSG_STATS	"STATS"		/* STAT */
#define TOK_STATS	"2"		/* 50 */
#define MSG_HELP	"HELP"		/* HELP */
#define MSG_HELPOP	"HELPOP"	/* HELP */
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
#define MSG_TRACE	"TRACE"		/* TRAC */
#define TOK_TRACE	"b"		/* 98 */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define TOK_SQLINE	"c"		/* 99 */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define TOK_UNSQLINE	"d"		/* 100 */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define TOK_SVSNICK	"e"		/* 101 */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define TOK_SVSNOOP	"f"		/* 101 */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define TOK_IDENTIFY	"g"		/* 103 */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
#define TOK_SVSKILL	"h"		/* 104 */
#define MSG_NICKSERV	"NICKSERV"	/* NICKSERV */
#define MSG_NS		"NS"
#define TOK_NICKSERV	"i"		/* 105 */
#define MSG_CHANSERV	"CHANSERV"	/* CHANSERV */
#define MSG_CS		"CS"
#define TOK_CHANSERV	"j"		/* 106 */
#define MSG_OPERSERV	"OPERSERV"	/* OPERSERV */
#define MSG_OS		"OS"
#define TOK_OPERSERV	"k"		/* 107 */
#define MSG_MEMOSERV	"MEMOSERV"	/* MEMOSERV */
#define MSG_MS		"MS"
#define TOK_MEMOSERV	"l"		/* 108 */
#define MSG_SERVICES	"SERVICES"	/* SERVICES */
#define TOK_SERVICES	"m"		/* 109 */
#define MSG_SVSMODE	"SVSMODE"	/* SVSMODE */
#define TOK_SVSMODE	"n"		/* 110 */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define TOK_SAMODE	"o"		/* 111 */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define TOK_CHATOPS	"p"		/* 112 */
#define MSG_HELPSERV    "HELPSERV"      /* HELPSERV */
#define TOK_HELPSERV    "r"             /* 114 */
#define MSG_ZLINE    	"ZLINE"		/* ZLINE */
#define TOK_ZLINE	"s"		/* 115 */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */                           
#define TOK_UNZLINE	"t"		/* 116 */
#define MSG_NETINFO	"NETINFO"	/* NETINFO */
#define TOK_NETINFO	"u"		/* 117 */
#define MSG_RULES       "RULES"         /* RULES */
#define TOK_RULES       "v"             /* 118 */
#define MSG_MAP         "MAP"           /* MAP */
#define TOK_MAP         "w"             /* 119 */
#define MSG_NETG	"NETG"		/* NETG */
#define TOK_NETG	"x"		/* 120 */
#define MSG_ADCHAT   	"ADCHAT"        /* Adchat */
#define TOK_ADCHAT   	"y"             /* 121 */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */
#define TOK_MAKEPASS	"z"		/* 122 */
#define MSG_ADDHUB   	"ADDHUB"	/* ADDHUB */
#define TOK_ADDHUB	"{"		/* 123 */
#define MSG_DELHUB   	"DELHUB"	/* DELHUB */
#define TOK_DELHUB	"|"		/* 124 */
#define MSG_ADDCNLINE  	"ADDCNLINE"	/* ADDCNLINE */
#define TOK_ADDCNLINE	"}"		/* 125 */
#define MSG_DELCNLINE  	"DELCNLINE"	/* DELCNLINE */
#define TOK_DELCNLINE	"~"		/* 126 */
#define MSG_ADDOPER   	"ADDOPER"	/* ADDOPER */
#define TOK_ADDOPER	""		/* 127 */ 
#define MSG_DELOPER   	"DELOPER"	/* DELOPER */
#define TOK_DELOPER	"!!"		/* 33 + 33 */
#define MSG_ADDQLINE   	"ADDQLINE"	/* ADDQLINE */
#define TOK_ADDQLINE	"!\""		/* 33 + 34 */
#define MSG_DELQLINE   	"DELQLINE"	/* DELQLINE */
#define TOK_DELQLINE	"!#"		/* 33 + 35 */
#define MSG_GSOP    	"GSOP"		/* GSOP */
#define TOK_GSOP	"!$"		/* 33 + 36 */
#define MSG_ISOPER	"ISOPER"        /* ISOPER */
#define TOK_ISOPER	"!%"		/* 33 + 37 */
#define MSG_ADG	    	"ADG"		/* ADG */
#define TOK_ADG     	"!&"		/* 33 + 38 */
#define MSG_NMON	"NMON"		/* NMON */
#define TOK_NMON	"!'"		/* 33 + 39 */
#define MSG_DALINFO	"DALINFO"	/* DALnet Credits */
#define TOK_DALINFO	"!("		/* 33 + 40 */
#define MSG_CREDITS	"CREDITS"	/* UltimateIRCd Credits and "Thanks To" */
#define TOK_CREDITS	"!)"		/* 33 + 41 */
#define MSG_OPERMOTD    "OPERMOTD"      /* OPERMOTD */
#define TOK_OPERMOTD    "!*"            /* 33 + 42 */
#define MSG_REMREHASH	"REMREHASH"	/* Remote Rehash */
#define TOK_REMREHASH   "!+"		/* 33 + 43 */
#define MSG_MONITOR	"MONITOR"	/* MONITOR */
#define TOK_MONITOR     "!,"		/* 33 + 44 */
#define MSG_GLINE	"GLINE"		/* The awesome g-line */
#define TOK_GLINE	"!-"		/* 33 + 45 */
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define TOK_REMGLINE	"!."		/* 33 + 46 */
#define MSG_STATSERV	"STATSERV"	/* StatServ */
#define TOK_STATSERV	"!/"		/* 33 + 47 */
#define MSG_RULESERV	"RULESERV"	/* RuleServ */
#define TOK_RULESERV	"!0"		/* 33 + 48 */
#define MSG_SNETINFO	"SNETINFO"	/* SNetInfo */
#define TOK_SNETINFO	"!1"		/* 33 + 49 */
#define MSG_TSCTL 	"TSCTL"		/* TSCTL */
#define TOK_TSCTL 	"!3"		/* 33 + 51 */
#define MSG_SVSJOIN 	"SVSJOIN"	/* SVSJOIN */
#define TOK_SVSJOIN 	"!4"		/* 33 + 52 */
#define MSG_SAJOIN 	"SAJOIN"	/* SAJOIN */
#define TOK_SAJOIN 	"!5"		/* 33 + 53 */
#define MSG_SDESC       "SDESC"     	/* SDESC */
#define TOK_SDESC       "!6"		/* 33 + 54 */
#define MSG_UNREALINFO	"UNREALINFO"	/* Unreal Info */
#define TOK_UNREALINFO	"!7"		/* 33 + 55 */
#define MSG_SETHOST 	"SETHOST"   	/* sethost */
#define TOK_SETHOST 	"!8"         	/* 33 + 56 */
#define MSG_SETIDENT 	"SETIDENT" 	/* set ident */
#define	TOK_SETIDENT	"!9"        	/* 33 + 57 */
#define MSG_SETNAME 	"SETNAME"   	/* set Realname */
#define TOK_SETNAME 	"!;"		/* 33 + 59 */
#define MSG_CHGHOST 	"CHGHOST"	/* Changehost */
#define TOK_CHGHOST 	"!<"		/* 33 + 60 */
#define MSG_CHGIDENT 	"CHGIDENT"	/* Change Ident */
#define TOK_CHGIDENT 	"!="		/* 33 + 61 */
#define MSG_RANDQUOTE	"RANDQUOTE"	/* Random Quote */
#define TOK_RANDQUOTE	"!>"		/* 33 + 62 */
#define MSG_ADDQUOTE	"ADDQUOTE"	/* Add Quote */
#define TOK_ADDQUOTE	"!?"		/* 33 + 63 */
#define MSG_ADDGQUOTE	"ADDGQUOTE"	/* Add Global Quote */
#define TOK_ADDGQUOTE	"!@"		/* 33 + 64 */
#define MSG_ADDULINE	"ADDULINE"	/* Adds an U Line to ircd.conf file */
#define TOK_ADDULINE	"!B"		/* 33 + 66 */
#define MSG_DELULINE	"DELULINE"	/* Removes an U line from the ircd.conf */
#define TOK_DELULINE	"!C"		/* 33 + 67 */
#define MSG_KNOCK	"KNOCK"		/* Knock Knock - Who's there? */
#define TOK_KNOCK	"!D"		/* 33 + 68 */
#define MSG_SETTINGS	"SETTINGS"	/* Settings */
#define TOK_SETTINGS	"!E"		/* 33 + 69 */
#define MSG_IRCOPS	"IRCOPS"	/* Shows Online IRCOps */
#define TOK_IRCOPS	"!F"		/* 33 + 70 */
#define MSG_SVSPART	"SVSPART"	/* SVSPART */
#define TOK_SVSPART	"!G"		/* 33 + 71 */
#define MSG_SAPART	"SAPART"	/* SAPART */
#define TOK_SAPART	"!H"		/* 33 + 72 */
#define MSG_VCTRL	"VCTRL"		/* VCTRL */
#define TOK_VCTRL	"!I"		/* 33 + 73 */
#define MSG_GCLIENT	"GCLIENT"	/* GLIENT */
#define TOK_GCLIENT	"!J"		/* 33 + 74 */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define TOK_CHANNEL	"!K"		/* 33 + 75 */
#define MSG_UPTIME	"UPTIME"	/* UPTIME */
#define TOK_UPTIME	"!L"		/* 33 + 76 */
#define MSG_FAILOPS	"FAILOPS"	/* FAILOPS */
#define TOK_FAILOPS	"!M"		/* 33 + 77 */

#define MSG_RPING	"RPING"		/* RPING */
#define TOK_RPING       "!P"         	/* 33 + 80 */
#define MSG_RPONG       "RPONG"         /* RPONG */
#define TOK_RPONG       "!Q"         	/* 33 + 81 */
#define MSG_UPING       "UPING"         /* UPING */
#define TOK_UPING       "!R"            /* 33 + 82 */
#define MSG_COPYRIGHT	"COPYRIGHT"	/* Copyright */
#define TOK_COPYRIGHT	"!S"		/* 33 + 83 */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_BS		"BS"
#define TOK_BOTSERV	"!T"		/* 33 + 84 */
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_RS		"RS"
#define TOK_ROOTSERV	"!U"		/* 33 + 85 */
#define MSG_SVINFO	"SVINFO"	
#define MSG_CAPAB	"CAPAB"
#define MSG_BURST	"BURST"
#define MSG_SJOIN	"SJOIN"



#define	UMODE_INVISIBLE  	0x0001 /* makes user invisible */
#define	UMODE_OPER       	0x0002	/* Operator */
#define	UMODE_WALLOP     	0x0004 /* send wallops to them */
#define UMODE_FAILOP	 	0x0008 /* Shows some global messages */
#define UMODE_HELPOP	 	0x0010 /* Help system operator */
#define UMODE_REGNICK	 	0x0020 /* Nick set by services as registered */
#define UMODE_SERVICESOPER	0x0040 /* Services Oper */
#define UMODE_ADMIN	 	0x0080 /* Admin */

#define	UMODE_SERVNOTICE 	0x0100 /* server notices such as kill */
#define	UMODE_LOCOP      	0x0200 /* Local operator -- SRB */
#define UMODE_KILLS	 	0x0400 /* Show server-kills... */
#define UMODE_CLIENT	 	0x0800 /* Show client information */
#define UMODE_FLOOD	 	0x1000 /* Receive flood warnings */
#define UMODE_CHATOP	 	0x2000 /* can receive chatops */
#define UMODE_SERVICES   	0x4000 /* services */
#define UMODE_HIDE	 	0x8000 /* Hide from Nukes */
#define UMODE_NETADMIN  	0x10000 /* Network Admin */
#define	UMODE_SUPER		0x20000 /* Oper Is Protected from Kick's and Kill's */
#define UMODE_RBOT      	0x40000 /* Marks the client as a Registered Bot */
#define UMODE_SBOT      	0x80000 /* Marks the client as a Server Bot */
#define UMODE_NGLOBAL  		0x100000 /* See Network Globals */
#define UMODE_WHOIS    		0x200000 /* Lets Opers see when people do a /WhoIs on them */
#define UMODE_NETINFO  		0x400000 /* Server link, Delink Notces etc. */
#define UMODE_MAGICK   		0x800000 /* Allows Opers To See +s and +p Channels */
#define UMODE_IRCADMIN 		0x1000000 /* Marks the client as an IRC Administrator */
#define UMODE_SERVICESADMIN	0x2000000 /* Marks the client as a Services Administrator */
#define UMODE_WATCHER		0x4000000 /* Recive Monitor Globals */
#define UMODE_NETMON		0x8000000 /* Marks the client as an Network Monitor */
#define UMODE_SERVADMIN		0x10000000 /* Marks the client as a Server Admin */
#define UMODE_TECHADMIN		0x20000000 /* Marks the client as a Technical Admin */
#define UMODE_DEAF		0x40000000 /* client is deaf on channels */

#define	MODE_CHANOP	0x0001
#define MODE_HALFOP	0x0002
#define	MODE_VOICE	0x0004
#define	MODE_PRIVATE	0x0008
#define	MODE_SECRET	0x0010
#define	MODE_MODERATED  0x0020
#define	MODE_TOPICLIMIT 0x0040
#define	MODE_INVITEONLY 0x0080
#define	MODE_NOPRIVMSGS 0x0100
#define	MODE_KEY	0x0200
#define MODE_EXCEPT	0x0400
#define	MODE_BAN	0x0800
#define	MODE_LIMIT	0x1000
#define MODE_RGSTR	0x2000
#define MODE_RGSTRONLY  0x4000
#define MODE_OPERONLY   0x8000
#define MODE_ADMONLY   	0x10000
#define MODE_LINK	0x20000
#define MODE_NOCOLOR	0x40000
#define MODE_STRIP	0x80000
#define MODE_NOKNOCK	0x100000
#define MODE_NOINVITE  	0x200000
#define MODE_FLOODLIMIT 0x400000
#define MODE_CHANADMIN  0x800000


#define is_hidden_chan(x) ((x) && (x->modes & (MODE_PRIVATE|MODE_SECRET|MODE_ADMONLY|MODE_OPERONLY)))
#define is_oper(x) ((x) && ((x->Umode & UMODE_OPER) || (x->Umode & UMODE_LOCOP)))


struct ircd_srv_ {
	int uprot;
	int modex;
	int nicklg;
	int gc;	
	char cloak[25];
	int burst;
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
extern void chanalert(char *,char *, ...);
extern int sserver_cmd(const char *, const int numeric, const char *);
extern int slogin_cmd(const char *, const int numeric, const char *, const char *);
extern int ssquit_cmd(const char *);
extern int sprotocol_cmd(const char *);
extern int squit_cmd(const char *, const char *);
extern int spart_cmd(const char *, const char *);
extern int sjoin_cmd(const char *, const char *);
extern int schmode_cmd(const char *, const char *, const char *, const char *);
#ifndef ULTIMATE3
extern int snewnick_cmd(const char *, const char *, const char *, const char *);
#else
extern int snewnick_cmd(const char *, const char *, const char *, const char *, long mode);
#endif
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
extern int vctrl_cmd();
extern int ssvinfo_cmd();
extern int sburst_cmd(int b);
#endif
