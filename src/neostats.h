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
#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <time.h>
#define __USE_GNU
#include <string.h>
#undef __USE_GNU
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <assert.h>

#ifdef WIN32
#include "configwin32.h"
#ifdef NEOSTATSCORE
#define EXPORTFUNC __declspec(dllexport)
#define EXPORTVAR __declspec(dllexport)
#define MODULEFUNC 
#define MODULEVAR 
#else
#define EXPORTVAR __declspec(dllimport)
#define EXPORTFUNC __declspec(dllimport)
#define MODULEFUNC __declspec(dllexport)
#define MODULEVAR __declspec(dllexport)
#endif
#else
#include "config.h"
#define MODULEFUNC 
#define MODULEVAR 
#define EXPORTVAR
#define EXPORTFUNC
#endif

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
#include "pcre.h"

#include "adns.h"
#include "list.h"
#include "hash.h"
#include "support.h"
#include "ircstring.h"
#include "events.h"

#define arraylen(a)	(sizeof(a) / sizeof(*(a)))

#define PROTOCOL_NOQUIT		0x00000001	/* NOQUIT */
#define PROTOCOL_TOKEN		0x00000002	/* TOKEN */
#define PROTOCOL_SJOIN		0x00000004	/* SJOIN */
#define PROTOCOL_NICKv2		0x00000008	/* NICKv2 */
#define PROTOCOL_SJOIN2		0x00000010	/* SJOIN2 */
#define PROTOCOL_UMODE2		0x00000020	/* UMODE2 */
#define PROTOCOL_NS			0x00000040	/* NS */
#define PROTOCOL_ZIP		0x00000080	/* ZIP - not actually supported by NeoStats */
#define PROTOCOL_VL			0x00000100	/* VL */
#define PROTOCOL_SJ3		0x00000200	/* SJ3 */
#define PROTOCOL_VHP		0x00000400	/* Send hostnames in NICKv2 even if not sethosted */
#define PROTOCOL_SJB64		0x00000800  /* */
#define PROTOCOL_CLIENT		0x00001000  /* CLIENT */
#define PROTOCOL_B64SERVER	0x00002000  /* Server names use Base 64 */
#define PROTOCOL_B64NICK	0x00004000  /* Nick names use Base 64 */
#define PROTOCOL_UNKLN		0x00008000  /*  */

#define FEATURE_SWHOIS		0x00000001	/* SWHOIS */
#define FEATURE_SVSTIME		0x00000002	/* SVSTIME */
#define FEATURE_SVSHOST		0x00000004	/* SVSHOST */
#define FEATURE_SVSJOIN		0x00000008	/* SVSJOIN */
#define FEATURE_SVSMODE		0x00000010	/* SVSMODE */
#define FEATURE_SVSPART		0x00000020	/* SVSPART */
#define FEATURE_SVSNICK		0x00000040	/* SVSNICK */
#define FEATURE_SVSKILL		0x00000080	/* SVSKILL */
#define FEATURE_UMODECLOAK  0x00000100	/* auto cloak host with umode */
#define FEATURE_NICKIP		0x00000200	/* NICK passes IP address */
#define FEATURE_SMODES		0x00000400	/* Smode field */
#define FEATURE_BOTMODES	0x00000800	/* Umodes for bots available */
#define FEATURE_SMO			0x00001000	/* SMO */
#define FEATURE_USERSMODES	0x00002000	/* User Smodes */

/* cumodes are channel modes which affect a user */
#define CUMODE_CHANOP		0x00000001
#define CUMODE_VOICE		0x00000002
#define CUMODE_HALFOP		0x00000004
#define CUMODE_CHANOWNER	0x00000008
/* Following are mutually exclusive in current IRCd support so share bits.
 * If this changes, all these must change.
 */
#define CUMODE_CHANPROT		0x00000010
#define CUMODE_CHANADMIN	0x00000010

