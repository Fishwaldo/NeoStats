/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      statserv.c, 
** Version: 3.1
** Date:    08/03/2002
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
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
static int s_user_away(User *);
int s_new_server(Server *);
static int s_del_server(Server *);
static int s_new_user(User *);
static int s_del_user(User *);
static int s_user_modes(User *);
static int s_user_kill(User *);
/* int s_bot_kill(char *); */
static void ss_cb_Config(char *, int);
static int new_m_version(char *av, char *tmp);
static void DelTLD(User *u);
static void ss_notices(User *u);
static void ss_htmlsettings(User *u, char *cmd, char *m);
int nm;
int notify_msgs = 0;
void ss_html();
void ss_chkhtml();

char s_StatServ[MAXNICK] = "StatServ";

static int synced;

Module_Info Statserv_Info[] = { {
    "StatServ",
    "Statistical Bot For NeoStats",
    "3.0"
} };

Functions StatServ_fn_list[] = { 
    { "VERSION",    new_m_version,    1 },
    { NULL,        NULL,     0}
};


EventFnList StatServ_Event_List[] = {
    { "ONLINE",     Online},
    { "PONG",     pong},
    { "NEWSERVER",    s_new_server},
    { "SQUIT",	s_del_server},
    { "SIGNON",     s_new_user},
    { "UMODE",     s_user_modes},
    { "SIGNOFF",     s_del_user},
    { "AWAY",	s_user_away},
    { "KILL",    s_user_kill},
    { NULL,     NULL}
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
    strcpy(segv_location, "StatServ-new_m_version");
    snumeric_cmd(351, av, "Module StatServ Loaded, Version: %s %s %s",Statserv_Info[0].module_version,version_date,version_time);
    return 0;
}

void _init() {
   Server *ss;
   User *u;
   hnode_t *node;
   hscan_t scan;
   strcpy(segv_location, "StatServ-_init");

    synced = 0;
    globops(me.name, "StatServ Module Loaded");    
    LoadTLD();
    init_tld();
    LoadStats();
   hash_scan_begin(&scan, sh);
   while ((node = hash_scan_next(&scan)) != NULL ) {
   	ss = hnode_get(node);
	s_new_server(ss);
#ifdef DEBUG
            log("Added Server %s to StatServ List", ss->name);
#endif
   }
   hash_scan_begin(&scan, uh);
   while ((node = hash_scan_next(&scan)) != NULL ) {
   	u = hnode_get(node);
	s_new_user(u);
	s_user_modes(u);
#ifdef DEBUG
            log("Adduser user %s to StatServ List", u->nick);
#endif
   }
}

void _fini() {
    SaveStats();
    if (synced) globops(me.name, "StatServ Module Unloaded");
    
}

int s_new_server(Server *s) {
    strcpy(segv_location, "StatServ-s_new_server");

    AddStats(s);
    IncreaseServers();
    if (stats_network.maxservers < stats_network.servers) {
        stats_network.maxservers = stats_network.servers;
        stats_network.t_maxservers = time(NULL);
        if (synced) swallops_cmd(s_StatServ, "\2NEW SERVER RECORD\2 Wow, there are now %d Servers on the Network", stats_network.servers); 
    }
    if (stats_network.servers > daily.servers) {
	daily.servers = stats_network.servers;
	daily.t_servers = time(NULL);
    }
    if (synced) notice(s_StatServ, "\2SERVER\2 %s has joined the Network at %s", s->name, s->uplink);
    return 1;

}

static int s_del_server(Server *s) {
    SStats *ss;

    strcpy(segv_location, "StatServ-s_del_server");

    DecreaseServers();
    if (synced) notice(s_StatServ, "\2SERVER\2 %s has left the Network at %s", s->name, s->uplink);
    ss = findstats(s->name);
    if (s->name != me.uplink)
    ss->numsplits = ss->numsplits +1;
    return 1;

}


static int s_user_kill(User *u) {
    SStats *s;
    SStats *ss;
    char *cmd, *who;
#ifdef DEBUG
    log(" Server %s", u->server->name);
#endif

    strcpy(segv_location, "StatServ-s_user_kill");

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
	ss = findstats(u->server->name); 
	ss->operkills = ss->operkills +1; 
    } else if (findserver(who)) {
        if (synced) notice(s_StatServ, "\2SERVER KILL\2 %s was Killed by the Server %s --> %s", u->nick, who, cmd);
	ss = findstats(who);
	ss->serverkills = ss->serverkills +1;
    }
    return 1;
}

