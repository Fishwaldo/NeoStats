/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: statserv.c,v 1.15 2002/02/27 11:15:16 fishwaldo Exp $
*/

#include "statserv.h"
#include "ss_help.c"

SStats *Shead;
TLD *tldhead;


extern const char version_date[], version_time[];
static void ss_daily(User *u);
static void ss_reset(User *u);
static void ss_stats(User *u, char *cmd, char *arg, char *arg2);
static void ss_JOIN(User *u, char *chan);
static void ss_tld(User *u, char *tld);
static void ss_tld_map(User *u) ;
static void ss_operlist(User *origuser, char *flags, char *server);
static void ss_botlist(User *origuser);
static void ss_version(User *u);
static void ss_server(User *u, char *server);
static void ss_map(User *);
static void ss_netstats(User *);
static int Online(Server *);
static int pong(Server *);
static int s_new_server(Server *);
static int s_new_user(User *);
static int s_del_user(User *);
static int s_user_modes(User *);
static int s_user_kill(User *);
/* int s_bot_kill(char *); */
static void ss_cb_Config(char *, int);
static int new_m_version(char *av, char *tmp);
static void DelTLD(User *u);

char s_StatServ[MAXNICK] = "StatServ";

static int synced;

Module_Info Statserv_Info[] = { {
	"StatServ",
	"Statistical Bot For NeoStats",
	"2.01"
} };

Functions StatServ_fn_list[] = { 
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL, 	0}
};


EventFnList StatServ_Event_List[] = {
	{ "ONLINE", 	Online},
	{ "PONG", 	pong},
	{ "NEWSERVER",	s_new_server},
	{ "SIGNON", 	s_new_user},
	{ "UMODE", 	s_user_modes},
	{ "SIGNOFF", 	s_del_user},
/*	{ "BOTKILL", 	s_bot_kill}, */
	{ "KILL",	s_user_kill},
	{ NULL, 	NULL}
};

Module_Info *__module_get_info() {
	return Statserv_Info;
};

Functions *__module_get_functions() {
	return StatServ_fn_list;
};

EventFnList *__module_get_events() {
	return StatServ_Event_List;
};

static config_option options[] = {
{ "STATSERV_NICK", ARG_STR, ss_cb_Config, 0},
{ "STATSERV_USER", ARG_STR, ss_cb_Config, 1},
{ "STATSERV_HOST", ARG_STR, ss_cb_Config, 2},
{ "STATSERV_LAG", ARG_STR, ss_cb_Config, 3}
};


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module StatServ Loaded, Version: %s %s %s",me.name,av,Statserv_Info[0].module_version,version_date,version_time);
	return 0;
}

void _init() {
   Server *ss;
   User *u;
   int i;

	synced = 0;
	sts(":%s GLOBOPS :StatServ Module Loaded", me.name);
	
	LoadTLD();
	init_tld();
	LoadStats();
	   for (i=0; i < S_TABLE_SIZE; i++) {
   		for (ss = serverlist[i]; ss; ss = ss->next) {
   			/* do server table stuff */ 
			s_new_server(ss);
#ifdef DEBUG
			log("Added Server %s to StatServ List", ss->name);
#endif
		}
	   }

	   for (i=0; i < U_TABLE_SIZE; i++) {
   		for (u = userlist[i]; u; u = u->next) {
   			/* do User stuff, as yet, we have nuffin... :( */
   			/* Should also process Usermodes and fun stuff like that (maybe?) */
			s_new_user(u);
			s_user_modes(u);
#ifdef DEBUG
			log("Adduser user %s to StatServ List", u->nick);
#endif
		}
	}   	

}

void _fini() {
	SaveStats();
	sts(":%s GLOBOPS :StatServ Module Unloaded", me.name);
	
}
static int s_new_server(Server *s) {

	AddStats(s);
	IncreaseServers();
	if (stats_network.maxservers < stats_network.servers) {
		stats_network.maxservers = stats_network.servers;
		stats_network.t_maxservers = time(NULL);
		if (synced) sts(":%s WALLOPS :\2NEW SERVER RECORD\2 Wow, there are now %d Servers on the Network", s_StatServ, stats_network.servers); 
	}
	if (synced) notice(s_StatServ, "\2SERVER\2 %s has joined the Network at %s", s->name, s->uplink);
	return 1;

}
static int s_user_kill(User *u) {
	SStats *s;
	char *cmd, *who;
#ifdef DEBUG
	log(" Server %s", u->server->name);
#endif
	s=findstats(u->server->name);
	if (UserLevel(u) >= 40) {
		DecreaseOpers(s);
	}
	DecreaseUsers(s);
	DelTLD(u);
	cmd = sstrdup(recbuf);
	who = strtok(cmd, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, "");
	cmd++;
	who++;
	if (finduser(who)) {
	/* it was a User that killed the target */
		if (synced) notice(s_StatServ, "\2KILL\2 %s was Killed by %s --> %s", u->nick, who, cmd);
	} else if (findserver(who)) {
		if (synced) notice(s_StatServ, "\2SERVER KILL\2 %s was Killed by the Server %s --> %s", u->nick, who, cmd);
	}
	return 1;
}

