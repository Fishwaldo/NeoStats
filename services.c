/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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

#include "stats.h"
#include "dl.h"
#include "log.h"


static void ns_reload (User * u, char *reason);
static void ns_logs (User * u);
static void ns_jupe (User * u, char * server);
void ns_debug_to_coders (char * u);
#ifdef USE_RAW
static void ns_raw (User * u, char * message);
#endif
static void ns_user_dump (User * u, char * nick);
static void ns_server_dump (User * u);
static void ns_chan_dump (User * u, char * channel);
static void ns_uptime (User * u);
static void ns_version (User * u);

void
servicesbot (char *nick, char **av, int ac)
{
	User *u;
	int rval;
	char *tmp;

	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", nick, s_Services);
		return;
	}
	SET_SEGV_LOCATION();
	me.requests++;

	if (me.onlyopers && (UserLevel (u) < 40)) {
		prefmsg (u->nick, s_Services, "This service is only available to IRCops.");
		chanalert (s_Services, "%s Requested %s, but he is Not an Operator!", u->nick, av[1]);
		return;
	}
	if (!strcasecmp (av[1], "HELP")) {
		if (ac > 2) {
			chanalert (s_Services, "%s Requested %s Help on %s", u->nick, s_Services, av[2]);
		} else {
			chanalert (s_Services, "%s Requested %s Help", u->nick, s_Services);
		}
		if (ac < 3) {
			privmsg_list (nick, s_Services, ns_help);
			if (UserLevel (u) >= 180)
				privmsg_list (nick, s_Services, ns_myuser_help);
			privmsg_list (nick, s_Services, ns_help_on_help);			
		} else if (!strcasecmp (av[2], "VERSION"))
			privmsg_list (nick, s_Services, ns_version_help);
		else if (!strcasecmp (av[2], "SHUTDOWN")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_shutdown_help);
		else if (!strcasecmp (av[2], "RELOAD")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_reload_help);
		else if (!strcasecmp (av[2], "LOGS")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_logs_help);
		else if (!strcasecmp (av[2], "LOAD")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_load_help);
		else if (!strcasecmp (av[2], "UNLOAD")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_unload_help);
		else if (!strcasecmp (av[2], "MODLIST")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_modlist_help);
		else if (!strcasecmp (av[2], "USERDUMP")
			 && (UserLevel (u) >= 180) && (me.coder_debug))
			privmsg_list (nick, s_Services, ns_userdump_help);
		else if (!strcasecmp (av[2], "CHANDUMP")
			 && (UserLevel (u) >= 180) && (me.coder_debug))
			privmsg_list (nick, s_Services, ns_chandump_help);
		else if (!strcasecmp (av[2], "SERVERDUMP")
			 && (UserLevel (u) >= 180) && (me.coder_debug))
			privmsg_list (nick, s_Services, ns_serverdump_help);
		else if (!strcasecmp (av[2], "JUPE")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_jupe_help);
#ifdef USE_RAW
		else if (!strcasecmp (av[2], "RAW")
			 && (UserLevel (u) >= 180))
			privmsg_list (nick, s_Services, ns_raw_help);
#endif
		else if (!strcasecmp (av[2], "LEVEL"))
			privmsg_list (nick, s_Services, ns_level_help);
		else if (!strcasecmp (av[2], "DEBUG"))
			privmsg_list (nick, s_Services, ns_debug_help);
		else if (!strcasecmp (av[2], "MODBOTLIST"))
			privmsg_list (nick, s_Services, ns_modbotlist_help);
		else if (!strcasecmp (av[2], "MODSOCKLIST"))
			privmsg_list (nick, s_Services, ns_modsocklist_help);
		else if (!strcasecmp (av[2], "MODTIMERLIST"))
			privmsg_list (nick, s_Services, ns_modtimerlist_help);
		else if (!strcasecmp (av[2], "MODBOTCHANLIST"))
			privmsg_list (nick, s_Services, ns_modbotchanlist_help);
		else if (!strcasecmp (av[2], "INFO"))
			privmsg_list (nick, s_Services, ns_info_help);
		else
			prefmsg (nick, s_Services, "Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp (av[1], "LEVEL")) {
		prefmsg (nick, s_Services, "Your Level is %d", UserLevel (u));
	} else if (!strcasecmp (av[1], "LOAD")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to LOAD, but is not at least a NetAdmin", nick);
			return;
		}
		if (ac <= 2) {
			prefmsg (nick, s_Services, "Please Specify a Module");
			return;
		}
		rval = load_module (av[2], u);
		if (rval > -1) {
			chanalert (s_Services, "%s Loaded Module %s", u->nick, av[2]);
		} else {
			chanalert (s_Services, "%s Tried to Load Module %s, but Failed", u->nick, av[2]);
		}
	} else if (!strcasecmp (av[1], "MODLIST")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to MODLIST, but is not authorised", nick);
			return;
		}
		list_module (u);
	} else if (!strcasecmp (av[1], "UNLOAD")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to UNLOAD, but is not authorised", nick);
			return;
		}
		if (ac <= 2) {
			prefmsg (nick, s_Services, " Please Specify a Module Name");
			return;
		}
		rval = unload_module (av[2], u);
		if (rval > 0) {
			chanalert (s_Services, "%s Unloaded Module %s", u->nick, av[2]);
		}
		return;
	} else if (!strcasecmp (av[1], "MODBOTLIST")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to MODBOTLIST, but is not authorised", nick);
			return;
		}
		list_module_bots (u);
	} else if (!strcasecmp (av[1], "MODSOCKLIST")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to MODSOCKLIST, but is not authorised", nick);
			return;
		}
		list_sockets (u);
	} else if (!strcasecmp (av[1], "MODTIMERLIST")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to MODTIMERLIST, but is not authorised", nick);
			return;
		}
		list_module_timer (u);
	} else if (!strcasecmp (av[1], "MODBOTCHANLIST")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s tried to MODBOTCHANLIST, but is not authorised", nick);
			return;
		}
		botchandump (u);
	} else if (!strcasecmp (av[1], "INFO")) {
		ns_uptime (u);
		chanalert (s_Services, "%s Wanted to see %s's info", u->nick, me.name);
	} else if (!strcasecmp (av[1], "SHUTDOWN")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to SHUTDOWN, but is not authorised", nick);
			return;
		}
		if (ac <= 2) {
			chanalert (s_Services, "%s Requested SHUTDOWN", u->nick);
			ns_shutdown (u, NULL);
		} else {
			tmp = joinbuf (av, ac, 2);
			chanalert (s_Services, "%s Requested SHUTDOWN for: ", u->nick, tmp);
			ns_shutdown (u, tmp);
			free (tmp);
		}
	} else if (!strcasecmp (av[1], "VERSION")) {
		ns_version (u);
		chanalert (s_Services, "%s Wanted to know our version number ", u->nick);
	} else if (!strcasecmp (av[1], "RELOAD")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to RELOAD, but is not authorised", nick);
			return;
		}
		if (ac <= 2) {
			prefmsg (nick, s_Services, "You must supply a Reason to Reload");
			return;
		}
		tmp = joinbuf (av, ac, 2);
		chanalert (s_Services, "%s Wants me to RELOAD! for %s", u->nick, tmp);
		ns_reload (u, tmp);
		free (tmp);
	} else if (!strcasecmp (av[1], "LOGS")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to view LOGS, but is not authorised", nick);
			return;
		}
		ns_logs (u);
		chanalert (s_Services, "%s Wants to Look at my Logs!!", u->nick);
	} else if (!strcasecmp (av[1], "JUPE")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to JUPE, but is not authorised", nick);
			return;
		}
		if (ac <= 2) {
			prefmsg (nick, s_Services, "You must supply a ServerName to Jupe");
			return;
		}
		ns_jupe (u, av[2]);
		chanalert (s_Services, "%s Wants to JUPE this Server %s", u->nick, av[2]);
	} else if (!strcasecmp (av[1], "DEBUG")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (u->nick, s_Services, "Permission Denied, you need to be authorised to Enable Debug Mode!");
			return;
		}
		ns_debug_to_coders (u->nick);
	} else if (!strcasecmp (av[1], "USERDUMP")) {
		if (!me.coder_debug) {
			prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (ac <= 2) {
			ns_user_dump (u, NULL);
		} else {
			ns_user_dump (u, av[2]);
		}
	} else if (!strcasecmp (av[1], "CHANDUMP")) {
		if (!me.coder_debug) {
			prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (ac < 3) {
			ns_chan_dump (u, NULL);
		} else {
			ns_chan_dump (u, av[2]);
		}
	} else if (!strcasecmp (av[1], "SERVERDUMP")) {
		if (!me.coder_debug) {
			prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_server_dump (u);
	} else if (!strcasecmp (av[1], "RAW")) {
		if (!(UserLevel (u) >= 180)) {
			prefmsg (nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s Tried to use RAW, but is not authorised", nick);
			return;
		}
#ifdef USE_RAW
		tmp = joinbuf (av, ac, 2);
		ns_raw (u, tmp);
		free (tmp);
		return;
#else
		prefmsg (nick, s_Services, "Raw is disabled");
		return;
#endif
	} else {
		prefmsg (nick, s_Services, "Unknown Command: \2%s\2", av[1]);
		chanalert (s_Services, "%s Reqested %s, but that is an Unknown Command", u->nick, av[1]);
	}
}


void
ns_shutdown (User * u, char *reason)
{
	char quitmsg[255];
	char *no_reason="no reason given";

	SET_SEGV_LOCATION();
	globops (s_Services, "%s requested \2SHUTDOWN\2 for %s", u->nick, (reason ? reason : no_reason));
	snprintf (quitmsg, 255, "%s requested SHUTDOWN: %s", u->nick, (reason ? reason : no_reason));
	nlog (LOG_NOTICE, LOG_CORE, "%s [%s](%s) requested SHUTDOWN for %s.", u->nick, u->username, u->hostname,(reason ? reason : no_reason));
	unload_modules(u);
	squit_cmd (s_Services, quitmsg);
	ssquit_cmd (me.name);
	sleep (1);
	close (servsock);
	do_exit (0);
}

static void
ns_reload (User * u, char *reason)
{
	char quitmsg[255];

	SET_SEGV_LOCATION();
	globops (s_Services, "%s requested \2RELOAD\2 for %s", u->nick, reason);
	snprintf (quitmsg, 255, "%s requested RELOAD: %s", u->nick, reason);
	nlog (LOG_NOTICE, LOG_CORE, "%s requested RELOAD. -> reason", u->nick);
	unload_modules(u);
	squit_cmd (s_Services, quitmsg);
	ssquit_cmd (me.name);
	sleep (5);
	close (servsock);
	do_exit (2);
}

static void
ns_logs (User * u)
{
#ifdef DEBUG
	prefmsg (u->nick, s_Services, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[512];

	SET_SEGV_LOCATION();
	fp = fopen ("logs/NeoStats.log", "r");
	if (!fp) {
		prefmsg (u->nick, s_Services, "Unable to open neostats.log");
		return;
	}
	while (fgets (buf, sizeof (buf), fp)) {
		buf[strlen (buf)] = '\0';
		prefmsg (u->nick, s_Services, "%s", buf);
	}
	fclose (fp);
#endif
}

static void
ns_jupe (User * u, char *server)
{
	char infoline[255];
	SET_SEGV_LOCATION();
	snprintf (infoline, 255, "[Jupitered by %s]", u->nick);
	sserver_cmd (server, 1, infoline);
	nlog (LOG_NOTICE, LOG_CORE, "%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, server);
}

void
ns_debug_to_coders (char *u)
{
	SET_SEGV_LOCATION();
	if (!me.coder_debug) {
		me.coder_debug = 1;
		if (u) {
			globops (me.name, "\2DEBUG MODE\2 Activated by %s", u);
			prefmsg (u, s_Services, "Debuging Mode Enabled!");
		} else {
			globops (me.name, "\2DEBUG MODE\3 Active");
		}
	} else {
		me.coder_debug = 0;
		if (!u) {
			globops (me.name, "\2DEBUG MODE\2 Deactivated by %s", u);
			prefmsg (u, s_Services, "Debuging Mode Disabled");
		} else {
			globops (me.name, "\2DEBUG MODE\2 Deactivated");
		}
	}
}

#ifdef USE_RAW
static void
ns_raw (User * u, char *message)
{
	int sent;
	SET_SEGV_LOCATION();
	chanalert (s_Services, "\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!(%s)", u->nick, message);
	nlog (LOG_INFO, LOG_CORE, "RAW COMMAND %sIssued a Raw Command!(%s)", u->nick, message);
	strcat (message, "\n");
	sent = write (servsock, message, strlen (message));
	if (sent == -1) {
		nlog (LOG_CRITICAL, LOG_CORE, "Write error.");
		do_exit (0);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
}
#endif
static void
ns_user_dump (User * u, char *nick)
{
	SET_SEGV_LOCATION();
	if (!(UserLevel (u) >= 180)) {
		prefmsg (u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a UserDump!", u->nick);
	UserDump (nick);
}
static void
ns_server_dump (User * u)
{
	SET_SEGV_LOCATION();
	if (!(UserLevel (u) >= 180)) {
		prefmsg (u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a ServerDump!", u->nick);
	ServerDump ();
}
static void
ns_chan_dump (User * u, char *chan)
{
	SET_SEGV_LOCATION();
	if (!(UserLevel (u) >= 180)) {

		prefmsg (u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a ChannelDump!", u->nick);
	chandump (chan);
}
static void
ns_uptime (User * u)
{
	int uptime = time (NULL) - me.t_start;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "%s Information:", s_Services);
	if (uptime > 86400) {
		prefmsg (u->nick, s_Services, "%s up \2%ld\2 day%s, \2%02ld:%02ld\2", s_Services, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		prefmsg (u->nick, s_Services, "%s up \2%ld hour%s, %ld minute%s\2", s_Services, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		prefmsg (u->nick, s_Services, "%s up \2%ld minute%s, %ld second%s\2", s_Services, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		prefmsg (u->nick, s_Services, "%s up \2%ld second%s\2", s_Services, uptime, uptime == 1 ? "" : "s");
	}
	prefmsg (u->nick, s_Services, "Sent %ld Messages Totaling %ld Bytes", me.SendM, me.SendBytes);
	prefmsg (u->nick, s_Services, "Received %ld Messages, Totaling %ld Bytes", me.RcveM, me.RcveBytes);
	prefmsg (u->nick, s_Services, "Reconnect Time: %d", me.r_time);
	prefmsg (u->nick, s_Services, "Statistic Requests: %d", me.requests);
	prefmsg (u->nick, s_Services, "Max Sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	if (me.coder_debug)
		prefmsg (u->nick, s_Services, "Debugging Mode is \2ON!\2");
	else
		prefmsg (u->nick, s_Services, "Debugging Mode is Disabled!");
	prefmsg (u->nick, s_Services, "End of Information.");
}
static void
ns_version (User * u)
{
	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "\2NeoStats Version Information\2");
	prefmsg (u->nick, s_Services, "NeoStats Version: %d.%d.%d%s", MAJOR, MINOR, REV, version);
	prefmsg (u->nick, s_Services, "http://www.neostats.net");
}
