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

ircd_server ircd_srv;

static char ircd_buf[BUFSIZE];

ProtocolInfo* protocol_info;

ircd_cmd* cmd_list;

static void *protocol_module_handle;

void (*irc_send_privmsg) (const char *from, const char *to, const char *buf);
void (*irc_send_notice) (const char *from, const char *to, const char *buf);
void (*irc_send_globops) (const char *from, const char *buf);
void (*irc_send_wallops) (const char *who, const char *buf);
void (*irc_send_numeric) (const char *from, const int numeric, const char *target, const char *buf);
void (*irc_send_umode) (const char *who, const char *target, const char *mode);
void (*irc_send_join) (const char *who, const char *chan, const unsigned long ts);
void (*irc_send_sjoin) (const char *source, const char *who, const char *chan, const unsigned long ts);
void (*irc_send_part) (const char *who, const char *chan);
void (*irc_send_nickchange) (const char *oldnick, const char *newnick, const unsigned long ts);
void (*irc_send_cmode) (const char *source, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
void (*irc_send_quit) (const char *who, const char *quitmsg);
void (*irc_send_kill) (const char *from, const char *target, const char *reason);
void (*irc_send_kick) (const char *source, const char *chan, const char *target, const char *reason);
void (*irc_send_invite) (const char *from, const char *to, const char *chan);
void (*irc_send_svskill) (const char *source, const char *target, const char *reason);
void (*irc_send_svsmode) (const char *source, const char *target, const char *modes);
void (*irc_send_svshost) (const char *source, const char *who, const char *vhost);
void (*irc_send_svsjoin) (const char *source, const char *target, const char *chan);
void (*irc_send_svspart) (const char *source, const char *target, const char *chan);
void (*irc_send_svsnick) (const char *source, const char *target, const char *newnick, const unsigned long ts);
void (*irc_send_swhois) (const char *source, const char *target, const char *swhois);
void (*irc_send_smo) (const char *from, const char *umodetarget, const char *msg);
void (*irc_send_akill) (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, unsigned long ts);
void (*irc_send_rakill) (const char *source, const char *host, const char *ident);
void (*irc_send_ping) (const char *from, const char *reply, const char *to);
void (*irc_send_pong) (const char *reply);
void (*irc_send_server) (const char *source, const char *name, const int numeric, const char *infoline);
void (*irc_send_squit) (const char *server, const char *quitmsg);
void (*irc_send_nick) (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
void (*irc_send_server_connect) (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
void (*irc_send_netinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
void (*irc_send_snetinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
void (*irc_send_svinfo) (const int tscurrent, const int tsmin, const unsigned long tsnow);
void (*irc_send_vctrl) (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
void (*irc_send_burst) (int b);
void (*irc_send_svstime) (const char *source, const unsigned long ts);
void (*irc_send_setname) (const char* nick, const char* realname);
void (*irc_send_sethost) (const char* nick, const char* host);
void (*irc_send_setident) (const char* nick, const char* ident);

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
	printf ("\nERROR: Unable to find %s in selected IRCd module\n", err);
	nlog (LOG_CRITICAL, "Unable to find %s in selected IRCd module", err);
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
	static char protocol_name[MAXHOST];
  
	ircsnprintf (protocol_name, 255, "%s/%s%s", MOD_PATH, me.protocol,MOD_EXT);
	protocol_module_handle = ns_dlopen(protocol_name, RTLD_NOW || RTLD_GLOBAL);
	if(!protocol_module_handle) {
		printf ("\nERROR: Unable to load protocol module %s\n", protocol_name);
		nlog (LOG_CRITICAL, "Unable to load protocol module %s\n", protocol_name);
		return NS_FAILURE;	
	}
	protocol_info = ns_dlsym( protocol_module_handle, "protocol_info");
	if(!protocol_info) {
		printf ("\nERROR: Unable to find protocol_info in protocol module %s\n", protocol_name);
		nlog (LOG_CRITICAL, "Unable to find protocol_info in protocol module %s\n", protocol_name);
		return NS_FAILURE;	
	}

	if (protocol_info->minprotocol & PROTOCOL_CLIENTMODE) {
		config.singlebotmode = 1;
	}

	strcpy(me.servicescmode,protocol_info->services_cmode);
	strcpy(me.servicesumode,protocol_info->services_umode);

	/* Allow protocol module to "override" the parser */
	irc_parse = ns_dlsym( protocol_module_handle, "parse");
	if(irc_parse == NULL)
		irc_parse = parse;

	cmd_list = ns_dlsym( protocol_module_handle, "cmd_list");
	if(!cmd_list) {
		IrcdError("command list");
		return NS_FAILURE;	
	}
	chan_umodes = ns_dlsym( protocol_module_handle, "chan_umodes");
	if(!chan_umodes) {
		IrcdError("channel umode table");
		return NS_FAILURE;	
	}
	chan_modes  = ns_dlsym( protocol_module_handle, "chan_modes");
	if(!chan_modes) {
		IrcdError("channel mode table");
		return NS_FAILURE;	
	}
	user_umodes = ns_dlsym( protocol_module_handle, "user_umodes");
	if(!user_umodes) {
		IrcdError("user mode table");
		return NS_FAILURE;	
	}
	/* Not required */
	user_smodes = ns_dlsym( protocol_module_handle, "user_smodes");
	if(user_smodes) {
		ircd_srv.features |= FEATURE_USERSMODES;
	}

	irc_send_privmsg = ns_dlsym( protocol_module_handle, "send_privmsg");
	if(!irc_send_privmsg) {
		IrcdError("send_privmsg handler");
		return NS_FAILURE;	
	}
	irc_send_notice = ns_dlsym( protocol_module_handle, "send_notice");
	if(!irc_send_notice) {
		IrcdError("send_notice handler");
		return NS_FAILURE;	
	}
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
	if(irc_send_svskill) {
		ircd_srv.features |= FEATURE_SVSKILL;
	}
	irc_send_svsmode = ns_dlsym( protocol_module_handle, "send_svsmode");
	if(irc_send_svsmode) {
		ircd_srv.features |= FEATURE_SVSMODE;
	}
	irc_send_svshost = ns_dlsym( protocol_module_handle, "send_svshost");
	if(irc_send_svshost) {
		ircd_srv.features |= FEATURE_SVSHOST;
	}
	irc_send_svsjoin = ns_dlsym( protocol_module_handle, "send_svsjoin");
	if(irc_send_svsjoin) {
		ircd_srv.features |= FEATURE_SVSJOIN;
	}
	irc_send_svspart = ns_dlsym( protocol_module_handle, "send_svspart");
	if(irc_send_svspart) {
		ircd_srv.features |= FEATURE_SVSPART;
	}
	irc_send_svsnick = ns_dlsym( protocol_module_handle, "send_svsnick");
	if(irc_send_svsnick) {
		ircd_srv.features |= FEATURE_SVSNICK;
	}
	irc_send_swhois = ns_dlsym( protocol_module_handle, "send_swhois");
	if(irc_send_swhois) {
		ircd_srv.features |= FEATURE_SWHOIS;
	}
	irc_send_smo = ns_dlsym( protocol_module_handle, "send_smo");
	if(irc_send_smo) {
		ircd_srv.features |= FEATURE_SMO;
	}
	irc_send_svstime = ns_dlsym( protocol_module_handle, "send_svstime");
	if(irc_send_svstime) {
		ircd_srv.features |= FEATURE_SVSTIME;
	}
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
	irc_send_setname = ns_dlsym( protocol_module_handle, "send_setname");
	irc_send_sethost = ns_dlsym( protocol_module_handle, "send_sethost");
	irc_send_setident = ns_dlsym( protocol_module_handle, "send_setident");

	return NS_SUCCESS;
}

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 * @return 
 */
int
InitIrcd ()
{
	/* Clear IRCD info */
	memset(&ircd_srv, 0, sizeof(ircd_srv));
	/* Setup IRCD function calls */
	if(InitIrcdSymbols() != NS_SUCCESS) 
		return NS_FAILURE;
	/* set min protocol */
	ircd_srv.protocol = protocol_info->minprotocol;
	/* Build mode tables */
	if(InitIrcdModes() != NS_SUCCESS) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief process notice
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
	dlog(DEBUGRX, "RX: %s", line);
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
	ac = ircsplitbuf (coreLine, &av, 1);
	process_ircd_cmd (cmdptr, cmd, origin, av, ac);
	ns_free (av);
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
	Client *s;
	CmdParams * cmdparams;

	s = find_server (origin);
	if (s) {
		s->server->ping = me.now - ping.last_sent;
		if (ping.ulag > 1)
			s->server->ping -= ping.ulag;
		if (!strcmp (me.s->name, s->name))
			ping.ulag = me.s->server->ping;
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
	if(!fp) {
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
	if(!fp) {
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
	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "do_stats: message from unknown user %s", nick);
		return;
	}
	if (!ircstrcasecmp (what, "u")) {
		/* server uptime - Shmad */
		time_t uptime = me.now - me.t_start;
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
		tmp2 = me.now - me.t_start;
		irc_numeric (RPL_STATSLINKINFO, u->name, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		irc_numeric (RPL_STATSLLINE, u->name, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, (int)me.SendM, (int)me.SendBytes, (int)me.RcveM, (int)me.RcveBytes, (int)tmp2, (int)tmp);
        } else if (!ircstrcasecmp(what, "Z")) {
                if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
                        do_dns_stats_Z(u);
                }
	} else if (!ircstrcasecmp (what, "M")) {
		ircd_cmd_ptr = cmd_list;
		while(ircd_cmd_ptr->name) {
			if (ircd_cmd_ptr->usage > 0) {
				irc_numeric (RPL_STATSCOMMANDS, u->name, "Command %s Usage %d", ircd_cmd_ptr->name, ircd_cmd_ptr->usage);
			}
			ircd_cmd_ptr ++;
		}
	}
	irc_numeric (RPL_ENDOFSTATS, u->name, __("%s :End of /STATS report", u), what);
	irc_chanalert (ns_botptr, _("%s Requested Stats %s"), u->name, what);
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
		else if (!ircstrcasecmp ("NICKv2", argv[i])) {
			if(protocol_info->optprotocol&PROTOCOL_NICKv2) {
				ircd_srv.protocol |= PROTOCOL_NICKv2;
			}			
		}
	}
}

int 
irc_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	irc_send_server_connect (name, numeric, infoline, pass, tsboot, tslink);
	return NS_SUCCESS;
}

void
irc_prefmsg_list (const Bot *botptr, const Client * target, const char **text)
{
	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping irc_prefmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return;
	}
	while (*text) {
		if (**text) {
			irc_prefmsg (botptr, target, (char*)*text);
		} else {
			irc_prefmsg (botptr, target, " ");
		}
		text++;
	}
}

void
irc_privmsg_list (const Bot *botptr, const Client * target, const char **text)
{
	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping irc_privmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return;
	}
	while (*text) {
		if (**text) {
			irc_privmsg (botptr, target, (char*)*text);
		} else {
			irc_privmsg (botptr, target, " ");
		}
		text++;
	}
}


void
irc_chanalert (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	if (!is_synched)
		return;
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->name, me.serviceschan, ircd_buf);
}

void
irc_prefmsg (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe (target)) {
		nlog (LOG_NOTICE, "Dropping irc_prefmsg from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (config.want_privmsg) {
		irc_send_privmsg (botptr->u->name, target->name, ircd_buf);
	} else {
		irc_send_notice (botptr?botptr->u->name:ns_botptr->u->name, target->name, ircd_buf);
	}
}

void
irc_privmsg (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping privmsg from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->u->name, target->name, ircd_buf);
}

