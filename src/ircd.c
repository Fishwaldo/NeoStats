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
#include "neostats.h"
#include "ircd.h"
#include "modules.h"
#include "dl.h"
#include "bots.h"
#include "commands.h"
#include "sock.h"
#include "users.h"
#include "channels.h"
#include "services.h"
#include "servers.h"
#include "bans.h"
#include "auth.h"
#include "dns.h"

ircd_server ircd_srv;

static char ircd_buf[BUFSIZE];
static char UmodeStringBuf[64];
static char SmodeStringBuf[64];
long service_umode_mask = 0;
unsigned int ircd_supported_umodes = 0;
unsigned int ircd_supported_smodes = 0;

ChanModes ircd_cmodes[MODE_TABLE_SIZE];
UserModes ircd_umodes[MODE_TABLE_SIZE];
UserModes ircd_smodes[MODE_TABLE_SIZE];
ProtocolInfo* protocol_info;
char* services_cmode;
char* services_umode;

ircd_cmd* cmd_list;
cumode_init* chan_umodes;
cmode_init* chan_modes;
umode_init* user_umodes;
umode_init* user_smodes;

static void *protocol_module_handle;

void (*irc_send_privmsg) (const char *from, const char *to, const char *buf);
void (*irc_send_notice) (const char *from, const char *to, const char *buf);
void (*irc_send_globops) (const char *from, const char *buf);
void (*irc_send_wallops) (const char *who, const char *buf);
void (*irc_send_numeric) (const char *from, const int numeric, const char *target, const char *buf);
void (*irc_send_umode) (const char *who, const char *target, const char *mode);
void (*irc_send_join) (const char *sender, const char *who, const char *chan, const unsigned long ts);
void (*irc_send_sjoin) (const char *sender, const char *who, const char *chan, const unsigned long ts);
void (*irc_send_part) (const char *who, const char *chan);
void (*irc_send_nickchange) (const char *oldnick, const char *newnick, const unsigned long ts);
void (*irc_send_cmode) (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
void (*irc_send_quit) (const char *who, const char *quitmsg);
void (*irc_send_kill) (const char *from, const char *target, const char *reason);
void (*irc_send_kick) (const char *who, const char *chan, const char *target, const char *reason);
void (*irc_send_invite) (const char *from, const char *to, const char *chan);
void (*irc_send_svskill) (const char *sender, const char *target, const char *reason);
void (*irc_send_svsmode) (const char *sender, const char *target, const char *modes);
void (*irc_send_svshost) (const char *sender, const char *who, const char *vhost);
void (*irc_send_svsjoin) (const char *sender, const char *target, const char *chan);
void (*irc_send_svspart) (const char *sender, const char *target, const char *chan);
void (*irc_send_svsnick) (const char *sender, const char *target, const char *newnick, const unsigned long ts);
void (*irc_send_swhois) (const char *sender, const char *target, const char *swhois);
void (*irc_send_smo) (const char *from, const char *umodetarget, const char *msg);
void (*irc_send_akill) (const char *sender, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, unsigned long ts);
void (*irc_send_rakill) (const char *sender, const char *host, const char *ident);
void (*irc_send_ping) (const char *from, const char *reply, const char *to);
void (*irc_send_pong) (const char *reply);
void (*irc_send_server) (const char *sender, const char *name, const int numeric, const char *infoline);
void (*irc_send_squit) (const char *server, const char *quitmsg);
void (*irc_send_nick) (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
void (*irc_send_server_connect) (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
void (*irc_send_netinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
void (*irc_send_snetinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
void (*irc_send_svinfo) (const int tscurrent, const int tsmin, const unsigned long tsnow);
void (*irc_send_vctrl) (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
void (*irc_send_burst) (int b);
void (*irc_send_svstime) (const char *sender, const unsigned long ts);

static void m_numeric242 (char *origin, char **argv, int argc, int srv);
static void m_numeric351 (char *origin, char **argv, int argc, int srv);

ircd_cmd numeric_cmd_list[] = {
	/*Message	Token	Function	usage */
	{"351", "351", m_numeric351, 0},
	{"242", "242", m_numeric242, 0},
	{0, 0, 0, 0},
};


/** @brief InitIrcdCalls
 *
 *  Map our function pointers to protocol functions
 *
 * @return 
 */
static void
InitIrcdCalls (void)
{
	static char protocol_name[MAXHOST];
  
#ifdef WIN32
	ircsnprintf (protocol_name, 255, "%s/%s.dll", MOD_PATH, me.protocol);
#else
	ircsnprintf (protocol_name, 255, "%s/%s.so", MOD_PATH, me.protocol);
#endif
	protocol_module_handle= ns_dlopen(protocol_name, RTLD_NOW || RTLD_GLOBAL);
	protocol_info = ns_dlsym( protocol_module_handle, "protocol_info");
	services_umode = protocol_info->services_umode;
	services_cmode = protocol_info->services_cmode;

	strcpy(me.servicescmode,protocol_info->services_cmode);
	strcpy(me.servicesumode,protocol_info->services_umode);

	irc_parse = ns_dlsym( protocol_module_handle, "parse");
	if(irc_parse == NULL)
		irc_parse = parse;

	cmd_list    = ns_dlsym( protocol_module_handle, "cmd_list");
	chan_umodes = ns_dlsym( protocol_module_handle, "chan_umodes");
	chan_modes  = ns_dlsym( protocol_module_handle, "chan_modes");
	user_umodes = ns_dlsym( protocol_module_handle, "user_umodes");
	user_smodes = ns_dlsym( protocol_module_handle, "user_smodes");

	irc_send_privmsg = ns_dlsym( protocol_module_handle, "send_privmsg");
	irc_send_notice = ns_dlsym( protocol_module_handle, "send_notice");
	irc_send_globops = ns_dlsym( protocol_module_handle, "send_globops");
	irc_send_wallops = ns_dlsym( protocol_module_handle, "send_wallops");
	irc_send_numeric = ns_dlsym( protocol_module_handle, "send_numeric");
	irc_send_umode = ns_dlsym( protocol_module_handle, "send_umode");
	irc_send_join = ns_dlsym( protocol_module_handle, "send_join");
	irc_send_sjoin = ns_dlsym( protocol_module_handle, "send_sjoin");
	irc_send_part = ns_dlsym( protocol_module_handle, "send_part");
	irc_send_nickchange = ns_dlsym( protocol_module_handle, "send_nickchange");
	irc_send_cmode = ns_dlsym( protocol_module_handle, "send_cmode");
	irc_send_quit = ns_dlsym( protocol_module_handle, "send_quit");
	irc_send_kill = ns_dlsym( protocol_module_handle, "send_kill");
	irc_send_kick = ns_dlsym( protocol_module_handle, "send_kick");
	irc_send_invite = ns_dlsym( protocol_module_handle, "send_invite");
	irc_send_svskill = ns_dlsym( protocol_module_handle, "send_svskill");
	irc_send_svsmode = ns_dlsym( protocol_module_handle, "send_svsmode");
	irc_send_svshost = ns_dlsym( protocol_module_handle, "send_svshost");
	irc_send_svsjoin = ns_dlsym( protocol_module_handle, "send_svsjoin");
	irc_send_svspart = ns_dlsym( protocol_module_handle, "send_svspart");
	irc_send_svsnick = ns_dlsym( protocol_module_handle, "send_svsnick");
	irc_send_swhois = ns_dlsym( protocol_module_handle, "send_swhois");
	irc_send_smo = ns_dlsym( protocol_module_handle, "send_smo");
	irc_send_akill = ns_dlsym( protocol_module_handle, "send_akill");
	irc_send_rakill = ns_dlsym( protocol_module_handle, "send_rakill");
	irc_send_ping = ns_dlsym( protocol_module_handle, "send_ping");
	irc_send_pong = ns_dlsym( protocol_module_handle, "send_pong");
	irc_send_server = ns_dlsym( protocol_module_handle, "send_server");
	irc_send_squit = ns_dlsym( protocol_module_handle, "send_squit");
	irc_send_nick = ns_dlsym( protocol_module_handle, "send_nick");
	irc_send_server_connect = ns_dlsym( protocol_module_handle, "send_server_connect");
	irc_send_netinfo = ns_dlsym( protocol_module_handle, "send_netinfo");
	irc_send_snetinfo = ns_dlsym( protocol_module_handle, "send_snetinfo");
	irc_send_svinfo = ns_dlsym( protocol_module_handle, "send_svinfo");
	irc_send_vctrl = ns_dlsym( protocol_module_handle, "send_vctrl");
	irc_send_burst = ns_dlsym( protocol_module_handle, "send_burst");
	irc_send_svstime = ns_dlsym( protocol_module_handle, "send_svstime");
}

/** @brief InitIrcdModes
 *
 *  Build internal mode tables by translating the protocol information
 *  into a faster indexed lookup table
 *
 * @return 
 */
static void
InitIrcdModes (void)
{
	cmode_init* cmodes;
	cumode_init* cumodes;
	umode_init* umodes;
	umode_init* smodes;

	/* build cmode lookup table */
	memset(&ircd_cmodes, 0, sizeof(ircd_cmodes));
	cmodes = chan_modes;
	while(cmodes->modechar) 
	{
		dlog(DEBUG4, "Adding channel mode %c", cmodes->modechar);
		ircd_cmodes[(int)cmodes->modechar].mode = cmodes->mode;
		ircd_cmodes[(int)cmodes->modechar].flags = cmodes->flags;
		cmodes ++;
	}
	cumodes = chan_umodes;
	while(cumodes->modechar) 
	{
		dlog(DEBUG4, "Adding channel user mode %c", cumodes->modechar);
		ircd_cmodes[(int)cumodes->modechar].mode = cumodes->mode;
		ircd_cmodes[(int)cumodes->modechar].sjoin = cumodes->sjoin;
		ircd_cmodes[(int)cmodes->modechar].flags = NICKPARAM;
		cumodes ++;
	}
	/* build umode lookup table */
	memset(&ircd_umodes, 0, sizeof(ircd_umodes));
	umodes = user_umodes;
	while(umodes->modechar) 
	{
		dlog(DEBUG4, "Adding user mode %c", umodes->modechar);
		ircd_umodes[(int)umodes->modechar].umode = umodes->umode;
		/* Build supported modes mask */
		ircd_supported_umodes |= umodes->umode;
		umodes ++;
	}
	/* build smode lookup table */
	memset(&ircd_smodes, 0, sizeof(ircd_smodes));
	smodes = user_smodes;
	while(umodes->modechar) 
	{
		dlog(DEBUG4, "Adding user smode %c", smodes->modechar);
		ircd_smodes[(int)smodes->modechar].umode = smodes->umode;
		/* Build supported smodes mask */
		ircd_supported_umodes |= umodes->umode;
		smodes ++;
	}
};

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 * @return 
 */
void
InitIrcd ()
{
	/* Clear IRCD info */
	memset(&ircd_srv, 0, sizeof(ircd_srv));
	/* Setup IRCD function calls */
	InitIrcdCalls();
	/* set min protocol */
	ircd_srv.protocol = protocol_info->minprotocol;
	/* Build mode tables */
	InitIrcdModes();
	/* preset our umode mask so we do not have to calculate in real time */
	service_umode_mask = UmodeStringToMask(me.servicesumode, 0);
}

/** @brief UmodeMaskToString
 *
 *  Translate a mode mask to the string equivalent
 *
 * @return 
 */
char* 
UmodeMaskToString(const long Umode) 
{
	int i, j;

	UmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (Umode & ircd_umodes[i].umode) {
			UmodeStringBuf[j] = i;
			j++;
		}
	}
	UmodeStringBuf[j] = '\0';
	return(UmodeStringBuf);
}

/** @brief UmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
long
UmodeStringToMask(const char* UmodeString, long Umode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = (char*)UmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Umode |= ircd_umodes[(int)*tmpmode].umode;
				break;
			} else {
				Umode &= ~ircd_umodes[(int)*tmpmode].umode;
				break;
			}
		}
		tmpmode++;
	}
	return(Umode);
}

/** @brief SmodeMaskToString
 *
 *  Translate a smode mask to the string equivalent
 *
 * @return 
 */
char* 
SmodeMaskToString(const long Smode) 
{
	int i, j;

	SmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (Smode & ircd_smodes[i].umode) {
			SmodeStringBuf[j] = i;
			j++;
		}
	}
	SmodeStringBuf[j] = '\0';
	return(SmodeStringBuf);
}

/** @brief SmodeStringToMask
 *
 *  Translate a smode string to the mask equivalent
 *
 * @return 
 */
long
SmodeStringToMask(const char* SmodeString, long Smode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to smode */
	tmpmode = (char*)SmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Smode |= ircd_smodes[(int)*tmpmode].umode;
				break;
			} else {
				Smode &= ~ircd_smodes[(int)*tmpmode].umode;
				break;
			}
		}
		tmpmode++;
	}
	return(Smode);
}

/** @brief CUmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
long
CUmodeStringToMask(const char* UmodeString, long Umode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = (char*)UmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Umode |= ircd_cmodes[(int)*tmpmode].mode;
				break;
			} else {
				Umode &= ~ircd_cmodes[(int)*tmpmode].mode;
				break;
			}
		}
		tmpmode++;
	}
	return(Umode);
}

/** @brief join_bot_to_chan
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
join_bot_to_chan (const char *who, const char *chan, const char *mode)
{
	/* Use sjoin if available */
	if(ircd_srv.protocol & PROTOCOL_SJOIN) {
		time_t ts;
		Channel *c;

		c = findchan (chan);
		ts = (!c) ? me.now : c->creationtime;
		if (mode == NULL) {
			irc_send_sjoin (me.name, who, chan, (unsigned long)ts);
		} else {
			ircsnprintf (ircd_buf, BUFSIZE, "%c%s", ircd_cmodes[(int)mode[1]].sjoin, who);
			irc_send_sjoin (me.name, ircd_buf, chan, (unsigned long)ts);
		}
		join_chan (who, chan);
		if (mode) {
			ChanUserMode(chan, who, 1, CUmodeStringToMask(mode,0));
		}
	} else {
		/* sjoin not available so use normal join */
		irc_send_join (me.name, who, chan, me.now);
		join_chan (who, chan);
		if(mode) {
			schanusermode_cmd(who, chan, mode, who);
		}
	}
	return NS_SUCCESS;
}

