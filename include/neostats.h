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

#ifdef WIN32
#include "configwin32.h"
#else
#include "config.h"
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRING_H
#define __USE_GNU
#include <string.h>
#undef __USE_GNU
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef WIN32
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
#define MODULEFUNC 
#define MODULEVAR 
#define EXPORTVAR
#define EXPORTFUNC
#include "version.h"
#endif

/* so our defines for _(x) are not active */
#undef USEGETTEXT

#ifdef WIN32
#define _(x) (x)
#define __(x, y) (x)
#else
#ifdef HAVE_DB_H
char *LANGgettext(const char *string, int mylang);
/* our own defines for language support */
/* this one is for standard language support */
#define _(x) LANGgettext(x, me.lang)
/* this one is for custom langs based on chan/user struct */
#define __(x,y) LANGgettext(x,(y)->lang)
#else
#define _(x) (x)
#define __(x, y) (x)
#endif
#endif

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#define __attribute__(x)  /* NOTHING */
#endif

#include "pcre.h"
#include "adns.h"
#include "list.h"
#include "hash.h"
#include "support.h"
#include "events.h"
#include "numeric.h"

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
#define PROTOCOL_UNKLN		0x00008000  /* Have UNKLINE support */
#define PROTOCOL_NICKIP		0x00010000  /* NICK passes IP address */
#define PROTOCOL_KICKPART	0x00020000  /* KICK also generates PART */

#define PROTOCOL_CLIENTMODE	0x80000000  /* Client mode */

#define FEATURE_SWHOIS		0x00000001	/* SWHOIS */
#define FEATURE_SVSTIME		0x00000002	/* SVSTIME */
#define FEATURE_SVSHOST		0x00000004	/* SVSHOST */
#define FEATURE_SVSJOIN		0x00000008	/* SVSJOIN */
#define FEATURE_SVSMODE		0x00000010	/* SVSMODE */
#define FEATURE_SVSPART		0x00000020	/* SVSPART */
#define FEATURE_SVSNICK		0x00000040	/* SVSNICK */
#define FEATURE_SVSKILL		0x00000080	/* SVSKILL */
#define FEATURE_UMODECLOAK  0x00000100	/* auto cloak host with umode */
#define FEATURE_USERSMODES	0x00000200	/* User Smode field */
#define FEATURE_SMO			0x00000400	/* SMO */

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
#define UMODE_BOT			0x00000080	/* User is a bot */

#define UMODE_RBOT			UMODE_BOT	/* Registered Bot */
#define UMODE_SBOT			UMODE_BOT	/* Server Bot */

#define UMODE_SADMIN		0x00000100	/* Services Admin */
#define UMODE_ADMIN			0x00000200	/* Admin */
#define UMODE_SERVICES		0x00000400	/* services */
#define UMODE_NETADMIN		0x00000800	/* Network Admin */
#define UMODE_COADMIN		0x00001000	/* Co Admin */
#define UMODE_TECHADMIN     0x00002000  /* Technical Administrator */
#define UMODE_CLIENT		0x00004000	/* Show client information */
#define UMODE_FCLIENT		0x00008000	/* recieve client on far connects.. */
#define UMODE_KIX			0x00010000	/* protected oper, only ulines can kick */

#define UMODE_HELPOP		0x00020000	/* Help system operator */
#define UMODE_RGSTRONLY		0x00040000	/* only registered nicks may PM */

/* Other user modes available on IRCds cannot be easily supported so 
 * should be defined locally beginning at 0x00000080
 */

/* Smodes */
#define SMODE_SSL			0x00000001	/* ssl client */
#define SMODE_COADMIN		0x00000002	/* co admin on a server */
#define SMODE_ADMIN			0x00000004	/* server admin */
#define SMODE_COTECHADMIN	0x00000008	/* co-tech admin */
#define SMODE_TECHADMIN		0x00000010	/* tech administrator */
#define SMODE_CONETADMIN	0x00000020	/* Co-Network Admin */
#define SMODE_NETADMIN		0x00000040	/* Network Admin */
#define SMODE_GUESTADMIN	0x00000080	/* Guest Admin */