/* Channel modes available on all IRCds */
#define CMODE_PRIVATE		0x00000020
#define CMODE_SECRET		0x00000040
#define CMODE_MODERATED		0x00000080
#define CMODE_TOPICLIMIT	0x00000100
#define CMODE_BAN			0x00000200
#define CMODE_INVITEONLY	0x00000400
#define CMODE_NOPRIVMSGS	0x00000800
#define CMODE_KEY			0x00001000
#define CMODE_LIMIT			0x00002000

/* Channel modes available on most IRCds */
#define CMODE_EXCEPT		0x00004000
#define CMODE_RGSTR			0x00008000
#define CMODE_RGSTRONLY		0x00010000
#define CMODE_LINK			0x00020000
#define CMODE_NOCOLOR		0x00040000
#define CMODE_OPERONLY		0x00080000
#define CMODE_ADMONLY		0x00100000
#define CMODE_STRIP			0x00200000
#define CMODE_NOKNOCK		0x00400000
#define CMODE_NOINVITE		0x00800000
#define CMODE_FLOODLIMIT	0x01000000

/* Other channel modes available on IRCds cannot be easily supported so 
 * should be defined locally beginning at 0x02000000
 */

/* Cmode macros */
#define is_hidden_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_ADMONLY|CMODE_OPERONLY)))
#define is_pub_chan(x)  ((x) && !(x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_ADMONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))
#define is_priv_chan(x) ((x) && (x->modes & (CMODE_PRIVATE|CMODE_SECRET|CMODE_RGSTRONLY|CMODE_ADMONLY|CMODE_OPERONLY|CMODE_INVITEONLY) || CheckChanMode(x, CMODE_KEY)))

/* User modes available on all IRCds */
#define UMODE_INVISIBLE		0x00000001	/* makes user invisible */
#define UMODE_OPER			0x00000002	/* Operator */
#define UMODE_WALLOP		0x00000004	/* send wallops to them */

/* User modes available on most IRCds */
#define UMODE_LOCOP			0x00000008	/* Local operator -- SRB */
#define UMODE_REGNICK		0x00000010	/* umode +r - registered nick */
#define UMODE_DEAF          0x00000020	/* Dont see chan msgs */
#define UMODE_HIDE          0x00000040	/* Hide from Nukes */

/* Other user modes available on IRCds cannot be easily supported so 
 * should be defined locally beginning at 0x00000080
 */

EXPORTVAR extern unsigned int ircd_supported_umodes;
EXPORTVAR extern unsigned int ircd_supported_smodes;
#define HaveUmodeRegNick() (ircd_supported_umodes&UMODE_REGNICK)
#define HaveUmodeDeaf() (ircd_supported_umodes&UMODE_DEAF)

/* Umode macros */
/* ifdef checks for macros until umodes updated */
#define is_oper(x) ((x) && ((x->Umode & (UMODE_OPER|UMODE_LOCOP))))
#ifdef UMODE_BOT
#define is_bot(x) ((x) && (x->Umode & UMODE_BOT))
#else
/* Hack for Ultimate 2 while umodes are updated */
#ifdef UMODE_RBOT
#define is_bot(x) ((x) && ((x->Umode & (UMODE_RBOT|UMODE_SBOT))))
#else
#define is_bot(x) (0)
#endif
#endif

#ifndef NEOSTATS_PACKAGE_VERSION
#define NEOSTATS_PACKAGE_VERSION PACKAGE
#endif


#ifdef NEOSTATS_REVISION
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION " (" NEOSTATS_REVISION ") " NS_HOST
#else
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION " " NS_HOST
#endif
#define CORE_MODULE_VERSION NEOSTATS_VERSION

#ifndef TS_CURRENT	/* Allow IRCd to overide */
#define TS5

#ifdef TS5
#define TS_CURRENT	5	/* current TS protocol version */
#else
#define TS_CURRENT	3	/* current TS protocol version */
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

#include "numeric.h"

#define CONFIG_NAME		"neostats.cfg"
#define MOD_PATH		"modules"
#define RECV_LOG		"logs/recv.log"
#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"
#define PID_FILENAME	"neostats.pid"

#ifndef BASE64SERVERSIZE
#define BASE64SERVERSIZE	2
#endif
#ifndef BASE64NICKSIZE
#define BASE64NICKSIZE		5
#endif

