/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      services.c, 
** Version: 2.0
** Date:    5/1/2002
*/
 
#include "stats.h"
#include "dl.h"


extern const char version_date[], version_time[];
extern const char protocol_version[];


static void ns_reload(User *u, char *reason);
static void ns_logs(User *);
static void ns_jupe(User *, char *);
static void ns_JOIN(User *, char *);
void ns_debug_to_coders(char *);
static void ns_raw(User *, char *);
static void ns_user_dump(User *);
static void ns_server_dump(User *);
static void ns_chan_dump(User *, char *);
static void ns_uptime(User *);
static void ns_version(User *); 
static void ns_roots(User *);

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

	if (flood(u))
		return;

	if (me.onlyopers && (UserLevel(u) < 40)) {
		privmsg(u->nick, s_Services,
			"This service is only available to IRCops.");
		notice (s_Services,"%s Requested %s, but he is Not a Operator!", u->nick, av[1]);
		return;
	}
	if (!strcasecmp(av[1], "HELP")) {
		if (ac > 2) {
			notice(s_Services,"%s Requested %s Help on %s",u->nick, s_Services, av[2]);
		} else {
			notice(s_Services, "%s Requested %s Help",u->nick, s_Services);
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
		else if (!strcasecmp(av[2], "JOIN") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_join_help);
		else if (!strcasecmp(av[2], "USERDUMP") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_userdump_help);
		else if (!strcasecmp(av[2], "CHANDUMP") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_chandump_help);
		else if (!strcasecmp(av[2], "SERVERDUMP") && (UserLevel(u) >= 180))
			privmsg_list(nick, s_Services, ns_serverdump_help);
		else
		privmsg(nick, s_Services, "Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp(av[1], "LOAD")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to LOAD, but is not at least a NetAdmin",nick);
			return;
		}
		if (ac <= 2) {
			privmsg(nick,s_Services,"Please Specify a Module");
			return;
		}
		rval = load_module(av[2],u);
		if (!rval) {
			notice(s_Services,"%s Loaded Module %s",u->nick,av[2]);

		} else {
			notice(s_Services,"%s Tried to Load Module %s, but Failed",u->nick,av[2]);
		}
	} else if (!strcasecmp(av[1],"MODLIST")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODLIST, but is not a Techadmin",nick);
			return;
		}
		list_module(u);
	} else if (!strcasecmp(av[1],"UNLOAD")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to UNLOAD, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			privmsg(nick,s_Services," Please Specify a Module Name");
			return;
		}
		rval = unload_module(av[2],u);
		if (rval) { 
			notice(s_Services,"%s Unloaded Module %s", u->nick, av[2]);
		} else {
			notice(s_Services,"%s Tried to Unload the Module %s, but that does not exist", u->nick, av[2]);
			privmsg(s_Services, u->nick, "Module %s does not exist. Try modlist", av[2]);
		}
	} else if (!strcasecmp(av[1], "MODBOTLIST")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODBOTLIST, but is not a Techadmin",nick);
			return;
		}
		list_module_bots(u);
	} else if (!strcasecmp(av[1], "MODSOCKLIST")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODSOCKLIST, but is not a Techadmin",nick);
			return;
		}
		list_sockets(u);
	} else if (!strcasecmp(av[1], "MODTIMERLIST")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODTIMERLIST, but is not a Techadmin",nick);
			return;
		}
		list_module_timer(u);
	} else if (!strcasecmp(av[1], "INFO")) {
		ns_uptime(u);
		notice(s_Services,"%s Wanted to see %s's info",u->nick,me.name);
	} else if (!strcasecmp(av[1], "SHUTDOWN")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to SHUTDOWN, but is not a Techadmin",nick);
			return;
				}
		notice(s_Services,"%s Wants me to Go to BED!!! Good Night!",u->nick);
		tmp = joinbuf(av, ac, 2);
		ns_shutdown(u,tmp);
		free(tmp);
	} else if (!strcasecmp(av[1], "VERSION")) {
		ns_version(u);
		notice(s_Services,"%s Wanted to know our version number ",u->nick);
	} else if (!strcasecmp(av[1], "ROOTS")) {
	 	ns_roots(u);
		notice(s_Services,"%s wanted to see our ROOTS",u->nick);
	} else if (!strcasecmp(av[1], "RELOAD")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to RELOAD, but is not a Techadmin",nick);
			return;
		}
		notice(s_Services,"%s Wants me to RELOAD! Be back in a few!",u->nick);
		tmp = joinbuf(av, ac, 2);
		ns_reload(u,tmp);
		free(tmp);
	} else if (!strcasecmp(av[1], "LOGS")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to view LOGS, but is not a Techadmin",nick);
			return;
		}
		ns_logs(u);
		notice(s_Services,"%s Wants to Look at my Logs!!",u->nick);
	} else if (!strcasecmp(av[1], "JUPE")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to JUPE, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			privmsg(nick, s_Services, "You must supply a ServerName to Jupe");
			return;
		}
		ns_jupe(u, av[2]);
		notice(s_Services,"%s Wants to JUPE this Server %s",u->nick,av[2]);
	} else if (!strcasecmp(av[1], "JOIN")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to use JOIN, but is not a Techadmin",nick);
			return;
		}
		if (ac <= 2) {
			privmsg(nick,s_Services, "You must supply a ChannelName to Join");
			return;
		}
		ns_JOIN(u, av[2]);
	} else if (!strcasecmp(av[1], "DEBUG")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(u->nick, s_Services, "Permission Denied, you need to be a Techadmin to Enable Debug Mode!");
			return;
		}		
		ns_debug_to_coders(u->nick);
	} else if (!strcasecmp(av[1], "USERDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_user_dump(u);
	} else if (!strcasecmp(av[1], "CHANDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (ac < 3) {
			ns_chan_dump(u, NULL);
		} else {
			ns_chan_dump(u, av[2]);
		}
	} else if (!strcasecmp(av[1], "SERVERDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_server_dump(u);
	} else if (!strcasecmp(av[1], "RAW")) {
		if (!(UserLevel(u) >= 180)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to use RAW, but is not a Techadmin",nick);
			return;
		}
		tmp = joinbuf(av, ac, 2);
		ns_raw(u, tmp);
		free(tmp);
	} else {
		privmsg(nick, s_Services, "Unknown Command: \2%s\2", av[1]);
		notice(s_Services,"%s Reqested %s, but that is a Unknown Command",u->nick,av[1]);
	}
}


extern void ns_shutdown(User *u, char *reason)
{
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;
	char quitmsg[255];

	segv_location = sstrdup("ns_shutdown");
	/* Unload the Modules */
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		mod_ptr = hnode_get(mn);
		notice(s_Services,"Module %s Unloaded by %s",mod_ptr->info->module_name,u->nick);
		unload_module(mod_ptr->info->module_name, u);
	}


	globops(s_Services, "%s requested \2SHUTDOWN\2 for %s", u->nick, reason);
	sprintf(quitmsg, "%s Set SHUTDOWN: %s", u->nick, (reason ? reason : "No Reason"));
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(1);
	close(servsock);
	remove("stats.pid");
	log("%s [%s](%s) requested SHUTDOWN.", u->nick, u->username,
		u->hostname);
	exit(0);
}

