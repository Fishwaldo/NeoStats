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
** $Id: Bahamut.h,v 1.5 2003/07/30 13:58:22 fishwaldo Exp $
*/
#ifndef BAHAMUT_H
#define BAHAMUT_H

/* we support tokens */
#undef HAVE_TOKEN_SUP


#define MSG_PRIVATE	"PRIVMSG"	/* PRIV */
#define MSG_WHO		"WHO"	/* WHO  -> WHOC */
#define MSG_WHOIS	"WHOIS"	/* WHOI */
#define MSG_WHOWAS	"WHOWAS"	/* WHOW */
#define MSG_USER	"USER"	/* USER */
#define MSG_NICK	"NICK"	/* NICK */
#define MSG_SERVER	"SERVER"	/* SERV */
#define MSG_LIST	"LIST"	/* LIST */
#define MSG_TOPIC	"TOPIC"	/* TOPI */
#define MSG_INVITE	"INVITE"	/* INVI */
#define MSG_VERSION	"VERSION"	/* VERS */
#define MSG_QUIT	"QUIT"	/* QUIT */
#define MSG_SQUIT	"SQUIT"	/* SQUI */
#define MSG_KILL	"KILL"	/* KILL */
#define MSG_INFO	"INFO"	/* INFO */
#define MSG_LINKS	"LINKS"	/* LINK */
#define MSG_WATCH	"WATCH"	/* WATCH */
#define MSG_STATS	"STATS"	/* STAT */
#define MSG_HELP	"HELP"	/* HELP */
#define MSG_HELPOP	"HELPOP"	/* HELP */
#define MSG_ERROR	"ERROR"	/* ERRO */
#define MSG_AWAY	"AWAY"	/* AWAY */
#define MSG_CONNECT	"CONNECT"	/* CONN */
#define MSG_PING	"PING"	/* PING */
#define MSG_PONG	"PONG"	/* PONG */
#define MSG_OPER	"OPER"	/* OPER */
#define MSG_PASS	"PASS"	/* PASS */
#define MSG_WALLOPS	"WALLOPS"	/* WALL */
#define MSG_TIME	"TIME"	/* TIME */
#define MSG_NAMES	"NAMES"	/* NAME */
#define MSG_ADMIN	"ADMIN"	/* ADMI */
#define MSG_NOTICE	"NOTICE"	/* NOTI */
#define MSG_JOIN	"JOIN"	/* JOIN */
#define MSG_PART	"PART"	/* PART */
#define MSG_LUSERS	"LUSERS"	/* LUSE */
#define MSG_MOTD	"MOTD"	/* MOTD */
#define MSG_MODE	"MODE"	/* MODE */
#define MSG_KICK	"KICK"	/* KICK */
#define MSG_SERVICE	"SERVICE"	/* SERV -> SRVI */
#define MSG_USERHOST	"USERHOST"	/* USER -> USRH */
#define MSG_ISON	"ISON"	/* ISON */
#define MSG_SQUERY	"SQUERY"	/* SQUE */
#define MSG_SERVLIST	"SERVLIST"	/* SERV -> SLIS */
#define MSG_SERVSET	"SERVSET"	/* SERV -> SSET */
#define	MSG_REHASH	"REHASH"	/* REHA */
#define	MSG_RESTART	"RESTART"	/* REST */
#define	MSG_CLOSE	"CLOSE"	/* CLOS */
#define	MSG_DIE		"DIE"	/* DIE */
#define	MSG_HASH	"HASH"	/* HASH */
#define	MSG_DNS		"DNS"	/* DNS  -> DNSS */
#define MSG_SILENCE	"SILENCE"	/* SILE */
#define MSG_AKILL	"AKILL"	/* AKILL */
#define MSG_KLINE	"KLINE"	/* KLINE */
#define MSG_UNKLINE     "UNKLINE"	/* UNKLINE */
#define MSG_RAKILL	"RAKILL"	/* RAKILL */
#define MSG_GNOTICE	"GNOTICE"	/* GNOTICE */
#define MSG_GOPER	"GOPER"	/* GOPER */
#define MSG_GLOBOPS	"GLOBOPS"	/* GLOBOPS */
#define MSG_LOCOPS	"LOCOPS"	/* LOCOPS */
#define MSG_PROTOCTL	"PROTOCTL"	/* PROTOCTL */
#define MSG_TRACE	"TRACE"	/* TRAC */
#define MSG_SQLINE	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE	"UNSQLINE"	/* UNSQLINE */
#define MSG_SVSNICK	"SVSNICK"	/* SVSNICK */
#define MSG_SVSNOOP	"SVSNOOP"	/* SVSNOOP */
#define MSG_IDENTIFY	"IDENTIFY"	/* IDENTIFY */
#define MSG_SVSKILL	"SVSKILL"	/* SVSKILL */
#define MSG_SVSMODE	"SVSMODE"	/* SVSMODE */
#define MSG_SAMODE	"SAMODE"	/* SAMODE */
#define MSG_CHATOPS	"CHATOPS"	/* CHATOPS */
#define MSG_HELPSERV    "HELPSERV"	/* HELPSERV */
#define MSG_ZLINE    	"ZLINE"	/* ZLINE */
#define MSG_UNZLINE  	"UNZLINE"	/* UNZLINE */
#define MSG_NETINFO	"NETINFO"	/* NETINFO */
#define MSG_RULES       "RULES"	/* RULES */
#define MSG_MAP         "MAP"	/* MAP */
#define MSG_NETG	"NETG"	/* NETG */
#define MSG_ADCHAT   	"ADCHAT"	/* Adchat */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */
#define MSG_ADDHUB   	"ADDHUB"	/* ADDHUB */
#define MSG_DELHUB   	"DELHUB"	/* DELHUB */
#define MSG_ADDCNLINE  	"ADDCNLINE"	/* ADDCNLINE */
#define MSG_DELCNLINE  	"DELCNLINE"	/* DELCNLINE */
#define MSG_ADDOPER   	"ADDOPER"	/* ADDOPER */
#define MSG_DELOPER   	"DELOPER"	/* DELOPER */
#define MSG_ADDQLINE   	"ADDQLINE"	/* ADDQLINE */
#define MSG_DELQLINE   	"DELQLINE"	/* DELQLINE */
#define MSG_GSOP    	"GSOP"	/* GSOP */
#define MSG_ISOPER	"ISOPER"	/* ISOPER */
#define MSG_ADG	    	"ADG"	/* ADG */
#define MSG_NMON	"NMON"	/* NMON */
#define MSG_DALINFO	"DALINFO"	/* DALnet Credits */
#define MSG_CREDITS	"CREDITS"	/* UltimateIRCd Credits and "Thanks To" */
#define MSG_OPERMOTD    "OPERMOTD"	/* OPERMOTD */
#define MSG_REMREHASH	"REMREHASH"	/* Remote Rehash */
#define MSG_MONITOR	"MONITOR"	/* MONITOR */
#define MSG_GLINE	"GLINE"	/* The awesome g-line */
#define MSG_REMGLINE	"REMGLINE"	/* remove g-line */
#define MSG_STATSERV	"STATSERV"	/* StatServ */
#define MSG_RULESERV	"RULESERV"	/* RuleServ */
#define MSG_SNETINFO	"SNETINFO"	/* SNetInfo */
#define MSG_TSCTL 	"TSCTL"	/* TSCTL */
#define MSG_SVSJOIN 	"SVSJOIN"	/* SVSJOIN */
#define MSG_SAJOIN 	"SAJOIN"	/* SAJOIN */
#define MSG_SDESC       "SDESC"	/* SDESC */
#define MSG_UNREALINFO	"UNREALINFO"	/* Unreal Info */
#define MSG_SETHOST 	"SETHOST"	/* sethost */
#define MSG_SETIDENT 	"SETIDENT"	/* set ident */
#define MSG_SETNAME 	"SETNAME"	/* set Realname */
#define MSG_CHGHOST 	"CHGHOST"	/* Changehost */
#define MSG_CHGIDENT 	"CHGIDENT"	/* Change Ident */
#define MSG_RANDQUOTE	"RANDQUOTE"	/* Random Quote */
#define MSG_ADDQUOTE	"ADDQUOTE"	/* Add Quote */
#define MSG_ADDGQUOTE	"ADDGQUOTE"	/* Add Global Quote */
#define MSG_ADDULINE	"ADDULINE"	/* Adds an U Line to ircd.conf file */
#define MSG_DELULINE	"DELULINE"	/* Removes an U line from the ircd.conf */
#define MSG_KNOCK	"KNOCK"	/* Knock Knock - Who's there? */
#define MSG_SETTINGS	"SETTINGS"	/* Settings */
#define MSG_IRCOPS	"IRCOPS"	/* Shows Online IRCOps */
#define MSG_SVSPART	"SVSPART"	/* SVSPART */
#define MSG_SAPART	"SAPART"	/* SAPART */
#define MSG_VCTRL	"VCTRL"	/* VCTRL */
#define MSG_GCLIENT	"GCLIENT"	/* GLIENT */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define MSG_UPTIME	"UPTIME"	/* UPTIME */
#define MSG_FAILOPS	"FAILOPS"	/* FAILOPS */
#define MSG_RPING	"RPING"	/* RPING */
#define MSG_RPONG       "RPONG"	/* RPONG */
#define MSG_UPING       "UPING"	/* UPING */
#define MSG_COPYRIGHT	"COPYRIGHT"	/* Copyright */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_RS		"RS"
#define MSG_SVINFO	"SVINFO"
#define MSG_CAPAB	"CAPAB"
#define MSG_BURST	"BURST"
#define MSG_SJOIN	"SJOIN"
#define MSG_CLIENT	"CLIENT"
#define MSG_SMODE	"SMODE"