EXPORTVAR extern unsigned int ircd_supported_umodes;
EXPORTVAR extern unsigned int ircd_supported_smodes;
EXPORTVAR extern unsigned int ircd_supported_cmodes;
EXPORTVAR extern unsigned int ircd_supported_cumodes;
#define HaveUmodeRegNick() (ircd_supported_umodes&UMODE_REGNICK)
#define HaveUmodeDeaf() (ircd_supported_umodes&UMODE_DEAF)

/* Umode macros */
/* ifdef checks for macros until umodes updated */
#define is_oper(x) ((x) && ((x->user->Umode & (UMODE_OPER|UMODE_LOCOP))))
#define is_bot(x) ((x) && (x->user->Umode & UMODE_BOT))

#define BOTMODE		0x00000001
#define OPERMODE	0x00000002

EXPORTFUNC int IsOperMode(const char mode);
EXPORTFUNC int IsOperSMode(const char mode);
EXPORTFUNC int IsBotMode(const char mode);
EXPORTFUNC int UmodeCharToMask(const char mode);
EXPORTFUNC const char *GetUmodeDesc (const unsigned int mask);
EXPORTFUNC int SmodeCharToMask(const char mode);
EXPORTFUNC const char *GetSmodeDesc (const unsigned int mask);
EXPORTFUNC unsigned int UmodeStringToMask (const char *UmodeString);
EXPORTFUNC char *UmodeMaskToString (const unsigned int mask);
EXPORTFUNC char UmodeMaskToChar (const unsigned int mask);
EXPORTFUNC unsigned int SmodeStringToMask (const char *UmodeString);
EXPORTFUNC char *SmodeMaskToString (const unsigned int mask);
EXPORTFUNC char SmodeMaskToChar (const unsigned int mask);
EXPORTFUNC unsigned int CmodeStringToMask (const char *UmodeString);
EXPORTFUNC char *CmodeMaskToString (const unsigned int mask);
EXPORTFUNC char *CmodeMaskToPrefixString (const unsigned int mask);
EXPORTFUNC int CmodeCharToMask (const char mode);
EXPORTFUNC char CmodeMaskToChar (const unsigned int mask);
EXPORTFUNC int CmodeCharToFlags (const char mode);
EXPORTFUNC unsigned int CmodePrefixToMask (const char prefix);
EXPORTFUNC char CmodePrefixToChar (const char prefix);
EXPORTFUNC char CmodeMaskToPrefix (const unsigned int mask);
EXPORTFUNC char CmodeCharToPrefix (const char mode);


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

#define MOD_PATH		"modules"
#define RECV_LOG		"logs/recv.log"

#define BASE64SERVERSIZE	2
#define BASE64NICKSIZE		5

#define BUFSIZE			512

#define MAXHOST			(128 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(32 + 1)
#define MAXUSER			(15 + 1)
#define MAXREALNAME		(50 + 1)
#define MAXCHANLEN		(50 + 1)
#define MAXTOPICLEN		(307 + 1)
#define CLOAKKEYLEN		(40 + 1)

#define HOSTIPLEN		15	/* Size of IP address in dotted quad */

#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10
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

#define is_synched		me.synched

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

#define NS_TRUE			1
#define NS_FALSE		0

/* General flags for for clients (users and servers) and channels */
#define NS_FLAG_EXCLUDED	0x00000001 /* matches a exclusion */

/* Flags for clients (users and servers) */
#define CLIENT_FLAG_EXCLUDED	NS_FLAG_EXCLUDED /* client is excluded */
#define CLIENT_FLAG_ME			0x00000002 /* client is a NeoStats one */
#define CLIENT_FLAG_SYNCHED		0x00000004 /* client is synched */
#define CLIENT_FLAG_SETHOST		0x00000008 /* client is synched */

#if 0
#define NS_FLAGS_NETJOIN	0x00000008 /* client is on a net join */
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

EXPORTVAR extern char segv_location[SEGV_LOCATION_BUFSIZE];

/* this is the dns structure */
extern adns_state ads;

