/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
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
** $Id: inspircd.c 3236 2007-11-07 04:02:54Z Fish $
*/

#define _GNU_SOURCE
#include "neostats.h"
#include "protocol.h"
#include "support.h"
#include "services.h"

/* Umodes */				
#define UMODE_FAILOP		0x00200000
#define UMODE_SERVNOTICE	0x00400000	
#define UMODE_NOCTCP		0x00800000
#define UMODE_SMODE			0x01000000
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

#define CUMODE_NOTHING		0x0


/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";/* PRIV */
char *MSG_WHOIS = "WHOIS";/* WHOI */
char *MSG_WHOWAS = "WHOWAS";/* WHOW */
char *MSG_USER = "USER";/* USER */
char *MSG_NICK = "NICK";/* NICK */
char *MSG_SERVER = "SERVER";/* SERV */
char *MSG_LIST = "LIST";/* LIST */
char *MSG_TOPIC = "TOPIC";/* TOPI */
char *MSG_INVITE = "INVITE";/* INVI */
char *MSG_VERSION = "VERSION";/* VERS */
char *MSG_QUIT = "QUIT";/* QUIT */
char *MSG_SQUIT = "SQUIT";/* SQUI */
char *MSG_KILL = "KILL";/* KILL */
char *MSG_INFO = "INFO";/* INFO */
char *MSG_LINKS = "LINKS";/* LINK */
char *MSG_SUMMON = "SUMMON";/* SUMM */
char *MSG_STATS = "STATS";/* STAT */
char *MSG_USERS = "USERS";/* USER -> USRS */
char *MSG_HELP = "HELP";/* HELP */
char *MSG_HELPOP = "HELPOP";/* HELP */
char *MSG_ERROR = "ERROR";/* ERRO */
char *MSG_AWAY = "AWAY";/* AWAY */
char *MSG_CONNECT = "CONNECT";/* CONN */
char *MSG_PING = "PING";/* PING */
char *MSG_PONG = "PONG";/* PONG */
char *MSG_OPER = "OPER";/* OPER */
char *MSG_PASS = "PASS";/* PASS */
char *MSG_WALLOPS = "WALLOPS";/* WALL */
char *MSG_TIME = "TIME";/* TIME */
char *MSG_NAMES = "NAMES";/* NAME */
char *MSG_ADMIN = "ADMIN";/* ADMI */
char *MSG_NOTICE = "NOTICE";/* NOTI */
char *MSG_JOIN = "JOIN";/* JOIN */
char *MSG_PART = "PART";/* PART */
char *MSG_LUSERS = "LUSERS";/* LUSE */
char *MSG_MOTD = "MOTD";/* MOTD */
char *MSG_MODE = "MODE";/* MODE */
char *MSG_KICK = "KICK";/* KICK */
char *MSG_SERVICE = "SERVICE";/* SERV -> SRVI */
char *MSG_USERHOST = "USERHOST";/* USER -> USRH */
char *MSG_ISON = "ISON";/* ISON */
char *MSG_REHASH = "REHASH";/* REHA */
char *MSG_RESTART = "RESTART";/* REST */
char *MSG_CLOSE = "CLOSE";/* CLOS */
char *MSG_DIE = "DIE";/* DIE */
char *MSG_HASH = "HASH";/* HASH */
char *MSG_DNS = "DNS";/* DNS -> DNSS */
char *MSG_SILENCE = "SILENCE";/* SILE */
char *MSG_AKILL = "AKILL";/* AKILL */
char *MSG_KLINE = "KLINE";/* KLINE */
char *MSG_UNKLINE = "UNKLINE";/* UNKLINE */
char *MSG_RAKILL = "RAKILL";/* RAKILL */
char *MSG_GNOTICE = "GNOTICE";/* GNOTICE */
char *MSG_GOPER = "GOPER";/* GOPER */
char *MSG_GLOBOPS = "GLOBOPS";/* GLOBOPS */
char *MSG_LOCOPS = "LOCOPS";/* LOCOPS */
char *MSG_PROTOCTL = "CAPAB";/* CAPAB */
char *MSG_WATCH = "WATCH";/* WATCH */
char *MSG_TRACE = "TRACE";/* TRAC */
char *MSG_SQLINE = "SQLINE";/* SQLINE */
char *MSG_UNSQLINE = "UNSQLINE";/* UNSQLINE */
char *MSG_SVSNICK = "SVSNICK";/* SVSNICK */
char *MSG_SVSNOOP = "SVSNOOP";/* SVSNOOP */
char *MSG_IDENTIFY = "IDENTIFY";/* IDENTIFY */
char *MSG_SVSKILL = "SVSKILL";/* SVSKILL */
char *MSG_NICKSERV = "NICKSERV";/* NICKSERV */
char *MSG_NS = "NS";
char *MSG_CHANSERV = "CHANSERV";/* CHANSERV */
char *MSG_CS = "CS";
char *MSG_OPERSERV = "OPERSERV";/* OPERSERV */
char *MSG_OS = "OS";
char *MSG_MEMOSERV = "MEMOSERV";/* MEMOSERV */
char *MSG_MS = "MS";
char *MSG_SERVICES = "SERVICES";/* SERVICES */
char *MSG_SVSMODE = "SVSMODE";/* SVSMODE */
char *MSG_SAMODE = "SAMODE";/* SAMODE */
char *MSG_CHATOPS = "CHATOPS";/* CHATOPS */
char *MSG_ZLINE = "ZLINE";/* ZLINE */
char *MSG_UNZLINE = "UNZLINE";/* UNZLINE */
char *MSG_HELPSERV = "HELPSERV";/* HELPSERV */
char *MSG_HS = "HS";
char *MSG_RULES = "RULES";/* RULES */
char *MSG_MAP = "MAP";/* MAP */
char *MSG_SVS2MODE = "SVS2MODE";/* SVS2MODE */
char *MSG_DALINFO = "DALINFO";/* dalinfo */
char *MSG_ADMINCHAT = "ADCHAT";/* Admin chat */
char *MSG_MKPASSWD = "MKPASSWD";/* MKPASSWD */
char *MSG_ADDLINE = "ADDLINE";/* ADDLINE */
char *MSG_GLINE = "GLINE";/* The awesome g-line */
char *MSG_SJOIN = "SJOIN";
char *MSG_SETHOST = "SETHOST";/* sethost */
char *MSG_NACHAT = "NACHAT";/* netadmin chat */
char *MSG_SETIDENT = "SETIDENT";
char *MSG_SETNAME = "SETNAME";/* set GECOS */
char *MSG_LAG = "LAG";/* Lag detect */
char *MSG_STATSERV = "STATSERV";/* alias */
char *MSG_KNOCK = "KNOCK";
char *MSG_CREDITS = "CREDITS";
char *MSG_LICENSE = "LICENSE";
char *MSG_CHGHOST = "CHGHOST";
char *MSG_RPING = "RPING";
char *MSG_RPONG = "RPONG";
char *MSG_NETINFO = "NETINFO";
char *MSG_SENDUMODE = "SENDUMODE";
char *MSG_ADDMOTD = "ADDMOTD";
char *MSG_ADDOMOTD = "ADDOMOTD";
char *MSG_SVSMOTD = "SVSMOTD";
char *MSG_SMO = "SMO";
char *MSG_OPERMOTD = "OPERMOTD";
char *MSG_TSCTL = "TSCTL";
char *MSG_SVSJOIN = "SVSJOIN";
char *MSG_SAJOIN = "SAJOIN";
char *MSG_SVSPART = "SVSPART";
char *MSG_SAPART = "SAPART";
char *MSG_CHGIDENT = "CHGIDENT";
char *MSG_SWHOIS = "SWHOIS";
char *MSG_SVSO = "SVSO";
char *MSG_SVSFLINE = "SVSFLINE";
char *MSG_TKL = "TKL";
char *MSG_VHOST = "VHOST";
char *MSG_BOTMOTD = "BOTMOTD";
char *MSG_REMGLINE = "REMGLINE";/* remove g-line */
char *MSG_HTM = "HTM";
char *MSG_UMODE2 = "UMODE2";
char *MSG_DCCDENY = "DCCDENY";
char *MSG_UNDCCDENY = "UNDCCDENY";
char *MSG_CHGNAME = "CHGNAME";
char *MSG_SVSNAME = "SVSNAME";
char *MSG_SHUN = "SHUN";
char *MSG_NEWJOIN = "NEWJOIN";/* For CR Java Chat */
char *MSG_POST = "POST";
char *MSG_INFOSERV = "INFOSERV";
char *MSG_IS = "IS";
char *MSG_BOTSERV = "BOTSERV";
char *MSG_CYCLE = "CYCLE";
char *MSG_MODULE = "MODULE";
char *MSG_SENDSNO = "SENDSNO";
char *MSG_BURST = "BURST";
char *MSG_EOS = "ENDBURST";
char *MSG_FJOIN = "FJOIN";
char *MSG_FMODE = "FMODE";
char *MSG_FTOPIC = "FTOPIC";
char *MSG_FNAME	= "FNAME";
char *MSG_FHOST	= "FHOST";

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_capab( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_endburst ( char *origin, char **argv, int argc, int srv );
static void m_nick ( char *origin, char **argv, int argc, int srv);
static void m_fjoin ( char *origin, char **argv, int argc, int srv);
static void m_fmode ( char *origin, char **argv, int argc, int srv);
static void m_topic ( char *origin, char **argv, int argc, int srv);
static void m_ftopic ( char *origin, char **argv, int argc, int srv);
static void m_fname ( char *origin, char **argv, int argc, int srv);
static void m_quit ( char *origin, char **argv, int argc, int srv);
static void m_fhost ( char *origin, char **argv, int argc, int srv);

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_NICKIP | PROTOCOL_NOQUIT | PROTOCOL_EOB |PROTOCOL_SJOIN,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	30,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	32,
	/* Max topic length */
	307,
	/* Default operator modes for NeoStats service bots */
	"+O",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

irc_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{&MSG_SERVER, 	NULL, 	m_server, 	0},
	{&MSG_PROTOCTL, NULL, 	m_capab, 	0},
	{&MSG_BURST, 	NULL, 	m_burst,	0},
	{&MSG_EOS,	NULL, 	m_endburst, 	0},
	{&MSG_NICK,	NULL,	m_nick,		0},
	{&MSG_FJOIN,	NULL, 	m_fjoin,	0},
	{&MSG_FMODE, 	NULL, 	m_fmode,	0},
	{&MSG_TOPIC,	NULL,	m_topic, 	0},
	{&MSG_FTOPIC,	NULL,	m_ftopic, 	0},
	{&MSG_FNAME,	NULL,	m_fname,	0},
	{&MSG_QUIT,	NULL,   m_quit, 	0},
	{&MSG_CHGHOST,	NULL,   NULL,		0},
	{&MSG_FHOST,	NULL,	m_fhost,	0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANPROT, 0, '&'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	/* this shouldn't break stuff, but will have to see */
	{' ', CUMODE_NOTHING, 0, ','},
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0, 0},
	{'R', CMODE_RGSTRONLY, 0, 0},
	{'c', CMODE_NOCOLOR, 0, 0},
	{'O', CMODE_OPERONLY, 0, 0},
/*	{'A', CMODE_ADMONLY, 0, 0}, */
	{'L', CMODE_LINK, MODEPARAM, 0},
	{'Q', CMODE_NOKICKS, 0, 0},
	{'S', CMODE_STRIP, 0, 0},
	{'e', CMODE_EXCEPT, MODEPARAM, 0},
	{'K', CMODE_NOKNOCK, 0, 0},
	{'V', CMODE_NOINVITE, 0, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM, 0},
	{'M', CMODE_MODREG, 0, 0},
	{'G', CMODE_STRIPBADWORDS, 0, 0},
	{'C', CMODE_NOCTCP, 0, 0},
/*	{'u', CMODE_AUDITORIUM, 0, 0}, */
	{'z', CMODE_ONLYSECURE, 0, 0},
	{'N', CMODE_NONICKCHANGE, 0, 0},
	MODE_INIT_END()
};

