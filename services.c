/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: services.c,v 1.48 2002/12/30 12:09:38 fishwaldo Exp $
*/
 
#include "stats.h"
#include "dl.h"



extern const char version_date[], version_time[];
extern const char protocol_version[];


static void ns_reload(User *u, char *reason);
static void ns_logs(User *);
static void ns_jupe(User *, char *);
void ns_debug_to_coders(char *);
#ifdef USE_RAW
static void ns_raw(User *, char *);
#endif
static void ns_user_dump(User *, char *);
static void ns_server_dump(User *);
static void ns_chan_dump(User *, char *);
static void ns_uptime(User *);
static void ns_version(User *); 

void servicesbot(char *nick, char **av, int ac) {
	User *u;
	int rval;
	char *tmp;
			
	u = finduser(nick);
	if (!u) {
		log("Unable to finduser %s (%s)", nick,s_Services);
		return;
	}

	me.requests++;

	if (me.onlyopers && (UserLevel(u) < 40)) {
		prefmsg(u->nick, s_Services,
			"This service is only available to IRCops.");
		chanalert (s_Services,"%s Requested %s, but he is Not a Operator!", u->nick, av[1]);
		return;
	}
	if (!strcasecmp(av[1], "HELP")) {
		if (ac > 2) {
			chanalert(s_Services,"%s Requested %s Help on %s",u->nick, s_Services, av[2]);
		} else {
			chanalert(s_Services, "%s Requested %s Help",u->nick, s_Services);
		}
		if (ac < 3) {
			privmsg_list(nick, s_Services, ns_help);
			if (UserLevel(u) >= 180)
				privmsg_list(nick, s_Services, ns_myuser_help);
		} else if (!strcasecmp(av[2], "VERSION"))
		 	privmsg_list(nick, s_Services, ns_version_help);
		else if (!strcasecmp(av[2], "SHUTDOWN") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_shutdown_help);
		else if (!strcasecmp(av[2], "RELOAD") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_reload_help);
		else if (!strcasecmp(av[2], "LOGS") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_logs_help);
		else if (!strcasecmp(av[2], "LOAD") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_load_help);
		else if (!strcasecmp(av[2], "UNLOAD") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_unload_help);
		else if (!strcasecmp(av[2], "MODLIST") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_modlist_help);
		else if (!strcasecmp(av[2], "USERDUMP") && (UserLevel(u) >= 180) && (me.coder_debug))
			privmsg_list(nick, s_Services, ns_userdump_help);
		else if (!strcasecmp(av[2], "CHANDUMP") && (UserLevel(u) >= 180) && (me.coder_debug))
			privmsg_list(nick, s_Services, ns_chandump_help);
		else if (!strcasecmp(av[2], "SERVERDUMP") && (UserLevel(u) >= 180) && (me.coder_debug))
			privmsg_list(nick, s_Services, ns_serverdump_help);
		else if (!strcasecmp(av[2], "JUPE") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_jupe_help);
#ifdef USE_RAW
		else if (!strcasecmp(av[2], "RAW") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_raw_help);
#endif
		else if (!strcasecmp(av[2], "LEVEL"))
			privmsg_list(nick, s_Services, ns_level_help);
		else
		prefmsg(nick, s_Services, "Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp(av[1], "LEVEL")) {
		prefmsg(nick, s_Services, "Your Level is %d", UserLevel(u));
	} else if (!strcasecmp(av[1], "LOAD")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to LOAD, but is not at least a NetAdmin",nick);
			return;
		}
		if (ac <= 2) {
			prefmsg(nick,s_Services,"Please Specify a Module");
			return;
		}
		rval = load_module(av[2],u);
		if (!rval) {
			chanalert(s_Services,"%s Loaded Module %s",u->nick,av[2]);

		} else {
			chanalert(s_Services,"%s Tried to Load Module %s, but Failed",u->nick,av[2]);
		}
	} else if (!strcasecmp(av[1],"MODLIST")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to MODLIST, but is not a Techadmin",nick);
			return;
		}
		list_module(u);
	} else if (!strcasecmp(av[1],"UNLOAD")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to UNLOAD, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			prefmsg(nick,s_Services," Please Specify a Module Name");
			return;
		}
		rval = unload_module(av[2],u);
		if (rval) { 
			chanalert(s_Services,"%s Unloaded Module %s", u->nick, av[2]);
		} else {
			chanalert(s_Services,"%s Tried to Unload the Module %s, but that does not exist", u->nick, av[2]);
			prefmsg(s_Services, u->nick, "Module %s does not exist. Try modlist", av[2]);
		}
	} else if (!strcasecmp(av[1], "MODBOTLIST")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to MODBOTLIST, but is not a Techadmin",nick);
			return;
		}
		list_module_bots(u);
	} else if (!strcasecmp(av[1], "MODSOCKLIST")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to MODSOCKLIST, but is not a Techadmin",nick);
			return;
		}
		list_sockets(u);
	} else if (!strcasecmp(av[1], "MODTIMERLIST")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to MODTIMERLIST, but is not a Techadmin",nick);
			return;
		}
		list_module_timer(u);
	} else if (!strcasecmp(av[1], "MODBOTCHANLIST")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick, s_Services, "Permission Denied");
			chanalert(s_Services, "%s tried to MODBOTCHANLIST, but is not a techadmin", nick);
			return;
		}
		botchandump(u);
	} else if (!strcasecmp(av[1], "INFO")) {
		ns_uptime(u);
		chanalert(s_Services,"%s Wanted to see %s's info",u->nick,me.name);
	} else if (!strcasecmp(av[1], "SHUTDOWN")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to SHUTDOWN, but is not a Techadmin",nick);
			return;
				}
		chanalert(s_Services,"%s Wants me to Go to BED!!! Good Night!",u->nick);
		tmp = joinbuf(av, ac, 2);
		ns_shutdown(u,tmp);
		free(tmp);
	} else if (!strcasecmp(av[1], "VERSION")) {
		ns_version(u);
		chanalert(s_Services,"%s Wanted to know our version number ",u->nick);
	} else if (!strcasecmp(av[1], "RELOAD")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to RELOAD, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			prefmsg(nick, s_Services, "You must supply a Reason to Reload");
			return;
		}
		tmp = joinbuf(av, ac, 2);
		chanalert(s_Services,"%s Wants me to RELOAD! for %s",u->nick, tmp);
		ns_reload(u,tmp);
		free(tmp);
	} else if (!strcasecmp(av[1], "LOGS")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to view LOGS, but is not a Techadmin",nick);
			return;
		}
		ns_logs(u);
		chanalert(s_Services,"%s Wants to Look at my Logs!!",u->nick);
	} else if (!strcasecmp(av[1], "JUPE")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to JUPE, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			prefmsg(nick, s_Services, "You must supply a ServerName to Jupe");
			return;
		}
		ns_jupe(u, av[2]);
		chanalert(s_Services,"%s Wants to JUPE this Server %s",u->nick,av[2]);
	} else if (!strcasecmp(av[1], "DEBUG")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(u->nick, s_Services, "Permission Denied, you need to be a Techadmin to Enable Debug Mode!");
			return;
		}		
		ns_debug_to_coders(u->nick);
	} else if (!strcasecmp(av[1], "USERDUMP")) {
		if (!me.coder_debug) {
			prefmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (ac <= 2) {
			ns_user_dump(u, NULL);
		} else {
			ns_user_dump(u, av[2]);
		}
	} else if (!strcasecmp(av[1], "CHANDUMP")) {
		if (!me.coder_debug) {
			prefmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (ac < 3) {
			ns_chan_dump(u, NULL);
		} else {
			ns_chan_dump(u, av[2]);
		}
	} else if (!strcasecmp(av[1], "SERVERDUMP")) {
		if (!me.coder_debug) {
			prefmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_server_dump(u);
	} else if (!strcasecmp(av[1], "RAW")) {
		if (!(UserLevel(u) >= 180)) {
			prefmsg(nick,s_Services,"Permission Denied");
			chanalert(s_Services,"%s Tried to use RAW, but is not a Techadmin",nick);
			return;
		}
#ifdef USE_RAW
		tmp = joinbuf(av, ac, 2);
		ns_raw(u, tmp);
		free(tmp);
		return;
#else
		prefmsg(nick, s_Services, "Raw is disabled");
		return;
#endif	
	} else {
		prefmsg(nick, s_Services, "Unknown Command: \2%s\2", av[1]);
		chanalert(s_Services,"%s Reqested %s, but that is a Unknown Command",u->nick,av[1]);
	}
}


extern void ns_shutdown(User *u, char *reason)
{
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;
	char quitmsg[255];

	strcpy(segv_location, "ns_shutdown");
	/* Unload the Modules */
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		mod_ptr = hnode_get(mn);
		chanalert(s_Services,"Module %s Unloaded by %s",mod_ptr->info->module_name,u->nick);
		unload_module(mod_ptr->info->module_name, u);
	}


	globops(s_Services, "%s requested \2SHUTDOWN\2 for %s", u->nick, reason);
	sprintf(quitmsg, "%s Set SHUTDOWN: %s", u->nick, (reason ? reason : "No Reason"));
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(1);
	close(servsock);
	remove("neostats.pid");
	log("%s [%s](%s) requested SHUTDOWN.", u->nick, u->username,
		u->hostname);
	exit(0);
}

static void ns_reload(User *u, char *reason)
{
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;
	char quitmsg[255];
	strcpy(segv_location, "ns_reload");
	globops(s_Services, "%s requested \2RELOAD\2 for %s", u->nick, reason);
	log("%s requested RELOAD. -> reason", u->nick);
	sprintf(quitmsg, "%s Sent RELOAD: %s", u->nick, reason);
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		mod_ptr = hnode_get(mn);
		chanalert(s_Services,"Module %s Unloaded by %s",mod_ptr->info->module_name,u->nick);
		unload_module(mod_ptr->info->module_name, u);
	}
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(5);
	execve("./neostats", NULL, NULL);
}