void
irc_notice (const Bot *botptr, const Client *target, const char *fmt, ...)
{
	va_list ap;

	if (IsMe(target)) {
		nlog (LOG_NOTICE, "Dropping notice from bot (%s) to bot (%s)", botptr->u->name, target->name);
		return;
	}
	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_notice (botptr->u->name, target->name, ircd_buf);
}

void
irc_chanprivmsg (const Bot *botptr, const char *chan, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_privmsg (botptr->u->name, chan, ircd_buf);
}

void
irc_channotice (const Bot *botptr, const char *chan, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_send_notice (botptr->u->name, chan, ircd_buf);
}

void
irc_globops (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);

	if (is_synched) {
		if(irc_send_globops) {
			irc_send_globops((botptr?botptr->u->name:me.name), ircd_buf);
		} else {
			nlog (LOG_NOTICE, "Dropping unhandled globops: %s", ircd_buf);
		}
	} else {
		nlog (LOG_NORMAL, ircd_buf);
	}
}

void
irc_wallops (const Bot *botptr, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if(irc_send_wallops) {
		irc_send_wallops ((botptr?botptr->name:me.name), ircd_buf);
	} else {
		nlog (LOG_NOTICE, "Dropping unhandled wallops: %s", ircd_buf);
	}
}

void
irc_numeric (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	irc_send_numeric (me.name, numeric, target, ircd_buf);
}