/** @brief signon_newbot
 *
 *  work in progress to replace ircd specific routines
 *  needs sjoin fix to complete
 *
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
signon_newbot (const char *nick, const char *user, const char *host, const char *realname, const char *modes, long Umode)
{
	AddUser (nick, user, host, realname, me.name, NULL, NULL, NULL);
	irc_send_nick (nick, (unsigned long)me.now, modes, user, host, me.name, realname);
	UserMode (nick, modes);
	if ((config.allbots > 0) || (Umode & service_umode_mask)) {
		join_bot_to_chan(nick, me.serviceschan, me.servicescmode);
	}
	return NS_SUCCESS;
}

/** @brief CloakBotHost
 *
 *  Create a hidden hostmask for the bot 
 *  Currently only Unreal support via UMODE auto cloaking
 *  but function created for future use and propogation to
 *  external modules to avoid a future joint release.
 *
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
CloakHost (Bot *bot_ptr)
{
	if (protocol_info->features&FEATURE_UMODECLOAK) {
		sumode_cmd (bot_ptr->nick, bot_ptr->nick, UMODE_HIDE);
		return NS_SUCCESS;	
	}
	return NS_FAILURE;	
}

/** @brief split_buf
 * Taken from Epona - Thanks! 
 * Split a buffer into arguments and store the arguments in an
 * argument vector pointed to by argv (which will be alloced
 * as necessary); return the argument count.  If colon_special
 * is non-zero, then treat a parameter with a leading ':' as
 * the last parameter of the line, per the IRC RFC.  Destroys
 * the buffer by side effect. 
 *
 * @return 
 */