static void ns_logs(User *u)
{
#ifdef DEBUG
	prefmsg(u->nick, s_Services, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[512];

	strcpy(segv_location, "ns_logs");

	fp = fopen("logs/stats.log", "r");
	if (!fp) {
		prefmsg(u->nick, s_Services, "Unable to open stats.log");
		return;
	}
	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf)] = '\0';
		prefmsg(u->nick, s_Services, "%s", buf);
	}
	fclose(fp);
#endif
}

static void ns_jupe(User *u, char *server)
{
	char infoline[255];
	strcpy(segv_location, "ns_jupe");
	sprintf(infoline, "[Jupitered by %s]", u->nick);
	sserver_cmd(server, 1, infoline);
	log("%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, server);
}

void ns_debug_to_coders(char *u)
{
	strcpy(segv_location, "ns_debug_to_coders");
	if (!me.coder_debug) {
		me.coder_debug = 1;
		if (u) { 
			globops(me.name, "\2DEBUG MODE\2 Activated by %s",u);
			prefmsg(u, s_Services, "Debuging Mode Enabled!");
		} else {
			globops(me.name, "\2DEBUG MODE\3 Active");
		}
        } else {
        	me.coder_debug = 0;
        	if (!u) {
        		globops(me.name, "\2DEBUG MODE\2 Deactivated by %s",u);
        		prefmsg(u, s_Services, "Debuging Mode Disabled");
        	} else {
        		globops(me.name, "\2DEBUG MODE\2 Deactivated");
        	}
	}
}		
#ifdef USE_RAW									  
static void ns_raw(User *u, char *message)
{
	int sent;
	strcpy(segv_location, "ns_raw");
	chanalert(s_Services,"\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!(%s)",u->nick, message);
#ifdef DEBUG
        log("SENT: %s", message);
#endif
	strcat (message, "\n");
        sent = write (servsock, message, strlen (message));
        if (sent == -1) {
        	log("Write error.");
                exit(0);
        }
        me.SendM++;
        me.SendBytes = me.SendBytes + sent;
}	
#endif		
static void ns_user_dump(User *u, char *nick)
{
	strcpy(segv_location, "ns_user_dump");
	if (!(UserLevel(u) >= 180)) {
		prefmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	chanalert(s_Services,"\2DEBUG\2 \2%s\2 Requested a UserDump!",u->nick);
	UserDump(nick);
}
static void ns_server_dump(User *u)
{
	strcpy(segv_location, "ns_server_dump");
	if (!(UserLevel(u) >= 180)) {
		prefmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	chanalert(s_Services,"\2DEBUG\2 \2%s\2 Requested a ServerDump!",u->nick);
	ServerDump();
}
static void ns_chan_dump(User *u, char *chan)
{
	strcpy(segv_location, "ns_chan_dump");
	if (!(UserLevel(u) >= 180)) {
	
		prefmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	chanalert(s_Services,"\2DEBUG\2 \2%s\2 Requested a ChannelDump!",u->nick);
	chandump(chan);
}
static void ns_uptime(User *u)
{
	int uptime = time (NULL) - me.t_start;
	strcpy(segv_location, "ns_uptime");

	log("Time Difference %d", uptime);
	log("Statistical Server Up %d days, %d:%02d:%02d", uptime/86400, (uptime/3600) % 24, (uptime/60) % 60, uptime % 60);

	prefmsg(u->nick, s_Services, "Statistics Information:");
	if (uptime > 86400) {
	prefmsg(u->nick, s_Services, "Statistics up \2%ld\2 day%s, \2%02ld:%02ld\2", uptime/86400, (uptime/86400 == 1) ? "" : "s", (uptime/3600) % 24, (uptime/60) % 60);
	} else if (uptime > 3600) {
	prefmsg(u->nick, s_Services, "Statistics up \2%ld hour%s, %ld minute%s\2",	 uptime/3600, uptime/3600==1 ? "" : "s", (uptime/60) % 60, (uptime/60)%60 == 1 ? "" : "s");
	} else if (uptime > 60) {
	prefmsg(u->nick, s_Services, "Statistics up \2%ld minute%s, %ld second%s\2", uptime/60, uptime/60 == 1 ? "" : "s", uptime%60, uptime%60 == 1 ? "" : "s");
	} else { 
	prefmsg(u->nick, s_Services, "Statistics up \2%ld second%s\2", uptime, uptime == 1 ? "" : "s");
	 }
	prefmsg(u->nick, s_Services, "Sent %ld Messages Totaling %ld Bytes", me.SendM, me.SendBytes);
	prefmsg(u->nick, s_Services, "Recieved %ld Messages, Totaling %ld Bytes", me.RcveM, me.RcveBytes);
	prefmsg(u->nick, s_Services, "Reconnect Time: %d", me.r_time);
	prefmsg(u->nick, s_Services, "Statistic Requests: %d", me.requests);
	prefmsg(u->nick, s_Services, "Max Sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	prefmsg(u->nick, s_Services, "Use SMO for Debug?: %s", (me.usesmo) ? "Enabled" : "Disabled"); 
	if (me.coder_debug)
		prefmsg(u->nick, s_Services, "Debugging Mode is \2ON!\2");
	else 
		prefmsg(u->nick, s_Services, "Debugging Mode is Disabled!");
	prefmsg(u->nick, s_Services, "End of Information.");
}
static void ns_version(User *u)
{
	strcpy(segv_location, "ns_version");
		prefmsg(u->nick, s_Services, "\2NeoStats Version Information\2");
		prefmsg(u->nick, s_Services, "NeoStats Version: %s", version);
		prefmsg(u->nick, s_Services, "http://www.neostats.net");
}

