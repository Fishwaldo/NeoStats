/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef NEOSTATS_H
#define NEOSTATS_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#define __USE_GNU
#include <string.h>
#undef __USE_GNU
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <assert.h>

#include "config.h"

#ifdef HAVE_DB_H
/*#define USE_BERKELEY*/
#endif

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#define __attribute__(x)  /* NOTHING */
#endif

/* 
 * NeoStats core API version.
 * A module should check this when loaded to ensure compatibility
 */
#define API_VER 3

#include "version.h"

#include "adns.h"
#include "pcre.h"
#include "list.h"
#include "hash.h"
#include "support.h"
#include "ircstring.h"
#include "events.h"

#if UNREAL == 1
#include "protocol/unreal.h"
#elif ULTIMATE == 1
#include "protocol/ultimate.h"
#elif HYBRID7 == 1	
#include "protocol/hybrid7.h"
#elif NEOIRCD == 1
#include "protocol/neoircd.h"
#elif MYSTIC == 1
#include "protocol/mystic.h" 
#elif IRCU == 1
#include "protocol/ircu.h"
#elif BAHAMUT == 1
#include "protocol/bahamut.h"
#elif QUANTUM == 1
#include "protocol/quantum.h"
#elif LIQUID == 1
#include "protocol/liquid.h"
#elif VIAGRA == 1 
#include "protocol/viagra.h"
#else
#error Error, you must select an IRCD to use. See ./configure --help for more information
#endif

#ifdef NEOSTATS_REVISION
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION " (" NEOSTATS_REVISION ")" NS_PROTOCOL
#else
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION NS_PROTOCOL
#endif
#define CORE_MODULE_VERSION NEOSTATS_VERSION

#ifndef TS_CURRENT	/* Allow IRCd to overide */
#define TS5

#ifdef TS5
#define	TS_CURRENT	5	/* current TS protocol version */
#else
#define	TS_CURRENT	3	/* current TS protocol version */
#endif

#endif /* TS_CURRENT */

#ifndef TS_MIN	/* Allow IRCd to overide */
/* #define TS5_ONLY */ /* Should not be defined globally! */

#ifdef TS5_ONLY
#define TS_MIN          5
#else
#define TS_MIN          3       /* minimum supported TS protocol version */
#endif

#endif /* TS_MIN */

/* SecureServ wants CHANADMIN but only a few ircds support it so we have to "fake" it */
#ifndef CMODE_CHANADMIN
#define CMODE_CHANADMIN CMODE_CHANOP
/* Flag for new sjoin call to avoid duplicate case value */
#define FAKE_CMODE_CHANADMIN
#endif

#include "numeric.h"

#define CONFIG_NAME		"neostats.cfg"
#define MOD_PATH		"modules"
#define RECV_LOG		"logs/recv.log"
#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"
#define PID_FILENAME	"neostats.pid"

#define BUFSIZE			512
#ifndef MAXHOST
#define MAXHOST			128
#endif
#ifndef MAXPASS
#define MAXPASS			32
#endif
#ifndef MAXNICK
#define MAXNICK			32
#endif
#ifndef MAXUSER
#define MAXUSER			15
#endif
#ifndef MAXREALNAME
#define MAXREALNAME		50
#endif
#ifndef CHANLEN
#define CHANLEN			50
#endif
#ifndef TOPICLEN
#define TOPICLEN		307
#endif
#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10
#define MAXCMDSIZE		15
#define MAXINFO			128
#define B64SIZE			16

#define	KEYLEN		32

/* MAXCHANLIST
 * the max length a string can be that holds channel lists 
 */
#define MAXCHANLIST		1024 

/* MAXPATH 
 * used to determine buffer sizes for file system operations
 */
#ifndef MAXPATH
#define MAXPATH			1024
#endif /* MAXPATH */

/* TIMEBUFSIZE
 * used to determine buffer sizes for time formatting buffers
 */
#define TIMEBUFSIZE		80

/* STR_TIME_T_SIZE
 * size of a time_t converted to a string. 
 */
#define STR_TIME_T_SIZE	24

/* MAX_MOD_NAME
   ModuleInfo will allow any length since it is merely a char *
   functions displaying ModuleInfo contents will display the full string.
   NeoStats core will truncate to this length for use internally. 
*/
#define MAX_MOD_NAME	32