EXPORTFUNC int
splitbuf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = scalloc (sizeof (char *) * argvsize);
	argc = 0;
	/*if (*buf == ':')
		buf++;*/
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = srealloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
			if(colon_special) {
				(*argv)[argc++] = buf;
				break;
			}
		}
		s = strpbrk (buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace (*s))
				s++;
		} else {
			s = buf + strnlen (buf, BUFSIZE);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc;
}

int
split_buf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = scalloc (sizeof (char *) * argvsize);
	argc = 0;
	if (*buf == ':')
		buf++;
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = srealloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
		}
		s = strpbrk (buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace (*s))
				s++;
		} else {
			s = buf + strnlen (buf, BUFSIZE);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc;
}

/** @brief joinbuf 
 *
 * 
 *
 * @return 
 */
char *
joinbuf (char **av, int ac, int from)
{
	int i;
	char *buf;

	buf = smalloc (BUFSIZE);
	/* from is zero based while ac has base of 1. 
	 * Therefore we need to check >= before trying to perform
	 * the join.
	 * The current (null) string we return may not be needed
	 * so should be removed once all joinbuf calls are checked.
	 * Maybe we should just return NULL if we fail and let
	 * the caller handle that case. 
	 */
	if(from >= ac) {
		dlog(DEBUG1, "joinbuf: from (%d) >= ac (%d)", from, ac);
		strlcpy (buf, "(null)", BUFSIZE);
	}
	else {
		strlcpy (buf, av[from], BUFSIZE);
		for (i = from + 1; i < ac; i++) {
			strlcat (buf, " ", BUFSIZE);
			strlcat (buf, av[i], BUFSIZE);
		}
	}
	return (char *) buf;
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */
void 
m_notice (char* origin, char **av, int ac, int cmdptr)
{
	SET_SEGV_LOCATION();
	if( av[0] == NULL) {
		dlog(DEBUG1, "m_notice: dropping notice from %s to NULL: %s", origin, av[ac-1]);
		return;
	}
	dlog(DEBUG1, "m_notice: from %s, to %s : %s", origin, av[0], av[ac-1]);
	/* who to */
	if(av[0][0] == '#') {
		bot_chan_notice (origin, av, ac);
		return;
	}
#if 0
	if( ircstrcasecmp(av[0], "AUTH")) {
		dlog(DEBUG1, "m_notice: dropping server notice from %s, to %s : %s", origin, av[0], av[ac-1]);
		return;
	}
#endif
	bot_notice (origin, av, ac);
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */

void
m_private (char* origin, char **av, int ac, int cmdptr)
{
	char target[64];

	SET_SEGV_LOCATION();
	if( av[0] == NULL) {
		dlog(DEBUG1, "m_private: dropping privmsg from %s to NULL: %s", origin, av[ac-1]);
		return;
	}
	dlog(DEBUG1, "m_private: from %s, to %s : %s", origin, av[0], av[ac-1]);
	/* who to */
	if(av[0][0] == '#') {
		bot_chan_private (origin, av, ac);
		return;
	}
	if (strstr (av[0], "!")) {
		strlcpy (target, av[0], 64);
		av[0] = strtok (target, "!");
	} else if (strstr (av[0], "@")) {
		strlcpy (target, av[0], 64);
		av[0] = strtok (target, "@");
	}
	bot_private (origin, av, ac);
}

/** @brief process ircd commands
 *
 * 
 *
 * @return none
 */
EXPORTFUNC void 
process_ircd_cmd (int cmdptr, char *cmd, char* origin, char **av, int ac)
{
	ircd_cmd* ircd_cmd_ptr;

	SET_SEGV_LOCATION();
	ircd_cmd_ptr = cmd_list;
	while(ircd_cmd_ptr->name) {
		if (!strcmp (ircd_cmd_ptr->name, cmd)
			||((ircd_srv.protocol & PROTOCOL_TOKEN) && ircd_cmd_ptr->token && !strcmp (ircd_cmd_ptr->token, cmd))
			) {
			if(ircd_cmd_ptr->function) {
				dlog(DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name);
				ircd_cmd_ptr->function (origin, av, ac, cmdptr);
			} else {
				dlog(DEBUG3, "process_ircd_cmd: ignoring command %s", cmd);
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}
	
	ircd_cmd_ptr = numeric_cmd_list;	
	/* Process numeric replies */
	while(ircd_cmd_ptr->name) {
		if (!strcmp (ircd_cmd_ptr->name, cmd)) {
			if(ircd_cmd_ptr->function) {
				dlog(DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name);
				ircd_cmd_ptr->function (origin, av, ac, cmdptr);
			} else {
				dlog(DEBUG3, "process_ircd_cmd: ignoring command %s", cmd);
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}

	nlog (LOG_INFO, "No support for %s", cmd);
}

/** @brief parse
 *
 * 
 *
 * @return none
 */
void
parse (char *line)
{
	char origin[64], cmd[64], *coreLine;
	int cmdptr = 0;
	int ac = 0;
	char **av = NULL;

	SET_SEGV_LOCATION();
	if (!(*line))
		return;
	dlog(DEBUG1, "------------------------BEGIN PARSE-------------------------");
	dlog(DEBUG1, "RX: %s", line);
	if (*line == ':') {
		coreLine = strpbrk (line, " ");
		if (!coreLine)
			return;
		*coreLine = 0;
		while (isspace (*++coreLine));
		strlcpy (origin, line + 1, sizeof (origin));
		memmove (line, coreLine, strnlen (coreLine, BUFSIZE) + 1);
		cmdptr = 1;
	} else {
		cmdptr = 0;
		*origin = 0;
	}
	if (!*line)
		return;
	coreLine = strpbrk (line, " ");
	if (coreLine) {
		*coreLine = 0;
		while (isspace (*++coreLine));
	} else {
		coreLine = line + strlen (line);
	}
	strlcpy (cmd, line, sizeof (cmd)); 
	dlog(DEBUG1, "origin: %s", origin);
	dlog(DEBUG1, "cmd   : %s", cmd);
	dlog(DEBUG1, "args  : %s", coreLine);
	ac = splitbuf (coreLine, &av, 1);
	process_ircd_cmd (cmdptr, cmd, origin, av, ac);
	sfree (av);
	dlog(DEBUG1, "-------------------------END PARSE--------------------------");
}

/** @brief do_ping
 *
 * 
 *
 * @return none
 */
void
do_ping (const char* origin, const char* destination)
{
	irc_send_pong (origin);
	if (ircd_srv.burst) {
		irc_send_ping (me.name, origin, origin);
	}
}

/** @brief do_pong
 *
 * 
 *
 * @return none
 */
void
do_pong (const char* origin, const char* destination)
{
	Server *s;
	CmdParams * cmdparams;

	s = findserver (origin);
	if (s) {
		s->ping = me.now - ping.last_sent;
		if (ping.ulag > 1)
			s->ping -= ping.ulag;
		if (!strcmp (me.s->name, s->name))
			ping.ulag = me.s->ping;
		cmdparams = (CmdParams*)scalloc (sizeof(CmdParams));
		cmdparams->source.server = s;
		SendAllModuleEvent (EVENT_PONG, cmdparams);
		sfree (cmdparams);
		return;
	}
	nlog (LOG_NOTICE, "Received PONG from unknown server: %s", origin);
}

/** @brief flood
 *
 * 
 *
 * @return 
 */
int
flood (User * u)
{
	if (!u) {
		nlog (LOG_WARNING, "flood: can't find user");
		return 0;
	}
	if (UserLevel (u) >= NS_ULEVEL_OPER)	/* locop or higher */
		return 0;
	if ((me.now - u->tslastmsg) > 10) {
		u->tslastmsg = me.now;
		u->flood = 0;
		return 0;
	}
	if (u->flood >= 5) {
		nlog (LOG_NORMAL, "FLOODING: %s!%s@%s", u->nick, u->username, u->hostname);
		ssvskill_cmd (u->nick, "%s!%s (Flooding Services.)", me.name, ns_botptr->nick);
		return 1;
	} else {
		u->flood++;
	}
	return 0;
}

/** @brief Display NeoStats version info
 *
 * 
 *
 * @return 
 */
void
do_version (const char* nick, const char *remoteserver)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, nick, "%s :%s -> %s %s", me.version, me.name, ns_module_info.build_date, ns_module_info.build_time);
	ModulesVersion (nick, remoteserver);
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 * @return 
 */
void
do_motd (const char* nick, const char *remoteserver)
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen (MOTD_FILENAME, "r");
	if(!fp) {
		numeric (ERR_NOMOTD, nick, ":- MOTD file Missing");
	} else {
		numeric (RPL_MOTDSTART, nick, ":- %s Message of the Day -", me.name);
		numeric (RPL_MOTD, nick, ":- %s. Copyright (c) 1999 - 2004 The NeoStats Group", me.version);
		numeric (RPL_MOTD, nick, ":-");

		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_MOTD, nick, ":- %s", buf);
		}
		fclose (fp);
		numeric (RPL_ENDOFMOTD, nick, ":End of MOTD command.");
	}
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 * @return 
 */
void
do_admin (const char* nick, const char *remoteserver)
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	fp = fopen (ADMIN_FILENAME, "r");
	if(!fp) {
		numeric (ERR_NOADMININFO, nick, "%s :No administrative info available", me.name);
	} else {
		numeric (RPL_ADMINME, nick, ":%s :Administrative info", me.name);
		numeric (RPL_ADMINME, nick, ":%s.  Copyright (c) 1999 - 2004 The NeoStats Group", me.version);
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_ADMINLOC1, nick, ":- %s", buf);
		}
		fclose (fp);
		numeric (RPL_ADMINLOC2, nick, "End of /ADMIN command.");
	}
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
do_credits (const char* nick, const char *remoteserver)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, nick, ":- NeoStats %s Credits ", me.version);
	numeric (RPL_VERSION, nick, ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	numeric (RPL_VERSION, nick, ":- For Support, you can find ^Enigma^ or Shmad at");
	numeric (RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats");
	numeric (RPL_VERSION, nick, ":- Thanks to:");
	numeric (RPL_VERSION, nick, ":- Enigma for being part of the dev team");
	numeric (RPL_VERSION, nick, ":- Stskeeps for writing the best IRCD ever!");
	numeric (RPL_VERSION, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	numeric (RPL_VERSION, nick, ":- monkeyIRCD for the Module Segv Catching code");
	numeric (RPL_VERSION, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	numeric (RPL_VERSION, nick, ":- Andy For Ideas");
	numeric (RPL_VERSION, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	numeric (RPL_VERSION, nick, ":- sre and Jacob for development systems and access");
	numeric (RPL_VERSION, nick, ":- Error51 for Translating our FAQ and README files");
	numeric (RPL_VERSION, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!");
	numeric (RPL_VERSION, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't (and all the bug reports!)");
	numeric (RPL_VERSION, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips :)");
	numeric (RPL_VERSION, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback");
	numeric (RPL_VERSION, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd");
	numeric (RPL_VERSION, nick, ":- Blud - Giving us patches for Mystic IRCd");
	numeric (RPL_VERSION, nick, ":- herrohr - Giving us patches for Liquid IRCd support");
	numeric (RPL_VERSION, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support");
	numeric (RPL_VERSION, nick, ":- Reed Loden - Contributions to IRCu support");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
do_stats (const char* nick, const char *what)
{
	ircd_cmd* ircd_cmd_ptr;
	time_t tmp;
	time_t tmp2;
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "do_stats: message from unknown user %s", nick);
		return;
	}
	if (!ircstrcasecmp (what, "u")) {
		/* server uptime - Shmad */
		int uptime = me.now - me.t_start;
		numeric (RPL_STATSUPTIME, u->nick, "Statistical Server up %d days, %d:%02d:%02d", uptime / 86400, (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
	} else if (!ircstrcasecmp (what, "c")) {
		/* Connections */
		numeric (RPL_STATSNLINE, u->nick, "N *@%s * * %d 50", me.uplink, config.port);
		numeric (RPL_STATSCLINE, u->nick, "C *@%s * * %d 50", me.uplink, config.port);
	} else if (!ircstrcasecmp (what, "o")) {
		/* Operators */
		ListAuth(u);
	} else if (!ircstrcasecmp (what, "l")) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.t_start;
		numeric (RPL_STATSLINKINFO, u->nick, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		numeric (RPL_STATSLLINE, u->nick, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, (int)me.SendM, (int)me.SendBytes, (int)me.RcveM, (int)me.RcveBytes, (int)tmp2, (int)tmp);
        } else if (!ircstrcasecmp(what, "Z")) {
                if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
                        do_dns_stats_Z(u);
                }
	} else if (!ircstrcasecmp (what, "M")) {
		ircd_cmd_ptr = cmd_list;
		while(ircd_cmd_ptr->name) {
			if (ircd_cmd_ptr->usage > 0) {
				numeric (RPL_STATSCOMMANDS, u->nick, "Command %s Usage %d", ircd_cmd_ptr->name, ircd_cmd_ptr->usage);
			}
			ircd_cmd_ptr ++;
		}
	}
	numeric (RPL_ENDOFSTATS, u->nick, "%s :End of /STATS report", what);
	chanalert (ns_botptr->nick, "%s Requested Stats %s", u->nick, what);
};

void
do_protocol (char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!ircstrcasecmp ("TOKEN", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_TOKEN) {
				ircd_srv.protocol |= PROTOCOL_TOKEN;
			}
		}
		else if (!ircstrcasecmp ("CLIENT", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_CLIENT) {
				ircd_srv.protocol |= PROTOCOL_CLIENT;
			}
		}
		else if (!ircstrcasecmp ("UNKLN", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_UNKLN) {
				ircd_srv.protocol |= PROTOCOL_UNKLN;
			}
		}
		else if (!ircstrcasecmp ("NOQUIT", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_NOQUIT) {
				ircd_srv.protocol |= PROTOCOL_NOQUIT;
			}			
		}
		else if (!ircstrcasecmp ("NICKIP", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_NICKIP) {
				ircd_srv.protocol |= PROTOCOL_NICKIP;
			}			
		}
	}
}

void
privmsg_list (char *to, char *from, const char **text)
{
	while (*text) {
		if (**text) {
			prefmsg (to, from, (char*)*text);
		} else {
			prefmsg (to, from, " ");
		}
		text++;
	}
}

void
chanalert (char *from, char *fmt, ...)
{
	va_list ap;

	if (!me.onchan)
		return;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (from, me.serviceschan, ircd_buf);
}

void
prefmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (config.want_privmsg) {
		irc_send_privmsg (from, to, ircd_buf);
	} else {
		irc_send_notice (from, to, ircd_buf);
	}
}

void
privmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (from, to, ircd_buf);
}

void
notice (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_notice (from, to, ircd_buf);
}

void
globops (char *from, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);

	if (me.onchan) {
		irc_send_globops(from, ircd_buf);
	} else {
		nlog (LOG_NORMAL, ircd_buf);
	}
}

void
wallops (const char *from, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_wallops ((char*)from, (char*)ircd_buf);
}

void
numeric (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	irc_send_numeric (me.name, numeric, target, ircd_buf);
}

void
unsupported_cmd(const char* cmd)
{
	chanalert (ns_botptr->nick, "Warning, %s tried to %s which is not supported", GET_CUR_MODNAME(), cmd);
	nlog (LOG_NOTICE, "Warning, %s tried to %s, which is not supported", GET_CUR_MODNAME(), cmd);
}

int
sumode_cmd (const char *who, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	irc_send_umode (who, target, newmode);
	UserMode (target, newmode);
	return NS_SUCCESS;
}

int
part_bot_from_chan (const char *who, const char *chan)
{
	irc_send_part(who, chan);
	part_chan (finduser (who), (char *) chan, NULL);
	return NS_SUCCESS;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	UserNick (oldnick, newnick, NULL);
	irc_send_nickchange (oldnick, newnick, me.now);
	return NS_SUCCESS;
}

int
scmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	irc_send_cmode (me.name, who, chan, mode, args, me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	sfree (av);
	return NS_SUCCESS;
}

int
schanusermode_cmd (const char *who, const char *chan, const char *mode, const char *bot)
{

	if((ircd_srv.protocol & PROTOCOL_B64NICK)) {
		irc_send_cmode (me.name, who, chan, mode, nicktobase64 (bot), me.now);
	} else {
		irc_send_cmode (me.name, who, chan, mode, bot, me.now);
	}
	ChanUserMode (chan, who, 1, CUmodeStringToMask(mode,0));
	return NS_SUCCESS;
}

int
squit_cmd (const char *who, const char *quitmsg)
{
	irc_send_quit (who, quitmsg);
	do_quit (who, quitmsg);
	return NS_SUCCESS;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	irc_send_kill (from, target, ircd_buf);
	do_quit (target, ircd_buf);
	return NS_SUCCESS;
}

int
ssvskill_cmd (const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	if (protocol_info->features&FEATURE_SVSKILL) {
		irc_send_svskill (me.name, target, ircd_buf);
	} else {
		irc_send_kill (me.name, target, ircd_buf);
		do_quit (target, ircd_buf);
	}
	return NS_SUCCESS;
}

int 
ssvstime_cmd (const time_t ts)
{
	if (protocol_info->features&FEATURE_SVSTIME) {
		irc_send_svstime(me.name, (unsigned long)ts);
		nlog (LOG_NOTICE, "ssvstime_cmd: synching server times to %lu", ts);
	} else {
		unsupported_cmd("SVSTIME");
	}
	return NS_SUCCESS;
}

int
skick_cmd (const char *who, const char *chan, const char *target, const char *reason)
{
	irc_send_kick (who, chan, target, reason);
	part_chan (finduser (target), (char *) chan, reason[0] != 0 ? (char *)reason : NULL);
	return NS_SUCCESS;
}

int 
sinvite_cmd (const char *from, const char *to, const char *chan) 
{
	irc_send_invite(from, to, chan);
	return NS_SUCCESS;
}

int
ssvsmode_cmd (const char *target, const char *modes)
{
	if (protocol_info->features&FEATURE_SVSMODE) {
		User *u;

		u = finduser (target);
		if (!u) {
			nlog (LOG_WARNING, "ssvsmode_cmd: can't find user %s", target);
			return 0;
		}
		irc_send_svsmode(me.name, target, modes);
		UserMode (target, modes);
	} else {
		unsupported_cmd("SVSMODE");
	}
	return NS_SUCCESS;
}

int
ssvshost_cmd (const char *target, const char *vhost)
{
	if (protocol_info->features&FEATURE_SVSHOST) {
		User *u;

		u = finduser (target);
		if (!u) {
			nlog (LOG_WARNING, "ssvshost_cmd: can't find user %s", target);
			return 0;
		}

		strlcpy (u->vhost, vhost, MAXHOST);
		irc_send_svshost(me.name, target, vhost);
	} else {
		unsupported_cmd("SVSHOST");
	}
	return NS_SUCCESS;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
	if (protocol_info->features&FEATURE_SVSJOIN) {
		irc_send_svsjoin (me.name, target, chan);
	} else {
		unsupported_cmd("SVSJOIN");
	}
	return NS_SUCCESS;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
	if (protocol_info->features&FEATURE_SVSPART) {
		irc_send_svspart (me.name, target, chan);
	} else {
		unsupported_cmd("SVSPART");
	}
	return NS_SUCCESS;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
	if (protocol_info->features&FEATURE_SWHOIS) {
		irc_send_swhois (me.name, target, swhois);
	} else {
		unsupported_cmd("SWHOIS");
	}
	return NS_SUCCESS;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
	if (protocol_info->features&FEATURE_SVSNICK) {
		irc_send_svsnick (me.name, target, newnick, me.now);
	} else {
		unsupported_cmd("SVSNICK");
	}
	return NS_SUCCESS;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	if (protocol_info->features&FEATURE_SMO) {
		irc_send_smo (from, umodetarget, msg);
	} else {
		unsupported_cmd("SMO");
	}
	return NS_SUCCESS;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	irc_send_akill(me.name, host, ident, setby, length, ircd_buf, me.now);
	return NS_SUCCESS;
}

int
srakill_cmd (const char *host, const char *ident)
{
	irc_send_rakill (me.name, host, ident);
	return NS_SUCCESS;
}

int
sping_cmd (const char *from, const char *reply, const char *to)
{
	irc_send_ping (from, reply, to);
	return NS_SUCCESS;
}

int
spong_cmd (const char *reply)
{
	irc_send_pong (reply);
	return NS_SUCCESS;
}

int
sserver_cmd (const char *name, const int numeric, const char *infoline)
{
	irc_send_server (me.name, name, numeric, infoline);
	return NS_SUCCESS;
}

int
ssquit_cmd (const char *server, const char *quitmsg)
{
	irc_send_squit (server, quitmsg);
	return NS_SUCCESS;
}

/* SJOIN <TS> #<channel> <modes> :[@][+]<nick_1> ...  [@][+]<nick_n> */
void 
do_sjoin (char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc)
{
	char nick[MAXNICK];
	char* nicklist;
	int modeexists;
	long mode = 0;
	int ok = 1, i, j = 3;
	ModesParm *m;
	Channel *c;
	lnode_t *mn = NULL;
	char **param;
	int paramcnt = 0;
	int paramidx = 0;

	if (*modes == '#') {
		join_chan (sjoinnick, modes);
		return;
	}

	paramcnt = split_buf(argv[argc-1], &param, 0);
		   
	while (paramcnt > paramidx) {
		nicklist = param[paramidx];
		if(ircd_srv.protocol & PROTOCOL_SJ3) {
			/* Unreal passes +b(&) and +e(") via SJ3 so skip them for now */	
			if(*nicklist == '&' || *nicklist == '"') {
				dlog(DEBUG1, "Skipping %s", nicklist);
				paramidx++;
				continue;
			}
		}
		mode = 0;
		for (i = 0; i < MODE_TABLE_SIZE; i++) {
			if (ircd_cmodes[i].sjoin != 0) {
				if (*nicklist == ircd_cmodes[i].sjoin) {
					mode |= ircd_cmodes[i].mode;
					nicklist++;
					i = -1;
				}
			}
		}
		strlcpy (nick, nicklist, MAXNICK);
		join_chan (nick, channame); 
		ChanUserMode (channame, nick, 1, mode);
		paramidx++;
		ok = 1;
	}
	c = findchan (channame);
	if(c) {
		/* update the TS time */
		SetChanTS (c, atoi (tstime)); 
		if (*modes == '+') {
			while (*modes) {
				unsigned int mode;
				unsigned int flags;      

				mode = ircd_cmodes[(int)*modes].mode;
				flags = ircd_cmodes[(int)*modes].flags;

				/* mode limit and mode key replace current values */
				if (mode == CMODE_LIMIT) {
					c->limit = atoi(argv[j]);
					j++;
				} else if (mode == CMODE_KEY) {
					strlcpy (c->key, argv[j], KEYLEN);
					j++;
				} else if (flags) {
					mn = list_first (c->modeparms);
					modeexists = 0;
					while (mn) {
						m = lnode_get (mn);
						if ((m->mode == mode) && !ircstrcasecmp (m->param, argv[j])) {
							dlog(DEBUG1, "ChanMode: Mode %c (%s) already exists, not adding again", *modes, argv[j]);
							j++;
							modeexists = 1;
							break;
						}
						mn = list_next (c->modeparms, mn);
					}
					if (modeexists != 1) {
						m = smalloc (sizeof (ModesParm));
						m->mode = mode;
						strlcpy (m->param, argv[j], PARAMSIZE);
						mn = lnode_create (m);
						if (list_isfull (c->modeparms)) {
							nlog (LOG_CRITICAL, "ChanMode: modelist is full adding to channel %s", c->name);
							do_exit (NS_EXIT_ERROR, "List full - see log file");
						} else {
							list_append (c->modeparms, mn);
						}
						j++;
					}
				} else {
					c->modes |= mode;
				}
				modes++;
			}
		}
	}
	sfree(param);
}

void 
do_netinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.maxglobalcnt = atoi (maxglobalcnt);
	ircd_srv.tsendsync = atoi (tsendsync);
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, CLOAKKEYLEN);
	strlcpy (me.netname, netname, MAXPASS);
	irc_send_netinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
	SendAllModuleEvent (EVENT_NETINFO, NULL);
	me.synced = 1;
}

void 
do_snetinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, CLOAKKEYLEN);
	strlcpy (me.netname, netname, MAXPASS);
	irc_send_snetinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
	SendAllModuleEvent (EVENT_NETINFO, NULL);
	me.synced = 1;
}