static int s_user_modes(User *u) {
	int add = 0;
	char *modes;
	SStats *s;

	if (!u) {
		log("Changing modes for unknown user: %s", u->nick);
		return -1;
	}
 	if (!u->modes) return -1; 
	modes = u->modes;
	/* Don't bother if we are not synceded yet */
	while (*modes++) {
#ifdef DEBUG
	log("s_modes %c", *modes); 
#endif

		switch(*modes) {
			case '+': add = 1;	break;
			case '-': add = 0;	break;
			case 'N':
				if (add) {
					if (synced) notice(s_StatServ, "\2NetAdmin\2 %s is Now a Network Administrator (+N)", u->nick);
				} else {
					if (synced) notice(s_StatServ, "\2NetAdmin\2 %s is No Longer a Network Administrator (-N)", u->nick);
				}
				break;
			case 'S':
				if (add) {
					if (synced) notice(s_StatServ, "\2Services\2 %s is Now a Network Service (+S)", u->nick);
				} else {
					if (synced) notice(s_StatServ, "\2Services\2 %s is No Longer a Network Service (-S)", u->nick);
				}
				break;
/*			case 'q':
				if (add) {
					if (synced) globops(s_StatServ,"\2%s\2 Has been Marked As Protected (+q)",u->nick);
				} else {
					if (synced) globops(s_StatServ,"\2%s\2 Has been Un-Marked As Protected (-q)",u->nick);
				}
				break; */
			case 'T':
				if (add) {
					if (synced) notice(s_StatServ, "\2TechAdmin\2 %s is Now a Network Technical Administrator (+T)", u->nick);
				} else {
					if (synced) notice(s_StatServ, "\2TechAdmin\2 %s is No Longer a Network Technical Administrator (-T)", u->nick);
				}
				break;
			case 'A':
				if (add) {
					if (synced) notice(s_StatServ, "\2ServerAdmin\2 %s is Now a Server Administrator on %s (+A)", u->nick, u->server->name);
				} else {
					if (synced) notice(s_StatServ, "\2ServerAdmin\2 %s is No Longer a Server Administrator on %s (-A)", u->nick, u->server->name);
				}
				break;
			case 'a':
				if (add) {
					if (synced) notice(s_StatServ, "\2ServicesAdmin\2 %s is Now a Services Administrator (+a)", u->nick);
				} else {
					if (synced) notice(s_StatServ, "\2ServicesAdmin\2 %s is No Longer a Services Administrator (-a)", u->nick);
				}
				break;
			case 'C':
				if (add) {
					if (synced) notice(s_StatServ, "\2Co-ServerAdmin\2 %s is Now a Co-Server Administrator on %s (+C)", u->nick, u->server->name);
				} else {
					if (synced) notice(s_StatServ, "\2Co-ServerAdmin\2 %s is No Longer a Co-Server Administrator on %s (-C)", u->nick, u->server->name);
				}
				break;
			case 'B':
				if (add) {
					if (synced) notice(s_StatServ, "\2Bot\2 %s is Now a Bot (+B)", u->nick);
				} else {
					if (synced) notice(s_StatServ, "\2Bot\2 %s is No Longer a Bot (-B)", u->nick);
				}
				break;
			case 'I':
				if (add) {
					
                                	if (synced) globops(s_StatServ,"\2%s\2 Is Using \2Invisible Mode\2 (+I)",u->nick);
				} else {
					
					if (synced) globops(s_StatServ,"\2%s\2 Is no longer using \2Invisible Mode\2 (-I)",u->nick);
				}
				break;
			case 'o':
				if (add) {
					if (synced) notice(s_StatServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
					IncreaseOpers(findstats(u->server->name));
					s = findstats(u->server->name);
					if (stats_network.maxopers < stats_network.opers) {
						stats_network.maxopers = stats_network.opers;
						stats_network.t_maxopers = time(NULL);
						if (synced) sts(":%s WALLOPS :\2Oper Record\2 The Network has reached a New Record for Opers at %d", s_StatServ, stats_network.opers);
					}
					if (s->maxopers < s->opers) {
						s->maxopers = s->opers;
						s->t_maxopers = time(NULL);
						if (synced) sts(":%s WALLOPS :\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s_StatServ, s->name, s->opers);
					}						
				} else {
					if (synced) notice(s_StatServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
					DecreaseOpers(findstats(u->server->name));
#ifdef DEBUG
					log("Decrease Opers");
#endif
				}
				break;
			case 'O':
				if (add) {
					if (synced) notice(s_StatServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
					IncreaseOpers(findstats(u->server->name));
					s = findstats(u->server->name);
					if (stats_network.maxopers < stats_network.opers) {
						stats_network.maxopers = stats_network.opers;
						stats_network.t_maxopers = time(NULL);
						if (synced) sts(":%s WALLOPS :\2Oper Record\2 The Network has reached a New Record for Opers at %d", s_StatServ, stats_network.opers);
					}
					if (s->maxopers < s->opers) {
						s->maxopers = s->opers;
						s->t_maxopers = time(NULL);
						if (synced) sts(":%s WALLOPS :\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s_StatServ, s->name, s->opers);
					}						
				} else {
					if (synced) notice(s_StatServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
					DecreaseOpers(findstats(u->server->name));
#ifdef DEBUG
					log("Decrease Opers");
#endif
				}
				break;
			default: 
				break;
		}
	}
	return 1;
}
/* int s_bot_kill(char *nick) {
	User *u;
	SStats *s;
	char *modes;
	
	u=finduser(nick);
	log("Oh Oh, the StatServ Bot Got Killed! - Re-Initializing");
	s=findstats(u->server->name);
	if (UserLevel(u) >= 40) {
		DecreaseOpers(s);
	}
	DelTLD(u);	
	DecreaseUsers(s); */
	/* we have to remove it from our List */
/* Shmad */
/*        if (!u->modes) return -1;
        modes = u->modes; */
        /* Don't bother if we are not synceded yet */
/*        while (*modes++) {
#ifdef DEBUG
        log("s_modes %c", *modes);
#endif
}
	del_mod_user(nick);	*/
	/* then we set up a timer to re-init it */
/*   	add_mod_timer("re_init_bot", "reinit", Statserv_Info[0].module_name, 10);
	return 1;

} */
void re_init_bot() {
	notice(s_Services, "Re-Initilizing %s Bot", s_StatServ);
        init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", Statserv_Info[0].module_name);
	del_mod_timer("reinit");
/* Re-Initilize the Bot"); */
}
static int s_del_user(User *u) {
	SStats *s;
	char *cmd;
#ifdef DEBUG
	log(" Server %s", u->server->name);
#endif
	s=findstats(u->server->name);
	if (UserLevel(u) >= 40) {
		DecreaseOpers(s);
	}
	DecreaseUsers(s);
	DelTLD(u);
	cmd = sstrdup(recbuf);
	cmd = strtok(cmd, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, "");
	cmd++;
	if (synced) notice(s_StatServ, "\2SIGNOFF\2 %s has Signed off at %s --> %s", u->nick, u->server->name, cmd);
	return 1;
}


static int s_new_user(User *u) {
	SStats *s;
	
	s=findstats(u->server->name);
	IncreaseUsers(s);
#ifdef DEBUG
	log("added a User %s to stats, now at %d", u->nick, s->users);
#endif
	if (s->maxusers < s->users) {
		/* New User Record */
		s->maxusers = s->users;
		s->t_maxusers = time(NULL);
		if (synced) sts(":%s WALLOPS :\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!", s_StatServ, s->name, s->users);	
	}
	if (stats_network.maxusers < stats_network.users) {
		stats_network.maxusers = stats_network.users;
		stats_network.t_maxusers = time(NULL);
		if (synced) sts(":%s WALLOPS :\2NEW NETWORK RECORD!\2 Wow, a New Global User record has been reached with %d users!", s_StatServ, stats_network.users);
	}
	
	if (synced) notice(s_StatServ, "\2SIGNON\2 %s(%s@%s) has Signed on at %s", u->nick, u->username, u->hostname, u->server->name); 
	AddTLD(u);
	return 1;
}

int pong(Server *s) {
	SStats *ss;
	/* we don't want negative pings! */
	if (s->ping < 0) return -1; 
	
	ss = findstats(s->name);
	if (!ss) return -1;
	
	/* this is a tidy up from old versions of StatServ that used to have negative pings! */
	if (ss->lowest_ping < 0) {
		ss->lowest_ping = 0; 
	}
	if (ss->highest_ping < 0) {
		ss->highest_ping = 0;
	}

	if (s->ping > ss->highest_ping) {
		ss->highest_ping = s->ping;
		ss->t_highest_ping = time(NULL);
	}
	if (s->ping < ss->lowest_ping) {
		ss->lowest_ping = s->ping;
		ss->t_lowest_ping = time(NULL);
	}
/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if (StatServ.lag > 0) {
		if (s->ping > StatServ.lag) {
			if (synced) globops(s_StatServ, "\2%s\2 is Lagged out with a ping of %d", s->name, s->ping);
		}
	}
	return 1;
}
int Online(Server *s) {

   memcpy(StatServ.user, Servbot.user, 8);
   memcpy(StatServ.host, Servbot.host, MAXHOST);
   StatServ.lag = 0;
   if (!config_read("stats.cfg", options) == 0) {
   	log("Error, Statserv could not be configured");
   	notice(s_Services, "Error, Statserv could not be configured");
	return -1;
   } else {
	   init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", Statserv_Info[0].module_name);
   }
   
/* now that we are online, setup the timer to save the Stats database every so often */
   add_mod_timer("SaveStats", "Save_Stats_DB", Statserv_Info[0].module_name, 600);
/* also add a timer to check if its midnight (to reset the daily stats */
   add_mod_timer("Is_Midnight", "Daily_Stats_Reset", Statserv_Info[0].module_name, 60);
   synced = 1;
   return 1;
   
}

void ss_cb_Config(char *arg, int configtype) {
	if (configtype == 0) {
		/* Nick */
		memcpy(StatServ.nick, arg, MAXNICK);
		memcpy(s_StatServ, StatServ.nick, MAXNICK);
	} else if (configtype == 1) {
		/* User */
		memcpy(StatServ.user, arg, 8);
	} else if (configtype == 2) {
		/* host */
		memcpy(StatServ.host, arg, MAXHOST);
	} else if (configtype == 3) {
		/* lag */
		StatServ.lag = atoi(arg);
	}
}
	
int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	char *cmd;

	u = finduser(origin);
	if (!u) {
		log("Unable to finduser %s (statserv)", origin);
		return -1;
	}

	stats_network.requests++;

	if (flood(u))
		return -1;
	log("%s received message from %s: %s", s_StatServ, u->nick, coreLine);

	if (me.onlyopers && UserLevel(u) < 40) {
		privmsg(u->nick, s_StatServ,
			"This service is only available to IRCops.");
		notice(s_StatServ, "%s tried to use me but is not Authorized", u->nick);
		return -1;
	}
	if (coreLine == NULL) return -1;
	cmd = strtok(coreLine, " ");

	if (!strcasecmp(cmd, "HELP")) {
		coreLine = strtok(NULL, " ");
		if(!coreLine) {
			notice(s_StatServ, "%s is a Dummy and wanted Help", u->nick);
		} else {
			notice(s_StatServ,"%s is a Dummy and wanted help on %s",u->nick, coreLine);
		}
		if (!coreLine) {
			privmsg_list(u->nick, s_StatServ, ss_help);
			if (UserLevel(u) >= 150)
				privmsg_list(u->nick, s_StatServ, ss_myuser_help);
		} else if (!strcasecmp(coreLine, "SERVER"))
			privmsg_list(u->nick, s_StatServ, ss_server_help);
                else if (!strcasecmp(coreLine, "RESET") && UserLevel(u) >= 190)
                        privmsg_list(u->nick, s_StatServ, ss_reset_help);
		else if (!strcasecmp(coreLine, "MAP"))
			privmsg_list(u->nick, s_StatServ, ss_map_help);
		else if (!strcasecmp(coreLine, "JOIN") && UserLevel(u) >= 190)
			privmsg_list(u->nick, s_StatServ, ss_join_help);
		else if (!strcasecmp(coreLine, "NETSTATS"))
			privmsg_list(u->nick, s_StatServ, ss_netstats_help);
		else if (!strcasecmp(coreLine, "DAILY"))
			privmsg_list(u->nick, s_StatServ, ss_daily_help);
		else if (!strcasecmp(coreLine, "TLD"))
			privmsg_list(u->nick, s_StatServ, ss_tld_help);
		else if (!strcasecmp(coreLine, "TLDMAP"))
			privmsg_list(u->nick, s_StatServ, ss_tld_map_help);
		else if (!strcasecmp(coreLine, "OPERLIST"))
			privmsg_list(u->nick, s_StatServ, ss_operlist_help);
                else if (!strcasecmp(coreLine, "BOTLIST"))
                        privmsg_list(u->nick, s_StatServ, ss_botlist_help);
                else if (!strcasecmp(coreLine, "VERSION"))
                        privmsg_list(u->nick, s_StatServ, ss_version_help);
		else if (!strcasecmp(coreLine, "STATS") && UserLevel(u) >= 190)
			privmsg_list(u->nick, s_StatServ, ss_stats_help);
		else
			privmsg(u->nick, s_StatServ, "Unknown Help Topic: \2%s\2",
				coreLine);
	} else if (!strcasecmp(cmd, "SERVER")) {
		cmd = strtok(NULL, " ");
		ss_server(u, cmd);
/*		notice(s_StatServ,"%s Wanted Server Information on %s",u->nick, cmd); */
	} else if (!strcasecmp(cmd, "JOIN")) {
		cmd = strtok(NULL, " ");
		ss_JOIN(u, cmd);
	} else if (!strcasecmp(cmd, "MAP")) {
		ss_map(u);
/*		notice(s_StatServ,"%s Wanted to see the Current Network MAP",u->nick); */
        } else if (!strcasecmp(cmd, "VERSION")) {
                ss_version(u);
/*                notice(s_StatServ,"%s Wanted to know our version number ",u->nick); */
	} else if (!strcasecmp(cmd, "NETSTATS")) {
		ss_netstats(u);
/*		notice(s_StatServ,"%s Wanted to see the NetStats ",u->nick); */
	} else if (!strcasecmp(cmd, "DAILY")) {
		ss_daily(u);
/*		notice(s_StatServ,"%s Wanted to see the Daily NetStats ",u->nick); */
	} else if (!strcasecmp(cmd, "TLD")) {
		cmd = strtok(NULL, " ");
		ss_tld(u, cmd);
/*		notice(s_StatServ,"%s Wanted to find the Country that is Represented by %s ",u->nick,cmd); */
	} else if (!strcasecmp(cmd, "TLDMAP")) {
		ss_tld_map(u);
/*		notice(s_StatServ,"%s Wanted to see a Country Breakdown",u->nick); */
	} else if (!strcasecmp(cmd, "OPERLIST")) {
		char *t;
		cmd = strtok(NULL, " ");
		t = strtok(NULL, " ");
		ss_operlist(u, cmd, t);
        } else if (!strcasecmp(cmd, "BOTLIST")) {
                ss_botlist(u);
/*                notice(s_StatServ,"%s Wanted to see the Bot List",u->nick); */
	} else if (!strcasecmp(cmd, "STATS")) {
		char *t, *m;
		m = strtok(NULL, " ");
		cmd = strtok(NULL, " ");
		t = strtok(NULL, " ");
		ss_stats(u, m, cmd, t);
/*		notice(s_StatServ,"%s Wants to Look at my Stats!! 34/24/34",u->nick); */
        } else if (!strcasecmp(cmd, "RESET")) {
/*                notice(s_StatServ,"%s Wants me to RESET the databases.. here goes..",u->nick); */
                ss_reset(u);
	} else {
		privmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2",
			cmd);
/*		notice(s_StatServ,"%s Reqested %s, but that is a Unknown Command",u->nick,cmd); */
	}
	return 1;
}

void Is_Midnight() {
	time_t current = time(NULL);
	struct tm *ltm = localtime(&current);
	TLD *t;
	if (ltm->tm_hour == 0) {
		if (ltm->tm_min == 0) {
			/* its Midnight! */
			notice(s_StatServ, "Reseting Daily Statistics - Its Midnight here!");
			log("Resetting Daily Statistics");
			daily.servers = 0;
			daily.t_servers = time(NULL);
			daily.users = 0;
			daily.t_users = time(NULL);
			daily.opers = 0;
			daily.t_opers = time(NULL);
			for (t = tldhead; t; t = t->next) 
				t->daily_users = 0;
				
		}
	}	
}
static void ss_tld_map(User *u) {
	TLD *t;
	
	privmsg(u->nick, s_StatServ, "Top Level Domain Statistics:");
	for (t = tldhead; t; t = t->next) {
		if (t->users != 0)
			privmsg(u->nick, s_StatServ, "%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d", t->tld, t->users, (float)t->users / (float)stats_network.users * 100, t->country, t->daily_users);
	}		
	privmsg(u->nick, s_StatServ, "End of List");
}	

static void ss_version(User *u)
{
        privmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
        privmsg(u->nick, s_StatServ, "%s - %s Compiled %s at %s", me.name, Statserv_Info[0].module_version, version_date, version_time);
        privmsg(u->nick, s_StatServ, "http://www.neostats.net");
}
static void ss_netstats(User *u) {
	privmsg(u->nick, s_StatServ, "Network Statistics:-----");
	privmsg(u->nick, s_StatServ, "Current Users: %ld", stats_network.users);
	privmsg(u->nick, s_StatServ, "Maximum Users: %ld [%s]", stats_network.maxusers, sftime(stats_network.t_maxusers));
	privmsg(u->nick, s_StatServ, "Current Opers: %ld", stats_network.opers);
	privmsg(u->nick, s_StatServ, "Maximum Opers: %ld [%s]", stats_network.maxopers, sftime(stats_network.t_maxopers));
	privmsg(u->nick, s_StatServ, "Users Set Away: %d", stats_network.away);
	privmsg(u->nick, s_StatServ, "Current Servers: %d", stats_network.servers);
	privmsg(u->nick, s_StatServ, "Maximum Servers: %d [%s]", stats_network.maxservers, sftime(stats_network.t_maxservers));
	privmsg(u->nick, s_StatServ, "--- End of List ---");
}
static void ss_daily(User *u) {
	privmsg(u->nick, s_StatServ, "Daily Network Statistics:");
	privmsg(u->nick, s_StatServ, "Maximum Servers: %-2d %s", daily.servers, sftime(daily.t_servers));
	privmsg(u->nick, s_StatServ, "Maximum Users: %-2d %s", daily.users, sftime(daily.t_users));
	privmsg(u->nick, s_StatServ, "Maximum Opers: %-2d %s", daily.opers, sftime(daily.t_opers));
	privmsg(u->nick, s_StatServ, "All Daily Statistics are reset at Midnight");
	privmsg(u->nick, s_StatServ, "End of Information.");
}

static void ss_map(User *u) {
	SStats *ss;
	Server *s;
	privmsg(u->nick, s_StatServ, "%-23s %-10s %-10s %-10s", "\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2",  "\2[LAG/MAX]\2");
	for (ss = Shead; ss; ss=ss->next) {
		s=findserver(ss->name);	
/*		if (s) privmsg(u->nick, s_StatServ, "%-23s %-14s %-10s %-10s", ss->name, 
                  "[%d/%d]");

, ss->users, ss->maxusers);

, "[%d/%d]", "[%d/%d]"); */

if (s) privmsg(u->nick, s_StatServ, "\2%-23s [ %d/%d ]    [ %d/%d ]    [ %ld/%ld ]", ss->name, ss->users, 
ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);

	}
	privmsg(u->nick, s_StatServ, "--- End of Listing ---");
}

static void ss_server(User *u, char *server) {

	SStats *ss;
	Server *s;
	if (!server) {
		privmsg(u->nick, s_StatServ, "Error, the Syntax is Incorrect. Please Specify a Server");
		privmsg(u->nick, s_StatServ, "Server Listing:");
		for (ss = Shead; ss; ss = ss->next) {
			if (findserver(ss->name)) {
				privmsg(u->nick, s_StatServ, "Server: %s (*)", ss->name);
			} else {
				privmsg(u->nick, s_StatServ, "Server: %s",ss->name);
			}
		}		
		privmsg(u->nick, s_StatServ, "***** End of List (* indicates Server is online at the moment) *****");		
		return;
	}
	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findstats(server);
	s=findserver(server);
	if (!ss) {
		log("Wups, Problem, Cant find Server Statistics for Server %s", server);
		privmsg(u->nick, s_StatServ, "Internal Error! Please Consult the Log file");
		return;
	}
	privmsg(u->nick, s_StatServ, "Statistics for \2%s\2 since %s", ss->name, sftime(ss->starttime));
	if (!s) privmsg(u->nick, s_StatServ, "Server Last Seen: %s", sftime(ss->lastseen));
	if (s) privmsg(u->nick, s_StatServ, "Current Users: %-3ld (%2.0f%%)", ss->users, (float)ss->users / (float)stats_network.users * 100);
	privmsg(u->nick, s_StatServ, "Maximum Users: %-3ld at %s", ss->maxusers, sftime(ss->t_maxusers));
	if (s) privmsg(u->nick, s_StatServ, "Current Opers: %-3ld", ss->opers);
	privmsg(u->nick, s_StatServ, "Maximum Opers: %-3ld at %s", ss->maxopers, sftime(ss->t_maxopers));
	privmsg(u->nick, s_StatServ, "IRCop Kills: %d", ss->operkills);
	privmsg(u->nick, s_StatServ, "Server Kills: %d", ss->serverkills);
	privmsg(u->nick, s_StatServ, "Lowest Ping: %-3d at %s", ss->lowest_ping, sftime(ss->t_lowest_ping));
	privmsg(u->nick, s_StatServ, "Higest Ping: %-3d at %s", ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) privmsg(u->nick, s_StatServ, "Current Ping: %-3d", s->ping);
	if (ss->numsplits >= 1) 
		privmsg(u->nick, s_StatServ, "%s has Split from the network %d time%s", ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	else
		privmsg(u->nick, s_StatServ, "%s has never split from the Network.", ss->name);
	privmsg(u->nick, s_StatServ, "***** End of Statistics *****");	
}
static void ss_tld(User *u, char *tld)
{
	TLD *tmp;

	if (!tld) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s TLD <tld>", s_StatServ);
		privmsg(u->nick, s_StatServ, "For additional help, /msg %s HELP", 
			s_StatServ);
		return;
	}

	if (*tld == '*')
		tld++;
	if (*tld == '.')
		tld++;

	tmp = findtld(tld);

	if (!tmp)
		privmsg(u->nick, s_StatServ, "Top Level Domain \2%s\2 does not exist.",
			tld);
	else
		privmsg(u->nick, s_StatServ, "%s", tmp->country);
}

static void ss_operlist(User *origuser, char *flags, char *server)
{
	register int i, j = 0;
	int away = 0;
	register User *u;
	int tech = 0;

	if (!flags) {
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops:");
		notice (s_StatServ, "%s Requested OperList",origuser->nick);
	}		

	if (flags && !strcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops (Not Away):");
		notice(s_StatServ,"%s Reqested Operlist of Non-Away Opers",origuser->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops on Server %s", server);
		notice(s_StatServ,"%s Reqested Operlist on Server %s",origuser->nick, server);
	}
	for (i = 0; i < U_TABLE_SIZE; i++) {
		for (u = userlist[i]; u; u = u->next) {
			tech = UserLevel(u);
			if (away && u->is_away)
				continue;
			if (!strcasecmp(u->server->name, me.services_name))
				continue;
			if (tech < 40)
				continue;
			if (!server) {
				if (UserLevel(u) < 40)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
					u->server->name, tech);
				continue;
			} else {
				if (strcasecmp(server, u->server->name))	continue;
				if (UserLevel(u) < 40)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
					u->server->name, tech);
				continue;
			}
		}
	}
	privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_botlist(User *origuser)
{
        register int i, j = 0;
        register User *u;
        privmsg(origuser->nick, s_StatServ, "On-Line Bots:");
        for (i = 0; i < U_TABLE_SIZE; i++) {
                for (u = userlist[i]; u; u = u->next) {
                        if (u->Umode & UMODE_BOT) {
                                j++;
                                privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick, u->server->name);
                                continue;
			}
                }
        }       
        privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_stats(User *u, char *cmd, char *arg, char *arg2)
{
	SStats *st;

	if (UserLevel(u) < 190) {
		log("Access Denied (STATS) to %s", u->nick);
		privmsg(u->nick, s_StatServ, "Access Denied.");
		return;
	}

	if (!cmd) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS [DEL|LIST|COPY]",
			s_StatServ);
		privmsg(u->nick, s_StatServ, "For additional help, /msg %s HELP",
			s_StatServ);	
		return;
	}
	if (!strcasecmp(cmd, "LIST")) {
		int i = 1;
		privmsg(u->nick, s_StatServ, "Statistics Database:");
		for (st = Shead; st; st = st->next) {
			privmsg(u->nick, s_StatServ, "[%-2d] %s", i, st->name);
			i++;
		}
		privmsg(u->nick, s_StatServ, "End of List.");
		log("%s requested STATS LIST.", u->nick);
	} else if (!strcasecmp(cmd, "DEL")) {
		SStats *m = NULL;
		if (!arg) {
			privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS DEL <name>",
				s_StatServ);
			privmsg(u->nick, s_StatServ, "For additonal help, /msg %s HELP",
				s_StatServ);
			return;
		}
		st = findstats(arg);
		if (!st) {
			privmsg(u->nick, s_StatServ, "%s is not in the database!",
				arg);
			return;
		}
		for (st = Shead; st; st = st->next) {
			if (!strcasecmp(arg, st->name))
				break;
			m = st;
		}
		if (m)
			m->next = st->next;
		else
			Shead = st->next;
		free(st);
		privmsg(u->nick, s_StatServ, "Removed %s from the database.", arg);
		log("%s requested STATS DEL %s", u->nick, arg);
	} else if (!strcasecmp(cmd, "COPY")) {
		Server *s;

		if (!arg || !arg2) {
			privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS COPY <name> "
				" <newname>", s_StatServ);
			return;
		}
		st = findstats(arg2);
		if (st)
			free(st);

		st = findstats(arg);
		if (!st) {
			privmsg(u->nick, s_StatServ, "No entry in the database for %s",
				arg);
			return;
		}
		s = findserver(arg);
		if (s) {
			privmsg(u->nick, s_StatServ, "Server %s is online!", arg);
			return;
		}
		s = NULL;
		memcpy(st->name, arg2, sizeof(st->name));
		privmsg(u->nick, s_StatServ, "Moved database entry for %s to %s",
			arg, arg2);
		log("%s requested STATS COPY %s -> %s", u->nick, arg, arg2);
	} else {
		privmsg(u->nick, s_StatServ, "Invalid Argument.");
		privmsg(u->nick, s_StatServ, "For help, /msg %s HELP", s_StatServ);
	}
}