/** @brief unsupported_cmd
 *
 *  report attempts to use a feature not supported by the loaded protocol
 *
 * @return none
 */
static void
unsupported_cmd(const char* cmd)
{
	irc_chanalert (ns_botptr, _("Warning, %s tried to %s which is not supported"), GET_CUR_MODNAME(), cmd);
	nlog (LOG_NOTICE, "Warning, %s tried to %s, which is not supported", GET_CUR_MODNAME(), cmd);
}

/** @brief irc_nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
irc_nick (const char *nick, const char *user, const char *host, const char *realname, const char *modes)
{
	if(irc_send_nick) {
		irc_send_nick (nick, (unsigned long)me.now, modes, user, host, me.name, realname);
	} else {
		unsupported_cmd("NICK");
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
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
irc_cloakhost (const Bot *botptr)
{
	if (ircd_srv.features&FEATURE_UMODECLOAK) {
		irc_usermode (botptr, botptr->name, UMODE_HIDE);
		return NS_SUCCESS;	
	}
	return NS_FAILURE;	
}

int
irc_usermode (const Bot *botptr, const char *target, long mode)
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

	c = find_chan (chan);
	ts = (!c) ? me.now : c->creationtime;
	/* Use sjoin if available */
	if((ircd_srv.protocol & PROTOCOL_SJOIN) && irc_send_sjoin) {
		if (mode == NULL) {
			irc_send_sjoin (me.name, botptr->u->name, chan, (unsigned long)ts);
		} else {
			ircsnprintf (ircd_buf, BUFSIZE, "%c%s", CmodeCharToPrefix (mode[1]), botptr->u->name);
			irc_send_sjoin (me.name, ircd_buf, chan, (unsigned long)ts);
		}
		join_chan (botptr->u->name, chan);
		if (mode) {
			ChanUserMode(chan, botptr->u->name, 1, CmodeStringToMask(mode));
		}
	/* sjoin not available so use normal join */	
	} else if(irc_send_join) {
		irc_send_join (botptr->u->name, chan, me.now);
		join_chan (botptr->u->name, chan);
		if(mode) {
			irc_chanusermode(botptr, chan, mode, botptr->u->name);
		}
	/* Error */
	} else {
		unsupported_cmd("SJOIN/JOIN");
	} 
	return NS_SUCCESS;
}

