/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: statserv.c,v 1.2 2000/02/04 00:48:16 fishwaldo Exp $
*/

#include "stats.h"
#include "statserv.h"
#include "stats.c"
#include "ss_help.c"

extern const char version_date[], version_time[];
static void ss_JOIN(User *u, char *chan);
static void ss_tld(User *u, char *tld);
static void ss_operlist(User *origuser, char *flags, char *server);
static void ss_botlist(User *origuser);
static void ss_version(User *u);
static int Online();
static void ss_cb_Config(char *, int);
static int new_m_version(char *av, char *tmp);

char s_StatServ[MAXNICK] = "StatServ";


Module_Info Statserv_Info[] = { {
	"StatServ",
	"Statistical Bot For NeoStats",
	"2.0"
} };

Functions StatServ_fn_list[] = { 
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL, 	0}
};


EventFnList StatServ_Event_List[] = {
	{"ONLINE", 	Online},
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
{ "STATSERV_HOST", ARG_STR, ss_cb_Config, 2}
};


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module StatServ Loaded, Version: %s %s %s",me.name,av,Statserv_Info[0].module_version,version_date,version_time);
	return 0;
}

void _init() {
	sts(":%s GLOBOPS :StatServ Module Loaded", me.name);
	
	LoadTLD();
	init_tld();

}

void _fini() {
	sts(":%s GLOBOPS :StatServ Module Unloaded", me.name);
	
}

int Online() {

/* We should go the the existing server/user lists and add them to stats, cause its possible 
   that this has been called after the server already connected */

   memcpy(StatServ.user, Servbot.user, 8);
   memcpy(StatServ.host, Servbot.host, MAXHOST);

   if (!config_read("stats.cfg", options) == 0) {
   	log("Error, Statserv could not be configured");
   	notice(s_Services, "Error, Statserv could not be configured");
	return -1;
   } else {
	   init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+Sd", Statserv_Info[0].module_name);
   }
   return 1;
   
}

void ss_cb_Config(char *arg, int configtype) {
	if (configtype == 0) {
		/* Nick */
		memcpy(StatServ.nick, arg, MAXNICK);
		memcpy(s_StatServ, StatServ.nick, MAXNICK);
		log("Statserv nick :%s ", arg);
	} else if (configtype == 1) {
		/* User */
		memcpy(StatServ.user, arg, 8);
	} else if (configtype == 2) {
		/* host */
		memcpy(StatServ.host, arg, MAXHOST);
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
 
	if (me.onlyopers && !u->is_oper) {
		privmsg(u->nick, s_StatServ,
			"This service is only available to IRCops.");
		notice ("%s Requested %s, but he is Not a Operator!", u->nick, coreLine);
		return -1;
	}
	cmd = strtok(coreLine, " ");

	if (!strcasecmp(cmd, "HELP")) {
		coreLine = strtok(NULL, " ");
		notice(s_StatServ,"%s is a Dummy and wanted help on %s",u->nick, coreLine);
		if (!coreLine) {
			privmsg_list(u->nick, s_StatServ, ss_help);
			if (u->is_tecmin)
				privmsg_list(u->nick, s_StatServ, ss_myuser_help);
		} else if (!strcasecmp(coreLine, "SERVER"))
			privmsg_list(u->nick, s_StatServ, ss_server_help);
                else if (!strcasecmp(coreLine, "RESET") && u->is_tecmin)
                        privmsg_list(u->nick, s_StatServ, ss_reset_help);
		else if (!strcasecmp(coreLine, "MAP"))
			privmsg_list(u->nick, s_StatServ, ss_map_help);
		else if (!strcasecmp(coreLine, "JOIN") && u->is_tecmin)
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
		else if (!strcasecmp(coreLine, "STATS") && u->is_tecmin)
			privmsg_list(u->nick, s_StatServ, ss_stats_help);
		else
			privmsg(u->nick, s_StatServ, "Unknown Help Topic: \2%s\2",
				coreLine);
/*	} else if (!strcasecmp(cmd, "SERVER")) {
		cmd = strtok(NULL, " ");
		ss_server(u, cmd);
		notice(s_StatServ,"%s Wanted Server Information on %s",u->nick, cmd);
*/
	} else if (!strcasecmp(cmd, "JOIN")) {
		cmd = strtok(NULL, " ");
		ss_JOIN(u, cmd);
/*	} else if (!strcasecmp(cmd, "MAP")) {
		ss_map(u);
		notice(s_StatServ,"%s Wanted to see the Current Network MAP",u->nick);		
*/
        } else if (!strcasecmp(cmd, "VERSION")) {
                ss_version(u);
                notice(s_StatServ,"%s Wanted to know our version number ",u->nick);
/*	} else if (!strcasecmp(cmd, "NETSTATS")) {
		ss_netstats(u);
		notice(s_StatServ,"%s Wanted to see the NetStats ",u->nick);
	} else if (!strcasecmp(cmd, "DAILY")) {
		ss_daily(u);
		notice(s_StatServ,"%s Wanted to see the Daily NetStats ",u->nick);
*/
	} else if (!strcasecmp(cmd, "TLD")) {
		cmd = strtok(NULL, " ");
		ss_tld(u, cmd);
		notice(s_StatServ,"%s Wanted to find the Country that is Represented by %s ",u->nick,cmd);
/*	} else if (!strcasecmp(cmd, "TLDMAP")) {
		ss_tld_map(u);
		notice(s_StatServ,"%s Wanted to see a Country Breakdown",u->nick);
*/
	} else if (!strcasecmp(cmd, "OPERLIST")) {
		char *t;
		cmd = strtok(NULL, " ");
		t = strtok(NULL, " ");
		ss_operlist(u, cmd, t);
        } else if (!strcasecmp(cmd, "BOTLIST")) {
                ss_botlist(u);
                notice(s_StatServ,"%s Wanted to see the Bot List",u->nick);
/*	} else if (!strcasecmp(cmd, "STATS")) {
		char *t, *m;
		m = strtok(NULL, " ");
		cmd = strtok(NULL, " ");
		t = strtok(NULL, " ");
		ss_stats(u, m, cmd, t);
		notice(s_StatServ,"%s Wants to Look at my Stats!! 34/24/34",u->nick);
        } else if (!strcasecmp(cmd, "RESET")) {
                notice(s_StatServ,"%s Wants me to RESET the databases.. here goes..",u->nick);
                ss_reset(u);
*/
	} else {
		privmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2",
			cmd);
		notice(s_StatServ,"%s Reqested %s, but that is a Unknown Command",u->nick,cmd);
	}
	return 1;
}