/* Buffer size for version string */
#define VERSIONSIZE		64

/* doesn't have to be so big atm */
#define NUM_MODULES		20
#define S_TABLE_SIZE	-1
#define U_TABLE_SIZE	-1
#define C_TABLE_SIZE	-1
#define CHAN_MEM_SIZE	-1
#define MAXJOINCHANS	-1
#define T_TABLE_SIZE	300	/* Number of Timers */
#define B_TABLE_SIZE	100	/* Number of Bots */
#define MAXMODES		-1
#define DNS_QUEUE_SIZE  300	/* number on concurrent DNS lookups */
#define MAX_TRANSFERS	10	/* number of curl transfers */

#define bzero(x, y)		memset(x, '\0', y);
#define is_synced		me.synced

/* Early creation of unified return values and error system */
/* These are program exit codes usually defined in stdlib.h but 
   if not found will be defined here */
#ifndef EXIT_FAILURE 
#define EXIT_FAILURE 1
#endif /* EXIT_FAILURE */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* EXIT_SUCCESS */

#define NS_SUCCESS			 1
#define NS_FAILURE			-1

/* these defines are for the flags for users, channels and servers */
#define NS_FLAGS_EXCLUDED	0x00000001 /* this entry matched a exclusion */
#define NS_FLAGS_ME			0x00000002 /* indicates the server/user is a NeoStats one */
#define NS_FLAGS_SYNCHED	0x00000004 /* indicates the server is now synched */
#if 0
#define NS_FLAGS_NETJOIN	0x00000008 /* indicates the user is on a net join */
#endif

/* Specific errors beyond SUCCESS/FAILURE so that functions can handle errors 
 * Treat as unsigned with top bit set to give us a clear distinction from 
 * other values and use a typedef ENUM so that we can indicate return type */
typedef enum NS_ERR {
	NS_ERR_NICK_IN_USE		= 0x8000001,
	NS_ERR_OUT_OF_MEMORY	= 0x8000002,
	NS_ERR_VERSION			= 0x8000003,
	NS_ERR_SYNTAX_ERROR		= 0x8000004,
	NS_ERR_NEED_MORE_PARAMS	= 0x8000005,
	NS_ERR_NO_PERMISSION	= 0x8000006,
	NS_ERR_UNKNOWN_COMMAND	= 0x8000007,
	NS_ERR_UNKNOWN_OPTION	= 0x8000008,
}NS_ERR ;

/* do_exit call exit type definitions */
typedef enum {
	NS_EXIT_NORMAL=0,
	NS_EXIT_RELOAD,
	NS_EXIT_RECONNECT,
	NS_EXIT_ERROR,
	NS_EXIT_SEGFAULT,
}NS_EXIT_TYPE;

/* NeoStats levels */
#define NS_ULEVEL_ROOT	200
#define NS_ULEVEL_ADMIN	185
#define NS_ULEVEL_OPER	50
#define NS_ULEVEL_LOCOPER 40
#define NS_ULEVEL_REG	10

/* transfer stuff */
typedef enum NS_TRANSFER {
	NS_FILE=0,
	NS_MEMORY=1,
} NS_TRANSFER;

#define SEGV_LOCATION_BUFSIZE	255
#ifdef LEAN_AND_MEAN
#define SET_SEGV_LOCATION()
#define SET_SEGV_LOCATION_EXTRA(debug_text)
#define CLEAR_SEGV_LOCATION()
#else
#define SET_SEGV_LOCATION() ircsnprintf(segv_location,SEGV_LOCATION_BUFSIZE,"%s %d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__); 
#define SET_SEGV_LOCATION_EXTRA(debug_text) ircsnprintf(segv_location,SEGV_LOCATION_BUFSIZE,"%s %d %s %s", __FILE__, __LINE__, __PRETTY_FUNCTION__,(debug_text)); 
#define CLEAR_SEGV_LOCATION() segv_location[0]='\0';
#endif

extern char recbuf[BUFSIZE];
extern const char services_bot_modes[];
extern char segv_location[SEGV_LOCATION_BUFSIZE];

/* this is the dns structure */
extern adns_state ads;

/* version info */
extern const char version_date[], version_time[];

/** @brief Server structure
 *  structure containing all details of a server
 */