#define BUFSIZE			512

#define MAXHOST			(128 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(32 + 1)
#define MAXUSER			(15 + 1)
#define MAXREALNAME		(50 + 1)
#define MAXCHANLEN		(50 + 1)
#define MAXTOPICLEN		(307 + 1)

#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10
#define MAXCMDSIZE		15
#define MAXINFO			128
#define B64SIZE			16

#define KEYLEN		(32 + 1)

/* MAXCHANLENLIST
 * the max length a string can be that holds channel lists 
 */
#define MAXCHANLENLIST		1024 

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
#define VERSIONSIZE		128

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
	NS_ERR_PARAM_OUT_OF_RANGE= 0x8000009,
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

extern EXPORTVAR char recbuf[BUFSIZE];
extern EXPORTVAR char segv_location[SEGV_LOCATION_BUFSIZE];

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
typedef struct tme {
	char name[MAXHOST];
	int numeric; /* For Unreal and any other server that needs a numeric */
	char protocol[MAXHOST];
	char uplink[MAXHOST];
	char infoline[MAXHOST];
	char netname[MAXPASS];
	char local[MAXHOST];
	time_t t_start;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_nickip:1;
	char servicescmode[64];
	char servicesumode[64];
	char serviceschan[MAXCHANLEN];
	unsigned int onchan:1;
	unsigned int synced:1;
	Server *s;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
	time_t now;
	char strnow[STR_TIME_T_SIZE];
#ifdef SQLSRV
	char sqlhost[MAXHOST];
	int sqlport;
#endif
	char version[VERSIONSIZE];
} tme;

extern EXPORTVAR tme me;

/** @brief Bans structure
 *  
 */
typedef struct Ban {
	char type[8];
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
	char name64[B64SIZE];
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
	unsigned int Umode;
	unsigned int Smode;
	int ulevel;
	list_t *chans;
	unsigned int flags;
	void *moddata[NUM_MODULES];
} User;

/** @brief Channel structure
 *  
 */
typedef struct Channel {
	char name[MAXCHANLEN];
	char name64[B64SIZE];
	long users;
	unsigned int modes;
	list_t *chanmembers;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* because a "server" can be a topic owner */
	time_t topictime;
	int  limit;
	char key[KEYLEN];
	list_t *modeparms;
	time_t creationtime;
	unsigned int flags;
	void *moddata[NUM_MODULES];
} Channel;

/** @brief Client structure
 *  work in progress
 */
/*
typedef struct Client {
		User* user;
	char nick[MAXNICK];
	char name64[B64SIZE];
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

		Server* server;
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
typedef int ModuleFeatures;

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
	 * Protocol flags for required protocol specfic features e.g. NICKIP
	 * use 0 if not needed */
	const ModuleProtocol protocol;
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	const ModuleFeatures features;
	/* DO NOT USE: 
	 * Reserved for future expansion */
	const int padding[5];	
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
	/* Link back to user struct associated with this bot*/
	User* u;
}_Bot;

EXPORTFUNC int ModuleConfig(bot_setting* bot_settings);

EXPORTFUNC int add_timer (timer_function func, const char* name, int interval);
EXPORTFUNC int del_timer (const char *timer_name);
EXPORTFUNC int set_timer_interval (const char *timer_name, int interval);
EXPORTFUNC Timer *findtimer(const char *timer_name);

EXPORTFUNC int add_socket (socket_function readfunc, socket_function writefunc, socket_function errfunc, const char *sock_name, int socknum);
EXPORTFUNC int add_sockpoll (before_poll_function beforepoll, after_poll_function afterpoll, const char *sock_name, void *data);
EXPORTFUNC int del_socket (const char *name);
EXPORTFUNC Sock *findsock (const char *sock_name);

EXPORTFUNC Bot * init_bot (BotInfo* botinfo, const char* modes, unsigned int flags, bot_cmd *bot_cmd_list, bot_setting *bot_setting_list);
EXPORTFUNC int del_bot (Bot *botptr, const char * reason);
EXPORTFUNC Bot *findbot (const char * bot_name);
EXPORTFUNC int bot_nick_change (const char * oldnick, const char *newnick);

