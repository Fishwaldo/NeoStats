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

/* Temp disable for upcoming release until all external modules 
 * have been released with warnings fixed
 */
#if 0
#define __attribute__(x)  /* NOTHING */
#else
/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#define __attribute__(x)  /* NOTHING */
#endif
#endif

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
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION " (" NEOSTATS_REVISION ")"
#else
#define NEOSTATS_VERSION NEOSTATS_PACKAGE_VERSION
#endif

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

/**  this is a security hack to give the coders the right levels to debug NeoStats. 
  *  Don't define unless we ask you to 
  */
#undef CODERHACK

#define CONFIG_NAME		"neostats.cfg"
#define MOD_PATH		"dl"
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
#define VERSIONSIZE		32

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
	/* Error value for incompatible version */
	/* Temporarily done as #define for forward port */
	/*	NS_ERR_VERSION			= 0x8000003,*/
}NS_ERR ;

#define	NS_ERR_VERSION 0x8000003

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

#define SEGV_INMODULE_BUFSIZE	MAX_MOD_NAME
#define SET_SEGV_INMODULE(module_name) strlcpy(segv_inmodule,(module_name),SEGV_INMODULE_BUFSIZE);
#define CLEAR_SEGV_INMODULE() segv_inmodule[0]='\0';

/* temporary define while module settings are ported to macro system */
#define segvinmodule segv_inmodule

/* macros to provide a couple missing string functions for code legibility 
 * and to ensure we perform these operations in a standard and optimal manner
 */
/* set a string to NULL */
#define strsetnull(str) (str)[0] = 0
/* test a string for NULL */
#define strisnull(str)  ((str) && (str)[0] == 0)

#define ARRAY_COUNT (a) ((sizeof ((a)) / sizeof ((a)[0]))

extern int servsock;
extern char recbuf[BUFSIZE];
extern char s_Services[MAXNICK];
extern const char ircd_version[];
extern const char services_bot_modes[];
extern char segv_location[SEGV_LOCATION_BUFSIZE];
extern char segv_inmodule[SEGV_INMODULE_BUFSIZE];
extern jmp_buf sigvbuf;

extern hash_t *sh;
extern hash_t *uh;
extern hash_t *ch;

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
	char infoline[MAXINFO];
	void *moddata[NUM_MODULES];
	long flags;
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
	char user[MAXUSER];			/* bot user */
	char host[MAXHOST];			/* bot host */
	char realname[MAXREALNAME];	/* bot real name */
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
	int action;
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
	char versionfull[VERSIONSIZE];
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
	Server *server;
	int flood;
	int is_away;
	time_t t_flood;
	char modes[MODESIZE];
	int ulevel;
	long Umode;
	list_t *chans;
	struct in_addr ipaddr;
	time_t TS;
	time_t servicesstamp;
	long Smode;
	void *moddata[NUM_MODULES];
	long flags;
} User;

/** @brief Chans structure
 *  
 */
typedef struct Chans {
	char name[CHANLEN];
	char name64[B64SIZE];
	long cur_users;
	long modes;
	list_t *chanmembers;
	list_t *modeparms;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* because a "server" can be a topic owner */
	time_t topictime;
	void *moddata[NUM_MODULES];
	time_t tstime;
	long flags;
} Chans;

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
typedef int (*bot_cmd_handler) (User * u, char **av, int ac);

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

/* SET Comand handling */

typedef enum SET_TYPE {
	SET_TYPE_BOOLEAN,	/* ON or OFF */
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
}bot_setting;

/* sock.c */
int sock_connect (int socktype, unsigned long ipaddr, int port, char *sockname, char *module, char *func_read, char *func_write, char *func_error);
int sock_disconnect (char *sockname);

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

/* (M) For backwards compatibility only, bots are moving to a new interface */
int init_bot (char * nick, char * user, char * host, char * realname, const char *modes, char * modname);
int del_bot (char * nick, char * reason);
void privmsg_list (char *to, char *from, const char **text);
void prefmsg (char * to, const char * from, char * fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void privmsg (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void notice (char *to, const char *from, char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
void globops (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
void chanalert (char * from, char * fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
int wallops (const char *from, const char *msg, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
int numeric (const int numeric, const char *target, const char *data, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
/* Temp for backwards compatibility */
#define snumeric_cmd numeric

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
int ssvshost_cmd (const char *who, const char *vhost);
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
Chans *findchan (const char *chan);
int CheckChanMode (Chans * c, long mode);
int IsChanMember(Chans *c, User *u);
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
int init_services(void);
int add_services_cmd_list(bot_cmd* bot_cmd_list);
int del_services_cmd_list(bot_cmd* bot_cmd_list);
void services_cmd_help (User * u, char **av, int ac);
int is_target_valid(char* bot_name, User* u, char* target_nick);

/* transfer.c stuff */
typedef void (transfer_callback) (void *data, int returncode, char *body, int bodysize);
void transfer_status();
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
extern const char *ns_help_set_user[];
extern const char *ns_help_set_host[];
extern const char *ns_help_set_realname[];

int validate_nick (char* nick);
int validate_user (char* user);
int validate_host (char* host);

#ifdef USE_BERKELEY
int DBOpenDatabase(void);
void DBCloseDatabase(void);
void* DBGetData(char* key);
void DBSetData(char* key, void * data, int size);
#endif

#include "dl.h"

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
} LOG_LEVEL;

/* Scope of Logging Defines: */

typedef enum LOG_SCOPE {
	LOG_CORE = 0,
	LOG_MOD = 1,
} LOG_SCOPE;

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

void nlog (int level, int scope, char *fmt, ...) __attribute__((format(printf,3,4))); /* 2=format 3=params */


#include "conf.h"

#endif /* NEOSTATS_H */