typedef struct Server {
	char name[MAXHOST];
	char name64[B64SIZE];
	int hops;
	int numeric;
	time_t connected_since;
	int ping;
	char uplink[MAXHOST];
	char version[MAXHOST];
	char infoline[MAXINFO];
	long flags;
	void *moddata[NUM_MODULES];
} Server;

/** @brief me structure
 *  structure containing information about the neostats core
 */
struct me {
	char name[MAXHOST];
	char nameb64[B64SIZE];
	int port;
	int r_time;
	int numeric; /* For Unreal and any other server that needs a numeric */
	char uplink[MAXHOST];
	char pass[MAXPASS];
	char services_name[MAXHOST];
	char infoline[MAXHOST];
	char netname[MAXPASS];
	char local[MAXHOST];
	time_t t_start;
	unsigned int allbots;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_privmsg:1;
	unsigned int onlyopers:1;
	unsigned int die:1;
	unsigned int debug_mode:1;
	unsigned int want_nickip:1;
#if defined(ULTIMATE3) || defined(QUANTUM)
	unsigned int client:1;
#endif
	unsigned int setservertimes;
	char chan[BUFSIZE];
	unsigned int onchan:1;
	unsigned int synced:1;
	Server *s;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
	int pingtime;
	time_t now;
	char strnow[STR_TIME_T_SIZE];
#ifdef SQLSRV
	char sqlhost[MAXHOST];
	int sqlport;
#endif
	char version[VERSIONSIZE];
} me;

/** @brief Bans structure
 *  
 */
typedef struct Ban {
	char type;
	char user[MAXUSER];
	char host[MAXHOST];
	char mask[MAXHOST];
	char reason[BUFSIZE];
	char setby[MAXHOST];
	time_t tsset;
	time_t tsexpires;
} Ban;



/** @brief User structure
 *  
 */
typedef struct User {
	char nick[MAXNICK];
	char nick64[B64SIZE];
	char hostname[MAXHOST];
	char username[MAXUSER];
	char realname[MAXREALNAME];
	char vhost[MAXHOST];
	char awaymsg[MAXHOST];
	char swhois[MAXHOST];
	struct in_addr ipaddr;
	Server *server;
	int flood;
	int is_away;
	time_t tslastmsg;
	time_t tslastnick;
	time_t tslastaway;
	time_t TS;
	time_t servicestamp;
	char modes[MODESIZE];
	long Umode;
	long Smode;
	int ulevel;
	list_t *chans;
	long flags;
	void *moddata[NUM_MODULES];
} User;

/** @brief Channel structure
 *  
 */
typedef struct Channel {
	char name[CHANLEN];
	char name64[B64SIZE];
	long users;
	long modes;
	list_t *chanmembers;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* because a "server" can be a topic owner */
	time_t topictime;
	int  limit;
	char key[KEYLEN + 1];
	list_t *modeparms;
	time_t creationtime;
	long flags;
	void *moddata[NUM_MODULES];
} Channel;

/** @brief Client structure
 *  work in progress
 */
/*
typedef struct Client {
do we use:
	User* user;
	Server* server;
or:
	source sptr;
	dest dptr;
	int command;
	int flags;
	int sock;
	char **av;
	int ac;
	char param[BUFSIZE];
	void *moddata[NUMMODS];
} Client; */

typedef struct _Bot Bot;

typedef struct CmdParams {
	struct {
		User* user;
		Server* server;
	}source;
	struct {
		User* user;
		Server* server;
		Bot * bot;
	}dest;
	char* param;
	Channel* channel;
	char **av;
	int ac;
} CmdParams; 

/** @brief ModesParm structure
 *  
 */
typedef struct ModesParm {
	long mode;
	char param[PARAMSIZE];
	void *moddata[NUM_MODULES];
} ModesParm;

/** @brief ping structure
 *  
 */
struct ping {
	time_t last_sent;
	int ulag;
} ping;

/* Comand list handling */
/** @brief flags for command list
 *  flags to provide more information on a command to the core
 */
#define CMD_FLAG_SET	0x00000001

/** @brief bot_cmd_handler type
 *  defines handler function definition
 */
typedef int (*bot_cmd_handler) (CmdParams* cmdparams);

/** @brief bot_cmd structure
 *  defines command lists for bots
 */
typedef struct bot_cmd {
	const char		*cmd;		/* command string */
	bot_cmd_handler	handler;	/* handler */
	int				minparams;	/* min num params */
	unsigned int	ulevel;		/* min user level */
	const char**	helptext;	/* pointer to help text */
	const char* 	onelinehelp;/* single line help for generic help function */
}bot_cmd;