/* sock.c */
EXPORTFUNC int sock_connect (int socktype, unsigned long ipaddr, int port, const char *name, socket_function func_read, socket_function func_write, socket_function func_error);
EXPORTFUNC int sock_disconnect (const char *name);

/* keeper interface */

#define CFGSTR   1
#define CFGINT   2
#define CFGFLOAT 3
#define CFGBOOL  4

#define CONFBUFSIZE 256

EXPORTFUNC int GetConf (void **data, int type, const char *item);
EXPORTFUNC int SetConf (void *data, int type, char *item);
EXPORTFUNC int GetDir (char *item, char ***data);
EXPORTFUNC int DelConf (char *item);
EXPORTFUNC int DelRow (char *table, char *row);
EXPORTFUNC int DelTable(char *table);
EXPORTFUNC int SetData (void *data, int type, char *table, char *row, char *field);
EXPORTFUNC int GetTableData (char *table, char ***data);
EXPORTFUNC int GetData (void **data, int type, const char *table, const char *row, const char *field);
EXPORTFUNC void flush_keeper();

/* main.c */
void do_exit (NS_EXIT_TYPE exitcode, char* quitmsg) __attribute__((noreturn));
void fatal_error(char* file, int line, char* func, char* error_text) __attribute__((noreturn));;
#define FATAL_ERROR(error_text) fatal_error(__FILE__, __LINE__, __PRETTY_FUNCTION__,(error_text)); 

/* misc.c */
void strip (char * line);
EXPORTFUNC void *smalloc ( const int size );
EXPORTFUNC void *scalloc ( const int size );
EXPORTFUNC void *srealloc ( void* ptr, const int size );
EXPORTFUNC void sfree ( void *buf );
EXPORTFUNC char *sstrdup (const char * s);
char *strlwr (char * s);
EXPORTFUNC void AddStringToList (char ***List, char S[], int *C);
EXPORTFUNC void strip_mirc_codes(char *text);
char *sctime (time_t t);
char *sftime (time_t t);

/* ircd.c */
EXPORTFUNC char *joinbuf (char **av, int ac, int from);
EXPORTFUNC int split_buf (char *buf, char ***argv, int colon_special);

