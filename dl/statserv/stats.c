/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      stats.c, 
** Version: 3.1
** Date:    08/03/2002
*/

#include "statserv.h"

void s_chan_new(Chans *c) {
	long count;
	
	IncreaseChans();
	count = hash_count(ch);
	if (count > stats_network.maxchans) {
		stats_network.maxchans = count;
		stats_network.t_chans = time(NULL);
		if (StatServ.onchan) swallops_cmd(s_StatServ, "\2NEW CHANNEL RECORD\2 Wow, there is now %d Channels on the Network", stats_network.maxchans);
	}
	if (count > daily.chans) {
		daily.chans = count;
		daily.t_chans = time(NULL);
	}
}

void s_chan_del(Chans *c) {
	DecreaseChans();
}

int s_new_server(Server *s) {
	strcpy(segv_location, "StatServ-s_new_server");

	AddStats(s);
	IncreaseServers();
	if (stats_network.maxservers < stats_network.servers) {
		stats_network.maxservers = stats_network.servers;
		stats_network.t_maxservers = time(NULL);
		if (StatServ.onchan) swallops_cmd(s_StatServ, "\2NEW SERVER RECORD\2 Wow, there are now %d Servers on the Network", stats_network.servers); 
	}
	if (stats_network.servers > daily.servers) {
	daily.servers = stats_network.servers;
	daily.t_servers = time(NULL);
	}
	if (StatServ.onchan) notice(s_StatServ, "\2SERVER\2 %s has joined the Network at %s", s->name, s->uplink);
	return 1;

}

extern int s_del_server(Server *s) {
	SStats *ss;

	strcpy(segv_location, "StatServ-s_del_server");

	DecreaseServers();
	if (StatServ.onchan) notice(s_StatServ, "\2SERVER\2 %s has left the Network at %s", s->name, s->uplink);
	ss = findstats(s->name);
	if (s->name != me.uplink)
	ss->numsplits = ss->numsplits +1;
	return 1;

}


extern int s_user_kill(User *u) {
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
		if (StatServ.onchan) notice(s_StatServ, "\2KILL\2 %s was Killed by %s --> %s", u->nick, who, cmd);
	ss = findstats(u->server->name); 
	ss->operkills = ss->operkills +1; 
	} else if (findserver(who)) {
		if (StatServ.onchan) notice(s_StatServ, "\2SERVER KILL\2 %s was Killed by the Server %s --> %s", u->nick, who, cmd);
	ss = findstats(who);
	ss->serverkills = ss->serverkills +1;
	}
	return 1;
}

extern int s_user_modes(User *u) {
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
	while (*modes++) {

		switch(*modes) {
			case '+': add = 1;	break;
			case '-': add = 0;	break;
			case 'o':
				if (add) {
					IncreaseOpers(findstats(u->server->name));
					s = findstats(u->server->name);
					if (stats_network.maxopers < stats_network.opers) {
						stats_network.maxopers = stats_network.opers;
						stats_network.t_maxopers = time(NULL);
						if (StatServ.onchan) swallops_cmd(s_StatServ, "\2Oper Record\2 The Network has reached a New Record for Opers at %d", stats_network.opers);
					}
					if (s->maxopers < s->opers) {
						s->maxopers = s->opers;
						s->t_maxopers = time(NULL);
						if (StatServ.onchan) swallops_cmd(s_StatServ, "\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s->name, s->opers);
					}
					if (s->opers > daily.opers) {
						daily.opers = s->opers;
						daily.t_opers = time(NULL);
					}	
				} else {
					 DecreaseOpers(findstats(u->server->name));
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
	init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", SSMNAME);
}
extern int s_del_user(User *u) {
	SStats *s;
#ifdef DEBUG
	log(" Server %s", u->server->name);
#endif
	s=findstats(u->server->name);
	if (UserLevel(u) >= 40) {
		DecreaseOpers(s);
	}
	DecreaseUsers(s);
	DelTLD(u);
	return 1;
}


extern int s_user_away(User *u) {
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



extern int s_new_user(User *u) {
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
		if (StatServ.onchan) swallops_cmd(s_StatServ, "\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!", s->name, s->users);	
	}

	if (stats_network.maxusers < stats_network.users) {
		stats_network.maxusers = stats_network.users;
		stats_network.t_maxusers = time(NULL);
		if (StatServ.onchan) swallops_cmd(s_StatServ, "\2NEW NETWORK RECORD!\2 Wow, a New Global User record has been reached with %d users!", stats_network.users);
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
			if (StatServ.onchan) globops(s_StatServ, "\2%s\2 is Lagged out with a ping of %d", s->name, s->ping);
		}
	}
	return 1;
}

int Online(void *s) {

	strcpy(segv_location, "StatServ-Online");


   init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", SSMNAME);
   StatServ.onchan = 1;
   /* now that we are online, setup the timer to save the Stats database every so often */
   add_mod_timer("SaveStats", "Save_Stats_DB", SSMNAME, 600);

   /* Add a timer for HTML writeouts */
   if (StatServ.html) {
	   add_mod_timer("TimerWeb", "ss_html", SSMNAME, 3600);
   }

   /* also add a timer to check if its midnight (to reset the daily stats */
   add_mod_timer("Is_Midnight", "Daily_Stats_Reset", SSMNAME, 60);

   return 1;
   
}


extern SStats *new_stats(const char *name)
{
	hnode_t *sn;
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
	sn = hnode_create(s);
	if (hash_isfull(Shead)) {
		log("Eeek, StatServ Server hash is full!");
	} else {
		hash_insert(Shead, sn, s->name);
	}
	return s;
}

void AddStats(Server *s)
{
	SStats *st = findstats(s->name);
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
	hnode_t *sn;
#ifdef DEBUG
	log("findstats(%s)", name);
#endif
	strcpy(segv_location, "StatServ-findstats");

	sn = hash_lookup(Shead, name);
	if (sn) {
		return hnode_get(sn);
	} else {
		return NULL;
	}
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