#define	UMODE_INVISIBLE  	0x0001	/* makes user invisible */
#define	UMODE_OPER       	0x0002	/* Operator */
/* UMODE_SERVICES is actually UMODE_OPER on Bahamut !*/
#define UMODE_SERVICES UMODE_OPER
#define UMODE_REGONLY		0x0010	/* only registered nicks may PM */
#define UMODE_REGNICK	 	0x0020	/* Nick set by services as registered */
#define UMODE_SERVADMIN		0x0040	/* server admin */
#define UMODE_SERVICESADMIN	0x0080	/* Marks the client as a Services Administrator */




#define MODE_CHANOP	0x0001
#define	MODE_VOICE	0x0004
#define	MODE_PRIVATE	0x0008
#define	MODE_SECRET	0x0010
#define	MODE_MODERATED  0x0020
#define	MODE_TOPICLIMIT 0x0040
#define	MODE_INVITEONLY 0x0080
#define	MODE_NOPRIVMSGS 0x0100
#define	MODE_KEY	0x0200
#define	MODE_BAN	0x0800
#define MODE_LIMIT	0x1000
#define MODE_RGSTR	0x2000
#define MODE_RGSTRONLY  0x4000
#define MODE_OPERONLY   0x8000
#define MODE_LINK	0x20000
#define MODE_NOCOLOR	0x40000