mode_init user_umodes[] =
{
	{'r', UMODE_REGNICK, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'h', UMODE_HELPOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'B', UMODE_BOT, 0, 0},
 	{'d', UMODE_DEAF, 0, 0},
	{'R', UMODE_RGSTRONLY, 0, 0},
	{'H', UMODE_HIDEOPER, 0, 0},
	{'G', UMODE_STRIPBADWORDS, 0, 0},
	{'x', UMODE_HIDE, 0, 0},
	{'W', UMODE_WHOIS, 0, 0},
	{'n', UMODE_SMODE, 0, 0},
	MODE_INIT_END()
};

/** @brief parse
 *
 *  parser for inspircd to handle PUSH messages
 *
 *  @param notused
 *  @param rline
 *  @param len
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int parse( void *notused, void *rline, int len )
{
	char origin[64], cmd[64], *coreLine;
	char *origline, *coreLine2;
	char *line = (char *)rline;
	int cmdptr = 0;
	int ac = 0;
	int ret = NS_FAILURE;
	char **av = NULL;

	SET_SEGV_LOCATION();
	if( *line == '\0' )
		return NS_FAILURE;
	origline = strdup(line);
	if( *line == ':' )
	{
		coreLine = strpbrk( line, " " );
		if( coreLine == NULL )
			return NS_FAILURE;
		*coreLine = 0;
		while( isspace( *++coreLine ) )
			;
		strlcpy( origin, line + 1, sizeof( origin ) );
		memmove( line, coreLine, strnlen( coreLine, BUFSIZE ) + 1 );
		cmdptr = 1;
	}
	else
	{
		cmdptr = 0;
		*origin = 0;
	}
	if( *line == '\0' ) {
		ns_free(origline);
		return NS_FAILURE;
	}
	coreLine = strpbrk( line, " " );
	if( coreLine )
	{
		*coreLine = 0;
		while( isspace( *++coreLine ) )
			;
	}
	else
	{
		coreLine = line + strlen( line );
	}
	strlcpy( cmd, line, sizeof( cmd ) ); 
	coreLine2 = strdup(coreLine);
	ac = ircsplitbuf( coreLine, &av, 1 );
	if (!ircstrcasecmp(line, "PUSH")) {
		ret = parse(NULL, av[1], strlen(av[1]));
		ns_free(av);
		ns_free(origline);
		return ret;
	}
	dlog( DEBUG1, "------------------------BEGIN PARSE-------------------------" );
	dlog( DEBUGRX, "%s", origline );
	dlog( DEBUG1, "origin: %s", origin );
	dlog( DEBUG1, "cmd   : %s", cmd );
	dlog( DEBUG1, "args  : %s", coreLine2 );
	process_ircd_cmd( cmdptr, cmd, origin, av, ac );
	ns_free( av );
	ns_free(origline);
	ns_free(coreLine2);
	dlog( DEBUG1, "-------------------------END PARSE--------------------------" );
	return NS_SUCCESS;
}



void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	 /*
	 * Sent: SERVER services-dev.chatspike.net password 0 :IRCServices test server
	 * Sent: BURST 1133994664
	 */
	send_cmd( "SERVER %s %s 0 :%s", name, pass, infoline );
	send_cmd( "BURST %ld", (long)me.now);
}