static void ss_reset(User *u)
{
        if (UserLevel(u) < 190) {
                log("Access Denied (RELOAD) to %s", u->nick);
                privmsg(u->nick, s_StatServ, "Access Denied.");
                return;
        }
        remove("data/nstats.db");
        remove("data/stats.db");
        globops(s_StatServ, "%s requested \2RESET\2 databases.", u->nick);
        log("%s requested RESET.", u->nick);
        globops(s_StatServ, "Rebuilding Statistics DataBase after RESET...");
	privmsg(u->nick, s_StatServ, "Databases Reset! I hope you wanted to really do that!");
}

static void ss_JOIN(User *u, char *chan)
{
	if (UserLevel(u) < 190) {
		log("Access Denied (JOIN) to %s", u->nick);
		privmsg(u->nick, s_StatServ, "Access Denied.");
		notice(s_StatServ,"%s Requested JOIN, but is not a god!",u->nick);
		return;
	}
	if (!chan) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s JOIN <chan>",s_StatServ);
		return;
	}
	globops(s_StatServ, "JOINING CHANNEL -\2(%s)\2- Thanks to %s!%s@%s)", chan, u->nick, u->username, u->hostname);
	privmsg(me.chan, s_StatServ, "%s Asked me to Join %s, So, I'm Leaving %s", u->nick, chan, me.chan);
	sts(":%s part %s", s_StatServ, me.chan);
	log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
	sts(":%s JOIN %s",s_StatServ,chan);
	sts(":%s MODE %s +o %s",me.name,chan,s_StatServ);
}