#define is_hidden_chan(x) ((x) && (x->modes & (MODE_PRIVATE|MODE_SECRET|MODE_OPERONLY)))
#define is_oper(x) ((x) && ((x->Umode & UMODE_OPER) || (x->Umode & UMODE_LOCOP)))
#define is_pub_chan(x) ((x) && (CheckChanMode(x, MODE_PRIVATE) || CheckChanMode(x, MODE_SECRET) || CheckChanMode(x, MODE_RGSTRONLY) || CheckChanMode(x, MODE_OPERONLY) || CheckChanMode(x, MODE_INVITEONLY) || CheckChanMode(x, MODE_KEY)))


struct ircd_srv_ {
	int uprot;
	int modex;
	int nicklg;
	int gc;
	char cloak[25];
	int burst;
	int cmdcount;
} ircd_srv;

typedef struct {
	long mode;
	char flag;
	unsigned nickparam:1;	/* 1 = yes 0 = no */
	unsigned parameters:1;
	char sjoin;
} aCtab;





typedef struct {
	unsigned long umodes;
	char mode;
	int level;
} Oper_Modes;


aCtab cFlagTab[33];
Oper_Modes usr_mds[29];
Oper_Modes susr_mds[9];



/* function declarations */
extern void init_ircd ();
extern void chanalert (char *, char *, ...);
extern int sserver_cmd (const char *, const int numeric, const char *);
extern int slogin_cmd (const char *, const int numeric, const char *, const char *);
extern int ssquit_cmd (const char *);
extern int sprotocol_cmd (const char *);
extern int squit_cmd (const char *, const char *);
extern int spart_cmd (const char *, const char *);
extern int sjoin_cmd (const char *, const char *, unsigned long flag);
extern int schmode_cmd (const char *, const char *, const char *, const char *);
extern int snewnick_cmd (const char *, const char *, const char *, const char *, long);
extern int sping_cmd (const char *from, const char *reply, const char *to);
extern int sumode_cmd (const char *who, const char *target, long mode);
extern int snumeric_cmd (const int numeric, const char *target, const char *data, ...);
extern int spong_cmd (const char *reply);
extern int snetinfo_cmd ();
extern int skill_cmd (const char *from, const char *target, const char *reason, ...);
extern int ssmo_cmd (const char *from, const char *umodetarget, const char *msg);
extern int snick_cmd (const char *oldnick, const char *newnick);
extern int sswhois_cmd (const char *target, const char *swhois);
extern int ssvsnick_cmd (const char *target, const char *newnick);
extern int ssvsjoin_cmd (const char *target, const char *chan);
extern int ssvspart_cmd (const char *target, const char *chan);
extern int ssvshost_cmd (const char *who, const char *vhost);
extern int skick_cmd (const char *who, const char *target, const char *chan, const char *reason);
extern int swallops_cmd (const char *who, const char *msg, ...);
extern int vctrl_cmd ();
extern int ssvinfo_cmd ();
extern int sburst_cmd (int b);
extern int sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...);
extern int srakill_cmd (const char *host, const char *ident);
extern int ssvshost_cmd (const char *who, const char *vhost);
extern int ssvskill_cmd (const char *who, const char *reason, ...);