/** m_server
 *
 *  process SERVER command
 *  RX:
 *    :test.chatspike.net SERVER test2.chatspike.net * 1 :Second server
 *  Format:
 *    SERVER servername * hopcount :serverdesc
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = servername
 *    argv[2] = hopcount
 *    argv[3] = serverinfo
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_server( char *origin, char **argv, int argc, int srv )
{
	if (*origin != 0) 
		do_server( argv[0], origin, argv[2], NULL, argv[3], srv);
	else {
		do_server( argv[0], me.name, argv[2], NULL, argv[3], srv);
		me.s->uplink = FindServer(argv[0]);
	}
}
/** m_capab
 *
 *  process CAPAB command
 *  RX:
 * 	Received: CAPAB START
 *	Received: CAPAB CAPABILITIES :NICKMAX=32 HALFOP=1 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=255
 * 	Received: CAPAB CAPABILITIES :MAXTOPIC=307 MAXKICK=255 MAXGECOS=128 MAXAWAY=200 IP6NATIVE=0 IP6SUPPORT=1 PROTOCOL=1100
 *	Received: CAPAB MODULES m_banexception.so,m_blockcolor.so,m_botmode.so,m_censor.so,m_chanfilter.so,m_chanprotect.so,m_cloaking.so
 *	Received: CAPAB MODULES m_globops.so,m_httpd.so,m_httpd_stats.so,m_inviteexception.so,m_joinflood.so
 *	Received: CAPAB MODULES m_kicknorejoin.so,m_knock.so,m_messageflood.so,m_noctcp.so,m_noinvite.so,m_nokicks.so,m_nonicks.so,m_nonotice.so,m_operchans.so
 *	Received: CAPAB MODULES m_redirect.so,m_services.so,m_spanningtree.so,m_stripcolor.so,m_testcommand.so,m_tline.so,m_uninvite.so
 *	Received: CAPAB END
 *  Format:
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_capab( char *origin, char **argv, int argc, int srv )
{
/* nothing yet */
}