void
do_join (const char* nick, const char* chanlist, const char* keys)
{
	char *s, *t;
	t = (char*)chanlist;
	while (*(s = t)) {
		t = s + strcspn (s, ",");
		if (*t)
			*t++ = 0;
		join_chan (nick, s);
	}
}

void 
do_part (const char* nick, const char* chan, const char* reason)
{
	part_chan (finduser (nick), chan, reason);
}

void 
do_nick (const char *nick, const char *hopcount, const char* TS, 
		 const char *user, const char *host, const char *server, 
		 const char *ip, const char *servicestamp, const char *modes, 
		 const char *vhost, const char *realname, const char *numeric, 
		 const char *smodes )
{
	if (!nick) {
		nlog (LOG_CRITICAL, "do_nick: trying to add user with NULL nickname");
		return;
	}
	AddUser (nick, user, host, realname, server, ip, TS, numeric);
	if(modes) {
		UserMode (nick, modes);
	}
	if(vhost) {
		SetUserVhost(nick, vhost);
	}
	if (protocol_info->features&FEATURE_USERSMODES) {
		if(smodes) {
			UserSMode (nick, smodes);
		}
	}
}

void 
do_client (const char *nick, const char *arg1, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *arg9, 
		const char *ip, const char *realname)
{
	if (!nick) {
		nlog (LOG_CRITICAL, "do_client: trying to add user with NULL nickname");
		return;
	}
	AddUser (nick, user, host, realname, server, ip, TS, NULL);
	if(modes) {
		UserMode (nick, modes);
	}
	if(vhost) {
		SetUserVhost(nick, vhost);
	}
	if(smodes) {
		UserSMode (nick, smodes);
	}
}