static void ns_reload(User *u, char *reason)
{
	char quitmsg[255];
	segv_location = sstrdup("ns_reload");
	globops(s_Services, "%s requested \2RELOAD\2 for %s", u->nick, reason);
	log("%s requested RELOAD.", u->nick);
	sprintf(quitmsg, "%s Sent RELOAD: %s", u->nick, (reason ? reason : "No reason"));
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(1);
	close(servsock);
	init_server_hash(); 
	init_user_hash();  
	me.onchan = 0;
	log("Connecting to %s:%d", me.uplink, me.port);
	if (servsock > 0)
		close(servsock);
		
	servsock = ConnectTo(me.uplink, me.port);
		
	if (servsock <= 0) {
		log("Unable to connect to %s", me.uplink);
	} else {
		login();
		read_loop();
	}
	log("Reconnecting to the server in %d seconds", me.r_time);
	sleep(me.r_time);

}


static void ns_logs(User *u)
{
#ifdef DEBUG
	privmsg(u->nick, s_Services, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[512];

	segv_location = sstrdup("ns_logs");

	fp = fopen("logs/stats.log", "r");
	if (!fp) {
		privmsg(u->nick, s_Services, "Unable to open stats.log");
		return;
	}
	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf)] = '\0';
		privmsg(u->nick, s_Services, "%s", buf);
	}
	fclose(fp);
#endif
}