/* version info */
EXPORTVAR extern const char version_date[];
EXPORTVAR extern const char version_time[];

typedef struct _Bot Bot;

/** @brief Server structure
 *  structure containing all details of a server
 */
typedef struct Server {
	int hops;
	int numeric;
	int ping;
	time_t uptime;
} Server;

/** @brief User structure
 *  
 */
typedef struct User {
	char hostname[MAXHOST];
	char username[MAXUSER];
	char vhost[MAXHOST];
	char awaymsg[MAXHOST];
	char swhois[MAXHOST];
	int flood;
	int is_away;
	time_t tslastmsg;
	time_t tslastnick;
	time_t tslastaway;
	time_t servicestamp;
	char modes[MODESIZE];
	unsigned int Umode;
	char smodes[MODESIZE];
	unsigned int Smode;
	int ulevel;
	list_t *chans;
	Bot *bot;
} User;

/** @brief Client structure
 *  
 */
typedef struct Client {
	User *user;
	Server *server;
	char name[MAXNICK];
	char name64[B64SIZE];
	char uplinkname[MAXHOST];
	struct Client* uplink;
	char info[MAXREALNAME];
	char version[MAXHOST];
	unsigned int flags;
	time_t tsconnect;
	struct in_addr ip;
	char hostip[HOSTIPLEN];
	int lang;
	void *moddata[NUM_MODULES];
} Client; 

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
	int port;
	int lang;
	time_t t_start;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_nickip:1;
	char servicescmode[MODESIZE];
	unsigned int servicescmodemask;
	char servicesumode[MODESIZE];
	unsigned int servicesumodemask;
	char serviceschan[MAXCHANLEN];
	unsigned int synched:1;
	Client *s;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
	time_t now;
	char strnow[STR_TIME_T_SIZE];
	char version[VERSIONSIZE];
	int dobind;
	struct sockaddr_in lsa;
} tme;

EXPORTVAR extern tme me;

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


/** @brief ModesParm structure
 *  
 */
typedef struct ModesParm {
	unsigned int mask;
	char param[PARAMSIZE];
} ModesParm;

/** @brief Chanmem structure
 *  
 */
typedef struct Chanmem {
	char nick[MAXNICK];
	time_t tsjoin;
	long flags;
	void *moddata[NUM_MODULES];
} Chanmem;

/** @brief Channel structure
 *  
 */
