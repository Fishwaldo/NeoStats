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
void ns_debug_to_coders(char *);
static void ns_raw(User *, char *);
static void ns_user_dump(User *, char *);
static void ns_server_dump(User *);
static void ns_chan_dump(User *, char *);
static void ns_uptime(User *);
static void ns_version(User *); 

void servicesbot(EvntMsg *EM) {
	int rval;
	char *tmp;
		
		
	printf("%d\n", EM->isserv);	
	/* we don't process messages from servers */	
	if ((EM->isserv == 1) || (EM->isserv == -1))
		return;
			
	printf("nick: %s\n", EM->u->nick);

	me.requests++;

	if (flood(EM->u))
		return;

	if (me.onlyopers && (UserLevel(EM->u) < 40)) {
		privmsg(EM->u->nick, s_Services,
			"This service is only available to IRCops.");
		notice (s_Services,"%s Requested %s, but he is Not a Operator!", EM->u->nick, EM->av[1]);
		return;
	}
	if (!strcasecmp(EM->av[1], "HELP")) {
		if (EM->ac > 2) {
			notice(s_Services,"%s Requested %s Help on %s",EM->u->nick, s_Services, EM->av[2]);
		} else {
			notice(s_Services, "%s Requested %s Help",EM->u->nick, s_Services);
		}
		if (EM->ac < 3) {
			privmsg_list(EM->u->nick, s_Services, ns_help);
			if (UserLevel(EM->u) >= 180)
				privmsg_list(EM->u->nick, s_Services, ns_myuser_help);
		} else if (!strcasecmp(EM->av[2], "VERSION"))
		 	privmsg_list(EM->u->nick, s_Services, ns_version_help);
		else if (!strcasecmp(EM->av[2], "SHUTDOWN") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_shutdown_help);
		else if (!strcasecmp(EM->av[2], "RELOAD") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_reload_help);
		else if (!strcasecmp(EM->av[2], "LOGS") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_logs_help);
		else if (!strcasecmp(EM->av[2], "LOAD") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_load_help);
		else if (!strcasecmp(EM->av[2], "UNLOAD") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_unload_help);
		else if (!strcasecmp(EM->av[2], "MODLIST") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_modlist_help);
		else if (!strcasecmp(EM->av[2], "USERDUMP") && (UserLevel(EM->u) >= 180) && (me.coder_debug))
			privmsg_list(EM->u->nick, s_Services, ns_userdump_help);
		else if (!strcasecmp(EM->av[2], "CHANDUMP") && (UserLevel(EM->u) >= 180) && (me.coder_debug))
			privmsg_list(EM->u->nick, s_Services, ns_chandump_help);
		else if (!strcasecmp(EM->av[2], "SERVERDUMP") && (UserLevel(EM->u) >= 180) && (me.coder_debug))
			privmsg_list(EM->u->nick, s_Services, ns_serverdump_help);
		else if (!strcasecmp(EM->av[2], "JUPE") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_jupe_help);
		else if (!strcasecmp(EM->av[2], "RAW") && (UserLevel(EM->u) >= 180))
			privmsg_list(EM->u->nick, s_Services, ns_raw_help);
		else if (!strcasecmp(EM->av[2], "LEVEL"))
			privmsg_list(EM->u->nick, s_Services, ns_level_help);
		else
		privmsg(EM->u->nick, s_Services, "Unknown Help Topic: \2%s\2", EM->av[2]);
	} else if (!strcasecmp(EM->av[1], "LEVEL")) {
		privmsg(EM->u->nick, s_Services, "Your Level is %d", UserLevel(EM->u));
	} else if (!strcasecmp(EM->av[1], "LOAD")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to LOAD, but is not at least a NetAdmin",EM->u->nick);
			return;
		}
		if (EM->ac <= 2) {
			privmsg(EM->u->nick,s_Services,"Please Specify a Module");
			return;
		}
		rval = load_module(EM->av[2],EM->u);
		if (!rval) {
			notice(s_Services,"%s Loaded Module %s",EM->u->nick,EM->av[2]);

		} else {
			notice(s_Services,"%s Tried to Load Module %s, but Failed",EM->u->nick,EM->av[2]);
		}
	} else if (!strcasecmp(EM->av[1],"MODLIST")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODLIST, but is not a Techadmin",EM->u->nick);
			return;
		}
		list_module(EM->u);
	} else if (!strcasecmp(EM->av[1],"UNLOAD")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to UNLOAD, but is not a Techadmin",EM->u->nick);
			return;
		}
		if (EM->ac <= 2) {
			privmsg(EM->u->nick,s_Services," Please Specify a Module Name");
			return;
		}
		rval = unload_module(EM->av[2],EM->u);
		if (rval) { 
			notice(s_Services,"%s Unloaded Module %s", EM->u->nick, EM->av[2]);
		} else {
			notice(s_Services,"%s Tried to Unload the Module %s, but that does not exist", EM->u->nick, EM->av[2]);
			privmsg(s_Services, EM->u->nick, "Module %s does not exist. Try modlist", EM->av[2]);
		}
	} else if (!strcasecmp(EM->av[1], "MODBOTLIST")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODBOTLIST, but is not a Techadmin",EM->u->nick);
			return;
		}
		list_module_bots(EM->u);
	} else if (!strcasecmp(EM->av[1], "MODSOCKLIST")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODSOCKLIST, but is not a Techadmin",EM->u->nick);
			return;
		}
		list_sockets(EM->u);
	} else if (!strcasecmp(EM->av[1], "MODTIMERLIST")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODTIMERLIST, but is not a Techadmin",EM->u->nick);
			return;
		}
		list_module_timer(EM->u);
	} else if (!strcasecmp(EM->av[1], "INFO")) {
		ns_uptime(EM->u);
		notice(s_Services,"%s Wanted to see %s's info",EM->u->nick,me.name);
	} else if (!strcasecmp(EM->av[1], "SHUTDOWN")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to SHUTDOWN, but is not a Techadmin",EM->u->nick);
			return;
				}
		notice(s_Services,"%s Wants me to Go to BED!!! Good Night!",EM->u->nick);
		tmp = joinbuf(EM->av, EM->ac, 2);
		ns_shutdown(EM->u,tmp);
		free(tmp);
	} else if (!strcasecmp(EM->av[1], "VERSION")) {
		ns_version(EM->u);
		notice(s_Services,"%s Wanted to know our version number ",EM->u->nick);
	} else if (!strcasecmp(EM->av[1], "RELOAD")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to RELOAD, but is not a Techadmin",EM->u->nick);
			return;
		}
		notice(s_Services,"%s Wants me to RELOAD! Be back in a few!",EM->u->nick);
		tmp = joinbuf(EM->av, EM->ac, 2);
		ns_reload(EM->u,tmp);
		free(tmp);
	} else if (!strcasecmp(EM->av[1], "LOGS")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to view LOGS, but is not a Techadmin",EM->u->nick);
			return;
		}
		ns_logs(EM->u);
		notice(s_Services,"%s Wants to Look at my Logs!!",EM->u->nick);
	} else if (!strcasecmp(EM->av[1], "JUPE")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to JUPE, but is not a Techadmin",EM->u->nick);
			return;
		}
		if (EM->ac <= 2) {
			privmsg(EM->u->nick, s_Services, "You must supply a ServerName to Jupe");
			return;
		}
		ns_jupe(EM->u, EM->av[2]);
		notice(s_Services,"%s Wants to JUPE this Server %s",EM->u->nick,EM->av[2]);
	} else if (!strcasecmp(EM->av[1], "DEBUG")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick, s_Services, "Permission Denied, you need to be a Techadmin to Enable Debug Mode!");
			return;
		}		
		ns_debug_to_coders(EM->u->nick);
	} else if (!strcasecmp(EM->av[1], "USERDUMP")) {
		if (!me.coder_debug) {
			privmsg(EM->u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (EM->ac <= 2) {
			ns_user_dump(EM->u, NULL);
		} else {
			ns_user_dump(EM->u, EM->av[2]);
		}
	} else if (!strcasecmp(EM->av[1], "CHANDUMP")) {
		if (!me.coder_debug) {
			privmsg(EM->u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		if (EM->ac < 3) {
			ns_chan_dump(EM->u, NULL);
		} else {
			ns_chan_dump(EM->u, EM->av[2]);
		}
	} else if (!strcasecmp(EM->av[1], "SERVERDUMP")) {
		if (!me.coder_debug) {
			privmsg(EM->u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_server_dump(EM->u);
	} else if (!strcasecmp(EM->av[1], "RAW")) {
		if (!(UserLevel(EM->u) >= 180)) {
			privmsg(EM->u->nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to use RAW, but is not a Techadmin",EM->u->nick);
			return;
		}
		tmp = joinbuf(EM->av, EM->ac, 2);
		ns_raw(EM->u, tmp);
		free(tmp);
	} else {
		privmsg(EM->u->nick, s_Services, "Unknown Command: \2%s\2", EM->av[1]);
		notice(s_Services,"%s Reqested %s, but that is a Unknown Command",EM->u->nick,EM->av[1]);
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
		notice(s_Services,"Module %s Unloaded by %s",mod_ptr->info->module_name,u->nick);
		unload_module(mod_ptr->info->module_name, u);
	}


	globops(s_Services, "%s requested \2SHUTDOWN\2 for %s", u->nick, (reason ? "No Reason" : reason));
	sprintf(quitmsg, "%s Set SHUTDOWN: %s", u->nick, (reason ? "No Reason" : reason));
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(1);
	close(servsock);
	remove("stats.pid");
	log("%s [%s](%s) requested SHUTDOWN.", u->nick, u->username,
		u->hostname);
	shutdown_neo();
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
	log("%s requested RELOAD.", u->nick);
	sprintf(quitmsg, "%s Sent RELOAD: %s", u->nick, (reason ? reason : "No reason"));
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		mod_ptr = hnode_get(mn);
		notice(s_Services,"Module %s Unloaded by %s",mod_ptr->info->module_name,u->nick);
		unload_module(mod_ptr->info->module_name, u);
	}
	squit_cmd(s_Services, quitmsg);
	ssquit_cmd(me.name);
	sleep(5);
   	shutdown_neo();
	execve("./stats", NULL, NULL);
}


static void ns_logs(User *u)
{
#ifdef DEBUG
	privmsg(u->nick, s_Services, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[512];

	strcpy(segv_location, "ns_logs");

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
	strcpy(segv_location, "ns_jupe");
	sprintf(infoline, "[Jupitered by %s]", u->nick);
	sserver_cmd(server, 1, infoline);
	log("%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, server);
}

void ns_debug_to_coders(char *u)
{
	char realname[63];
	strcpy(segv_location, "ns_debug_to_coders");
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
#ifdef ULTIMATE3
			snewnick_cmd(s_Debug, Servbot.user, Servbot.host, realname, UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#else
			snewnick_cmd(s_Debug, Servbot.user, Servbot.host, realname);
#endif
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
	strcpy(segv_location, "ns_raw");
	notice(s_Services,"\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!(%s)",u->nick, message);
#ifdef DEBUG
        log("SENT: %s", message);
#endif
	strcat (message, "\n");
        sent = write (servsock, message, strlen (message));
        if (sent == -1) {
        	log("Write error.");
        	shutdown_neo();
                exit(0);
        }
        me.SendM++;
        me.SendBytes = me.SendBytes + sent;
}	
static void ns_user_dump(User *u, char *nick)
{
	strcpy(segv_location, "ns_user_dump");
	if (!(UserLevel(u) >= 180)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a UserDump!",u->nick);
	UserDump(nick);
}
static void ns_server_dump(User *u)
{
	strcpy(segv_location, "ns_server_dump");
	if (!(UserLevel(u) >= 180)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ServerDump!",u->nick);
	ServerDump();
}
static void ns_chan_dump(User *u, char *chan)
{
	strcpy(segv_location, "ns_chan_dump");
	if (!(UserLevel(u) >= 180)) {
	
		privmsg(u->nick, s_Services, "Permission Denied, you need to be at least a NetAdmin to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ChannelDump!",u->nick);
	chandump(chan);
}
static void ns_uptime(User *u)
{
	int uptime = time (NULL) - me.t_start;
	strcpy(segv_location, "ns_uptime");

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
	strcpy(segv_location, "ns_version");
		privmsg(u->nick, s_Services, "\2NeoStats Version Information\2");
		privmsg(u->nick, s_Services, "NeoStats Version: %s", version);
		privmsg(u->nick, s_Services, "http://www.neostats.net");
}