/** @brief flags for bots
 *  flags to influence how bots are managed
 *  e.g. restrict to opers
 */

/* Restrict module bot to only respond to oper requests
 * when ONLY_OPERS is set in the config file 
 * E.g. StatServ
 */
#define BOT_FLAG_ONLY_OPERS		0x00000001
/* Restrict module bot to only respond to oper requests
 * regardless of ONLY_OPERS setting in the config file
 * E.g. Connectserv
 */
#define BOT_FLAG_RESTRICT_OPERS	0x00000002
/* Stop bot listening to channel chatter when they do not need to
 * E.g. Connectserv
 */
#define BOT_FLAG_DEAF	0x00000004
/* Mark bot as a service bot that can receive commands
 * If not set, private messages will not be scanned for commands
 * and all messages received will be passed directly to the module
 * E.g. Connectserv
 */
#define BOT_FLAG_SERVICEBOT	0x00000008

/* SET Comand handling */

typedef enum SET_TYPE {
	SET_TYPE_BOOLEAN = 0,	/* ON or OFF */
	SET_TYPE_INT,		/* valid integer */
	SET_TYPE_STRING,	/* single string */
	SET_TYPE_MSG,		/* multiple strings to be treated as a message and stored in one field */
	SET_TYPE_NICK,		/* valid nick */
	SET_TYPE_USER,		/* valid user */
	SET_TYPE_HOST,		/* valid host name */
	SET_TYPE_REALNAME,	/* valid realname */
	SET_TYPE_CHANNEL,	/* valid channel */
	SET_TYPE_IPV4,		/* valid IPv4 dotted quad */
#if 0
/* For future expansion */
	SET_TYPE_INTRANGE,
	SET_TYPE_STRINGRANGE,
#endif
	SET_TYPE_CUSTOM,	/* handled by module */
}SET_TYPE;

typedef union uval {
	char*			s;
	unsigned int	i;
}uval;

/** @brief bot_setting structure
 *  defines SET list for bots
 */
typedef struct bot_setting {
	char			*option;	/* option string */
	void*			varptr;		/* pointer to var */
	SET_TYPE		type;		/* type of var */
	unsigned int	min;		/* min value */
	unsigned int	max;		/* max value */
	unsigned int	ulevel;		/* min user level */
	char			*confitem;	/* config string for kptool */
	const char		*desc;		/* description of setting for messages e.g. seconds, days*/
	const char**	helptext;	/* pointer to help text */
	bot_cmd_handler	handler;	/* handler for custom/post-set processing */
	void*			defaultval; /* default value for setting */
}bot_setting;


/** @brief Message function types
 * 
 */
typedef int (*timer_function) (void);

/** @brief Socket function types
 * 
 */
typedef int (*socket_function) (int sock_no, char *name);
typedef int (*before_poll_function) (void *data, struct pollfd *);
typedef void (*after_poll_function) (void *data, struct pollfd *, unsigned int);


/* socket interface type */
#define SOCK_POLL 1
#define SOCK_STANDARD 2

/** @brief ModuleEvent functions structure
 * 
 */
typedef int (*event_function) (CmdParams *cmdparams);

typedef struct ModuleEvent {
	Event event;
	event_function function;
}ModuleEvent;

typedef int ModuleFlags;
typedef int ModuleProtocol;

/** @brief Module Info structure
 *	This describes the module to the NeoStats core and provides information
 *  to end users when modules are queried.
 *  The presence of this structure is required but some fields are optional.
 */
typedef struct ModuleInfo {
	/* REQUIRED: 
	 * name of module e.g. StatServ */
	const char *name;
	/* REQUIRED: 
	 * one line brief description of module */
	const char *description;
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with copyright information
	 * NeoStats will automatically provide a CREDITS command to output this
	 * use NULL for none */
	const char **copyright;
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with extended description
	 * NeoStats will automatically provide an ABOUT command to output this
	 * use NULL for none */
	const char **about_text;
	/* REQUIRED: 
	 * version of neostats used to build module
	 * must be NEOSTATS_VERSION */
	const char *neostats_version;
	/* REQUIRED: 
	 * string containing version of module */
	const char *version;
	/* REQUIRED: string containing build date of module 
	 * should be __DATE__ */
	const char *build_date;
	/* REQUIRED: string containing build time of module 
	 * should be __TIME__ */
	const char *build_time;
	/* OPTIONAL: 
	 * Module control flags, 
	 * use 0 if not needed */
	const ModuleFlags flags;
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	const ModuleProtocol protocol;
	/* DO NOT USE: 
	 * Reserved for future expansion */
	const int padding[6];	
}ModuleInfo;