void DelTLD(User *u) {
	TLD *t = NULL;
	char *m;
	
	m = strrchr(u->hostname, '.');

	if (!m)
		t = findtld("num");
	else
		m++;

	if (!t) {
		if (!isdigit(*m))
			t = findtld(m);
		else
			t = findtld("num");
	}

	if (!t) {
		log("Unable to find TLD entry for %s (%s), damn vhosts!", u->nick, m);
		return;
	}
	t->users--;
}	

/* "net" -- not DOT"net" */
TLD *findtld(char *tld)
{
	TLD *t;

	for (t = tldhead; t; t = t->next) {
		if (!strcasecmp(t->tld, tld))
			return t;
	}

	return NULL;
}

TLD *AddTLD(User *u)
{
	TLD *t = NULL;
	char *m;

	m = strrchr(u->hostname, '.');

	if (!m)
		t = findtld("num");
	else
		m++;

	if (!t) {
		if (!isdigit(*m))
			t = findtld(m);
		else
			t = findtld("num");
	}

	if (!t) {
		log("Unable to find TLD entry for %s (%s), damn vhosts!", u->nick, m);
		return NULL;
	}
	t->users++;
	t->daily_users++;

	return t;
}

void LoadTLD()
{
	register int i;
	FILE *fp;
	char buf[BUFSIZE], buf2[BUFSIZE];
	char *tmp = NULL, *tmp2 = NULL;
	TLD *t;

	if ((fp = fopen("data/tlds.nfo", "r")) == NULL) {
		log("Top Level Domain Statistics not loaded: file not found.");
		return;
	}

    while (fgets(buf, BUFSIZE, fp)) {
		memcpy(buf2, buf, sizeof(buf));
		tmp = strrchr(buf, '(');
		tmp++;
		tmp[strlen(tmp) - 1] = '\0';
		tmp[strlen(tmp) - 1] = '\0';
		for (i = 0; buf[i] != '('; i++)
			buf2[strlen(buf2) + 1] = buf[i];

		if ((tmp2 = strchr(buf2, '\n')))	*tmp2 = '\0';

		t = smalloc(sizeof(TLD));
		t->users = 0;
		t->country = sstrdup(buf2);
		memcpy(t->tld, tmp, sizeof(t->tld));

		if (!tldhead) {
			tldhead = t;
			tldhead->next = NULL;
		} else {
			t->next = tldhead;
			tldhead = t;
		}
    }
	fclose(fp);
}