static int s_user_modes(User *u) {
    int add = 1;
    char *modes;
    SStats *s;

    strcpy(segv_location, "StatServ-s_user_modes");


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
            case '+': add = 1;    break;
            case '-': add = 0;    break;
            case 'N':
            case 'o':
                if (add) {
                    IncreaseOpers(findstats(u->server->name));
                    s = findstats(u->server->name);
                    if (stats_network.maxopers < stats_network.opers) {
                        stats_network.maxopers = stats_network.opers;
                        stats_network.t_maxopers = time(NULL);
                        if (synced) swallops_cmd(s_StatServ, "\2Oper Record\2 The Network has reached a New Record for Opers at %d", stats_network.opers);
                    }
                    if (s->maxopers < s->opers) {
                        s->maxopers = s->opers;
                        s->t_maxopers = time(NULL);
                        if (synced) swallops_cmd(s_StatServ, "\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s->name, s->opers);
                    }
		    if (s->opers > daily.opers) {
			daily.opers = s->opers;
			daily.t_opers = time(NULL);
                    }    
                } else {
                     DecreaseOpers(findstats(u->server->name));
#ifdef DEBUG
                     log("Decrease Opers");
#endif
                }
                break;
            case 'O':
                if (add) {
                    IncreaseOpers(findstats(u->server->name));
                    s = findstats(u->server->name);
                    if (stats_network.maxopers < stats_network.opers) {
                        stats_network.maxopers = stats_network.opers;
                        stats_network.t_maxopers = time(NULL);
                        if (synced) swallops_cmd(s_StatServ, "\2Oper Record\2 The Network has reached a New Record for Opers at %d", stats_network.opers);
                    }
                    if (s->maxopers < s->opers) {
                        s->maxopers = s->opers;
                        s->t_maxopers = time(NULL);
                        if (synced) swallops_cmd(s_StatServ, "\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s->name, s->opers);
                    }                        
                    if (s->opers > daily.opers) {
                        daily.opers = s->opers;
                        daily.t_opers = time(NULL);
                    }

                } else {
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

void re_init_bot() {
    strcpy(segv_location, "StatServ-re_init_bot");

    notice(s_Services, "Re-Initilizing %s Bot", s_StatServ);
    init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", Statserv_Info[0].module_name);
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
    return 1;
}


static int s_user_away(User *u) {
    strcpy(segv_location, "StatServ-s_user_away");


    if (u->is_away) {
	u->is_away = 1;
    	stats_network.away = stats_network.away +1;
    } else {
	u->is_away = 0;
	stats_network.away = stats_network.away -1;
    }
    return 1;

}



static int s_new_user(User *u) {
    SStats *s;
    
if (u->server->name == me.name) return 0;

    strcpy(segv_location, "StatServ-s_new_user");

    s = findstats(u->server->name);

    IncreaseUsers(s); 

#ifdef DEBUG
    log("added a User %s to stats, now at %d", u->nick, s->users);
#endif

    if (s->maxusers < s->users) {
        /* New User Record */
        s->maxusers = s->users;
        s->t_maxusers = time(NULL);
        if (synced) swallops_cmd(s_StatServ, "\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!", s->name, s->users);    
    }

    if (stats_network.maxusers < stats_network.users) {
        stats_network.maxusers = stats_network.users;
        stats_network.t_maxusers = time(NULL);
        if (synced) swallops_cmd(s_StatServ, "\2NEW NETWORK RECORD!\2 Wow, a New Global User record has been reached with %d users!", stats_network.users);
    }

    if (stats_network.users > daily.users) {
	daily.users = stats_network.users;
 	daily.t_users = time(NULL);
    }

    AddTLD(u);
    return 1;
}

int pong(Server *s) {
    SStats *ss;

    strcpy(segv_location, "StatServ-pong");


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

    strcpy(segv_location, "StatServ-Online");


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

   /* Add a timer for HTML writeouts */
   add_mod_timer("TimerWeb", "WebStats", Statserv_Info[0].module_name, 3600);


   /* also add a timer to check if its midnight (to reset the daily stats */
   add_mod_timer("Is_Midnight", "Daily_Stats_Reset", Statserv_Info[0].module_name, 60);
   synced = 1;

   return 1;
   
}

void ss_cb_Config(char *arg, int configtype) {

    strcpy(segv_location, "StatServ-ss_cb_Config");


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
    
int __Bot_Message(char *origin, char **av, int ac)
{
    User *u;

    strcpy(segv_location, "StatServ-__Bot_Message");


    u = finduser(origin);
    if (!u) {
        log("Unable to finduser %s (statserv)", origin);
        return -1;
    }

    stats_network.requests++;

    if (flood(u))
        return -1;
    log("%s received message from %s: %s", s_StatServ, u->nick, av[1]);

    if (me.onlyopers && UserLevel(u) < 40) {
        privmsg(u->nick, s_StatServ,
            "This service is only available to IRCops.");
        notice(s_StatServ, "%s tried to use me but is not Authorized", u->nick);
        return -1;
    }

    if (!strcasecmp(av[1], "HELP")) {
        if(ac < 3) {
            notice(s_StatServ, "%s requested %s Help", u->nick, s_StatServ);
        } else {
          if (notify_msgs) notice(s_StatServ,"%s requested more help from %s on %s",u->nick, s_StatServ, av[2]);
        }
        if (ac < 3) {
            privmsg_list(u->nick, s_StatServ, ss_help);
            if (UserLevel(u) >= 150)
                privmsg_list(u->nick, s_StatServ, ss_myuser_help);
        } else if (!strcasecmp(av[2], "SERVER"))
            privmsg_list(u->nick, s_StatServ, ss_server_help);
        else if (!strcasecmp(av[2], "RESET") && UserLevel(u) >= 190)
            privmsg_list(u->nick, s_StatServ, ss_reset_help);
        else if (!strcasecmp(av[2], "MAP"))
            privmsg_list(u->nick, s_StatServ, ss_map_help);
        else if (!strcasecmp(av[2], "JOIN") && UserLevel(u) >= 190)
            privmsg_list(u->nick, s_StatServ, ss_join_help);
        else if (!strcasecmp(av[2], "NETSTATS"))
            privmsg_list(u->nick, s_StatServ, ss_netstats_help);
        else if (!strcasecmp(av[2], "DAILY"))
            privmsg_list(u->nick, s_StatServ, ss_daily_help);
        else if (!strcasecmp(av[2], "HTMLSTATS"))
            privmsg_list(u->nick, s_StatServ, ss_htmlstats_help);
        else if (!strcasecmp(av[2], "FORCEHTML"))
            privmsg_list(u->nick, s_StatServ, ss_forcehtml_help);
        else if (!strcasecmp(av[2], "NOTICES"))
            privmsg_list(u->nick, s_StatServ, ss_notices_help);
        else if (!strcasecmp(av[2], "TLD"))
            privmsg_list(u->nick, s_StatServ, ss_tld_help);
        else if (!strcasecmp(av[2], "TLDMAP"))
            privmsg_list(u->nick, s_StatServ, ss_tld_map_help);
        else if (!strcasecmp(av[2], "OPERLIST"))
            privmsg_list(u->nick, s_StatServ, ss_operlist_help);
        else if (!strcasecmp(av[2], "BOTLIST"))
            privmsg_list(u->nick, s_StatServ, ss_botlist_help);
        else if (!strcasecmp(av[2], "VERSION"))
            privmsg_list(u->nick, s_StatServ, ss_version_help);
        else if (!strcasecmp(av[2], "STATS") && UserLevel(u) >= 190)
            privmsg_list(u->nick, s_StatServ, ss_stats_help);
        else
            privmsg(u->nick, s_StatServ, "Unknown Help Topic: \2%s\2", av[2]);
    } else if (!strcasecmp(av[1], "SERVER")) {
        ss_server(u, av[2]);
        if (notify_msgs) notice(s_StatServ,"%s Wanted Server Information on %s",u->nick, av[2]);
    } else if (!strcasecmp(av[1], "JOIN") && (UserLevel(u) >= 185)) {
        ss_JOIN(u, av[2]);
    } else if (!strcasecmp(av[1], "MAP")) {
        ss_map(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to see the Current Network MAP",u->nick);
    } else if (!strcasecmp(av[1], "VERSION")) {
        ss_version(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to know our version number ",u->nick);
    } else if (!strcasecmp(av[1], "NETSTATS")) {
        ss_netstats(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to see the NetStats ",u->nick);
    } else if (!strcasecmp(av[1], "DAILY")) {
        ss_daily(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to see the Daily NetStats ",u->nick);
    } else if ((!strcasecmp(av[1], "HTMLSTATS")) && (UserLevel(u) >= 185)) {
	    /* Check For Minimum Requirements */
	    if (ac < 4) {
	      privmsg(u->nick, s_StatServ, "HTMLSTATS Syntax Not Valid");
	      privmsg(u->nick, s_StatServ, "For addtional help: /msg %s HELP HTMLSTATS", s_StatServ);
	      return 1;
	    }
        ss_htmlsettings(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "FORCEHTML") && (UserLevel(u) >= 185)) {
        log("%s!%s@%s Forced an update of the NeoStats Statistics HTML file with the most current statistics", u->nick, u->username, u->hostname);
        notice(s_StatServ,"%s Forced the NeoStats Statistics HTML file to be updated with the most current statistics",u->nick);
        ss_chkhtml();
    } else if (!strcasecmp(av[1], "NOTICES") && (UserLevel(u) >= 185)) {
        ss_notices(u);
    } else if (!strcasecmp(av[1], "TLD")) {
        ss_tld(u, av[2]);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to find the Country that is Represented by %s ",u->nick,av[2]);
    } else if (!strcasecmp(av[1], "TLDMAP")) {
        ss_tld_map(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to see a Country Breakdown",u->nick);
    } else if (!strcasecmp(av[1], "OPERLIST")) {
	if (ac < 4) {
		privmsg(u->nick, s_StatServ, "OperList Syntax Not Valid");
		privmsg(u->nick, s_StatServ, "For Help: /msg %s HELP OPERLIST", s_StatServ);
	}
        ss_operlist(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "BOTLIST")) {
        ss_botlist(u);
        if (notify_msgs) notice(s_StatServ,"%s Wanted to see the Bot List",u->nick);
    } else if (!strcasecmp(av[1], "STATS") && (UserLevel(u) >= 185)) {
	if (ac < 5) {
		privmsg(u->nick, s_StatServ, "Incorrect Syntax: /msg %s HELP STATS", s_StatServ);
	}
        ss_stats(u, av[2], av[3], av[4]);
        if (notify_msgs) notice(s_StatServ,"%s Wants to Look at my Stats!! 34/24/34",u->nick);
    } else if (!strcasecmp(av[1], "RESET") && (UserLevel(u) >= 185)) {
        if (notify_msgs) notice(s_StatServ,"%s Wants me to RESET the databases.. here goes..",u->nick);
        ss_reset(u);
    } else {
        privmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2", av[1]);
        if (notify_msgs) notice(s_StatServ,"%s Reqested %s, but that is a Unknown Command",u->nick,av[1]);
    }
    return 1;
}

void Is_Midnight() {
    time_t current = time(NULL);
    struct tm *ltm = localtime(&current);
    TLD *t;

    strcpy(segv_location, "StatServ-Is_Midnight");


    if (ltm->tm_hour == 0) {
        if (ltm->tm_min == 0) {
            /* its Midnight! */
            notice(s_StatServ, "Reseting Daily Statistics - Its Midnight here!");
            log("Resetting Daily Statistics");
            daily.servers = stats_network.servers;
            daily.t_servers = time(NULL);
            daily.users = stats_network.users;
            daily.t_users = time(NULL);
            daily.opers = stats_network.opers;
            daily.t_opers = time(NULL);
            for (t = tldhead; t; t = t->next) 
                t->daily_users = stats_network.users;
                
        }
    }    
}
static void ss_tld_map(User *u) {
    TLD *t;
    
    strcpy(segv_location, "StatServ-ss_tld_map");


    privmsg(u->nick, s_StatServ, "Top Level Domain Statistics:");
    for (t = tldhead; t; t = t->next) {
        if (t->users != 0)
            privmsg(u->nick, s_StatServ, "%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d", t->tld, t->users, (float)t->users / (float)stats_network.users * 100, t->country, t->daily_users);
    }        
    privmsg(u->nick, s_StatServ, "End of List");
}    

static void ss_version(User *u)
{
    strcpy(segv_location, "StatServ-ss_version");

        privmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
        privmsg(u->nick, s_StatServ, "-------------------------------------");
        privmsg(u->nick, s_StatServ, "StatServ Version: %s Compiled %s at %s", Statserv_Info[0].module_version, version_date, version_time);
        privmsg(u->nick, s_StatServ, "http://www.neostats.net");
        privmsg(u->nick, s_StatServ, "-------------------------------------");
}
static void ss_netstats(User *u) {

    strcpy(segv_location, "StatServ-ss_netstats");

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

    strcpy(segv_location, "StatServ-ss_daily");

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

    strcpy(segv_location, "StatServ-ss_map");


    privmsg(u->nick, s_StatServ, "%-23s %-10s %-10s %-10s", "\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2",  "\2[LAG/MAX]\2");
    for (ss = Shead; ss; ss=ss->next) {
        s=findserver(ss->name);    
        if (s) privmsg(u->nick, s_StatServ, "\2%-23s [ %d/%d ]    [ %d/%d ]    [ %ld/%ld ]", ss->name, ss->users, ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);
    }
    privmsg(u->nick, s_StatServ, "--- End of Listing ---");
}

static void ss_server(User *u, char *server) {

    SStats *ss;
    Server *s;

    strcpy(segv_location, "StatServ-ss_server");


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

    strcpy(segv_location, "StatServ-ss_tld");


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
    register int j = 0;
    int away = 0;
    register User *u;
    int tech = 0;
    hscan_t scan;
    hnode_t *node;

    strcpy(segv_location, "StatServ-ss_operlist");


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
    hash_scan_begin(&scan, uh);
    while ((node = hash_scan_next(&scan)) != NULL) {
    	    u = hnode_get(node);
            tech = UserLevel(u);
            if (away && u->is_away)
                continue;
            if (!strcasecmp(u->server->name, me.services_name))
                continue;
            if (tech < 40)
                continue;
            if (!server) {
                if (UserLevel(u) < 40)    continue;
                j++;
                privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
                    u->server->name, tech);
                continue;
            } else {
                if (strcasecmp(server, u->server->name))    continue;
                if (UserLevel(u) < 40)    continue;
                j++;
                privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
                    u->server->name, tech);
                continue;
            }
    }
    privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_botlist(User *origuser)
{
        register int j = 0;
        register User *u;
	hscan_t scan;
	hnode_t *node;
        strcpy(segv_location, "StatServ-ss_botlist");


        privmsg(origuser->nick, s_StatServ, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
#ifdef UNREAL
                if (u->Umode & UMODE_BOT) {
#elif ULTIMATE
		if ((u->Umode & UMODE_RBOT) || (u->Umode & UMODE_SBOT)) {
#endif
	                j++;
                        privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick, u->server->name);
                        continue;
       		}
        }       
        privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_stats(User *u, char *cmd, char *arg, char *arg2)
{
    SStats *st;

    strcpy(segv_location, "StatServ-ss_stats");


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

    strcpy(segv_location, "StatServ-ss_reset");


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

    strcpy(segv_location, "StatServ-ss_JOIN");


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
    spart_cmd(s_StatServ, me.chan);
    log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
    sjoin_cmd(s_StatServ, chan);
    schmode_cmd(me.name, chan, "+o", s_StatServ);
}

void DelTLD(User *u) {
    TLD *t = NULL;
    char *m;

    strcpy(segv_location, "StatServ-DelTLD");
    

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

    strcpy(segv_location, "StatServ-findtld");


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

    strcpy(segv_location, "StatServ-AddTLD");

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

    strcpy(segv_location, "StatServ-LoadTLD");


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

        if ((tmp2 = strchr(buf2, '\n')))    *tmp2 = '\0';

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

    strcpy(segv_location, "StatServ-init_tld");


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

    strcpy(segv_location, "StatServ-SStats");


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
    log("add stats 1");
    strcpy(segv_location, "StatServ-AddStats");


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
    strcpy(segv_location, "StatServ-findstats");

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

    strcpy(segv_location, "StatServ-SaveStats");


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
    fprintf(fp, "%d %ld %d %ld %ld %ld %ld\n", stats_network.maxopers, stats_network.maxusers, stats_network.maxservers, stats_network.t_maxopers, stats_network.t_maxusers, stats_network.t_maxservers, stats_network.totusers);
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

    strcpy(segv_location, "StatServ-LoadStats");


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


/* Routine for Turning the HTMLSTATS 'ON' or 'OFF' */
static void ss_htmlsettings(User *u, char *cmd, char *m) {
    FILE *fp;
    FILE *ht;

    /* Approximate Segfault Location */
    strcpy(segv_location, "ss_htmlsettings");


    if ((!strcasecmp(cmd, "ON")) && (m != NULL)) {
        fp = fopen("data/html.conf", "w");
        fprintf(fp, "HTMLDATA 1");
        fclose(fp);

        ht = fopen("data/html.path", "w");
        fprintf(ht, "%s", m);
        fclose(ht);

        log("%s!%s@%s Activated the HTML output function", u->nick, u->username, u->hostname);
        notice(s_StatServ,"%s Turned the HTML ouput function 'ON'",u->nick);
    return;
    }

    else if (!strcasecmp(cmd, "OFF")) {
        FILE *fp = fopen("data/html.conf", "w");
        fprintf(fp, "HTMLDATA 0");
        fclose(fp);

        log("%s!%s@%s De-Activated the HTML output function", u->nick, u->username, u->hostname);
        notice(s_StatServ,"%s Turned the HTML output function 'OFF'",u->nick);
    return;
    }

    else {
      privmsg(u->nick, s_StatServ, "HTMLSTATS Syntax Not Valid");
      privmsg(u->nick, s_StatServ, "For addtional help: /msg %s HELP HTMLSTATS", s_StatServ);
      return;
    }

}


/* Routine for Checking the HTML Config File */
void ss_chkhtml()
{

FILE *fp = fopen("data/html.conf", "r");
char buf[BUFSIZE];
char *tmp;
strcpy(segv_location, "ss_html_conf");

if (fp) {
   while (fgets(buf, BUFSIZE, fp)) {
          strip(buf);
          tmp = strtok(buf, " ");
          nm = atoi(strtok(NULL, " "));

          if ((!strcasecmp(tmp, "HTMLDATA")) && (nm)) {
               ss_html();
          }
     }

fclose(fp);
}

else {
  return;
}

return;
}


/* Routine for HTML stats output */
void ss_html()
{
    SStats *s;
    char *htmdat;
    char buf[BUFSIZE];
    FILE *fp;
    FILE *op;

    /* Approximate Segfault Location */
    strcpy(segv_location, "ss_html_output");

    /* Save Path to HTML File into htmdat var */
    fp = fopen("data/html.path", "r");

    htmdat = smalloc(strlen(buf));

    if (fp) {
       while (fgets(buf, BUFSIZE, fp)) {
              strip(buf);
              htmdat = strtok(buf, " ");
              }
        }

    fclose(fp);

    if (!fp) {
       log("ERROR: Unable to open the data/html.path file for reading");
       notice(s_StatServ,"Unable to open the data/html.path file for reading");
       return;
    }

    /* Open HTML Output file for writting */
    op = fopen(htmdat, "w");

    fprintf(fp, "<center><table border=\"0\" width=\"590\"><tr>\n");
    fprintf(fp, "<td colspan=\"4\"><center><b><font size=\"+1\">%s Current Network Statistics</center></b></font></td>\n", me.netname);
    fprintf(fp, "</tr><tr>\n");
    fprintf(fp, "<td colspan=\"4\"><center>\n");
    fprintf(fp, "<table border=\"0\">\n");

    /* Output of Online/Offline Servers */
    fprintf(fp, "</tr><tr>\n");

    for (s = Shead; s; s = s->next) {
        if (findserver(s->name)) {
            fprintf(fp, "<tr><td height=\"4\">Server: </td>\n");
            fprintf(fp, "<td height=\"4\"> %s (*) </td></tr>\n", s->name);
        } else {
            fprintf(fp, "<tr><td height=\"4\">Server: </td>\n");
            fprintf(fp, "<td height=\"4\"> %s </td></tr>\n", s->name);
        }
    }

    fprintf(fp, "</table></center>\n");
    fprintf(fp, "</td></tr>\n");
    fprintf(fp, "<tr><td colspan=\"4\"><center>(* indicates Server is online at the moment)</center>\n");
    fprintf(fp, "</td></tr>\n");

    /* Network Statistics To Date */
    fprintf(fp, "<tr><td colspan=\"4\"><br><center><b>Network Statistics:</b></center></td></tr>\n");
    fprintf(fp, "<td>Current Users: </td>\n");
    fprintf(fp, "<td> %ld </td>\n", stats_network.users);
    fprintf(fp, "<td>Maximum Users: </td>\n");
    fprintf(fp, "<td> %ld [%s] </td>\n", stats_network.maxusers, sftime(stats_network.t_maxusers));
    fprintf(fp, "<tr><td>Current Opers: </td>\n");
    fprintf(fp, "<td> %i </td>\n", stats_network.opers);
    fprintf(fp, "<td>Maximum Opers: </td>\n");
    fprintf(fp, "<td> %i [%s] </td></tr>\n", stats_network.maxopers, sftime(stats_network.t_maxopers));
    fprintf(fp, "<td>Current Servers: </td>\n");
    fprintf(fp, "<td> %d </td>\n", stats_network.servers);
    fprintf(fp, "<td>Maximum Servers: </td>\n");
    fprintf(fp, "<td> %d [%s] </td>\n", stats_network.maxservers, sftime(stats_network.t_maxservers));
    fprintf(fp, "<tr><td colspan=\"2\">Users Set Away: </td>\n");
    fprintf(fp, "<td colspan=\"2\"> %ld </td></tr>\n", stats_network.away);

    /* Current Daily Network Statistics */
    fprintf(fp, "<tr><td colspan=\"4\"><center><b><br>Daily Network Statistics:</b></center></td></tr>\n");
    fprintf(fp, "<tr><td colspan=\"2\">Max Daily Users: </td>\n");
    fprintf(fp, "<td colspan=\"2\"> %-2d %s </td></tr>\n", daily.users, sftime(daily.t_users));
    fprintf(fp, "<tr><td colspan=\"2\">Max Daily Opers: </td>\n");
    fprintf(fp, "<td colspan=\"2\"> %-2d %s </td></tr>\n", daily.opers, sftime(daily.t_opers));
    fprintf(fp, "<tr><td colspan=\"2\">Max Daily Servers: </td>\n");
    fprintf(fp, "<td colspan=\"2\"> %-2d %s </td></tr>\n", daily.servers, sftime(daily.t_servers));
    fprintf(fp, "<tr><td colspan=\"4\"><center>(All Daily Statistics are reset at Midnight)</center></td>\n");
    fprintf(fp, "</tr><tr>\n");

    /* Current WebStats Information */
    fprintf(fp, "<td colspan=\"4\"><br><center><b>StatServ Information:</b>\n");
    fprintf(fp, "<br> %s - %s Compiled %s at %s\n", me.name, Statserv_Info[0].module_version, version_date, version_time);
    fprintf(fp, "<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n");
    fprintf(fp, "</center></td>\n");
    fprintf(fp, "</tr></table>\n");

    /* Close HTML file and exit routine */
    fclose(fp);
    return;

}

/* Time Routine for the HTML stats output */
void     TimerWeb()
{
    /* Run HTMLSTATS Routine */

    strcpy(segv_location, "StatServ-TimerWeb");


    ss_chkhtml();
return;
}


/* Routine For Notices to be Enabled/Disabled */
static void ss_notices(User *u) {

    /* Approximate Segfault Location */
    strcpy(segv_location, "ss_notices");

    if (!notify_msgs) {
        notify_msgs = 1;
        notice(s_StatServ,"%s Enabled the Statserv information request notices",u->nick);
        } else {
            notify_msgs = 0;
            notice(s_StatServ,"%s Disabled the Statserv information request notices",u->nick);
       }

return;
}