static void ns_jupe(User *u, char *server)
{
	char infoline[255];
	segv_location = sstrdup("ns_jupe");
	sprintf(infoline, "[Jupitered by %s]", u->nick);
	sserver_cmd(server, 1, infoline);
	log("%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, server);
}

static void ns_JOIN(User *u, char *chan)
{
	segv_location = sstrdup("ns_JOIN");
	globops(s_Services, "JOINING CHANNEL -\2(%s)\2- Thanks to %s!%s@%s)", chan, u->nick, u->username, u->hostname);
	privmsg(me.chan, s_Services, "%s Asked me to Join %s, So, I'm Leaving %s", u->nick, chan, me.chan);
	spart_cmd(s_Services, me.chan);
	log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
	sprintf(me.chan,"%s",chan);
	sjoin_cmd(s_Services, chan);
	schmode_cmd(me.name, chan, "+o", s_Services);
}
void ns_debug_to_coders(char *u)
{
	char realname[63];
	segv_location = sstrdup("ns_debug_to_coders");
	if (!me.coder_debug) {
		me.coder_debug = 1;
		if (!u) { 
			globops(me.name, "\2DEBUG MODE\2 Activated by %s",u);
			privmsg(u, s_Services, "Debuging Mode Enabled!");
		} else {
			globops(me.name, "\2DEBUG MODE\3 Active");
		}
		if (me.usesmo) {
			sprintf(realname, "/msg %s \2HELP\2", s_Services);
			snewnick_cmd(s_Debug, Servbot.user, Servbot.host, realname);
     		}
        } else {
        	me.coder_debug = 0;
        	if (!u) {
        		globops(me.name, "\2DEBUG MODE\2 Deactivated by %s",u);
        		privmsg(u, s_Services, "Debuging Mode Disabled");
        	} else {
        		globops(me.name, "\2DEBUG MODE\2 Deactivated");
        	}
		if (me.usesmo) {
			sprintf("Debug Mode Deactivated by %s", u);
			squit_cmd(s_Debug, realname);
		}
	}
}										  
static void ns_raw(User *u, char *message)
{
	int sent;
	segv_location = sstrdup("ns_raw");
	notice(s_Services,"\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!(%s)",u->nick, message);
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
static void ns_user_dump(User *u)
{
	segv_location = sstrdup("ns_user_dump");
	if (!(UserLevel(u) >= 180)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a UserDump!",u->nick);
	UserDump();
}
static void ns_server_dump(User *u)
{
	segv_location = sstrdup("ns_server_dump");
	if (!(UserLevel(u) >= 180)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ServerDump!",u->nick);
	ServerDump();
}
static void ns_chan_dump(User *u, char *chan)
{
	segv_location = sstrdup("ns_chan_dump");
	if (!(UserLevel(u) >= 180)) {
	
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ChannelDump!",u->nick);
	chandump(u, chan);
}
static void ns_uptime(User *u)
{
	int uptime = time (NULL) - me.t_start;
	segv_location = sstrdup("ns_uptime");

	log("Time Difference %d", uptime);
	log("Statistical Server Up %d days, %d:%02d:%02d", uptime/86400, (uptime/3600) % 24, (uptime/60) % 60, uptime % 60);

	privmsg(u->nick, s_Services, "Statistics Information:");
	if (uptime > 86400) {
	privmsg(u->nick, s_Services, "Statistics up \2%ld\2 day%s, \2%02ld:%02ld\2", uptime/86400, (uptime/86400 == 1) ? "" : "s", (uptime/3600) % 24, (uptime/60) % 60);
	} else if (uptime > 3600) {
	privmsg(u->nick, s_Services, "Statistics up \2%ld hour%s, %ld minute%s\2",	 uptime/3600, uptime/3600==1 ? "" : "s", (uptime/60) % 60, (uptime/60)%60 == 1 ? "" : "s");
	} else if (uptime > 60) {
	privmsg(u->nick, s_Services, "Statistics up \2%ld minute%s, %ld second%s\2", uptime/60, uptime/60 == 1 ? "" : "s", uptime%60, uptime%60 == 1 ? "" : "s");
	} else { 
	privmsg(u->nick, s_Services, "Statistics up \2%ld second%s\2", uptime, uptime == 1 ? "" : "s");
	 }
	privmsg(u->nick, s_Services, "Sent %ld Messages Totaling %ld Bytes", me.SendM, me.SendBytes);
	privmsg(u->nick, s_Services, "Recieved %ld Messages, Totaling %ld Bytes", me.RcveM, me.RcveBytes);
	privmsg(u->nick, s_Services, "Reconnect Time: %d", me.r_time);
	privmsg(u->nick, s_Services, "Statistic Requests: %d", me.requests);
	privmsg(u->nick, s_Services, "Use SMO for Debug?: %s", (me.usesmo) ? "Enabled" : "Disabled"); 
	if (me.coder_debug)
		privmsg(u->nick, s_Services, "Debugging Mode is \2ON!\2");
	else 
		privmsg(u->nick, s_Services, "Debugging Mode is Disabled!");
	privmsg(u->nick, s_Services, "End of Information.");
}
static void ns_version(User *u)
{
	segv_location = sstrdup("ns_version");
		privmsg(u->nick, s_Services, "\2NeoStats Version Information\2");
		privmsg(u->nick, s_Services, "NeoStats Version: %s", version);
		privmsg(u->nick, s_Services, "http://www.neostats.net");
	privmsg(u->nick, s_Services, "Services roots: %s", me.roots);
}

static void ns_roots(User *u)
{
	segv_location = sstrdup("ns_roots");
		privmsg(u->nick, s_Services, "\2NeoStats ROOT users\2");
		privmsg(u->nick, s_Services, "%s",me.roots);
	privmsg(u->nick, s_Services, "These are setable in stats.cfg now");
}

void init_services() {
	if (usr_mds);
}