typedef int (*mod_auth) (User * u);

/** @brief Module structure
 * 
 */
typedef struct Module {
	ModuleInfo *info;
	ModuleEvent *event_list;
	mod_auth mod_auth_cb;
	void *dl_handle;
	unsigned int modnum;
}Module;

extern Module* RunModule[10];
extern int RunLevel;

/* Simple stack to manage run level replacing original segv_module stuff 
 * which will hopefully make it easier to determine where we are running
 * and avoid the need for modules to ever manage this and the core to
 * have to set/reset when a module calls a core function which triggers
 * other modules to run (e.g. init_bot)
 */
#define SET_RUN_LEVEL(moduleptr) {if(RunLevel<10 && RunModule[RunLevel] != moduleptr){RunLevel++;RunModule[RunLevel] = moduleptr;}}
#define RESET_RUN_LEVEL() {if(RunLevel>0){RunLevel--;}}
#define GET_CUR_MODULE() RunModule[RunLevel]
#define GET_CUR_MODNUM() RunModule[RunLevel]->modnum
#define GET_CUR_MODNAME() RunModule[RunLevel]->info->name

/** @brief Module socket list structure
 * 
 */
typedef struct Sock {
	/** Owner module ptr */
	Module* moduleptr;
	/** Socket number */
	int sock_no;
	/** Socket name */
	char name[MAX_MOD_NAME];
	/** socket interface (poll or standard) type */
	int socktype;
	/** if socktype = SOCK_POLL, before poll function */
	/** Socket before poll function */
	before_poll_function beforepoll;
	/** Socket after poll function */
	after_poll_function afterpoll;
	/** data */
	void *data;
	/* if socktype = SOCK_STANDARD, function calls */
	/** Socket read function */
	socket_function readfnc;
	/** Socket write function */
	socket_function writefnc;
	/** Socket error function */
	socket_function errfnc;
	/** rmsgs */
	long rmsgs;
	/** rbytes */
	long rbytes;
} Sock;

/** @brief Module Timer structure
 * 
 */
typedef struct Timer {
	/** Owner module ptr */
	Module* moduleptr;
	/** Timer type */
	int type;
	/** Timer name */
	char name[MAX_MOD_NAME];
	/** Timer interval */
	int interval;
	/** Time last run */
	time_t lastrun;
	/** Timer function */
	timer_function function;
} Timer;

/** @brief BotInfo structure
 * 
 */
typedef struct BotInfo {		
	/* REQUIRED: nick */
	char nick[MAXNICK];
	/* OPTIONAL: altnick, use NULL if not needed */
	char altnick[MAXNICK];
	/* REQUIRED: user */
	char user[MAXUSER];
	/* REQUIRED: host */
	char host[MAXHOST];
	/* REQUIRED: realname */
	char realname[MAXREALNAME];
} BotInfo;

/** @brief Bot structure
 * 
 */

typedef struct _Bot {
	/** Owner module ptr */
	Module* moduleptr;
	/** Nick */
	char nick[MAXNICK];
	/* bot flags */
	unsigned int flags;
	/* hash for command list */
	hash_t *botcmds;
	/* hash for settings */
	bot_setting *bot_settings;
	/* min ulevel for settings */
	unsigned int set_ulevel;
}_Bot;

int ModuleConfig(bot_setting* bot_settings);

int add_timer (timer_function func, char* name, int interval);
int del_timer (char *timer_name);
int set_timer_interval (char *timer_name, int interval);
Timer *findtimer(char *timer_name);

int add_socket (socket_function readfunc, socket_function writefunc, socket_function errfunc, char *sock_name, int socknum);
int add_sockpoll (before_poll_function beforepoll, after_poll_function afterpoll, char *sock_name, void *data);
int del_socket (char *name);
Sock *findsock (char *sock_name);