static void m_burst( char *origin, char **argv, int argc, int srv )
{
/* nothing */
}

static void m_endburst( char *origin, char **argv, int argc, int srv )
{
	do_eos( me.s->uplink->name );
	send_cmd ("ENDBURST");
	do_synch_neostats();
}

/* 
 * :<local server> NICK <timestamp> <nick> <hostname> <displayed-hostname> <ident> +<modes> <ip> :<gecos>
 * :<old nick> NICK <new nick>
 */
 
static void m_nick( char *origin, char **argv, int argc, int srv)
{
	if (argc == 8) {
		/*      nick     hop   ts       user     host     server  ip       svcid mode     vhost    realname nume smode */
		do_nick(argv[1], NULL, argv[0], argv[4], argv[2], origin, argv[6], NULL, argv[5], argv[3], argv[7], NULL, NULL);
	} else {
		do_nickchange(origin, argv[0], NULL);
	}

}
/*
 * :<server> FJOIN <channel> <timestamp> :<[prefixes],nickname> {<[prefixes],nickname>}
 */
static void m_fjoin( char *origin, char **argv, int argc, int srv) 
{
	char *nicks[1];
	nicks[0] = argv[2];
	do_sjoin(argv[1], argv[0], "+", NULL, nicks, 1);
}

/*
 * :<source server or nickname> FMODE <target> <timestamp> <modes and parameters>
 */