void init_tld()
{
	TLD *t;

	for (t = tldhead; t; t = t->next) {
		t->users = 0;
	}
}

static SStats *new_stats(const char *name)
{
	SStats *s = calloc(sizeof(SStats), 1);

#ifdef DEBUG
	log("new_stats(%s)", name);
#endif

	if (!s) {
		log("Out of memory.");
		exit(0);
	}

	memcpy(s->name, name, MAXHOST);
	s->numsplits = 0;
	s->maxusers = 0;
	s->t_maxusers = time(NULL);
	s->t_maxopers = time(NULL);
	s->maxopers = 0;
	s->totusers = 0;
	s->daily_totusers = 0;
	s->lastseen = time(NULL);
	s->starttime = time(NULL);
	s->t_highest_ping = time(NULL);
	s->t_lowest_ping = time(NULL);
	s->lowest_ping = 0;
	s->highest_ping = 0;
	s->users = 0;
	s->opers = 0;
	s->operkills = 0;
	s->serverkills = 0;
	



	if (!Shead) {
		Shead = s;
		Shead->next = NULL;
	} else {
		s->next = Shead;
		Shead = s;
	}
	return s;
}

void AddStats(Server *s)
{
	SStats *st = findstats(s->name);

#ifdef DEBUG
	log("AddStats(%s)", s->name);
#endif

	if (!st) {
		st = new_stats(s->name);
	} else {
		st->lastseen = time(NULL);
	}
}