Bot * init_bot (BotInfo* botinfo, const char* modes, unsigned int flags, bot_cmd *bot_cmd_list, bot_setting *bot_setting_list);
int del_bot (Bot *botptr, char * reason);
Bot *findbot (char * bot_name);
int bot_nick_change (char * oldnick, char *newnick);

/* sock.c */
int sock_connect (int socktype, unsigned long ipaddr, int port, char *module, socket_function func_read, socket_function func_write, socket_function func_error);
int sock_disconnect (char *name);

/* conf.c */
int ConfLoad (void);
void rehash (void);
int ConfLoadModules (void);

/* main.c */
void do_exit (NS_EXIT_TYPE exitcode, char* quitmsg) __attribute__((noreturn));
void fatal_error(char* file, int line, char* func, char* error_text) __attribute__((noreturn));;
#define FATAL_ERROR(error_text) fatal_error(__FILE__, __LINE__, __PRETTY_FUNCTION__,(error_text)); 

/* misc.c */
void strip (char * line);
void *smalloc (long size);
void *scalloc (long size);
char *sstrdup (const char * s);
char *strlwr (char * s);
void AddStringToList (char ***List, char S[], int *C);
void strip_mirc_codes(char *text);
char *sctime (time_t t);
char *sftime (time_t t);
void debugtochannel(char *message, ...) __attribute__((format(printf,1,2))); /* 1=format 2=params */

/* ircd.c */
void parse (char* line);
char *joinbuf (char **av, int ac, int from);
int split_buf (char *buf, char ***argv, int colon_special);
int flood (User * u);
int join_bot_to_chan (const char *who, const char *chan, unsigned long chflag);