void
do_kill (const char *nick, const char *reason)
{
	KillUser (nick, reason);
}

void
do_quit (const char *nick, const char *quitmsg)
{
	QuitUser (nick, quitmsg);
}

void 
do_squit(const char *name, const char* reason)
{
	DelServer (name, reason);
}

void
do_kick (const char *kickby, const char *chan, const char *kicked, const char *kickreason)
{
	kick_chan (kickby, chan, kicked, kickreason);
}

void 
do_svinfo (void)
{
	irc_send_svinfo (TS_CURRENT, TS_MIN, (unsigned long)me.now);
}

void 
do_vctrl (const char* uprot, const char* nicklen, const char* modex, const char* gc, const char* netname)
{
	ircd_srv.uprot = atoi(uprot);
	ircd_srv.nicklen = atoi(nicklen);
	ircd_srv.modex = atoi(modex);
	ircd_srv.gc = atoi(gc);
	strlcpy (me.netname, netname, MAXPASS);
	irc_send_vctrl (ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname);
}

void 
do_smode (const char* nick, const char* modes)
{
	UserSMode (nick, modes);
}

void 
do_mode_user (const char* nick, const char* modes)
{
	UserMode (nick, modes);
}

void 
do_svsmode_user (const char* nick, const char* modes, const char* ts)
{
	char modebuf[MODESIZE];
	
	if (ts && isdigit(*ts)) {
		const char* pModes;	
		char* pNewModes;	

		SetUserServicesTS (nick, ts);
		/* If only setting TS, we do not need further mode processing */
		if(strcasecmp(modes, "+d") == 0) {
			dlog(DEBUG3, "dropping modes since this is a services TS %s", modes);
			return;
		}
		/* We need to strip the d from the mode string */
		pNewModes = modebuf;
		pModes = modes;
		while(*pModes) {
			if(*pModes != 'd') {
				*pNewModes = *pModes;
			}
			pModes++;
			pNewModes++;			
		}
		/* NULL terminate */
		*pNewModes = 0;
		UserMode (nick, modebuf);
	} else {
		UserMode (nick, modes);
	}
}

