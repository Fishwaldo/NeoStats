/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: services.c,v 1.12 2000/12/10 06:25:51 fishwaldo Exp $
*/
 
#include "stats.h"
#include "dl.h"
#include "Unreal.h"


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
static void ns_chan_dump(User *);
static void ns_uptime(User *);
static void ns_version(User *); 



void servicesbot(char *nick, char *line) {
	char *cmd ;
	User *u;
	char *reason;
	int rval;
	cmd = strtok(line, " ");
	if (cmd == NULL)
		cmd = strtok(line, "");
	u = finduser(nick);
	if (!u) {
		log("Unable to finduser %s (%s)", nick,s_Services);
		return;
	}

	me.requests++;

	if (flood(u))
		return;

	log("%s received message from %s: %s", s_Services, nick, cmd);
	if (me.onlyopers && (UserLevel(u) < 40)) {
		privmsg(u->nick, s_Services,
			"This service is only available to IRCops.");
		notice (s_Services,"%s Requested %s, but he is Not a Operator!", u->nick, cmd);
		return;
	}

	if (!strcasecmp(cmd, "HELP")) {
		cmd = strtok(NULL, " ");
		if (cmd) {
			notice(s_Services,"%s is a Dummy and wanted help on %s",u->nick, cmd);
		} else {
			notice(s_Services, "%s is a Dummy and wanted Help... I refered him to God!", u->nick);
		}
		if (!cmd) {
			privmsg_list(nick, s_Services, ns_help);
			if (u->Umode & (UserLevel(u) >= 190))
				privmsg_list(nick, s_Services, ns_myuser_help);
			if (u->Umode & (UserLevel(u) >= 200))
				privmsg_list(nick, s_Services, ns_coder_help);
                } else if (!strcasecmp(cmd, "VERSION"))
                        privmsg_list(nick, s_Services, ns_version_help);
		else if (!strcasecmp(cmd, "SHUTDOWN") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_shutdown_help);
                else if (!strcasecmp(cmd, "RELOAD") && (UserLevel(u) >= 190))
                        privmsg_list(nick, s_Services, ns_reload_help);
		else if (!strcasecmp(cmd, "LOGS") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_logs_help);
		else if (!strcasecmp(cmd, "LOAD") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_load_help);
		else if (!strcasecmp(cmd, "UNLOAD") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_unload_help);
		else if (!strcasecmp(cmd, "MODLIST") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_modlist_help);
		else if (!strcasecmp(cmd, "JOIN") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_join_help);
		else if (!strcasecmp(cmd, "USERDUMP") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_userdump_help);
		else if (!strcasecmp(cmd, "CHANDUMP") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_chandump_help);
		else if (!strcasecmp(cmd, "SERVERDUMP") && (UserLevel(u) >= 190))
			privmsg_list(nick, s_Services, ns_serverdump_help);
		else
			privmsg(nick, s_Services, "Unknown Help Topic: \2%s\2", cmd);
	} else if (!strcasecmp(cmd, "LOAD")) {
		if (!(UserLevel(u) >= 190)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to UNLOAD, but is not a Techadmin",nick);
			return;
		}
		cmd = strtok(NULL, " ");
		if (cmd == NULL) {
			privmsg(nick,s_Services,"Please Specify a Module");
			return;
		}
		rval = load_module(cmd,u);
		if (!rval) {
			notice(s_Services,"%s Loaded Module %s",u->nick,cmd);

		} else {
			notice(s_Services,"%s Tried to Load Module %s, but Failed",u->nick,cmd);
		}
	} else if (!strcasecmp(cmd,"MODLIST")) {
		if (!(UserLevel(u) >= 190)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODLIST, but is not a Techadmin",nick);
			return;
		}
		list_module(u);
	} else if (!strcasecmp(cmd,"UNLOAD")) {
		if (!(UserLevel(u) >= 190)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to UNLOAD, but is not a Techadmin",nick);
			return;
		}
		cmd = strtok(NULL, " ");
		if (cmd == NULL) {
			privmsg(nick,s_Services," Please Specify a Module Name");
			return;
		}
		rval = unload_module(cmd,u);
		if (!rval) { 
			notice(s_Services,"%s Unloaded Module %s", u->nick, cmd);
		} else {
			notice(s_Services,"%s Tried to Unload the Module %s, but that does not exist", u->nick, cmd);
		}
	} else if (!strcasecmp(cmd, "MODBOTLIST")) {
		if (!(UserLevel(u) >= 200)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODBOTLIST, but is not a Coder",nick);
			return;
		}
		list_module_bots(u);
	} else if (!strcasecmp(cmd, "MODSOCKLIST")) {
		if (!(UserLevel(u) >= 200)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODSOCKLIST, but is not a Coder",nick);
			return;
		}
		list_sockets(u);
	} else if (!strcasecmp(cmd, "MODTIMERLIST")) {
		if (!(UserLevel(u) >= 200)) {
			privmsg(nick,s_Services,"Permission Denied");
			notice(s_Services,"%s Tried to MODTIMERLIST, but is not a Coder",nick);
			return;
		}
		list_module_timer(u);
	} else if (!strcasecmp(cmd, "UPTIME")) {
		ns_uptime(u);
		notice(s_Services,"%s Wanted to see %s's Uptime ",u->nick,me.name);
	} else if (!strcasecmp(cmd, "SHUTDOWN")) {
                reason=strtok(NULL,"");
		notice(s_Services,"%s Wants me to Go to BED!!! Good Night!",u->nick);
		ns_shutdown(u,reason);
        } else if (!strcasecmp(cmd, "VERSION")) {
                ns_version(u);
                notice(s_Services,"%s Wanted to know our version number ",u->nick);
        } else if (!strcasecmp(cmd, "RELOAD")) {
                reason=strtok(NULL,"");
                notice(s_Services,"%s Wants me to RELOAD! Be back in a few!",u->nick);
                ns_reload(u,reason);
	} else if (!strcasecmp(cmd, "LOGS")) {
		ns_logs(u);
		notice(s_Services,"%s Wants to Look at my Logs!!",u->nick);
	} else if (!strcasecmp(cmd, "JUPE")) {
		cmd = strtok(NULL, " ");
		ns_jupe(u, cmd);
		notice(s_Services,"%s Wants to JUPE this Server %s",u->nick,cmd);
	} else if (!strcasecmp(cmd, "JOIN")) {
		cmd = strtok(NULL, " ");
		ns_JOIN(u, cmd);
	} else if (!strcasecmp(cmd, "DEBUG")) {
		if (!(UserLevel(u) >= 200)) {
			privmsg(u->nick, s_Services, "Permission Denied, you need to be a Coder to Enable Debug Mode!");
			return;
		}		
		ns_debug_to_coders(u->nick);
	} else if (!strcasecmp(cmd, "USERDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_user_dump(u);
	} else if (!strcasecmp(cmd, "CHANDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_chan_dump(u);
	} else if (!strcasecmp(cmd, "SERVERDUMP")) {
		if (!me.coder_debug) {
			privmsg(u->nick,s_Services,"\2Error:\2 Debug Mode Disabled");
			return;
		}
		ns_server_dump(u);
	} else if (!strcasecmp(cmd, "RAW")) {
		cmd = strtok(NULL, "");
		ns_raw(u,cmd);

	} else {
		privmsg(nick, s_Services, "Unknown Command: \2%s\2", cmd);
		notice(s_Services,"%s Reqested %s, but that is a Unknown Command",u->nick,cmd);
	}
}
char *uptime(time_t when)
{
	char *buf = NULL;
	time_t u = time(NULL) - when;
#ifdef DEBUG
	log("time %d",u);
#endif
	if (u > 86400) {
		sprintf(buf, "Statistics up \2%ld\2 day%s, \2%02ld:%02ld\2",
			u/86400, (u/86400 == 1) ? "" : "s",
			(u/3600) % 24, (u/60) % 60);
	} else if (u > 3600) {
		sprintf(buf, "Statistics up \2%ld hour%s, %ld minute%s\2",
			u/3600, u/3600==1 ? "" : "s",
			(u/60) % 60, (u/60)%60 == 1 ? "" : "s");
	} else if (u > 60) {
		sprintf(buf, "Statistics up \2%ld minute%s, %ld second%s\2",
			u/60, u/60 == 1 ? "" : "s",
			u%60, u%60 == 1 ? "" : "s");
	} else {
		sprintf(buf, "Statistics up \2%ld second%s\2",
			u, u == 1 ? "" : "s");
	}
	return buf;
}

extern void ns_shutdown(User *u, char *reason) {
	Module *mod_ptr = NULL;
	segv_loc("ns_shutdown");
	if (strcasecmp(u->nick, s_Services)) {
		if (!(UserLevel(u) >= 190)) {
			log("Access Denied (SHUTDOWN) to %s", u->nick);
			privmsg(u->nick, s_Services, "Access Denied.");
			return;
		}
	}
	/* Unload the Modules */
	mod_ptr = module_list->next;
	while (mod_ptr != NULL) {
		notice(s_Services, "Module %s Unloaded by %s", mod_ptr->info->module_name, u->nick);
		unload_module(mod_ptr->info->module_name, u);
		mod_ptr = module_list->next;
	}
	if (mod_ptr) free(mod_ptr);
	notice(s_Services, "\2UNLOAD\2 All Modules Unloaded");
        globops(s_Services, "%s requested \2SHUTDOWN\2 for %s", u->nick, reason);
	sleep(1);
	sts(":%s QUIT :%s Sent SHUTDOWN: %s",s_Services,u->nick,reason);
	sts("SQUIT %s",me.name);
	sleep(1);
	close(servsock);
	log("%s [%s](%s) requested SHUTDOWN.", u->nick, u->username,
		u->hostname);
	cleanmem();
	exit(0);
}

static void ns_reload(User *u, char *reason)
{
	segv_loc("ns_reload");
        if (!(UserLevel(u) >= 190)) {
                log("Access Denied (RELOAD) to %s", u->nick);
                privmsg(u->nick, s_Services, "Access Denied.");
                return;
        }
        if (reason != NULL) {
      	  	globops(s_Services, "%s requested \2RELOAD\2 for %s", u->nick, reason);
	        log("%s requested RELOAD.", u->nick);

		sts("SQUIT %s :Reload", me.name);

		sts(":%s QUIT :%s Sent RELOAD: %s",s_Services,u->nick,reason);
	} else {
        	globops(s_Services, "%s requested \2RELOAD\2 for no Reason!", u->nick, reason);
        	log("%s requested RELOAD.", u->nick);

		sts("SQUIT %s :Reload", me.name);

		sts(":%s QUIT :%s Sent RELOAD",s_Services,u->nick,reason);
	}
	sleep(1);
        close(servsock);
        init_server_hash(); 
        init_user_hash();  
/*        init_tld();
*/
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

	segv_loc("ns_logs");
	if (!(UserLevel(u) >= 190)) {
		log("Access Denied (LOGS) to %s", u->nick);
		privmsg(u->nick, s_Services, "Access Denied.");
		return;
	}

	fp = fopen("stats.log", "r");
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
	segv_loc("ns_jupe");
	if (!(UserLevel(u) >= 190)) {
		privmsg(u->nick, s_Services, "Access Denied.");
		return;
	}
	if (!server) {
		privmsg(u->nick, s_Services, "Syntax: /msg %s JUPE <server>",
		s_Services);
		return;
	}
	sts(":%s SERVER %s 1 :[Jupitered] by %s", me.name, server, u->nick);
	log("%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, server);
}

static void ns_JOIN(User *u, char *chan)
{
	segv_loc("ns_JOIN");
	if (!(UserLevel(u) >= 190)) {
		log("Access Denied (JOIN) to %s", u->nick);
		privmsg(u->nick, s_Services, "Access Denied.");
		notice(s_Services,"%s Requested JOIN, but is not a god!",u->nick);
		return;
	}
	if (!chan) {
		privmsg(u->nick, s_Services, "Syntax: /msg %s JOIN <chan>",s_Services);
		return;
	}
	globops(s_Services, "JOINING CHANNEL -\2(%s)\2- Thanks to %s!%s@%s)", chan, u->nick, u->username, u->hostname);
	privmsg(me.chan, s_Services, "%s Asked me to Join %s, So, I'm Leaving %s", u->nick, chan, me.chan);
	sts(":%s part %s", s_Services, me.chan);
	log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
	sprintf(me.chan,"%s",chan);
	sts(":%s JOIN %s",s_Services,chan);
	sts(":%s MODE %s +o %s",me.name,chan,s_Services);
}
void ns_debug_to_coders(char *u)
{
	segv_loc("ns_debug_to_coders");
	if (!me.coder_debug) {
		me.coder_debug = 1;
		globops(me.name, "\2DEBUG MODE\2 Activated by %s",u);
		privmsg(u, s_Services, "Debuging Mode Enabled!");
		if (me.usesmo) {
			sts("NICK %s 1 1 %s %s %s 0 :/msg %s \2HELP\2", s_Debug, Servbot.user, Servbot.host, me.name, s_Services);
        		AddUser(s_Debug, Servbot.user, Servbot.host, me.name);
     		}
        } else {
        	me.coder_debug = 0;
        	globops(me.name, "\2DEBUG MODE\2 Deactivated by %s",u);
        	privmsg(u, s_Services, "Debuging Mode Disabled");
		if (me.usesmo) {
			sts(":%s QUIT :Debug Mode Deactivated by %s",s_Debug,u);
			DelUser(s_Debug);
		}
	}
}                                          
static void ns_raw(User *u, char *message)
{
	segv_loc("ns_raw");
	if (!(UserLevel(u) >= 190)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be a TechAdmin to do that!");
		return;
	}
	notice(s_Services,"\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!",u->nick);
	sts("%s",message);
}	
static void ns_user_dump(User *u)
{
	segv_loc("ns_user_dump");
	if (!(UserLevel(u) >= 200)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be a Coder to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a UserDump!",u->nick);
	UserDump();
}
static void ns_server_dump(User *u)
{
	segv_loc("ns_server_dump");
	if (!(UserLevel(u) >= 200)) {
		privmsg(u->nick, s_Services, "Permission Denied, you need to be a Coder to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ServerDump!",u->nick);
	ServerDump();
}
static void ns_chan_dump(User *u)
{
	segv_loc("ns_chan_dump");
	if (!(UserLevel(u) >= 200)) {
	
		privmsg(u->nick, s_Services, "Permission Denied, you need to be a Coder to Enable Debug Mode!");
		return;
	}		
	notice(s_Services,"\2DEBUG\2 \2%s\2 Requested a ChannelDump!",u->nick);
	ChanDump();
}
static void ns_uptime(User *u)
{
	segv_loc("ns_uptime");

	log("time %d", me.t_start);
	privmsg(u->nick, s_Services, "Statistics Information:");
	/* Broken atm */
	privmsg(u->nick, s_Services, "Sent %ld Messages Totaling %ld Bytes", me.SendM, me.SendBytes);
	privmsg(u->nick, s_Services, "Recieved %ld Messages, Totaling %ld Bytes", me.RcveM, me.RcveBytes);
/*	privmsg(u->nick, s_Services, "%s", uptime(me.t_start)); */
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
	segv_loc("ns_version");
        privmsg(u->nick, s_Services, "\2StatServ Version Information\2");
        privmsg(u->nick, s_Services, "%s - %s", me.name, version,
                                me.name, version_date, version_time);
        privmsg(u->nick, s_Services, "http://www.neostats.net");
}