typedef struct Channel {
	char name[MAXCHANLEN];
	char name64[B64SIZE];
	unsigned int users;
	unsigned int neousers;
	unsigned int persistentusers;
	int lang;
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

typedef struct CmdParams {
	Client *source;
	Client *target;
	Bot *bot;
	char *param;
	char *cmd;
	Channel* channel;
	char **av;
	int ac;
} CmdParams; 

/** @brief ping structure
 *  
 */
struct ping {
	time_t last_sent;
	int ulag;
} ping;

/* Comand list handling */

/** @brief bot_cmd_handler type
 *  defines handler function definition
 */

typedef enum SET_REASON {
	SET_LOAD = 0,
	SET_LIST,
	SET_CHANGE,
} SET_REASON;

typedef int (*bot_cmd_handler) (CmdParams* cmdparams);
typedef int (*bot_set_handler) (CmdParams* cmdparams, SET_REASON reason);

/** @brief bot_cmd structure
 *  defines command lists for bots
 */
typedef struct bot_cmd {
	const char		*cmd;		/* command string */
	bot_cmd_handler	handler;	/* handler */
	int				minparams;	/* min num params */
	int				ulevel;		/* min user level */
	const char		**helptext;	/* pointer to help text */
	const char		*onelinehelp;/* single line help for generic help function */
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
/* Mark bot as persistent even when no users are left in a channel
 * If not set, and there are no bots with this flag set, when all
 * users leave a channel, the bot will automatically leave aswell.
 * You should watch the NEWCHAN event to join channels when they
 * are created. 
 */
#define BOT_FLAG_PERSIST	0x00000010

/* This defines a "NULL" string for the purpose of BotInfo structures that 
 * want to inherit the main host used by NeoStats and still make the info
 * readable
 */
#define BOT_COMMON_HOST	""

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

/** @brief bot_setting structure
 *  defines SET list for bots
 */
typedef struct bot_setting {
	char			*option;	/* option string */
	void			*varptr;		/* pointer to var */
	SET_TYPE		type;		/* type of var */
	int				min;		/* min value */
	int				max;		/* max value */
	int				ulevel;		/* min user level */
	char			*confitem;	/* config string for kptool */
	const char		*desc;		/* description of setting for messages e.g. seconds, days*/
	const char		**helptext;	/* pointer to help text */
	bot_set_handler	handler;	/* handler for custom/post-set processing */
	void			*defaultval; /* default value for setting */
}bot_setting;


/** @brief Message function types
 * 
 */
typedef int (*timer_function) (void);

/** @brief Socket function types
 * 
 */
typedef int (*sock_func) (int sock_no, char *name);
typedef int (*before_poll_func) (void *data, struct pollfd *);
typedef void (*after_poll_func) (void *data, struct pollfd *, unsigned int);

/* socket interface type */
#define SOCK_POLL 1
#define SOCK_STANDARD 2

/** @brief ModuleEvent functions structure
 * 
 */

#define	EVENT_FLAG_DISABLED			0x00000001
#define	EVENT_FLAG_IGNORE_SYNCH		0x00000002
#define	EVENT_FLAG_EXCLUDE_ME		0x00000004
#define	EVENT_FLAG_EXCLUDE_MODME	0x00000008
#define	EVENT_FLAG_USE_EXCLUDE		0x00000010

typedef int (*event_function) (CmdParams *cmdparams);

typedef struct ModuleEvent {
	Event event;
	event_function function;
	unsigned int flags;
}ModuleEvent;

typedef int ModuleProtocol;
typedef int ModuleFeatures;

typedef enum ModuleFlags {
	MODULE_FLAG_NONE = 0,
	MODULE_FLAG_AUTH
} ModuleFlags;

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

typedef int (*mod_auth) (Client *u);
typedef int (*userauthfunc) (Client *u);

/** @brief Module structure
 * 
 */
typedef struct Module {
	ModuleInfo *info;
	ModuleEvent **event_list;
	mod_auth mod_auth_cb;
	mod_auth userauth;
	void *dl_handle;
	unsigned int modnum;
	unsigned int insynch;
	unsigned int synched;
	unsigned int error;
}Module;

extern Module *RunModule[10];
extern int RunLevel;

/* Simple stack to manage run level replacing original segv_module stuff 
 * which will hopefully make it easier to determine where we are running
 * and avoid the need for modules to ever manage this and the core to
 * have to set/reset when a module calls a core function which triggers
 * other modules to run (e.g. AddBot)
 */
#define SET_RUN_LEVEL(moduleptr) {if(RunLevel<10){RunLevel++;RunModule[RunLevel] = moduleptr;}}
#define RESET_RUN_LEVEL() {if(RunLevel>0){RunLevel--;}}
#define GET_CUR_MODULE() RunModule[RunLevel]
#define GET_CUR_MODNUM() RunModule[RunLevel]->modnum
#define GET_CUR_MODNAME() RunModule[RunLevel]->info->name

/** @brief Module socket list structure
 * 
 */
typedef struct Sock {
	/** Owner module ptr */
	Module *moduleptr;
	/** Socket number */
	int sock_no;
	/** Socket name */
	char name[MAX_MOD_NAME];
	/** socket interface (poll or standard) type */
	int socktype;
	/** if socktype = SOCK_POLL, before poll function */
	/** Socket before poll function */
	before_poll_func beforepoll;
	/** Socket after poll function */
	after_poll_func afterpoll;
	/** data */
	void *data;
	/* if socktype = SOCK_STANDARD, function calls */
	/** Socket read function */
	sock_func readfnc;
	/** Socket write function */
	sock_func writefnc;
	/** Socket error function */
	sock_func errfnc;
	/** rmsgs */
	long rmsgs;
	/** rbytes */
	long rbytes;
} Sock;

typedef enum TIMER_TYPE {
	TIMER_TYPE_INTERVAL,
	TIMER_TYPE_MIDNIGHT,
	TIMER_TYPE_COUNTDOWN,
} TIMER_TYPE;

/** @brief Module Timer structure
 * 
 */
typedef struct Timer {
	/** Owner module ptr */
	Module *moduleptr;
	/** Timer type */
	TIMER_TYPE type;
	/** Timer name */
	char name[MAX_MOD_NAME];
	/** Timer interval */
	int interval;
	/** Timer at values */
	int hours;
	int minutes;
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
	/* OPTIONAL: flags */
	unsigned int flags;
	/* OPTIONAL: bot command list pointer */
	bot_cmd *bot_cmd_list;
	/* OPTIONAL: bot command setting pointer */
	bot_setting *bot_setting_list;
} BotInfo;

/** @brief Bot structure
 * 
 */

typedef struct _Bot {
	/** Owner module ptr */
	Module *moduleptr;
	/** Nick */
	char name[MAXNICK];
	/* bot flags */
	unsigned int flags;
	/* hash for command list */
	hash_t *botcmds;
	/* hash for settings */
	hash_t *botsettings;
	/* hash for bot_info settings */
	bot_setting *bot_info_settings;
	/* min ulevel for settings */
	int set_ulevel;
	/* Link back to user struct associated with this bot*/
	Client *u;
}_Bot;

EXPORTFUNC int ModuleConfig(bot_setting *bot_settings);

EXPORTFUNC int add_timer (TIMER_TYPE type, timer_function func, const char *name, int interval);
EXPORTFUNC int del_timer (const char *timer_name);
EXPORTFUNC int set_timer_interval (const char *timer_name, int interval);
EXPORTFUNC Timer *find_timer(const char *timer_name);

EXPORTFUNC int add_sock (const char *sock_name, int socknum, sock_func readfunc, sock_func writefunc, sock_func errfunc);
EXPORTFUNC int add_sockpoll (const char *sock_name, void *data, before_poll_func beforepoll, after_poll_func afterpoll);
EXPORTFUNC int del_sock (const char *sock_name);
EXPORTFUNC Sock *find_sock (const char *sock_name);
EXPORTFUNC int sock_connect (int socktype, unsigned long ipaddr, int port, const char *name, sock_func func_read, sock_func func_write, sock_func func_error);
EXPORTFUNC int sock_disconnect (const char *name);

EXPORTFUNC Bot *AddBot (BotInfo *botinfo);
EXPORTFUNC Bot *find_bot (const char *bot_name);

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
void do_exit (NS_EXIT_TYPE exitcode, char *quitmsg) __attribute__((noreturn));
EXPORTFUNC void fatal_error(char *file, int line, char *func, char *error_text) __attribute__((noreturn));;
#define FATAL_ERROR(error_text) fatal_error(__FILE__, __LINE__, __PRETTY_FUNCTION__,(error_text)); 

/* nsmemory.c */
EXPORTFUNC void *ns_malloc (const int size);
EXPORTFUNC void *ns_calloc (const int size);
EXPORTFUNC void *ns_realloc (void *ptr, const int size);
EXPORTFUNC void _ns_free (void **buf);
#define ns_free(ptr) _ns_free ((void **) &(ptr));

/* misc.c */
EXPORTFUNC void strip (char *line);
EXPORTFUNC char *sstrdup (const char *s);
char *strlwr (char *s);
EXPORTFUNC void AddStringToList (char ***List, char S[], int *C);
EXPORTFUNC void strip_mirc_codes(char *text);
EXPORTFUNC char *sctime (time_t t);
EXPORTFUNC char *sftime (time_t t);
EXPORTFUNC char *make_safe_filename (char *name);

/* ircd.c */
EXPORTFUNC char *joinbuf (char **av, int ac, int from);
EXPORTFUNC int split_buf (char *buf, char ***argv, int colon_special);

/* IRC interface for modules 
 *  Modules use these functions to perform actions on IRC
 *  They use a similar naming convention to the same actions as IRC commands 
 *  issued by users from an IRC client.
 */

/*  Messaging functions to send messages to users and channels
 */
EXPORTFUNC int irc_prefmsg (const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_prefmsg_list (const Bot *botptr, const Client *target, const char **text);
EXPORTFUNC int irc_privmsg (const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_privmsg_list (const Bot *botptr, const Client *target, const char **text);
EXPORTFUNC int irc_notice (const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_chanprivmsg (const Bot *botptr, const char *chan, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_channotice (const Bot *botptr, const char *chan, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

/*  Specialised messaging functions for global messages, services channel 
 *  alerts and numeric responses
 */
EXPORTFUNC int irc_chanalert (const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_globops (const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_wallops (const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_numeric (const int numeric, const char *target, const char *data, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

/*  General irc actions for join/part channels etc
 */
EXPORTFUNC int irc_nickchange (const Bot *botptr, const char *newnick);
EXPORTFUNC int irc_quit (const Bot *botptr, const char *quitmsg);
EXPORTFUNC int irc_join (const Bot *botptr, const char *chan, const char *chanmodes);
EXPORTFUNC int irc_part (const Bot *botptr, const char *chan);
EXPORTFUNC int irc_kick (const Bot *botptr, const char *chan, const char *target, const char *reason);
EXPORTFUNC int irc_invite (const Bot *botptr, const char *target, const char *chan);
EXPORTFUNC int irc_cloakhost (const Bot *botptr);
EXPORTFUNC int irc_setname (const Bot *botptr, const char *realname);

/*  Mode functions
 */
EXPORTFUNC int irc_umode (const Bot *botptr, const char *target, long mode);
EXPORTFUNC int irc_cmode (const Bot *botptr, const char *chan, const char *mode, const char *args);
EXPORTFUNC int irc_chanusermode (const Bot *botptr, const char *chan, const char *mode, const char *target);

/*  Oper functions
 *  Require an opered bot to operate
 */
EXPORTFUNC int irc_kill (const Bot *botptr, const char *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_akill (const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ...);
EXPORTFUNC int irc_rakill (const Bot *botptr, const char *host, const char *ident);
EXPORTFUNC int irc_swhois (const char *target, const char *swhois);
EXPORTFUNC int irc_sethost (const Bot *botptr, const char *host);
EXPORTFUNC int irc_setident (const Bot *botptr, const char *ident);

int irc_ping (const char *source, const char *reply, const char *to);
int irc_pong (const char *reply);

/*  SVS functions 
 *  these operate from the server rather than a bot 
 */
EXPORTFUNC int irc_svsnick (const Bot *botptr, Client *target, const char *newnick);
EXPORTFUNC int irc_svsjoin (const Bot *botptr, Client *target, const char *chan);
EXPORTFUNC int irc_svspart (const Bot *botptr, Client *target, const char *chan);
EXPORTFUNC int irc_svshost (const Bot *botptr, Client *target, const char *vhost);
EXPORTFUNC int irc_svsmode (const Bot *botptr, Client *target, const char *modes);
EXPORTFUNC int irc_svskill (const Bot *botptr, Client *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_svstime (const Bot *botptr, Client *target, const time_t ts);

/*  CTCP functions to correctly format CTCP requests and replies
 */
EXPORTFUNC int irc_ctcp_version_req (Bot* botptr, Client* target);
EXPORTFUNC int irc_ctcp_version_rpl (Bot* botptr, Client* target, const char *version);
EXPORTFUNC int irc_ctcp_ping_req (Bot* botptr, Client* target);

EXPORTFUNC int irc_ctcp_finger_req (Bot* botptr, Client* target);

EXPORTFUNC int irc_ctcp_action_req (Bot* botptr, Client* target, const char *action);

/* users.c */
EXPORTFUNC Client *find_user (const char *nick);
EXPORTFUNC int UserLevel (Client *u);

/* server.c */
EXPORTFUNC Client *find_server (const char *name);

/* chans.c */
EXPORTFUNC Channel *find_chan (const char *chan);
EXPORTFUNC int CheckChanMode (Channel *c, const unsigned int mode);
EXPORTFUNC int IsChanMember(Channel *c, Client *u);
EXPORTFUNC int test_cumode(char *chan, char *nick, int flag);

#define is_chanop(chan, nick)		test_cumode(chan, nick, CUMODE_CHANOP)
#define is_chanhalfop(chan, nick)	test_cumode(chan, nick, CUMODE_HALFOP)
#define is_chanvoice(chan, nick)	test_cumode(chan, nick, CUMODE_VOICE)
#define is_chanowner(chan, nick)	test_cumode(chan, nick, CUMODE_CHANOWNER)
#define is_chanprot(chan, nick)		test_cumode(chan, nick, CUMODE_CHANPROT)
#define is_chanadmin(chan, nick)	test_cumode(chan, nick, CUMODE_CHANADMIN)

EXPORTVAR unsigned char UmodeChRegNick;

EXPORTFUNC int IsBotMode(const char mode);

/* dns.c */
EXPORTFUNC int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data);

/* services.c */
EXPORTFUNC int add_services_cmd_list (bot_cmd* bot_cmd_list);
EXPORTFUNC int add_services_set_list (bot_setting *bot_setting_list);
EXPORTFUNC int del_services_cmd_list (bot_cmd* bot_cmd_list);
EXPORTFUNC int del_services_set_list (bot_setting *bot_setting_list);
EXPORTFUNC Client *find_valid_user(Bot* botptr, Client *u, const char *target_nick);

/* transfer.c stuff */
typedef void (transfer_callback) (void *data, int returncode, char *body, int bodysize);
EXPORTFUNC void transfer_status(void);
EXPORTFUNC int new_transfer(char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback);

/* exclude */
#define IsExcluded(x) ((x) && ((x)->flags & NS_FLAG_EXCLUDED))

/* Is the user or server a NeoStats one? */
#define IsMe(x) ((x) && ((x)->flags & CLIENT_FLAG_ME))

/* Is the user or server synched? */
#define IsSynched(x) ((x) && ((x)->flags & CLIENT_FLAG_SYNCHED))

/* Mark server as synched */
#define SynchServer(x) (((x)->flags |= CLIENT_FLAG_SYNCHED))

/* Has NeoStats issued a SETHOST for this user? */
#define IsUserSetHosted(x)  ((x) && ((x)->flags & CLIENT_FLAG_SETHOST))

EXPORTFUNC int validate_nick (char *nick);
EXPORTFUNC int validate_user (char *username);
EXPORTFUNC int validate_host (char *hostname);
EXPORTFUNC int validate_url (char *url);
EXPORTFUNC int validate_channel (char *channel_name);

#ifdef HAVE_DB_H
EXPORTFUNC int DBOpenDatabase(void);
EXPORTFUNC void DBCloseDatabase(void);
EXPORTFUNC void *DBGetData(char *key);
EXPORTFUNC void DBSetData(char *key, void *data, int size);
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
	DEBUGRX=1,
	DEBUGTX,
	DEBUG1,	/* debug notices about important functions that are going on */
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

typedef void (*ChannelListHandler) (Channel *c);
EXPORTFUNC void GetChannelList(ChannelListHandler handler);
typedef void (*UserListHandler) (Client *u);
EXPORTFUNC void GetUserList(UserListHandler handler);
typedef void (*ServerListHandler) (Client *s);
EXPORTFUNC void GetServerList(ServerListHandler handler);

EXPORTFUNC hash_t *GetServerHash (void);
EXPORTFUNC hash_t *GetBanHash (void);
EXPORTFUNC hash_t *GetChannelHash (void);
EXPORTFUNC hash_t *GetUserHash (void);
EXPORTFUNC hash_t *GetModuleHash (void);

EXPORTFUNC int HaveFeature (int mask);

EXPORTFUNC void RegisterEvent (ModuleEvent *event);
EXPORTFUNC void RegisterEventList (ModuleEvent *event);
EXPORTFUNC void DeleteEvent (Event event);
EXPORTFUNC void DeleteEventList (ModuleEvent *event);
EXPORTFUNC void SetAllEventFlags (unsigned int flag, unsigned int enable);
EXPORTFUNC void SetEventFlags (Event event, unsigned int flag, unsigned int enable);
EXPORTFUNC void EnableEvent (Event event);
EXPORTFUNC void DisableEvent (Event event);

/* String functions */
/* [v]s[n]printf replacements */
EXPORTFUNC int ircvsprintf(char *buf, const char *fmt, va_list args);
EXPORTFUNC int ircvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
EXPORTFUNC int ircsprintf(char *buf, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int ircsnprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

/* str[n]casecmp replacements */
EXPORTFUNC int ircstrcasecmp(const char *s1, const char *s2);
EXPORTFUNC int ircstrncasecmp(const char *s1, const char *s2, size_t size);

EXPORTFUNC extern int match(const char *mask, const char *name);

/* 
 *  Portability wrapper functions
 */

/* File system functions */
EXPORTVAR int os_errno;

EXPORTFUNC int os_mkdir (const char *filename, mode_t mode);
EXPORTFUNC int os_check_create_dir (const char *dirname);
EXPORTFUNC FILE *os_fopen (const char *filename, const char *filemode);
EXPORTFUNC int os_fclose (FILE *handle);
EXPORTFUNC int os_fseek (FILE *handle, long offset, int origin);
EXPORTFUNC long os_ftell (FILE *handle);
EXPORTFUNC int os_fprintf (FILE *handle, char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int os_fread (void *buffer, size_t size, size_t count, FILE *handle);
EXPORTFUNC char *os_fgets (char *string, int n, FILE *handle);
EXPORTFUNC int os_fwrite (const void *buffer, size_t size, size_t count, FILE *handle);
EXPORTFUNC int os_fflush (FILE *handle);
EXPORTFUNC int os_rename (const char *oldname, const char *newname);
EXPORTFUNC int os_stat (const char *path, struct stat *buffer);
EXPORTFUNC int os_access(const char *path, int mode);
EXPORTFUNC char *os_strerror (void);
EXPORTFUNC size_t os_strftime (char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);
EXPORTFUNC struct tm* os_localtime (const time_t *timer);
EXPORTFUNC int os_file_get_size (const char *filename);
EXPORTFUNC void *os_memset (void *dest, int c, size_t count);
EXPORTFUNC void *os_memcpy (void *dest, const void *src, size_t count);

/* Socket functions */
#ifdef WIN32
typedef SOCKET OS_SOCKET;
#else
typedef int OS_SOCKET;
#endif
EXPORTFUNC int os_sock_close (OS_SOCKET sock);
EXPORTFUNC int os_sock_write (OS_SOCKET s, const char *buf, int len);
EXPORTFUNC int os_sock_read (OS_SOCKET s, char *buf, int len);
EXPORTFUNC int os_sock_set_nonblocking (OS_SOCKET s);

/* 
 * Module Interface 
 */
/* Module Basic Interface */
MODULEVAR extern ModuleInfo module_info;   
MODULEFUNC int ModInit (Module *mod_ptr);
MODULEFUNC int ModSynch (void);
MODULEFUNC void ModFini (void);
/* Module Event Interface */
MODULEVAR extern ModuleEvent module_events[];  
/* Module Auth Interface */
MODULEFUNC int ModAuthUser (Client *u);

EXPORTFUNC void clear_channel_moddata (Channel* c);
EXPORTFUNC void set_channel_moddata (Channel* c, void *data);
EXPORTFUNC void *get_channel_moddata (Channel* c);
EXPORTFUNC void clear_user_moddata (Client* u);
EXPORTFUNC void set_user_moddata (Client* u, void *data);
EXPORTFUNC void *get_user_moddata (Client* u);
EXPORTFUNC void clear_server_moddata (Client* s);
EXPORTFUNC void set_server_moddata (Client* s, void *data);
EXPORTFUNC void *get_server_moddata (Client* s);

EXPORTFUNC void rtaserv_add_table (void *ptbl);


#endif /* NEOSTATS_H */