static void m_fmode ( char *origin, char **argv, int argc, int srv)
{
	char **nargv;
	int nargc =0;
	int i, j = 0;
	for (i = 0; i < argc; i++) {
		if (i == 1)
			continue;
 		AddStringToList(&nargv, argv[i], &nargc); 
		j++;
	}
 	do_mode_channel(origin, nargv, nargc);
	ns_free(nargv);
}
/*
 * DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : TOPIC
 * DEBUG1 NeoStats - args  : #services :hahaha
 */
static void m_topic (char *origin, char **argv, int argc, int srv) 
{
	do_topic(argv[0], origin, NULL, argv[1]);	
}
/* 
 * :<server/nickname> FTOPIC <channel> <time-set> <set-by> :<topic>
 */
static void m_ftopic(char *origin, char **argv, int argc, int srv)
{
	do_topic(argv[0], argv[2], argv[1], argv[3]);	
}

/*
 * :<nickname> FNAME :<new GECOS field>
 */
static void m_fname (char *origin, char **argv, int argc, int srv)
{
	do_setname(origin, argv[0]);
}
/*
 * some kills are sent out as quits (WTF?) so detect it here.
 *  DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : QUIT
 * DEBUG1 NeoStats - args  : :Killed (Fish (haha))
 */
 
static void m_quit (char *origin, char **argv, int argc, int srv)
{
	char **av;
	int ac;
	char *buf;
	char *quitbuf;
	
	quitbuf = strdup(argv[0]); 
	ac = split_buf(argv[0], &av);
	if (!ircstrcasecmp(av[0], "Killed")) {
		/* its a kill */
		buf = av[1];
		buf++;
		do_kill(origin, buf, quitbuf);
	} else {
		/* standard quit */
		do_quit(origin, quitbuf);
	}
	ns_free(av);
	ns_free(quitbuf);
}

/*
 * DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : FHOST
 * DEBUG1 NeoStats - args  : fish.dynam.acasdf
 */
 
static void m_fhost (char *origin, char **argv, int argc, int srv)
{
	do_chghost(origin, argv[0]);
}