SStats *findstats(char *name)
{
	SStats *t;
#ifdef DEBUG
	log("findstats(%s)", name);
#endif
	for (t = Shead; t; t = t->next) {
		if (!strcasecmp(name, t->name))
			return t;
	}
	return NULL;
}



void SaveStats()
{
	FILE *fp = fopen("data/stats.db", "w");
	SStats *s;

	if (!fp) {
		log("Unable to open stats.db for writing.");
		return;
	}

	for (s = Shead; s; s = s->next) {
#ifdef DEBUG
	log("Writing statistics to database for %s", s->name);
#endif
		fprintf(fp, "%s %d %ld %ld %d %ld %ld %ld %d %d %ld\n", s->name,
			s->numsplits, s->maxusers, s->t_maxusers, s->maxopers,
			s->t_maxopers, s->lastseen, s->starttime, s->operkills,
			s->serverkills, s->totusers);
	}
	fclose(fp);
	if ((fp = fopen("data/nstats.db", "w")) == NULL) {
		log("Unable to open nstats.db for writing.");
		return;
	}
	fprintf(fp, "%d %ld %d %ld %ld %ld %ld\n", stats_network.maxopers, stats_network.maxusers,
		stats_network.maxservers, stats_network.t_maxopers, stats_network.t_maxusers, stats_network.t_maxservers, stats_network.totusers);
	fclose(fp);
}