void privmsg_list (char *to, char *from, const char **text);
void prefmsg (char * to, const char * from, char * fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void privmsg (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void notice (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void globops (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
void chanalert (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
void wallops (const char *from, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
void numeric (const int numeric, const char *target, const char *data, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

/* function declarations */
#ifdef GOTSJOIN
int ssjoin_cmd (const char *who, const char *chan, unsigned long chflag);
#endif
int sjoin_cmd (const char *who, const char *chan);
int spart_cmd (const char *who, const char *chan);
int squit_cmd (const char *who, const char *quitmsg);
int skick_cmd (const char *who, const char *chan, const char *target, const char *reason);
int sinvite_cmd (const char *from, const char *to, const char *chan);
int schmode_cmd (const char *who, const char *chan, const char *mode, const char *args);
int snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname, long mode);
int sumode_cmd (const char *who, const char *target, long mode);
int skill_cmd (const char *from, const char *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
int sping_cmd (const char *from, const char *reply, const char *to);
int spong_cmd (const char *reply);
int snick_cmd (const char *oldnick, const char *newnick);
int sswhois_cmd (const char *target, const char *swhois);
int ssvsnick_cmd (const char *target, const char *newnick);
int ssvsjoin_cmd (const char *target, const char *chan);
int ssvspart_cmd (const char *target, const char *chan);
int ssvshost_cmd (const char *target, const char *vhost);
int ssvsmode_cmd (const char *target, const char *modes);
int ssvskill_cmd (const char *target, const char *reason, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
int sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...);
int srakill_cmd (const char *host, const char *ident);
int ssvstime_cmd (const time_t ts);

/* users.c */
User *finduser (const char *nick);
int UserLevel (User *u);

/* server.c */
Server *findserver (const char *name);

/* chans.c */
Channel *findchan (const char *chan);
int CheckChanMode (Channel * c, long mode);
int IsChanMember(Channel *c, User *u);
int test_chan_user_mode(char* chan, char* nick, int flag);

#ifdef CMODE_CHANOP
#define is_chanop(chan, nick)		test_chan_user_mode(chan, nick, CMODE_CHANOP)
#endif
#ifdef CMODE_HALFOP
#define is_chanhalfop(chan, nick)	test_chan_user_mode(chan, nick, CMODE_HALFOP)
#endif
#ifdef CMODE_VOICE
#define is_chanvoice(chan, nick)	test_chan_user_mode(chan, nick, CMODE_VOICE)
#endif
#ifdef CMODE_CHANOWNER
#define is_chanowner(chan, nick)	test_chan_user_mode(chan, nick, CMODE_CHANOWNER)
#endif
#ifdef CMODE_CHANPROT
#define is_chanprot(chan, nick)		test_chan_user_mode(chan, nick, CMODE_CHANPROT)
#endif
#ifdef CMODE_CHANADMIN
#define is_chanadmin(chan, nick)	test_chan_user_mode(chan, nick, CMODE_CHANADMIN)
#endif

/* dns.c */
int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data);

/* services.c */
int add_services_cmd_list(bot_cmd* bot_cmd_list);
int del_services_cmd_list(bot_cmd* bot_cmd_list);
int is_target_valid(char* bot_name, User* u, char* target_nick);

/* transfer.c stuff */
typedef void (transfer_callback) (void *data, int returncode, char *body, int bodysize);
void transfer_status(void);
int new_transfer(char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback);

/* exclude */
#define IsExcluded(x) ((x) && ((x)->flags & NS_FLAGS_EXCLUDED))

/* Is the user or server a NeoStats one? */
#define IsMe(x) ((x) && ((x)->flags & NS_FLAGS_ME))

/* Is the user or server synched? */
#define IsSynched(x) ((x) && ((x)->flags & NS_FLAGS_SYNCHED))

/* Mark server as synched */
#define SynchServer(x) (((x)->flags |= NS_FLAGS_SYNCHED))

/* Some standard text help messages */
extern const char *ns_help_set_nick[];
extern const char *ns_help_set_altnick[];
extern const char *ns_help_set_user[];
extern const char *ns_help_set_host[];
extern const char *ns_help_set_realname[];

extern const char *ns_copyright[];

int validate_nick (char* nick);
int validate_user (char* user);
int validate_host (char* host);

#ifdef USE_BERKELEY
int DBOpenDatabase(void);
void DBCloseDatabase(void);
void* DBGetData(char* key);
void DBSetData(char* key, void * data, int size);
#endif

/* log.c API export */
/* define the log levels */

typedef enum LOG_LEVEL {
	LOG_CRITICAL=1,	/* critical crash type notices */
	LOG_ERROR,		/* something is majorly wrong */
	LOG_WARNING,	/* Hey, you should know about this type messages */
	LOG_NOTICE,		/* did you know messages */
	LOG_NORMAL,		/* our normal logging level? */
	LOG_INFO,		/* lots of info about what we are doing */
	LOG_DEBUG1,		/* debug notices about important functions that are going on */
	LOG_DEBUG2,		/* more debug notices that are usefull */
	LOG_DEBUG3,		/* even more stuff, that would be useless to most normal people */
	LOG_DEBUG4,		/* are you insane? */
	LOG_LEVELMAX,		/* are you insane? */
} LOG_LEVEL;

/* define debug levels */

typedef enum DEBUG_LEVEL {
	DEBUG1=1,
	DEBUG2,
	DEBUG3,
	DEBUG4,
	DEBUG5,
	DEBUG6,
	DEBUG7,
	DEBUG8,
	DEBUG9,
	DEBUG10,
	DEBUGMAX,
} DEBUG_LEVEL;

/* this is for the neostats assert replacement. */
/* Version 2.4 and later of GCC define a magical variable _PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */

#define __NASSERT_FUNCTION    __PRETTY_FUNCTION__
/* Not all compilers provide __STRING so define it here if it is unknown */
#ifndef __STRING
#define __STRING(x) #x
#endif /* __STRING */ 


#ifndef __ASSERT_VOID_CAST
#define __ASSERT_VOID_CAST (void)
#endif
extern void nassert_fail (const char *expr, const char *file, const int line, const char *infunk);

#ifndef NDEBUG
#define nassert(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :                                           \
	(nassert_fail(__STRING(expr), __FILE__, __LINE__, __NASSERT_FUNCTION), 0)))
#else
#define nassert(expr) (__ASSERT_VOID_CAST (0))
#endif

void nlog (LOG_LEVEL level, char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */

#include "conf.h"

int CloakHost (Bot *bot_ptr);

typedef void (*ChannelListHandler) (Channel * c);
void GetChannelList(ChannelListHandler handler);
typedef void (*UserListHandler) (User * u);
void GetUserList(UserListHandler handler);
typedef void (*ServerListHandler) (Server * s);
void GetServerList(ServerListHandler handler);

/* 
 * Module Interface 
 */
int ModInit(Module* mod_ptr);
void ModFini(void);
int ModAuth (User * u);
extern ModuleInfo module_info;   
extern ModuleEvent module_events[];  

#endif /* NEOSTATS_H */