/* send_pong needs the target in its params */
void send_pong(const char *reply, const char *data) {
	if (data)
		send_cmd(":%s PONG %s :%s", me.name, reply, data);
	else
		send_cmd(":%s PONG %s", me.name, reply);
} 

void send_ping(const char *source, const char *reply, const char *to) {
	send_cmd(":%s PING %s :%s", source, source, to); 
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( ":%s %s %ld %s %s %s %s %s 0.0.0.0 :%s", server, MSG_NICK, ts, nick, host, host, ident, newmode, realname);
}
/* 
 * DEBUG1 NeoStats - origin: penguin.omega.org.za
 * DEBUG1 NeoStats - cmd   : PUSH
 * DEBUG1 NeoStats - args  : NeoStats ::penguin.omega.org.za 242 NeoStats :Server up 0 days, 16:10:09
 */
void send_numeric(const char *source, const int numeric, const char *target, const char *buf) {
	send_cmd( ":%s PUSH %s ::%s %d %s :%s", source, target, source, numeric, target, buf);
}

/* 
 * DEBUG1 NeoStats - origin: penguin.omega.org.za
 * DEBUG1 NeoStats - cmd   : FMODE
 * DEBUG1 NeoStats - args  : #services 1196825064 +
 */
void send_cmode(const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts) {
	send_cmd( ":%s FMODE %s %ld %s %s", sourceserver, chan, ts, mode, args);
}

/* 
 * DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : FHOST
 * DEBUG1 NeoStats - args  : blah.com
 */
void send_chghost(const char *server, const char *target, const char *vhost) {
	send_cmd( ":%s CHGHOST %s :%s", ns_botptr->name, target, vhost);
}

/*
 * DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : CHGIDENT
 * DEBUG1 NeoStats - args  : Fish :haha
 */

void send_chgident(const char *server, const char *target, const char *ident) {
	send_cmd( ":%s CHGIDENT %s :%s", ns_botptr->name, target, ident);
}
/*
 * DEBUG1 NeoStats - origin: Fish
 * DEBUG1 NeoStats - cmd   : CHGNAME
 * DEBUG1 NeoStats - args  : Fish :haha
 */

void send_chgname(const char *server, const char *target, const char *name) {
	send_cmd( ":%s CHGNAME %s :%s", ns_botptr->name, target, name);
}

static void send_addline(const char type, const char *source, const char *mask, const char *bot, const char *reason, const unsigned long length) {
	send_cmd( ":%s ADDLINE %c %s %s %ld %ld :%s", source, type, mask, bot != NULL ? bot : me.name, me.now, length, reason);
}
void send_akill(const char *server, const char *host, const char *ident, const char *bot, const unsigned long length, const char *reason, const unsigned long ts) {
	char buf[BUFSIZE];
	ircsnprintf(buf, BUFSIZE, "%s@%s", ident, host);
	send_addline('G', server, buf, bot, reason, length);
}
void send_sqline(const char *source, const char *mask, const char *reason) {
	send_addline('Q', source, mask, NULL, reason, 0);
}
void send_zline(const char *source, const char *mask, const char *reason) {
	send_addline('Z', source, mask, NULL, reason, 0);
}
void send_kline(const char *source, const char *mask, const char *reason) {
	send_addline('G', source, mask, NULL, reason, 0);
}
void send_gline(const char *source, const char *mask, const char *reason) {
	send_addline('G', source, mask, NULL, reason, 0);
}

/* :<source server> SVSJOIN <nick> <channel>
 */
void send_svsjoin(const char *source, const char *target, const char *chan) {
	send_cmd( ":%s SVSJOIN %s %s", source, target, chan);
}

/* :<source server> SVSPART <nick> <channel>
 */
void send_svspart(const char *source, const char *target, const char *chan) {
	send_cmd( ":%s SVSPART %s %s", source, target, chan);
}

/* :<source server> SVSNICK <old nick> <new nick> <timestamp>
 */
void send_svsnick(const char *source, const char *target, const char *newnick, const unsigned long ts) {
	send_cmd( ":%s SVSNICK %s %s %ld", source, target, newnick, ts);
}

void send_svsmode(const char *source, const char *target, const char *modes) {
	send_cmd( ":%s SVSMODE %s %s", source, target, modes);
}