void LoadStats()
{
	FILE *fp = fopen("data/nstats.db", "r");
	SStats *s;
	char buf[BUFSIZE];
	char *tmp;
	char *name, *numsplits, *maxusers, *t_maxusers,
		*maxopers, *t_maxopers, *lastseen, *starttime,
		*operkills, *serverkills, *totusers;
	if (fp) {
	while (fgets(buf, BUFSIZE, fp)) {
		stats_network.maxopers = atoi(strtok(buf, " "));
		stats_network.maxusers = atol(strtok(NULL, " "));
		stats_network.maxservers = atoi(strtok(NULL, " "));
		stats_network.t_maxopers = atoi(strtok(NULL, " "));
		stats_network.t_maxusers = atol(strtok(NULL, " "));
		stats_network.t_maxservers = atoi(strtok(NULL, " "));
		tmp = strtok(NULL, "");
		if (tmp==NULL) {
			fprintf(stderr, "Detected Old Version of Network Database, Upgrading\n");
			stats_network.totusers = stats_network.maxusers;
		} else {
			stats_network.totusers = atoi(tmp);
		}
	}
	fclose(fp);
	}

	if ((fp = fopen("data/stats.db", "r")) == NULL)
		return;

	memset(buf, '\0', BUFSIZE);
	while (fgets(buf, BUFSIZE, fp)) {
		s = smalloc(sizeof(SStats));
		name = strtok(buf, " ");
		numsplits = strtok(NULL, " ");
		maxusers = strtok(NULL, " ");
		t_maxusers = strtok(NULL, " ");
		maxopers = strtok(NULL, " ");
		t_maxopers = strtok(NULL, " ");
		lastseen = strtok(NULL, " ");
		starttime = strtok(NULL, " ");
		operkills = strtok(NULL, " ");
		serverkills = strtok(NULL, " ");
		totusers = strtok(NULL, " ");

		memcpy(s->name, name, MAXHOST);
		s->numsplits = atoi(numsplits);
		s->maxusers = atol(maxusers);
		s->t_maxusers = atol(t_maxusers);
		s->maxopers = atoi(maxopers);
		s->t_maxopers = atol(t_maxopers);
		s->lastseen = atol(lastseen);
		s->starttime = atol(starttime);
		s->operkills = atoi(operkills);
		s->serverkills = atol(serverkills);
		s->users = 0;
		s->opers = 0;
		s->daily_totusers = 0;
		if (totusers==NULL) {
			s->totusers = 0;
			fprintf(stderr, "Detected Old Version of Server Database, Upgrading\n");
		} else {
			s->totusers = atol(totusers);
		}

#ifdef DEBUG
	log("LoadStats(): Loaded statistics for %s", s->name);
#endif
		if (!Shead) {
			Shead = s;
			Shead->next = NULL;
		} else {
			s->next = Shead;
			Shead = s;
		}
	}
	fclose(fp);
}