void 
do_mode_channel (char *origin, char **argv, int argc)
{
	ChanMode (origin, argv, argc);
}

void 
do_away (const char* nick, const char *reason)
{
	UserAway (nick, reason);
}

void 
do_vhost (const char* nick, const char *vhost)
{
	SetUserVhost(nick, vhost);
}

void
do_nickchange (const char * oldnick, const char *newnick, const char * ts)
{
	UserNick (oldnick, newnick, ts);
}

void 
do_topic (const char* chan, const char *owner, const char* ts, const char *topic)
{
	ChanTopic (chan, owner, ts, topic);
}

void 
do_server (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv)
{
	if(!srv) {
		if (uplink == NULL) {
			me.s = AddServer (name, me.name, hops, numeric, infoline);
		} else {
			me.s = AddServer (name, uplink, hops, numeric, infoline);
		}
	} else {
		AddServer (name, uplink, hops, numeric, infoline);
	}
	send_cmd(":%s VERSION %s", me.name, name);
}

void 
do_burst (char *origin, char **argv, int argc)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			irc_send_burst (0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_services_bot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

void 
do_swhois (char *who, char *swhois)
{
	User* u;
	u = finduser(who);
	if(u) {
		strlcpy(u->swhois, swhois, MAXHOST);
	}
}

void 
do_tkl(const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason)
{
	char mask[MAXHOST];
	ircsnprintf(mask, MAXHOST, "%s@%s", user, host);
	if(add[0] == '+') {
		AddBan(type, user, host, mask, reason, setby, tsset, tsexpire);
	} else {
		DelBan(type, user, host, mask, reason, setby, tsset, tsexpire);
	}
}

void 
do_eos (const char *name)
{
	Server *s;

	s = findserver (name);
	if(s) {
		SynchServer(s);
		dlog(DEBUG1, "do_eos: server %s is now synched", name);
	} else {
		nlog (LOG_WARNING, "do_eos: server %s not found", name);
	}
}

void
send_cmd (char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZE];
	int buflen;
	
	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);

	dlog(DEBUG2, "TX: %s", buf);
	if(strnlen (buf, BUFSIZE) < BUFSIZE - 2) {
		strlcat (buf, "\n", BUFSIZE);
	} else {
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\n';
	}
	buflen = strnlen (buf, BUFSIZE);
	sts (buf, buflen);
}