int
irc_part (const Bot *botptr, const char *chan)
{
	irc_send_part(botptr->u->name, chan);
	part_chan (botptr->u, (char *) chan, NULL);
	return NS_SUCCESS;
}

int
irc_nickchange (const Bot *botptr, const char *newnick)
{
	if (!botptr) {
		nlog (LOG_WARNING, "Unknown bot tried to change nick to %s", newnick);
		return NS_FAILURE;
	}
	/* Check newnick is not in use */
	if (find_user (newnick)) {
		nlog (LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", botptr->name, newnick);
		return NS_FAILURE;
	}
	UserNick (botptr->name, newnick, NULL);
	irc_send_nickchange (botptr->name, newnick, me.now);
	bot_nick_change(botptr, newnick);
	return NS_SUCCESS;
}

int irc_setname(const Bot *botptr, const char* realname)
{
	if (!irc_send_setname) {
		unsupported_cmd ("SETNAME");
		return NS_FAILURE;
	}
	irc_send_setname (botptr->name, realname);
	strlcpy (botptr->u->info, (char*)realname, MAXHOST);
	return NS_SUCCESS;
}

int irc_sethost (const Bot *botptr, const char* host)
{
	if (!irc_send_sethost) {
		unsupported_cmd("SETNAME");
		return NS_FAILURE;
	}
	irc_send_sethost (botptr->name, host);
	strlcpy (botptr->u->user->hostname, (char*)host, MAXHOST);
	return NS_SUCCESS;
}
 
int irc_setident (const Bot *botptr, const char* ident)
{
	if (!irc_send_setident) {
		unsupported_cmd ("SETNAME");
		return NS_FAILURE;
	}
	irc_send_setident (botptr->name, ident);
	strlcpy (botptr->u->user->username, (char*)ident, MAXHOST);
	return NS_SUCCESS;
}


int
irc_chanmode (const Bot *botptr, const char *chan, const char *mode, const char *args)
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

int
irc_chanusermode (const Bot *botptr, const char *chan, const char *mode, const char *target)
{
	if((ircd_srv.protocol & PROTOCOL_B64NICK)) {
		irc_send_cmode (me.name, botptr->u->name, chan, mode, nicktobase64 (target), me.now);
	} else {
		irc_send_cmode (me.name, botptr->u->name, chan, mode, target, me.now);
	}
	ChanUserMode (chan, target, 1, CmodeStringToMask(mode));
	return NS_SUCCESS;
}

int
irc_quit (const Bot * botptr, const char *quitmsg)
{
	irc_send_quit (botptr->u->name, quitmsg);
	do_quit (botptr->u->name, quitmsg);
	return NS_SUCCESS;
}

int
irc_kill (const Bot *botptr, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	irc_send_kill (botptr->u->name, target, ircd_buf);
	do_quit (target, ircd_buf);
	return NS_SUCCESS;
}

int
irc_kick (const Bot *botptr, const char *chan, const char *target, const char *reason)
{
	if(irc_send_kick) {
		irc_send_kick (botptr->u->name, chan, target, reason);
		part_chan (find_user (target), (char *) chan, reason[0] != 0 ? (char *)reason : NULL);
	} else {
		unsupported_cmd("KICK");
	}
	return NS_SUCCESS;
}

int 
irc_invite (const Bot *botptr, const char *to, const char *chan) 
{
	if(irc_send_invite) {
		irc_send_invite(botptr->u->name, to, chan);
	} else {
		unsupported_cmd("KICK");
	}
	return NS_SUCCESS;
}

int 
irc_svstime (const Bot *botptr, Client *target, const time_t ts)
{
	if (irc_send_svstime) {
		irc_send_svstime(me.name, (unsigned long)ts);
		nlog (LOG_NOTICE, "irc_svstime: synching server times to %lu", ts);
	} else {
		unsupported_cmd("SVSTIME");
	}
	return NS_SUCCESS;
}

int
irc_svskill (const Bot *botptr, Client *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	if (irc_send_svskill) {
		irc_send_svskill (me.name, target->name, ircd_buf);
	} else if(irc_send_kill) {
		irc_send_kill (me.name, target->name, ircd_buf);
		do_quit (target->name, ircd_buf);
	} else {
		unsupported_cmd("SVSKILL");
	}
	return NS_SUCCESS;
}

int
irc_svsmode (const Bot *botptr, Client *target, const char *modes)
{
	if (irc_send_svsmode) {
		irc_send_svsmode(me.name, target->name, modes);
		UserMode (target->name, modes);
	} else {
		unsupported_cmd("SVSMODE");
	}
	return NS_SUCCESS;
}

int
irc_svshost (const Bot *botptr, Client *target, const char *vhost)
{
	if (irc_send_svshost) {
		strlcpy (target->user->vhost, vhost, MAXHOST);
		irc_send_svshost(me.name, target->name, vhost);
	} else {
		unsupported_cmd("SVSHOST");
	}
	return NS_SUCCESS;
}

int
irc_svsjoin (const Bot *botptr, Client *target, const char *chan)
{
	if (irc_send_svsjoin) {
		irc_send_svsjoin (me.name, target->name, chan);
	} else {
		unsupported_cmd("SVSJOIN");
	}
	return NS_SUCCESS;
}

int
irc_svspart (const Bot *botptr, Client *target, const char *chan)
{
	if (irc_send_svspart) {
		irc_send_svspart (me.name, target->name, chan);
	} else {
		unsupported_cmd("SVSPART");
	}
	return NS_SUCCESS;
}

int
irc_swhois (const char *target, const char *swhois)
{
	if (irc_send_swhois) {
		irc_send_swhois (me.name, target, swhois);
	} else {
		unsupported_cmd("SWHOIS");
	}
	return NS_SUCCESS;
}

int
irc_svsnick (const Bot *botptr, Client *target, const char *newnick)
{
	if (irc_send_svsnick) {
		irc_send_svsnick (me.name, target->name, newnick, me.now);
	} else {
		unsupported_cmd("SVSNICK");
	}
	return NS_SUCCESS;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	if (irc_send_smo) {
		irc_send_smo (from, umodetarget, msg);
	} else {
		unsupported_cmd("SMO");
	}
	return NS_SUCCESS;
}

int
irc_akill (const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	if(irc_send_akill) {
		irc_send_akill(me.name, host, ident, botptr->name, length, ircd_buf, me.now);
	} else {
		unsupported_cmd("AKILL");
	}
	return NS_SUCCESS;
}

int
irc_rakill (const Bot *botptr, const char *host, const char *ident)
{
	if(irc_send_rakill) {
		irc_send_rakill (me.name, host, ident);
	} else {
		unsupported_cmd("RAKILL");
	}
	return NS_SUCCESS;
}

int
irc_ping (const char *from, const char *reply, const char *to)
{
	if(irc_send_ping) {
		irc_send_ping (from, reply, to);
	} else {
		unsupported_cmd("PING");
	}
	return NS_SUCCESS;
}

int
irc_pong (const char *reply)
{
	if(irc_send_pong) {
		irc_send_pong (reply);
	} else {
		unsupported_cmd("PONG");
	}
	return NS_SUCCESS;
}

int
irc_server (const char *name, const int numeric, const char *infoline)
{
	if(irc_send_server) {
		irc_send_server (me.name, name, numeric, infoline);
	} else {
		unsupported_cmd("SERVER");
	}
	return NS_SUCCESS;
}

int
irc_squit (const char *server, const char *quitmsg)
{
	if(irc_send_squit) {
		irc_send_squit (server, quitmsg);
	} else {
		unsupported_cmd("SQUIT");
	}
	return NS_SUCCESS;
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
		mask = 0;
		while (CmodePrefixToMask (*nicklist)) {
			mask |= CmodePrefixToMask (*nicklist);
			nicklist ++;
		}
		strlcpy (nick, nicklist, MAXNICK);
		join_chan (nick, channame); 
		ChanUserMode (channame, nick, 1, mask);
		paramidx++;
		ok = 1;
	}
	c = find_chan (channame);
	if(c) {
		/* update the TS time */
		SetChanTS (c, atoi (tstime)); 
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
	init_services_bot ();
	irc_globops (NULL, _("Link with Network \2Complete!\2"));
	SendAllModuleEvent (EVENT_NETINFO, NULL);	
}

void 
do_snetinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, CLOAKKEYLEN);
	strlcpy (me.netname, netname, MAXPASS);
	irc_send_snetinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	init_services_bot ();
	irc_globops (NULL, _("Link with Network \2Complete!\2"));
	SendAllModuleEvent (EVENT_NETINFO, NULL);
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
	part_chan (find_user (nick), chan, reason);
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
	if(smodes) {
		UserSMode (nick, smodes);
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
	if(irc_send_vctrl) {
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
		if(ircstrcasecmp(modes, "+d") == 0) {
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
			irc_send_burst (0);
			ircd_srv.burst = 0;
			init_services_bot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

void 
do_swhois (char *who, char *swhois)
{
	Client * u;
	u = find_user(who);
	if(u) {
		strlcpy(u->user->swhois, swhois, MAXHOST);
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
	Client *s;

	s = find_server (name);
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

	u = find_user(nick);
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

	u = find_user(nick);
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

	u = find_user(nick);
	if (u) {
		dlog (DEBUG1, "do_setident: setting ident of user %s to %s", nick, ident);
		strlcpy(u->user->username, (char*)ident, MAXHOST);
	} else {
		nlog (LOG_WARNING, "do_setident: user %s not found", nick);
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

	dlog(DEBUGTX, "%s", buf);
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
	Client *s;

	s = find_server(name);
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
	Client *s;

	dlog(DEBUG1, "servertobase64: scanning for %s", name);
	s = find_server(name);
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
	Client *s;

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
	Client *u;

	u = find_user(nick);
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
	Client *u;

	dlog(DEBUG1, "nicktobase64: scanning for %s", nick);
	u = find_user(nick);
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
	Client *u;

	dlog(DEBUG1, "base64tonick: scanning for %s", num);
	u = finduserbase64(num);
	if(u) {
		return u->name;
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
	Client *s;

	s = find_server(origin);
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
	Client *s;

	s = find_server(origin);
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

		s->server->uptime = secs;
	}
}

int HaveFeature (int mask)
{
	return (ircd_srv.features&mask);
}