void Usr_Version (char *, char **, int argc);
void Usr_ShowMOTD (char *, char **, int argc);
void Usr_ShowADMIN (char *, char **, int argc);
void Usr_Showcredits (char *, char **, int argc);
void Usr_AddServer (char *, char **, int argc);
void Usr_DelServer (char *, char **, int argc);
void Usr_DelUser (char *, char **, int argc);
void Usr_Mode (char *, char **, int argc);
void Usr_Smode (char *, char **, int argc);
void Usr_Kill (char *, char **, int argc);
void Usr_Pong (char *, char **, int argc);
void Usr_Away (char *, char **, int argc);
void Usr_Nick (char *, char **, int argc);
void Usr_Topic (char *, char **, int argc);
void Usr_Kick (char *, char **, int argc);
void Usr_Join (char *, char **, int argc);
void Usr_Part (char *, char **, int argc);
void Usr_Stats (char *, char **, int argc);
void Usr_Vhost (char *, char **, int argc);
void Srv_Topic (char *, char **, int argc);
void Srv_Ping (char *, char **, int argc);
void Srv_Netinfo (char *, char **, int argc);
void Srv_Pass (char *, char **, int argc);
void Srv_Server (char *, char **, int argc);
void Srv_Squit (char *, char **, int argc);
void Srv_Nick (char *, char **, int argc);
void Srv_Svsnick (char *, char **, int argc);
void Srv_Kill (char *, char **, int argc);
void Srv_Connect (char *, char **, int argc);
void Srv_Svinfo (char *, char **, int argc);
void Srv_Burst (char *origin, char **argv, int argc);
void Srv_Sjoin (char *origin, char **argv, int argc);
void Srv_Tburst (char *origin, char **argv, int argc);
void Srv_Vctrl (char *origin, char **argv, int argc);
void Srv_Client (char *origin, char **argv, int argc);
void Srv_Smode (char *origin, char **argv, int argc);
int SignOn_NewBot (const char *, const char *, const char *, const char *, long);


#endif