void
setserverbase64 (const char *name, const char* num)
{
	Server *s;

	s = findserver(name);
	if(s) {
		dlog(DEBUG1, "setserverbase64: setting %s to %s", name, num);
		strlcpy(s->name64, num, 6);
	} else {
		dlog(DEBUG1, "setserverbase64: cannot find %s for %s", name, num);
	}
}

char* 
servertobase64 (const char* name)
{
	Server *s;

	dlog(DEBUG1, "servertobase64: scanning for %s", name);
	s = findserver(name);
	if(s) {
		return s->name64;
	} else {
		dlog(DEBUG1, "servertobase64: cannot find %s", name);
	}
	return NULL;
}

char* 
base64toserver (const char* num)
{
	Server *s;

	dlog(DEBUG1, "base64toserver: scanning for %s", num);
	s = findserverbase64(num);
	if(s) {
		return s->name;
	} else {
		dlog(DEBUG1, "base64toserver: cannot find %s", num);
	}
	return NULL;
}

void
setnickbase64 (const char *nick, const char* num)
{
	User *u;

	u = finduser(nick);
	if(u) {
		dlog(DEBUG1, "setnickbase64: setting %s to %s", nick, num);
		strlcpy(u->name64, num, B64SIZE);
	} else {
		dlog(DEBUG1, "setnickbase64: cannot find %s for %s", nick, num);
	}
}

