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
#include "modes.h"
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
#include "base64.h"
#include "dcc.h"

typedef struct ircd_sym {
	void **ptr;
	char *sym;
	unsigned int required;
	unsigned int feature;
} ircd_sym;

typedef struct protocol_entry {
	char *token;
	unsigned int flag;
} protocol_entry;

#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"

ircd_server ircd_srv;

static char ircd_buf[BUFSIZE];
static char protocol_path[MAXPATH];

static ProtocolInfo *protocol_info;

static ircd_cmd *cmd_list;

static void *protocol_module_handle;

static void (*irc_send_privmsg) (const char *source, const char *to, const char *buf);
static void (*irc_send_notice) (const char *source, const char *to, const char *buf);
static void (*irc_send_globops) (const char *source, const char *buf);
static void (*irc_send_wallops) (const char *source, const char *buf);
static void (*irc_send_numeric) (const char *source, const int numeric, const char *target, const char *buf);
static void (*irc_send_umode) (const char *source, const char *target, const char *mode);
static void (*irc_send_join) (const char *source, const char *chan, const char *key, const unsigned long ts);
static void (*irc_send_sjoin) (const char *source, const char *who, const char *chan, const unsigned long ts);
static void (*irc_send_part) (const char *source, const char *chan, const char *reason);
static void (*irc_send_nickchange) (const char *oldnick, const char *newnick, const unsigned long ts);
static void (*irc_send_cmode) (const char *source, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
static void (*irc_send_quit) (const char *source, const char *quitmsg);
static void (*irc_send_kill) (const char *source, const char *target, const char *reason);
static void (*irc_send_kick) (const char *source, const char *chan, const char *target, const char *reason);
static void (*irc_send_invite) (const char *source, const char *to, const char *chan);
static void (*irc_send_svskill) (const char *source, const char *target, const char *reason);
static void (*irc_send_svsmode) (const char *source, const char *target, const char *modes);
static void (*irc_send_svshost) (const char *source, const char *who, const char *vhost);
static void (*irc_send_svsjoin) (const char *source, const char *target, const char *chan);
static void (*irc_send_svspart) (const char *source, const char *target, const char *chan);
static void (*irc_send_svsnick) (const char *source, const char *target, const char *newnick, const unsigned long ts);
static void (*irc_send_swhois) (const char *source, const char *target, const char *swhois);
static void (*irc_send_smo) (const char *source, const char *umodetarget, const char *msg);
static void (*irc_send_akill) (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, unsigned long ts);
static void (*irc_send_rakill) (const char *source, const char *host, const char *ident);
static void (*irc_send_ping) (const char *source, const char *reply, const char *to);
static void (*irc_send_pong) (const char *reply);
static void (*irc_send_server) (const char *source, const char *name, const int numeric, const char *infoline);
static void (*irc_send_squit) (const char *server, const char *quitmsg);
static void (*irc_send_nick) (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
static void (*irc_send_server_connect) (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
static void (*irc_send_netinfo) (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts);
static void (*irc_send_snetinfo) (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts);
static void (*irc_send_svinfo) (const int tscurrent, const int tsmin, const unsigned long tsnow);
static void (*irc_send_vctrl) (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
static void (*irc_send_burst) (int b);
static void (*irc_send_svstime) (const char *source, const unsigned long ts);
static void (*irc_send_setname) (const char* nick, const char* realname);
static void (*irc_send_sethost) (const char* nick, const char* host);
static void (*irc_send_setident) (const char* nick, const char* ident);

protocol_entry protocol_list[] =
{
	{"TOKEN",	PROTOCOL_TOKEN},
	{"CLIENT",	PROTOCOL_CLIENT},
	{"UNKLN",	PROTOCOL_UNKLN},
	{"NOQUIT",	PROTOCOL_NOQUIT},
	{"NICKIP",	PROTOCOL_NICKIP},
	{"NICKv2",	PROTOCOL_NICKv2},
	{"SJ3",		PROTOCOL_SJ3},	
	{NULL, 0}
};

static ircd_sym ircd_sym_table[] = 
{
	{(void *)&irc_send_privmsg, "send_privmsg", 1, 0},
	{(void *)&irc_send_notice, "send_notice", 1, 0},
	{(void *)&irc_send_globops, "send_globops", 0, 0},
	{(void *)&irc_send_wallops, "send_wallops", 0, 0},
	{(void *)&irc_send_numeric, "send_numeric", 0, 0},
	{(void *)&irc_send_umode, "send_umode", 1, 0},
	{(void *)&irc_send_join, "send_join", 1, 0},
	{(void *)&irc_send_sjoin, "send_sjoin", 0, 0},
	{(void *)&irc_send_part, "send_part", 1, 0},
	{(void *)&irc_send_nickchange, "send_nickchange", 1, 0},
	{(void *)&irc_send_cmode, "send_cmode", 1, 0},
	{(void *)&irc_send_quit, "send_quit", 1, 0},
	{(void *)&irc_send_kill, "send_kill", 0, 0},
	{(void *)&irc_send_kick, "send_kick", 0, 0},
	{(void *)&irc_send_invite, "send_invite", 0, 0},
	{(void *)&irc_send_svskill, "send_svskill", 0, FEATURE_SVSKILL},
	{(void *)&irc_send_svsmode, "send_svsmode", 0, FEATURE_SVSMODE},
	{(void *)&irc_send_svshost, "send_svshost", 0, FEATURE_SVSHOST},
	{(void *)&irc_send_svsjoin, "send_svsjoin", 0, FEATURE_SVSJOIN},
	{(void *)&irc_send_svspart, "send_svspart", 0, FEATURE_SVSPART},
	{(void *)&irc_send_svsnick, "send_svsnick", 0, FEATURE_SVSNICK},
	{(void *)&irc_send_swhois, "send_swhois", 0, FEATURE_SWHOIS},
	{(void *)&irc_send_smo, "send_smo", 0, FEATURE_SMO},
	{(void *)&irc_send_svstime, "send_svstime", 0, FEATURE_SVSTIME},
	{(void *)&irc_send_akill, "send_akill", 0, 0},
	{(void *)&irc_send_rakill, "send_rakill", 0, 0},
	{(void *)&irc_send_ping, "send_ping", 0, 0},
	{(void *)&irc_send_pong, "send_pong", 0, 0},
	{(void *)&irc_send_server, "send_server", 0, 0},
	{(void *)&irc_send_squit, "send_squit", 0, 0},
	{(void *)&irc_send_nick, "send_nick", 1, 0},
	{(void *)&irc_send_server_connect, "send_server_connect", 1, 0},
	{(void *)&irc_send_netinfo, "send_netinfo", 0, 0},
	{(void *)&irc_send_snetinfo, "send_snetinfo", 0, 0},
	{(void *)&irc_send_svinfo, "send_svinfo", 0, 0},
	{(void *)&irc_send_vctrl, "send_vctrl", 0, 0},
	{(void *)&irc_send_burst, "send_burst", 0, 0},
	{(void *)&irc_send_setname, "send_setname", 0, 0},
	{(void *)&irc_send_sethost, "send_sethost", 0, 0},
	{(void *)&irc_send_setident, "send_setident", 0, 0},
	{NULL, NULL, 0, 0},
};

static void m_numeric242 (char *origin, char **argv, int argc, int srv);
static void m_numeric351 (char *origin, char **argv, int argc, int srv);

ircd_cmd numeric_cmd_list[] = {
	/*Message	Token	Function	usage */
	{"351", "351", m_numeric351, 0},
	{"242", "242", m_numeric242, 0},
	{0, 0, 0, 0},
};

static void IrcdError(char* err)
{
	nlog (LOG_CRITICAL, "Unable to find %s in selected IRCd module", err);
}

/** @brief InitIrcdProtocol
 *
 *  Init protocol info
 *
 * @return 
 */
static int
InitIrcdProtocol ()
{
	protocol_info = ns_dlsym (protocol_module_handle, "protocol_info");
	if (!protocol_info) {
		nlog (LOG_CRITICAL, "Unable to find protocol_info in protocol module %s", protocol_path);
		return NS_FAILURE;	
	}
	if (protocol_info->minprotocol & PROTOCOL_CLIENTMODE) {
		nsconfig.singlebotmode = 1;
	}
	strlcpy (me.servicescmode, protocol_info->services_cmode, MODESIZE);
	strlcpy (me.servicesumode, protocol_info->services_umode, MODESIZE);
	/* set min protocol */
	ircd_srv.protocol = protocol_info->minprotocol;
	/* Allow protocol module to "override" the parser */
	irc_parse = ns_dlsym (protocol_module_handle, "parse");
	if (irc_parse == NULL) {
		irc_parse = parse;
	}
	cmd_list = ns_dlsym (protocol_module_handle, "cmd_list");
	if (!cmd_list) {
		IrcdError("command list");
		return NS_FAILURE;	
	}
	return NS_SUCCESS;
}

/** @brief InitIrcdSymbols
 *
 *  Map protocol module pointers to core pointers and check for minimum 
 *  requirements
 *
 * @return 
 */
static int
InitIrcdSymbols (void)
{
	ircd_sym * pircd_sym;

	pircd_sym = ircd_sym_table;
	while (pircd_sym->ptr)
	{
		*pircd_sym->ptr = ns_dlsym (protocol_module_handle, pircd_sym->sym);
		if (pircd_sym->required) {
			if (!*pircd_sym->ptr) {
				IrcdError(pircd_sym->sym);
				return NS_FAILURE;	
			}
		}
		ircd_srv.features |= pircd_sym->feature;
		pircd_sym ++;
	}
	return NS_SUCCESS;
}

static int
InitIrcdModes (void)
{
	mode_init *chan_umodes;
	mode_init *chan_modes;
	mode_init *user_umodes;
	mode_init *user_smodes;

	chan_umodes = ns_dlsym (protocol_module_handle, "chan_umodes");
	if (!chan_umodes) {
		IrcdError("channel umode table");
		return NS_FAILURE;	
	}
	chan_modes  = ns_dlsym (protocol_module_handle, "chan_modes");
	if (!chan_modes) {
		IrcdError("channel mode table");
		return NS_FAILURE;	
	}
	user_umodes = ns_dlsym (protocol_module_handle, "user_umodes");
	if (!user_umodes) {
		IrcdError("user mode table");
		return NS_FAILURE;	
	}
	/* Not required */
	user_smodes = ns_dlsym (protocol_module_handle, "user_smodes");
	if (user_smodes) {
		ircd_srv.features |= FEATURE_USERSMODES;
	}
	if (InitModeTables(chan_umodes, chan_modes, user_umodes, user_smodes) != NS_SUCCESS) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 * @return 
 */
int
InitIrcd (void)
{
	/* Clear IRCD info */
	memset(&ircd_srv, 0, sizeof(ircd_srv));
	/* Open protocol module */
	ircsnprintf (protocol_path, 255, "%s/%s%s", MOD_PATH, me.protocol,MOD_EXT);
	nlog (LOG_NORMAL, "Using protocol module %s", protocol_path);
	protocol_module_handle = ns_dlopen (protocol_path, RTLD_NOW || RTLD_GLOBAL);
	if (!protocol_module_handle) {
		nlog (LOG_CRITICAL, "Unable to load protocol module %s", protocol_path);
		return NS_FAILURE;	
	}
	/* Setup protocol options */
	if (InitIrcdProtocol () != NS_SUCCESS) {
		return NS_FAILURE;	
	}
	/* Setup IRCD function calls */
	if (InitIrcdSymbols() != NS_SUCCESS) 
		return NS_FAILURE;
	/* Build mode tables */
	if (InitIrcdModes () != NS_SUCCESS) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/*  _m_command functions are highly generic support functions for 
 *  use in protocol modules. If a protocol differs at all from 
 *  the RFC, they must provide their own local version of this 
 *  function. These	are purely to avoid protocol module bloat for
 *  the more common forms of these commands and allow protocol module
 *  coders to concentrate on the areas that need it.
 */

/** @brief process PASS
 *
 * _m_pass
 *  RX: 
 *  PASS :password
 *	argv[0] = password
 *
 * @return none
 */
void _m_pass (char* origin, char **av, int ac, int cmdptr)
{
	
}

/** @brief process protoctl
 *
 * _m_protoctl
 *  RX: 
 *  PROTOCTL <token list>
 * @return none
 */
void _m_protoctl (char *origin, char **argv, int argc, int cmdptr)
{
	do_protocol (origin, argv, argc);
}

/** @brief process VERSION
 *
 * _m_version
 *  origin VERSION :stats.neostats.net
 *	argv[0] = remote server
 *
 * @return none
 */
void _m_version (char *origin, char **argv, int argc, int srv)
{
	do_version (origin, argv[0]);
}

/** @brief process MOTD
 *
 * _m_motd
 *  origin MOTD :stats.neostats.net
 *
 * @return none
 */
void _m_motd (char *origin, char **argv, int argc, int srv)
{
	do_motd (origin, argv[0]);
}

/** @brief process ADMIN
 *
 * _m_admin
 *  origin ADMIN :stats.neostats.net
 *	argv[0] = servername
 *
 * @return none
 */
void _m_admin (char *origin, char **argv, int argc, int srv)
{
	do_admin (origin, argv[0]);
}

/** @brief process CREDITS
 *
 * _m_credits
 *  origin CREDITS :stats.neostats.net
 *	argv[0] = servername
 *
 * @return none
 */
void _m_credits (char *origin, char **argv, int argc, int srv)
{
	do_credits (origin, argv[0]);
}

/** @brief process STATS
 *
 * _m_stats
 *  RX: 
 *  :origin STATS u :stats.neostats.net
 *	argv[0] = stats type
 *	argv[1] = destination
 *
 * @return none
 */
void _m_stats (char *origin, char **argv, int argc, int srv)
{
	do_stats (origin, argv[0]);
}

/** @brief process PING
 *
 * _m_ping
 *  RX: 
 *  PING :irc.foonet.com
 *	argv[0] = origin
 *	argv[1] = destination
 *
 * @return none
 */
void _m_ping (char *origin, char **argv, int argc, int srv)
{
	do_ping (argv[0], argc > 1 ? argv[1] : NULL);
}

/** @brief process PONG
 *
 * _m_pong
 *  irc.foonet.com PONG irc.foonet.com :stats.neostats.net
 *  argv[0] = origin
 *  argv[1] = destination
 *
 * @return none
 */
void _m_pong (char *origin, char **argv, int argc, int srv)
{
	do_pong (argv[0], argv[1]);
}

/** @brief process QUIT
 *
 * _m_quit
 *  origin QUIT :comment
 *  argv[0] = comment
 *
 * @return none
 */
void _m_quit (char *origin, char **argv, int argc, int srv)
{
	do_quit (origin, argv[0]);
}

/** @brief process JOIN
 *
 * m_join
 *	argv[0] = channel
 *	argv[1] = channel password (key)
 */
void _m_join (char *origin, char **argv, int argc, int srv)
{
	do_join (origin, argv[0], argv[1]);
}

/** @brief process PART
 *
 * m_part
 *	argv[0] = channel
 *	argv[1] = comment
 */
void _m_part (char *origin, char **argv, int argc, int srv)
{
	do_part (origin, argv[0], argv[1]);
}

/** @brief process KICK
 *
 * _m_kick
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 */
void _m_kick (char *origin, char **argv, int argc, int srv)
{
	do_kick (origin, argv[0], argv[1], argv[2]);
}

/** @brief process TOPIC
 *
 * _m_topic
 *  origin TOPIC #channel owner TS :topic
 *  argv[0] = topic text
 * For servers using TS:
 *  argv[0] = channel name
 *  argv[1] = topic nickname
 *  argv[2] = topic time
 *  argv[3] = topic text
 *
 * @return none
 */
void _m_topic (char *origin, char **argv, int argc, int srv)
{
	do_topic (argv[0], argv[1], argv[2], argv[3]);
}

/** @brief process AWAY
 *
 * _m_away
 *	argv[0] = away message
 */
void _m_away (char *origin, char **argv, int argc, int srv)
{
	do_away (origin, (argc > 0) ? argv[0] : NULL);
}

/** @brief process KILL
 *
 * _m_kill
 *	argv[0] = kill victim(s) - comma separated list
 *	argv[1] = kill path
 */
void _m_kill (char *origin, char **argv, int argc, int srv)
{
	do_kill (origin, argv[0], argv[1]);
}

/** @brief process SQUIT
 *
 * _m_squit
 *	argv[0] = server name
 *	argv[argc-1] = comment
 */
void _m_squit (char *origin, char **argv, int argc, int srv)
{
	do_squit (argv[0], argv[argc-1]);
}

/** @brief process notice
 *
 * 
 *
 * @return none
 */
void 
_m_notice (char* origin, char **argv, int argc, int cmdptr)
{
	SET_SEGV_LOCATION();
	if (argv[0] == NULL) {
		dlog (DEBUG1, "_m_notice: dropping notice from %s to NULL: %s", origin, argv[argc-1]);
		return;
	}
	dlog (DEBUG1, "_m_notice: from %s, to %s : %s", origin, argv[0], argv[argc-1]);
	/* who to */
	if (argv[0][0] == '#') {
		bot_chan_notice (origin, argv, argc);
		return;
	}
#if 0
	if ( ircstrcasecmp(argv[0], "AUTH")) {
		dlog (DEBUG1, "_m_notice: dropping server notice from %s, to %s : %s", origin, argv[0], argv[argc-1]);
		return;
	}
#endif
	bot_notice (origin, argv, argc);
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */

void
_m_private (char* origin, char **argv, int argc, int cmdptr)
{
	char target[64];

	SET_SEGV_LOCATION();
	if (argv[0] == NULL) {
		dlog (DEBUG1, "_m_private: dropping privmsg from %s to NULL: %s", origin, argv[argc-1]);
		return;
	}
	dlog (DEBUG1, "_m_private: from %s, to %s : %s", origin, argv[0], argv[argc-1]);
	/* who to */
	if (argv[0][0] == '#') {
		bot_chan_private (origin, argv, argc);
		return;
	}
	if (strstr (argv[0], "!")) {
		strlcpy (target, argv[0], 64);
		argv[0] = strtok (target, "!");
	} else if (strstr (argv[0], "@")) {
		strlcpy (target, argv[0], 64);
		argv[0] = strtok (target, "@");
	}
	bot_private (origin, argv, argc);
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
	while( ircd_cmd_ptr->name ) {
		if( !ircstrcasecmp( ircd_cmd_ptr->name, cmd ) || 
		  ( ( ircd_srv.protocol & PROTOCOL_TOKEN ) && ircd_cmd_ptr->token && !ircstrcasecmp( ircd_cmd_ptr->token, cmd ) ) ) {
			if (ircd_cmd_ptr->function) {
				dlog (DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name);
				ircd_cmd_ptr->function (origin, av, ac, cmdptr);
			} else {
				dlog (DEBUG3, "process_ircd_cmd: ignoring command %s", cmd);
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}
	
	ircd_cmd_ptr = numeric_cmd_list;	
	/* Process numeric replies */
	while( ircd_cmd_ptr->name ) {
		if ( !ircstrcasecmp( ircd_cmd_ptr->name, cmd ) ) {
			if (ircd_cmd_ptr->function) {
				dlog (DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name);
				ircd_cmd_ptr->function (origin, av, ac, cmdptr);
			} else {
				dlog (DEBUG3, "process_ircd_cmd: ignoring command %s", cmd);
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
	dlog (DEBUG1, "------------------------BEGIN PARSE-------------------------");
	dlog (DEBUGRX, "RX: %s", line);
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
	dlog (DEBUG1, "origin: %s", origin);
	dlog (DEBUG1, "cmd   : %s", cmd);
	dlog (DEBUG1, "args  : %s", coreLine);
	ac = ircsplitbuf (coreLine, &av, 1);
	process_ircd_cmd (cmdptr, cmd, origin, av, ac);
	ns_free (av);
	dlog (DEBUG1, "-------------------------END PARSE--------------------------");
}

/** @brief unsupported_cmd
 *
 *  report attempts to use a feature not supported by the loaded protocol
 *
 * @return none
 */
static void unsupported_cmd (const char* cmd)
{
	irc_chanalert (ns_botptr, _("Warning, %s tried to %s which is not supported"), GET_CUR_MODNAME(), cmd);
	nlog (LOG_NOTICE, "Warning, %s tried to %s, which is not supported", GET_CUR_MODNAME(), cmd);
}

/** @brief irc_connect
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	if (!irc_send_server_connect) {
		unsupported_cmd ("SERVER CONNECT");
		return NS_FAILURE;
	}
	irc_send_server_connect (name, numeric, infoline, pass, tsboot, tslink);
	return NS_SUCCESS;
}

/** @brief irc_prefmsg_list 
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_prefmsg_list (const Bot *botptr, const Client * target, const char **text)
{
	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping irc_prefmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return NS_SUCCESS;
	}
	while (*text) {
		if (**text) {
			irc_prefmsg (botptr, target, (char*)*text);
		} else {
			irc_prefmsg (botptr, target, " ");
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg_list
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_privmsg_list (const Bot *botptr, const Client * target, const char **text)
{
	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping irc_privmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return NS_SUCCESS;
	}
	while (*text) {
		if (**text) {
			irc_privmsg (botptr, target, (char*)*text);
		} else {
			irc_privmsg (botptr, target, " ");
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_chanalert
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_chanalert (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	if (!is_synched || !botptr)
		return NS_SUCCESS;
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->name, me.serviceschan, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_prefmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_prefmsg (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe (target)) {
		nlog (LOG_NOTICE, "Dropping irc_prefmsg from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return NS_SUCCESS;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (target->flags & CLIENT_FLAG_DCC) {
		dcc_send_msg (target, ircd_buf);
	} else if (nsconfig.want_privmsg) {
		irc_send_privmsg (botptr->u->name, target->name, ircd_buf);
	} else {
		irc_send_notice (botptr?botptr->u->name:ns_botptr->u->name, target->name, ircd_buf);
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_privmsg (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping privmsg from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return NS_SUCCESS;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->u->name, target->name, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_notice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_notice (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping notice from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return NS_SUCCESS;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_notice (botptr->u->name, target->name, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_chanprivmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_chanprivmsg (const Bot *botptr, const char *chan, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->u->name, chan, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_channotice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_channotice (const Bot *botptr, const char *chan, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_notice (botptr->u->name, chan, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_globops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_globops (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (is_synched) {
		if (!irc_send_globops) {
			unsupported_cmd ("GLOBOPS");
			nlog (LOG_NOTICE, "Dropping unhandled globops: %s", ircd_buf);
			return NS_FAILURE;
		}
		irc_send_globops ((botptr?botptr->u->name:me.name), ircd_buf);
	} else {
		nlog(LOG_NOTICE, "globops before sync: %s", ircd_buf);
	}
	return NS_SUCCESS;
}

/** @brief irc_wallops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_wallops (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	if (!irc_send_wallops) {
		unsupported_cmd ("WALLOPS");
		nlog (LOG_NOTICE, "Dropping unhandled wallops: %s", ircd_buf);
		return NS_FAILURE;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_wallops ((botptr?botptr->name:me.name), ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_numeric
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_numeric (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	if (!irc_send_numeric) {
		unsupported_cmd ("NUMERIC");
		return NS_FAILURE;
	}
	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	irc_send_numeric (me.name, numeric, target, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_nick (const char *nick, const char *user, const char *host, const char *realname, const char *modes)
{
	irc_send_nick (nick, (unsigned long)me.now, modes, user, host, me.name, realname);
	return NS_SUCCESS;
}

/** @brief irc_cloakhost
 *
 *  Create a hidden hostmask for the bot 
 *  Support is currently just via UMODE auto cloaking
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_cloakhost (const Bot *botptr)
{
	if (ircd_srv.features&FEATURE_UMODECLOAK) {
		irc_umode (botptr, botptr->name, UMODE_HIDE);
		return NS_SUCCESS;	
	}
	return NS_FAILURE;	
}

/** @brief irc_umode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_umode (const Bot *botptr, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	irc_send_umode (botptr->u->name, target, newmode);
	UserMode (target, newmode);
	return NS_SUCCESS;
}

/** @brief irc_join
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_join (const Bot *botptr, const char *chan, const char *mode)
{
	time_t ts;
	Channel *c;

	c = FindChannel (chan);
	ts = (!c) ? me.now : c->creationtime;
	/* Use sjoin if available */
	if ((ircd_srv.protocol & PROTOCOL_SJOIN) && irc_send_sjoin) {
		if (mode == NULL) {
			irc_send_sjoin (me.name, botptr->u->name, chan, (unsigned long)ts);
		} else {
			ircsnprintf (ircd_buf, BUFSIZE, "%c%s", CmodeCharToPrefix (mode[1]), botptr->u->name);
			irc_send_sjoin (me.name, ircd_buf, chan, (unsigned long)ts);
		}
		JoinChannel (botptr->u->name, chan);
		/* Increment number of persistent users if needed */
		if (botptr->flags & BOT_FLAG_PERSIST) {
			c->persistentusers ++;
		}
		if (mode) {
			ChanUserMode (chan, botptr->u->name, 1, CmodeStringToMask(mode));
		}
		return NS_SUCCESS;
	}
	/* sjoin not available so use normal join */	
	irc_send_join (botptr->u->name, chan, NULL, me.now);
	JoinChannel (botptr->u->name, chan);
	if (mode) {
		irc_chanusermode (botptr, chan, mode, botptr->u->name);
	}
	return NS_SUCCESS;
}

/** @brief irc_part
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_part (const Bot *botptr, const char *chan, const char *quitmsg)
{
	Channel *c;

	if (!irc_send_part) {
		unsupported_cmd ("PART");
		return NS_FAILURE;
	}
	c = FindChannel (chan);
	/* Decrement number of persistent users if needed 
	 * Must be BEFORE we part the channel in order to trigger
	 * empty channel processing for other bots
	 */
	if (botptr->flags & BOT_FLAG_PERSIST) {
		c->persistentusers --;
	}
	irc_send_part (botptr->u->name, chan, quitmsg ? quitmsg : "");
	PartChannel (botptr->u, (char *) chan, quitmsg);
	return NS_SUCCESS;
}

/** @brief irc_nickchange
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_nickchange (const Bot *botptr, const char *newnick)
{
	if (!botptr) {
		nlog (LOG_WARNING, "Unknown bot tried to change nick to %s", newnick);
		return NS_FAILURE;
	}
	/* Check newnick is not in use */
	if (FindUser (newnick)) {
		nlog (LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", botptr->name, newnick);
		return NS_FAILURE;
	}
	irc_send_nickchange (botptr->name, newnick, me.now);
	UserNickChange (botptr->name, newnick, NULL);
	return NS_SUCCESS;
}

/** @brief irc_setname
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_setname (const Bot *botptr, const char* realname)
{
	if (!irc_send_setname) {
		unsupported_cmd ("SETNAME");
		return NS_FAILURE;
	}
	irc_send_setname (botptr->name, realname);
	strlcpy (botptr->u->info, (char*)realname, MAXHOST);
	return NS_SUCCESS;
}

/** @brief irc_sethost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_sethost (const Bot *botptr, const char* host)
{
	if (!irc_send_sethost) {
		unsupported_cmd ("SETNAME");
		return NS_FAILURE;
	}
	irc_send_sethost (botptr->name, host);
	strlcpy (botptr->u->user->hostname, (char*)host, MAXHOST);
	return NS_SUCCESS;
}
 
/** @brief irc_setident
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_setident (const Bot *botptr, const char* ident)
{
	if (!irc_send_setident) {
		unsupported_cmd ("SETNAME");
		return NS_FAILURE;
	}
	irc_send_setident (botptr->name, ident);
	strlcpy (botptr->u->user->username, (char*)ident, MAXHOST);
	return NS_SUCCESS;
}

/** @brief irc_cmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_cmode (const Bot *botptr, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	irc_send_cmode (me.name, botptr->u->name, chan, mode, args, me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	ns_free (av);
	return NS_SUCCESS;
}

/** @brief irc_chanusermode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_chanusermode (const Bot *botptr, const char *chan, const char *mode, const char *target)
{
	if ((ircd_srv.protocol & PROTOCOL_B64NICK)) {
		irc_send_cmode (me.name, botptr->u->name, chan, mode, nick_to_base64 (target), me.now);
	} else {
		irc_send_cmode (me.name, botptr->u->name, chan, mode, target, me.now);
	}
	ChanUserMode (chan, target, 1, CmodeStringToMask(mode));
	return NS_SUCCESS;
}

/** @brief irc_quit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_quit (const Bot * botptr, const char *quitmsg)
{
	if (!botptr) {
		return NS_FAILURE;
	}
	irc_send_quit (botptr->u->name, quitmsg);
	do_quit (botptr->u->name, quitmsg);
	return NS_SUCCESS;
}

/** @brief irc_kill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_kill (const Bot *botptr, const char *target, const char *reason, ...)
{
	va_list ap;

	if (!irc_send_kill) {
		unsupported_cmd ("KILL");
		return NS_FAILURE;
	}
	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	irc_send_kill (botptr->u->name, target, ircd_buf);
	do_quit (target, ircd_buf);
	return NS_SUCCESS;
}

/** @brief irc_kick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_kick (const Bot *botptr, const char *chan, const char *target, const char *reason)
{
	if (!irc_send_kick) {
		unsupported_cmd ("KICK");
		return NS_FAILURE;
	}
	irc_send_kick (botptr->u->name, chan, target, reason);
	PartChannel (FindUser (target), (char *) chan, reason[0] != 0 ? (char *)reason : NULL);
	return NS_SUCCESS;
}

/** @brief irc_invite
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_invite (const Bot *botptr, const Client *target, const char *chan) 
{
	if (!irc_send_invite) {
		unsupported_cmd ("INVITE");
		return NS_FAILURE;
	}
	irc_send_invite (botptr->u->name, target->name, chan);
	return NS_SUCCESS;
}

/** @brief irc_svstime
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_svstime (const Bot *botptr, Client *target, const time_t ts)
{
	if (!irc_send_svstime) {
		unsupported_cmd ("SVSTIME");
		return NS_FAILURE;
	}
	irc_send_svstime (me.name, (unsigned long)ts);
	nlog (LOG_NOTICE, "irc_svstime: synching server times to %lu", ts);
	return NS_SUCCESS;
}

/** @brief irc_svskill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svskill (const Bot *botptr, Client *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	if (irc_send_svskill) {
		irc_send_svskill (me.name, target->name, ircd_buf);
	} else if (irc_send_kill) {
		irc_send_kill (me.name, target->name, ircd_buf);
		do_quit (target->name, ircd_buf);
	} else {
		unsupported_cmd ("SVSKILL");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief irc_svsmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svsmode (const Bot *botptr, Client *target, const char *modes)
{
	if (!irc_send_svsmode) {
		unsupported_cmd ("SVSMODE");
		return NS_FAILURE;
	}
	irc_send_svsmode(me.name, target->name, modes);
	UserMode (target->name, modes);
	return NS_SUCCESS;
}

/** @brief irc_svshost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svshost (const Bot *botptr, Client *target, const char *vhost)
{
	if (!irc_send_svshost) {
		unsupported_cmd ("SVSHOST");
		return NS_FAILURE;
	}
	strlcpy (target->user->vhost, vhost, MAXHOST);
	target->flags |= CLIENT_FLAG_SETHOST;
	irc_send_svshost (me.name, target->name, vhost);
	return NS_SUCCESS;
}

/** @brief irc_svsjoin
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svsjoin (const Bot *botptr, Client *target, const char *chan)
{
	if (!irc_send_svsjoin) {
		unsupported_cmd ("SVSJOIN");
		return irc_invite (botptr, target, chan);
	}
	irc_send_svsjoin (me.name, target->name, chan);
	return NS_SUCCESS;
}

/** @brief irc_svspart
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svspart (const Bot *botptr, Client *target, const char *chan)
{
	if (!irc_send_svspart) {
		unsupported_cmd ("SVSPART");
		return NS_FAILURE;
	}
	irc_send_svspart (me.name, target->name, chan);
	return NS_SUCCESS;
}

/** @brief irc_swhois
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_swhois (const char *target, const char *swhois)
{
	if (!irc_send_swhois) {
		unsupported_cmd ("SWHOIS");
		return NS_FAILURE;
	}
	irc_send_swhois (me.name, target, swhois);
	return NS_SUCCESS;
}

/** @brief irc_svsnick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_svsnick (const Bot *botptr, Client *target, const char *newnick)
{
	if (!irc_send_svsnick) {
		unsupported_cmd ("SVSNICK");
		return NS_FAILURE;
	}
	irc_send_svsnick (me.name, target->name, newnick, me.now);
	return NS_SUCCESS;
}

/** @brief irc_smo
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_smo (const char *from, const char *umodetarget, const char *msg)
{
	if (!irc_send_smo) {
		unsupported_cmd ("SMO");
		return NS_FAILURE;
	}
	irc_send_smo (from, umodetarget, msg);
	return NS_SUCCESS;
}

/** @brief irc_akill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_akill (const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ...)
{
	va_list ap;

	if (!irc_send_akill) {
		unsupported_cmd ("AKILL");
		return NS_FAILURE;
	}
	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	irc_send_akill(me.name, host, ident, botptr->name, length, ircd_buf, me.now);
	return NS_SUCCESS;
}

/** @brief irc_rakill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_rakill (const Bot *botptr, const char *host, const char *ident)
{
	if (!irc_send_rakill) {
		unsupported_cmd ("RAKILL");
		return NS_FAILURE;
	}
	irc_send_rakill (me.name, host, ident);
	return NS_SUCCESS;
}

/** @brief irc_ping
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_ping (const char *from, const char *reply, const char *to)
{
	if (!irc_send_ping) {
		unsupported_cmd ("PING");
		return NS_FAILURE;
	}
	irc_send_ping (from, reply, to);
	return NS_SUCCESS;
}

/** @brief irc_pong
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_pong (const char *reply)
{
	if (!irc_send_pong) {
		unsupported_cmd ("PONG");
		return NS_FAILURE;
	}
	irc_send_pong (reply);
	return NS_SUCCESS;
}

/** @brief irc_server
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_server (const char *name, const int numeric, const char *infoline)
{
	if (!irc_send_server) {
		unsupported_cmd ("SERVER");
		return NS_FAILURE;
	}
	irc_send_server (me.name, name, numeric, infoline);
	return NS_SUCCESS;
}

/** @brief irc_squit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_squit (const char *server, const char *quitmsg)
{
	if (!irc_send_squit) {
		unsupported_cmd ("SQUIT");
		return NS_FAILURE;
	}
	irc_send_squit (server, quitmsg);
	return NS_SUCCESS;
}

/** @brief do_synch_neostats
 *
 * 
 *
 * @return none
 */
void
do_synch_neostats (void)
{
	irc_globops (NULL, _("Link with Network \2Complete!\2"));
	init_services_bot ();
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
	irc_pong (origin);
	if (ircd_srv.burst) {
		irc_ping (me.name, origin, origin);
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
	Client *s;
	CmdParams * cmdparams;

	s = FindServer (origin);
	if (s) {
		s->server->ping = me.now - me.tslastping;
		if (me.ulag > 1)
			s->server->ping -= me.ulag;
		if( IsMe( s ) )
			me.ulag = me.s->server->ping;
		cmdparams = (CmdParams*)ns_calloc (sizeof(CmdParams));
		cmdparams->source = s;
		SendAllModuleEvent (EVENT_PONG, cmdparams);
		ns_free (cmdparams);
		return;
	}
	nlog (LOG_NOTICE, "Received PONG from unknown server: %s", origin);
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
	irc_numeric (RPL_VERSION, nick, "%s :%s -> %s %s", me.version, me.name, ns_module_info.build_date, ns_module_info.build_time);
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
	if (!fp) {
		irc_numeric (ERR_NOMOTD, nick, _(":- MOTD file Missing"));
	} else {
		irc_numeric (RPL_MOTDSTART, nick, _(":- %s Message of the Day -"), me.name);
		irc_numeric (RPL_MOTD, nick, _(":- %s. Copyright (c) 1999 - 2004 The NeoStats Group"), me.version);
		irc_numeric (RPL_MOTD, nick, ":-");

		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			irc_numeric (RPL_MOTD, nick, ":- %s", buf);
		}
		fclose (fp);
		irc_numeric (RPL_ENDOFMOTD, nick, _(":End of MOTD command."));
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
	if (!fp) {
		irc_numeric (ERR_NOADMININFO, nick, _("%s :No administrative info available"), me.name);
	} else {
		irc_numeric (RPL_ADMINME, nick, _(":%s :Administrative info"), me.name);
		irc_numeric (RPL_ADMINME, nick, _(":%s.  Copyright (c) 1999 - 2004 The NeoStats Group"), me.version);
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			irc_numeric (RPL_ADMINLOC1, nick, ":- %s", buf);
		}
		fclose (fp);
		irc_numeric (RPL_ADMINLOC2, nick, _("End of /ADMIN command."));
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
	irc_numeric (RPL_VERSION, nick, ":- NeoStats %s Credits ", me.version);
	irc_numeric (RPL_VERSION, nick, ":- Now Maintained by Fish (fish@dynam.ac) and Mark (mark@ctcp.net)");
	irc_numeric (RPL_VERSION, nick, ":- Previous Authors: Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	irc_numeric (RPL_VERSION, nick, ":- For Support, you can find us at");
	irc_numeric (RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats");
	irc_numeric (RPL_VERSION, nick, ":- Thanks to:");
	irc_numeric (RPL_VERSION, nick, ":- Enigma for being part of the dev team");
	irc_numeric (RPL_VERSION, nick, ":- Stskeeps for writing the best IRCD ever!");
	irc_numeric (RPL_VERSION, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	irc_numeric (RPL_VERSION, nick, ":- monkeyIRCD for the Module Segv Catching code");
	irc_numeric (RPL_VERSION, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	irc_numeric (RPL_VERSION, nick, ":- Andy For Ideas");
	irc_numeric (RPL_VERSION, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	irc_numeric (RPL_VERSION, nick, ":- sre and Jacob for development systems and access");
	irc_numeric (RPL_VERSION, nick, ":- Error51 for Translating our FAQ and README files");
	irc_numeric (RPL_VERSION, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!");
	irc_numeric (RPL_VERSION, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't (and all the bug reports!)");
	irc_numeric (RPL_VERSION, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips :)");
	irc_numeric (RPL_VERSION, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback");
	irc_numeric (RPL_VERSION, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd");
	irc_numeric (RPL_VERSION, nick, ":- Blud - Giving us patches for Mystic IRCd");
	irc_numeric (RPL_VERSION, nick, ":- herrohr - Giving us patches for Liquid IRCd support");
	irc_numeric (RPL_VERSION, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support");
	irc_numeric (RPL_VERSION, nick, ":- Reed Loden - Contributions to IRCu support");
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
	Client *u;

	SET_SEGV_LOCATION();
	u = FindUser (nick);
	if (!u) {
		nlog (LOG_WARNING, "do_stats: message from unknown user %s", nick);
		return;
	}
	if (!ircstrcasecmp (what, "u")) {
		/* server uptime - Shmad */
		time_t uptime = me.now - me.ts_boot;
		irc_numeric (RPL_STATSUPTIME, u->name, __("Statistical Server up %ld days, %ld:%02ld:%02ld", u), uptime / 86400, (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
	} else if (!ircstrcasecmp (what, "c")) {
		/* Connections */
		irc_numeric (RPL_STATSNLINE, u->name, "N *@%s * * %d 50", me.uplink, me.port);
		irc_numeric (RPL_STATSCLINE, u->name, "C *@%s * * %d 50", me.uplink, me.port);
	} else if (!ircstrcasecmp (what, "o")) {
		/* Operators */
	} else if (!ircstrcasecmp (what, "l")) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.ts_boot;
		irc_numeric (RPL_STATSLINKINFO, u->name, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		irc_numeric (RPL_STATSLLINE, u->name, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, (int)me.SendM, (int)me.SendBytes, (int)me.RcveM, (int)me.RcveBytes, (int)tmp2, (int)tmp);
        } else if (!ircstrcasecmp(what, "Z")) {
                if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
                        do_dns_stats_Z(u);
                }
	} else if (!ircstrcasecmp (what, "M")) {
		ircd_cmd_ptr = cmd_list;
		while (ircd_cmd_ptr->name) {
			if (ircd_cmd_ptr->usage > 0) {
				irc_numeric (RPL_STATSCOMMANDS, u->name, "Command %s Usage %d", ircd_cmd_ptr->name, ircd_cmd_ptr->usage);
			}
			ircd_cmd_ptr ++;
		}
	}
	irc_numeric (RPL_ENDOFSTATS, u->name, __("%s :End of /STATS report", u), what);
	irc_chanalert (ns_botptr, _("%s Requested Stats %s"), u->name, what);
};

void do_protocol (char *origin, char **argv, int argc)
{
	protocol_entry *protocol_ptr;
	int i;

	for (i = 0; i < argc; i++) {
		protocol_ptr = protocol_list;
		while (protocol_ptr->token)
		{
			if (!ircstrcasecmp (protocol_ptr->token, argv[i])) {
				if (protocol_info->optprotocol&protocol_ptr->flag) {
					ircd_srv.protocol |= protocol_ptr->flag;
					break;
				}
			}
			protocol_ptr ++;
		}
	}
}

/* SJOIN <TS> #<channel> <modes> :[@][+]<nick_1> ...  [@][+]<nick_n> */
void 
do_sjoin (char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc)
{
	char nick[MAXNICK];
	char* nicklist;
	long mask = 0;
	int ok = 1, j = 3;
	Channel *c;
	char **param;
	int paramcnt = 0;
	int paramidx = 0;

	if (*modes == '#') {
		JoinChannel (sjoinnick, modes);
		return;
	}

	paramcnt = split_buf(argv[argc-1], &param, 0);
		   
	while (paramcnt > paramidx) {
		nicklist = param[paramidx];
		if (ircd_srv.protocol & PROTOCOL_SJ3) {
			/* Unreal passes +b(&) and +e(") via SJ3 so skip them for now */	
			if (*nicklist == '&' || *nicklist == '"') {
				dlog (DEBUG1, "Skipping %s", nicklist);
				paramidx++;
				continue;
			}
		}
		mask = 0;
		while (CmodePrefixToMask (*nicklist)) {
			mask |= CmodePrefixToMask (*nicklist);
			nicklist ++;
		}
		strlcpy (nick, nicklist, MAXNICK);
		JoinChannel (nick, channame); 
		ChanUserMode (channame, nick, 1, mask);
		paramidx++;
		ok = 1;
	}
	c = FindChannel (channame);
	if (c) {
		/* update the TS time */
		c->creationtime = atoi (tstime);
		j = ChanModeHandler (c, modes, j, argv, argc);
	}
	ns_free(param);
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
	do_synch_neostats ();
}

void 
do_snetinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, CLOAKKEYLEN);
	strlcpy (me.netname, netname, MAXPASS);
	irc_send_snetinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	do_synch_neostats ();
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
		JoinChannel (nick, s);
	}
}

void 
do_part (const char* nick, const char* chan, const char* reason)
{
	PartChannel (FindUser (nick), chan, reason);
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
	if (modes) {
		UserMode (nick, modes);
	}
	if (vhost) {
		SetUserVhost(nick, vhost);
	}
	if (smodes) {
		UserSMode (nick, smodes);
	}
}

void 
do_client (const char *nick, const char *hopcount, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *servicestamp, 
		const char *ip, const char *realname)
{
	do_nick (nick, hopcount, TS, user, host, server, ip, servicestamp, 
		modes, vhost, realname, NULL, smodes );
}

void
do_kill (const char *source, const char *nick, const char *reason)
{
	KillUser (source, nick, reason);
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
	KickChannel (kickby, chan, kicked, kickreason);
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
	if (irc_send_vctrl) {
		irc_send_vctrl (ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname);
	} 
}

void 
do_smode (const char* targetnick, const char* modes)
{
	UserSMode (targetnick, modes);
}

void 
do_mode_user (const char* targetnick, const char* modes)
{
	UserMode (targetnick, modes);
}

void 
do_svsmode_user (const char* targetnick, const char* modes, const char* ts)
{
	char modebuf[MODESIZE];
	
	if (ts && isdigit(*ts)) {
		const char* pModes;	
		char* pNewModes;	

		SetUserServicesTS (targetnick, ts);
		/* If only setting TS, we do not need further mode processing */
		if (ircstrcasecmp(modes, "+d") == 0) {
			dlog (DEBUG3, "dropping modes since this is a services TS %s", modes);
			return;
		}
		/* We need to strip the d from the mode string */
		pNewModes = modebuf;
		pModes = modes;
		while (*pModes) {
			if (*pModes != 'd') {
				*pNewModes = *pModes;
			}
			pModes++;
			pNewModes++;			
		}
		/* NULL terminate */
		*pNewModes = 0;
		UserMode (targetnick, modebuf);
	} else {
		UserMode (targetnick, modes);
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
	SetUserVhost (nick, vhost);
}

void
do_nickchange (const char * oldnick, const char *newnick, const char * ts)
{
	UserNickChange (oldnick, newnick, ts);
}

void 
do_topic (const char* chan, const char *owner, const char* ts, const char *topic)
{
	ChannelTopic (chan, owner, ts, topic);
}

void 
do_server (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv)
{
	if (!srv) {
		if (uplink == NULL || *uplink == 0) {
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
			if (irc_send_burst) {
				irc_send_burst (0);
			}
			ircd_srv.burst = 0;
			do_synch_neostats ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

void 
do_swhois (char *who, char *swhois)
{
	Client * u;
	u = FindUser(who);
	if (u) {
		strlcpy(u->user->swhois, swhois, MAXHOST);
	}
}

void 
do_tkl(const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason)
{
	static char mask[MAXHOST];

	ircsnprintf(mask, MAXHOST, "%s@%s", user, host);
	if (add[0] == '+') {
		AddBan (type, user, host, mask, reason, setby, tsset, tsexpire);
	} else {
		DelBan (type, user, host, mask, reason, setby, tsset, tsexpire);
	}
}

void 
do_eos (const char *name)
{
	Client *s;

	s = FindServer (name);
	if (s) {
		SynchServer(s);
		dlog (DEBUG1, "do_eos: server %s is now synched", name);
	} else {
		nlog (LOG_WARNING, "do_eos: server %s not found", name);
	}
}

void do_setname (const char* nick, const char* realname)
{
	Client *u;

	u = FindUser(nick);
	if (u) {
		dlog (DEBUG1, "do_setname: setting realname of user %s to %s", nick, realname);
		strlcpy(u->info, (char*)realname, MAXHOST);
	} else {
		nlog (LOG_WARNING, "do_setname: user %s not found", nick);
	}
}

void do_sethost (const char* nick, const char* host)
{
	Client *u;

	u = FindUser(nick);
	if (u) {
		dlog (DEBUG1, "do_sethost: setting host of user %s to %s", nick, host);
		strlcpy(u->user->hostname, (char*)host, MAXHOST);
	} else {
		nlog (LOG_WARNING, "do_sethost: user %s not found", nick);
	}
}

void do_setident (const char* nick, const char* ident)
{
	Client *u;

	u = FindUser(nick);
	if (u) {
		dlog (DEBUG1, "do_setident: setting ident of user %s to %s", nick, ident);
		strlcpy(u->user->username, (char*)ident, MAXHOST);
	} else {
		nlog (LOG_WARNING, "do_setident: user %s not found", nick);
	}
}

/** @brief send_cmd
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
void
send_cmd (char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZE];
	int buflen;
	
	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);

	dlog (DEBUGTX, "%s", buf);
	if (strnlen (buf, BUFSIZE) < BUFSIZE - 2) {
		strlcat (buf, "\n", BUFSIZE);
	} else {
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\n';
	}
	buflen = strnlen (buf, BUFSIZE);
	send_to_socket (buf, buflen);
}

/*
RX: :irc.foo.com 351 stats.neostats.net Unreal3.2. irc.foo.com :FinWXOoZ [*=2303]
*/
static void m_numeric351 (char *origin, char **argv, int argc, int srv)
{
	Client *s;

	s = FindServer(origin);
	if (s) {
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
	Client *s;

	s = FindServer(origin);
	if (s) {
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

		s->server->uptime = secs;
	}
}

/** @brief HaveFeature
 *
 *  @return 1 if have else 0
 */
int 
HaveFeature (int mask)
{
	return (ircd_srv.features&mask);
}
