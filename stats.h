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
** $Id$
*/

#ifndef STATS_H
#define STATS_H

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
#include <adns.h>
#include "pcre.h"
#include "list.h"
#include "hash.h"
#include "config.h"
#include "support.h"
#include "ircstring.h"
#include "events.h"

#if UNREAL == 1
#include "Unreal.h"
#elif ULTIMATE == 1
#include "Ultimate.h"
#elif HYBRID7 == 1
#include "hybrid7.h"
#elif NEOIRCD == 1
#include "neoircd.h"
#elif MYSTIC == 1
#include "mystic.h"
#elif IRCU == 1
#include "Ircu.h"
#elif BAHAMUT == 1
#include "Bahamut.h"
#elif QUANTUM == 1
#include "QuantumIRCd.h"
#elif LIQUID == 1
#include "liquidircd.h"
#else
#error Error, you must select an IRCD to use. See ./configure --help for more information
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

#define CHANLEN			50
#define BUFSIZE			512
#define MAXHOST			128
#define MAXPASS			32
#define MAXNICK			32
#define MAXUSER			15
#define MAXUSERWARN		8
#define MAXREALNAME		50
#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10
#define MAXCMDSIZE		15

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

/* MAX_MOD_NAME
   ModuleInfo will allow any length since it is merely a char *
   functions displaying ModuleInfo contents will display the full string.
   NeoStats core will truncate to this length for use internally. 
*/
#define MAX_MOD_NAME	32

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
#define DNS_QUEUE_SIZE  100	/* number on concurrent DNS lookups */

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

/* Specific errors beyond SUCCESS/FAILURE so that functions can handle errors 
 * Treat as unsigned with top bit set to give us a clear distinction from 
 * other values and use a typedef ENUM so that we can indicate return type */
typedef enum NS_ERR {
	NS_ERR_NICK_IN_USE		= 0x8000001,
	NS_ERR_OUT_OF_MEMORY	= 0x8000002,
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
#define NS_ULEVEL_OPER	40

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
	int hops;
	time_t connected_since;
	int ping;
	char uplink[MAXHOST];
	void *moddata[NUM_MODULES];
} Server;

/** @brief me structure
 *  structure containing information about the neostats core
 */
struct me {
	char name[MAXHOST];
	int port;
	int r_time;
	int lag_time;
	int numeric; /* For Unreal and any other server that needs a numeric */
	char uplink[MAXHOST];
	char pass[MAXPASS];
	char services_name[MAXHOST];
	char infoline[MAXHOST];
	char netname[MAXPASS];
	char local[MAXHOST];
	char user[MAXUSER];			/* bot user */
	char host[MAXHOST];			/* bot host */
	char rname[MAXREALNAME];	/* bot real name */
	time_t t_start;
	unsigned int allbots;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_privmsg:1;
	unsigned int send_extreme_lag_notices:1;
	unsigned int onlyopers:1;
	unsigned int die:1;
	unsigned int collisions;
	unsigned int enable_proxy:1;
	unsigned int debug_mode:1;
	unsigned int noticelag:1;
	unsigned int token:1;
#if defined(ULTIMATE3) || defined(QUANTUM)
	unsigned int client:1;
#endif
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
} me;

/** @brief User structure
 *  
 */
typedef struct User {
	char nick[MAXNICK];
	char hostname[MAXHOST];
	char username[MAXUSER];
	char realname[MAXREALNAME];
	char vhost[MAXHOST];
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
	long Smode;
	void *moddata[NUM_MODULES];
} User;

/** @brief Chans structure
 *  
 */
typedef struct Chans {
	char name[CHANLEN];
	long cur_users;
	long modes;
	list_t *chanmembers;
	list_t *modeparms;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* because a "server" can be a topic owner */
	time_t topictime;
	void *moddata[NUM_MODULES];
	time_t tstime;
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
#define CMD_FLAG_NORMAL	0x00000001
#define CMD_FLAG_SET	0x00000002
#define CMD_FLAG_HELP	0x00000004

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
	int 			internal;	/* is this a internal function? */
	const char* 	onelinehelp;	/* single line help for generic help function */
}bot_cmd;

/** @brief flags for bots
 *  flags to influence how bots are managed
 *  e.g. restrict to opers
 */

/* Restrict module bot to only respond to oper requests
 * when ONLY_OPERS is set in the config file
 */
#define BOT_FLAG_ONLY_OPERS		0x00000001
/* Restrict module bot to only respond to oper requests
 * regardless of ONLY_OPERS setting in the config file
 */
#define BOT_FLAG_RESTRICT_OPERS	0x00000002

/* SET Comand handling */
/* (Work in progress) */

typedef enum SET_TYPE {
	SET_TYPE_BOOLEAN,
	SET_TYPE_INT,
	SET_TYPE_INTRANGE,
	SET_TYPE_STRING,
	SET_TYPE_STRINGRANGE,
	SET_TYPE_NICK,
	SET_TYPE_USER,
	SET_TYPE_HOST,
	SET_TYPE_RNAME,
	SET_TYPE_CUSTOM,
}SET_TYPE;

/* "TESTSTRING", &teststring, TYPE_STRING, 0,string_buffer_size 
   "TESTINT",    &testint, TYPE_INT 0, 200 */ 
typedef struct bot_settings {
	const char		*option;	/* option string */
	void*			varptr;		/* pointer to var */
	SET_TYPE		type;		/* type of var */
	unsigned int	min;		/* min value */
	unsigned int	max;		/* max value */
	const char		*confitem;	/* config string for kptool */
	const char		*desc;		/* description of setting for messages */
	bot_cmd_handler	handler;	/* handler for custom/post-set processing */
}bot_settings;

/* sock.c */
int sock_connect (int socktype, unsigned long ipaddr, int port, char *sockname, char *module, char *func_read, char *func_write, char *func_error);
int sock_disconnect (char *sockname);

/* conf.c */
int ConfLoad (void);
void rehash (void);
int ConfLoadModules (void);

/* main.c */
void do_exit (NS_EXIT_TYPE exitcode, char* quitmsg);
void fatal_error(char* file, int line, char* func, char* error_text);
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
void debugtochannel(char *message, ...);

/* ircd.c */
void parse (char* line);
char *joinbuf (char **av, int ac, int from);
int split_buf (char *buf, char ***argv, int colon_special);
int flood (User * u);
/* (M) For backwards compatibility only, bots are moving to a new interface */
int init_bot (char * nick, char * user, char * host, char * rname, const char *modes, char * modname);
int del_bot (char * nick, char * reason);

/* ircd specific files */
void prefmsg (char * to, const char * from, char * fmt, ...);
void privmsg (char *to, const char *from, char *fmt, ...);
void notice (char *to, const char *from, char *fmt, ...);
void privmsg_list (char *to, char *from, const char **text);
void globops (char * from, char * fmt, ...);

/* users.c */
User *finduser (const char *nick);
int UserLevel (User *u);

/* server.c */
Server *findserver (const char *name);

/* chans.c */
Chans *findchan (char *chan);
int CheckChanMode (Chans * c, long mode);
int IsChanMember(Chans *c, User *u);

/* dns.c */
int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data);

/* services.c */
int init_services(void);
int add_services_cmd_list(bot_cmd* cmd_list);
int del_services_cmd_list(bot_cmd* cmd_list);
void services_cmd_help (User * u, char **av, int ac);

#endif