char* 
nicktobase64 (const char* nick)
{
	User *u;

	dlog(DEBUG1, "nicktobase64: scanning for %s", nick);
	u = finduser(nick);
	if(u) {
		return u->name64;
	} else {
		dlog(DEBUG1, "nicktobase64: cannot find %s", nick);
	}
	return NULL;
}

char* 
base64tonick (const char* num)
{
	User *u;

	dlog(DEBUG1, "base64tonick: scanning for %s", num);
	u = finduserbase64(num);
	if(u) {
		return u->nick;
	} else {
		dlog(DEBUG1, "base64tonick: cannot find %s", num);
	}
	return NULL;
}

/*
RX: :irc.foo.com 351 stats.mark.net Unreal3.2. irc.foo.com :FinWXOoZ [*=2303]
*/
static void m_numeric351 (char *origin, char **argv, int argc, int srv)
{
	Server* s;

	s = findserver(origin);
	if(s) {
		strlcpy(s->version, argv[1], MAXHOST);
	}
}

/*
RX: :irc.foo.com 242 NeoStats :Server Up 6 days, 23:52:55
RX: :irc.foo.com 250 NeoStats :Highest connection count: 3 (2 clients)
RX: :irc.foo.com 219 NeoStats u :End of /STATS report
*/
static void m_numeric242 (char *origin, char **argv, int argc, int srv)
{
	Server* s;

	s = findserver(origin);
	if(s) {
		/* Convert "Server Up 6 days, 23:52:55" to seconds*/
		char *ptr;
		time_t secs;

		/* Server Up 6 days, 23:52:55 */
		strtok(argv[argc-1], " ");
		/* Up 6 days, 23:52:55 */
		strtok(NULL, " ");
		/* 6 days, 23:52:55 */
		ptr = strtok(NULL, " ");
		secs = atoi(ptr) * 86400;
		/* days, 23:52:55 */
		strtok(NULL, " ");
		/* , 23:52:55 */
		ptr = strtok(NULL, "");
		/* 23:52:55 */
		ptr = strtok(ptr , ":");
		secs += atoi(ptr)*3600;
		/* 52:55 */
		ptr = strtok(NULL, ":");
		secs += atoi(ptr)*60;
		/* 55 */
		ptr = strtok(NULL, "");
		secs += atoi(ptr);

		s->uptime = secs;
	}
}