EXPORTFUNC void privmsg_list (char *to, char *from, const char **text);
EXPORTFUNC void prefmsg (char * to, const char * from, char * fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC void privmsg (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC void notice (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC void globops (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC void chanalert (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC void wallops (const char *from, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC void numeric (const int numeric, const char *target, const char *data, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

EXPORTFUNC int join_bot_to_chan (const char *who, const char *chan, const char *modes);
EXPORTFUNC int part_bot_from_chan (const char *who, const char *chan);

/* function declarations */
int squit_cmd (const char *who, const char *quitmsg);
int skick_cmd (const char *who, const char *chan, const char *target, const char *reason);
int sinvite_cmd (const char *from, const char *to, const char *chan);
int scmode_cmd (const char *who, const char *chan, const char *mode, const char *args);
int sumode_cmd (const char *who, const char *target, long mode);
int skill_cmd (const char *from, const char *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
int sping_cmd (const char *from, const char *reply, const char *to);
int spong_cmd (const char *reply);
int snick_cmd (const char *oldnick, const char *newnick);
int sswhois_cmd (const char *target, const char *swhois);
int ssvsnick_cmd (const char *target, const char *newnick);
int ssvsjoin_cmd (const char *target, const char *chan);
int ssvspart_cmd (const char *target, const char *chan);
EXPORTFUNC int ssvshost_cmd (const char *target, const char *vhost);
int ssvsmode_cmd (const char *target, const char *modes);
int ssvskill_cmd (const char *target, const char *reason, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
int sakill_cmd (const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, ...);
int srakill_cmd (const char *host, const char *ident);
int ssvstime_cmd (const time_t ts);
int schanusermode_cmd (const char *who, const char *chan, const char *mode, const char *bot);

/* users.c */
EXPORTFUNC User *finduser (const char *nick);
EXPORTFUNC int UserLevel (User *u);

/* server.c */
EXPORTFUNC Server *findserver (const char *name);

/* chans.c */
EXPORTFUNC Channel *findchan (const char *chan);
EXPORTFUNC int CheckChanMode (Channel * c, long mode);
EXPORTFUNC int IsChanMember(Channel *c, User *u);
EXPORTFUNC int test_chan_user_mode(char* chan, char* nick, int flag);

#define is_chanop(chan, nick)		test_chan_user_mode(chan, nick, CUMODE_CHANOP)
#define is_chanhalfop(chan, nick)	test_chan_user_mode(chan, nick, CUMODE_HALFOP)
#define is_chanvoice(chan, nick)	test_chan_user_mode(chan, nick, CUMODE_VOICE)
#define is_chanowner(chan, nick)	test_chan_user_mode(chan, nick, CUMODE_CHANOWNER)
#define is_chanprot(chan, nick)		test_chan_user_mode(chan, nick, CUMODE_CHANPROT)
#define is_chanadmin(chan, nick)	test_chan_user_mode(chan, nick, CUMODE_CHANADMIN)

/* dns.c */
EXPORTFUNC int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data);

/* services.c */
EXPORTFUNC int add_services_cmd_list(bot_cmd* bot_cmd_list);
EXPORTFUNC int del_services_cmd_list(bot_cmd* bot_cmd_list);
EXPORTFUNC int is_target_valid(char* bot_name, User* u, char* target_nick);

/* transfer.c stuff */
typedef void (transfer_callback) (void *data, int returncode, char *body, int bodysize);
EXPORTFUNC void transfer_status(void);
EXPORTFUNC int new_transfer(char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback);

/* exclude */
#define IsExcluded(x) ((x) && ((x)->flags & NS_FLAGS_EXCLUDED))

/* Is the user or server a NeoStats one? */
#define IsMe(x) ((x) && ((x)->flags & NS_FLAGS_ME))

/* Is the user or server synched? */
#define IsSynched(x) ((x) && ((x)->flags & NS_FLAGS_SYNCHED))

/* Mark server as synched */
#define SynchServer(x) (((x)->flags |= NS_FLAGS_SYNCHED))

/* Some standard text help messages */
extern EXPORTVAR const char *ns_help_set_nick[];
extern EXPORTVAR const char *ns_help_set_altnick[];
extern EXPORTVAR const char *ns_help_set_user[];
extern EXPORTVAR const char *ns_help_set_host[];
extern EXPORTVAR const char *ns_help_set_realname[];

extern const char *ns_copyright[];

EXPORTFUNC int validate_nick (char* nick);
EXPORTFUNC int validate_user (char* user);
EXPORTFUNC int validate_host (char* host);

#ifdef USE_BERKELEY
EXPORTFUNC int DBOpenDatabase(void);
EXPORTFUNC void DBCloseDatabase(void);
EXPORTFUNC void* DBGetData(char* key);
EXPORTFUNC void DBSetData(char* key, void * data, int size);
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
	LOG_LEVELMAX,		
} LOG_LEVEL;

/* define debug levels */

typedef enum DEBUG_LEVEL {
	DEBUG1=1,	/* debug notices about important functions that are going on */
	DEBUG2,		/* more debug notices that are usefull */
	DEBUG3,		/* even more stuff, that would be useless to most normal people */
	DEBUG4,		/* are you insane? */
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

EXPORTFUNC void nlog (LOG_LEVEL level, char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC void dlog (DEBUG_LEVEL level, char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */

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
int MODULEFUNC ModInit(Module* mod_ptr);
void MODULEFUNC ModFini(void);
int MODULEFUNC ModAuth (User * u);
int MODULEFUNC ModAuthUser(User * u, int curlvl);
int MODULEFUNC ModAuthList(User * u);
extern MODULEVAR ModuleInfo module_info;   
extern MODULEVAR ModuleEvent module_events[];  

#endif /* NEOSTATS_H */