static void ss_version(User *u)
{
        privmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
        privmsg(u->nick, s_StatServ, "%s - %s", me.name, version,
                                me.name, version_date, version_time);
        privmsg(u->nick, s_StatServ, "http://codeworks.kamserve.com");
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
	int net = 0;

	if (!flags) {
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops:");
		notice ("%s Requested OperList",origuser->nick);
	}		

	if (flags && !strcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops (Not Away):");
		notice(s_StatServ,"%s Reqested Operlist of Non-Away Opers",origuser->nick);
	}
	if (flags && !strcasecmp(flags, "tech")) {
		tech = 1;
		flags = NULL;
		privmsg(origuser->nick, s_StatServ, "On-Line Tech Admins:");
		notice(s_StatServ,"%s Reqested a Technical Administrator List",origuser->nick);
	}
	if (flags && !strcasecmp(flags, "net")) {
		net = 1;
		flags = NULL;
		privmsg(origuser->nick, s_StatServ, "On-Line Net Admins:");
		notice(s_StatServ,"%s Reqested a Network Administrator List",origuser->nick);
	}	
	if (!away && flags && strchr(flags, '.'))
		server = flags;

	for (i = 0; i < U_TABLE_SIZE; i++) {
		for (u = userlist[i]; u; u = u->next) {
			if (away && u->is_away)
				continue;
			if (tech && u->is_tecmin == 0)
				continue;
			if (net && u->is_netmin == 0)
				continue;
			if (!strcasecmp(u->server->name, me.services_name))
				continue;
			if (u->is_oper != 1)
				continue;
			if (!server) {
				if (u->is_oper != 1)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick,
					u->server->name);
				continue;
			} else {
				if (strcasecmp(server, u->server->name))	continue;
				if (u->is_oper != 1)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick,
					u->server->name);
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
        int bot = 1;
        privmsg(origuser->nick, s_StatServ, "On-Line Bots:");
        notice(s_StatServ,"%s Reqested a Bot List",origuser->nick);
        for (i = 0; i < U_TABLE_SIZE; i++) {
                for (u = userlist[i]; u; u = u->next) {
                        if (bot && u->is_bot ==0)
                                continue;
                                j++;
                                privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick,
                                        u->server->name);
                                continue;
                }
        }       
        privmsg(origuser->nick, s_StatServ, "End of Listing.");
}

/*
static void ss_stats(User *u, char *cmd, char *arg, char *arg2)
{
	Stats *st;

	if (!u->is_tecmin) {
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
		for (st = shead; st; st = st->next) {
			privmsg(u->nick, s_StatServ, "[%-2d] %s", i, st->name);
			i++;
		}
		privmsg(u->nick, s_StatServ, "End of List.");
		log("%s requested STATS LIST.", u->nick);
	} else if (!strcasecmp(cmd, "DEL")) {
		Stats *m = NULL;
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
		for (st = shead; st; st = st->next) {
			if (!strcasecmp(arg, st->name))
				break;
			m = st;
		}
		if (m)
			m->next = st->next;
		else
			shead = st->next;
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
		s = findserver(st->name);
		if (s)
			s->stats = st;
		privmsg(u->nick, s_StatServ, "Moved database entry for %s to %s",
			arg, arg2);
		log("%s requested STATS COPY %s -> %s", u->nick, arg, arg2);
	} else {
		privmsg(u->nick, s_StatServ, "Invalid Argument.");
		privmsg(u->nick, s_StatServ, "For help, /msg %s HELP", s_StatServ);
	}
}
*/
/*
static void ss_reset(User *u)
{
        if (!u->is_tecmin) {
                log("Access Denied (RELOAD) to %s", u->nick);
                privmsg(u->nick, s_StatServ, "Access Denied.");
                return;
        }
        remove("data/nstats.db");
        remove("data/stats.db");
        globops(s_StatServ, "%s requested \2RESET\2 databases.", u->nick);
        log("%s requested RESET.", u->nick);
        globops(s_StatServ, "Rebuilding Statistics DataBase after RESET...");
}
*/
static void ss_JOIN(User *u, char *chan)
{
	if (!u->is_tecmin) {
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
	sprintf(me.chan,"%s",chan);
	sts(":%s JOIN %s",s_StatServ,chan);
	sts(":%s MODE %s +o %s",me.name,chan,s_StatServ);
}
