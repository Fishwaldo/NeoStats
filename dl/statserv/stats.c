/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: stats.c,v 1.31 2003/01/06 12:07:27 fishwaldo Exp $
*/

#include "statserv.h"

int ok_to_wallop() {
	static int lasttime;
	static int count;
	
	if (StatServ.newdb || !StatServ.onchan || !me.synced) {
		return -1;
	}
	if (time(NULL) - lasttime < StatServ.interval) {
		if (++count > 5)
			return -1;
	} else {
		lasttime = time(NULL);
		count = 0;
	}
	return 1;
	

}



int s_chan_new(char **av, int ac) {
	long count;
	IncreaseChans();
	count = hash_count(ch);
	if (count > stats_network.maxchans) {
		stats_network.maxchans = count;
		stats_network.t_chans = time(NULL);
		if (ok_to_wallop() > 0) swallops_cmd(s_StatServ, "\2NEW CHANNEL RECORD\2 Wow, there is now %d Channels on the Network", stats_network.maxchans);
	}
	if (count > daily.chans) {
		daily.chans = count;
		daily.t_chans = time(NULL);
	}
	return 1;
}

int s_chan_del(char **av, int ac) {
	DecreaseChans();
	return 1;
}

int s_chan_join(char **av, int ac) {
	CStats *cs;
	cs = findchanstats(av[0]);
	if (cs) {
		Increasemems(cs);
		if (cs->maxmemtoday < cs->members) {
			cs->maxmemtoday = cs->members;
			cs->t_maxmemtoday = time(NULL);
		}
		if (cs->maxmems < cs->maxmemtoday) {
			cs->maxmems = cs->members;
			cs->t_maxmems = time(NULL);
		}
		if (cs->maxjoins < cs->joinstoday) {
			cs->maxjoins = cs->joinstoday;
			cs->t_maxjoins = time(NULL);
		}
	} else {
		cs = AddChanStats(av[0]);		
		Increasemems(cs);
		cs->maxmemtoday++;
		cs->t_maxmemtoday = time(NULL);
		cs->maxmems++;
		cs->t_maxmems = time(NULL);
	}
	return 1;
}			
int s_chan_part(char **av, int ac) {
	CStats *cs;
	cs = findchanstats(av[0]);
	if (cs) {
		Decreasemems(cs);
	}
	return 1;
}

int s_topic_change(char **av, int ac) {
	CStats *cs;
	cs = findchanstats(av[0]);
	if (cs) {
		IncreaseTops(cs);
	}
	return 1;
}
int s_chan_kick(char **av, int ac) {
	CStats *cs;
	cs = findchanstats(av[0]);
	if (cs) {
		IncreaseKicks(cs);
		if (cs->maxkicks < cs->maxkickstoday) {
			cs->maxkicks = cs->maxkickstoday;
			cs->t_maxkicks = time(NULL);
		}
	}	
	return 1;	

}

CStats *findchanstats(char *name) {
	CStats *cs;
	lnode_t *cn;
	cn = list_find(Chead, name, comparef);
	if (cn) {
		cs = lnode_get(cn);
	} else {
#ifdef DEBUG
		log("findchanstats(%s) -> NOT FOUND", name);
#endif
		return NULL;
	}
	return cs;
}
void DelOldChan() {
	lnode_t *cn, *cn1;
	CStats *c;
	cn = list_first(Chead);
	while (cn) {
		c = lnode_get(cn);
		if ((c->members <= 0 ) && ((time(NULL) - c->lastseen) < 604800)) {
			if (!findchan(c->name)) {
				log("StatServ: Deleting Old Channel %s", c->name);
				cn1 = cn;
				cn = list_next(Chead, cn);
				list_delete(Chead, cn1);
        			lnode_destroy(cn1);
				free(c);
				continue;
			}
		}
		cn = list_next(Chead, cn);
	}
}

CStats *AddChanStats(char *name) {
	CStats *cs;
	lnode_t *cn;

	cs = malloc(sizeof(CStats));
	strcpy(cs->name, name);
	cs->members = 0;
	cs->topics = 0;
	cs->totmem = 0;
	cs->kicks = 0;
	cs->topicstoday = 0;
	cs->joinstoday = 0;
	cs->maxkickstoday = 0;
	cs->maxmemtoday = 0;
	cs->t_maxmemtoday = 0;
	cs->maxmems  = 0;
	cs->t_maxmems = 0;
	cs->maxkicks = 0;
	cs->t_maxkicks = 0;
	cs->maxjoins = 0;
	cs->t_maxjoins = 0;
	cs->lastseen = time(NULL);
	cn = lnode_create(cs);
	if (list_isfull(Chead)) {
		log("Eeek, Can't add Channel to Statserv Channel Hash. Has is full");
	} else {
		list_append(Chead, cn);
	}	
	return cs;
}

int s_new_server(char **av, int ac) {
	Server *s;
	strcpy(segv_location, "StatServ-s_new_server");
	s = findserver(av[0]);
	if (!s) return 0;
	AddStats(s);
	IncreaseServers();
	if (stats_network.maxservers < stats_network.servers) {
		stats_network.maxservers = stats_network.servers;
		stats_network.t_maxservers = time(NULL);
		if (ok_to_wallop() > 0) swallops_cmd(s_StatServ, "\2NEW SERVER RECORD\2 Wow, there are now %d Servers on the Network", stats_network.servers); 
	}
	if (stats_network.servers > daily.servers) {
	daily.servers = stats_network.servers;
	daily.t_servers = time(NULL);
	}
	if (ok_to_wallop() > 0) chanalert(s_StatServ, "\2SERVER\2 %s has joined the Network at %s", s->name, s->uplink);
	return 1;

}

int s_del_server(char **av, int ac) {
	SStats *ss;
	Server *s;
	strcpy(segv_location, "StatServ-s_del_server");
	s = findserver(av[0]);
	if (!s) return 0;
	DecreaseServers();
	if (ok_to_wallop() > 0) chanalert(s_StatServ, "\2SERVER\2 %s has left the Network at %s", s->name, s->uplink);
	ss = findstats(s->name);
	if (s->name != me.uplink)
	ss->numsplits = ss->numsplits +1;
	return 1;

}


int s_user_kill(char **av, int ac) {
	SStats *s;
	SStats *ss;
	char *cmd, *who;
	User *u;
	u = finduser(av[0]);
	if (!u) return 0;
#ifdef DEBUG
	log(" Server %s", u->server->name);
#endif

	strcpy(segv_location, "StatServ-s_user_kill");

	s=findstats(u->server->name);
	if (is_oper(u)) {
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
	ss = findstats(u->server->name); 
	ss->operkills = ss->operkills +1; 
	} else if (findserver(who)) {
		ss = findstats(who);
		ss->serverkills = ss->serverkills +1;
	}
	return 1;
}

int s_user_modes(char **av, int ac) {
	int add = 1;
	char *modes;
	SStats *s;
	User *u;
	
	strcpy(segv_location, "StatServ-s_user_modes");

	u = finduser(av[0]);
	if (!u) {
		log("Changing modes for unknown user: %s", u->nick);
		return -1;
	}
	if (!u->modes) return -1; 
	modes = u->modes;
	while (*modes) {
		switch(*modes) {
			case '+': add = 1;	break;
			case '-': add = 0;	break;
			case 'O':
			case 'o':
				if (add) {
					IncreaseOpers(findstats(u->server->name));
					s = findstats(u->server->name);
					if (stats_network.maxopers < stats_network.opers) {
						stats_network.maxopers = stats_network.opers;
						stats_network.t_maxopers = time(NULL);
						if (ok_to_wallop() > 0) swallops_cmd(s_StatServ, "\2Oper Record\2 The Network has reached a New Record for Opers at %d", stats_network.opers);
					}
					if (s->maxopers < s->opers) {
						s->maxopers = s->opers;
						s->t_maxopers = time(NULL);
						if (ok_to_wallop() > 0) swallops_cmd(s_StatServ, "\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers", s->name, s->opers);
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
		modes++;
	}
	return 1;
}

void re_init_bot() {
	strcpy(segv_location, "StatServ-re_init_bot");

	chanalert(s_Services, "Re-Initilizing %s Bot", s_StatServ);
	init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSwgle", SSMNAME);
}
int s_del_user(char **av, int ac) {
	SStats *s;
	User *u;
	u = finduser(av[0]);
	if (!u) return 0;

#ifdef DEBUG
	log(" Server %s", u->server->name);
#endif
	s=findstats(u->server->name);

	if (!u->modes) return -1; 
	if (is_oper(u)) {
		DecreaseOpers(s);
	}
	DecreaseUsers(s);
	DelTLD(u);
	return 1;
}


int s_user_away(char **av, int ac) {
	User *u;
	strcpy(segv_location, "StatServ-s_user_away");
	u = finduser(av[0]);
	if (!u) return 0;
	if (u->is_away) {
		stats_network.away = stats_network.away +1;
	} else {
		stats_network.away = stats_network.away -1;
	}
	return 1;

}



int s_new_user(char **av, int ac) {
	SStats *s;
	User *u;
	u = finduser(av[0]);
	if (!u) return 0;
	
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
		if (ok_to_wallop() > 0)
			 swallops_cmd(s_StatServ, "\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!", s->name, s->users);	
			 
	}

	if (stats_network.maxusers < stats_network.users) {
		stats_network.maxusers = stats_network.users;
		stats_network.t_maxusers = time(NULL);
		if (ok_to_wallop() > 0) swallops_cmd(s_StatServ, "\2NEW NETWORK RECORD!\2 Wow, a New Global User record has been reached with %d users!", stats_network.users);
	}

	if (stats_network.users > daily.users) {
	daily.users = stats_network.users;
 	daily.t_users = time(NULL);
	}


	AddTLD(u);
	return 1;
}

int pong(char **av, int ac) {
	SStats *ss;
	Server *s;

	strcpy(segv_location, "StatServ-pong");
	s = findserver(av[0]);
	if (!s) return 0;

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
			if (ok_to_wallop() > 0) globops(s_StatServ, "\2%s\2 is Lagged out with a ping of %d", s->name, s->ping);
		}
	}
	return 1;
}

int Online(char **av, int ac) {

	strcpy(segv_location, "StatServ-Online");

   #ifdef ULTIMATE3
	   init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSwgle", SSMNAME);
   #else
	   init_bot(s_StatServ, StatServ.user,StatServ.host,"/msg Statserv HELP", "+oikSdwgle", SSMNAME);
   #endif
   StatServ.onchan = 1;
   /* now that we are online, setup the timer to save the Stats database every so often */
   add_mod_timer("SaveStats", "Save_Stats_DB", SSMNAME, 600);

   /* Add a timer for HTML writeouts */
   if (StatServ.html) {
	   add_mod_timer("ss_html", "TimerWeb", SSMNAME, 3600);
   }

   /* also add a timer to check if its midnight (to reset the daily stats */
   add_mod_timer("Is_Midnight", "Daily_Stats_Reset", SSMNAME, 60);
   add_mod_timer("DelOldChan", "DelOldStatServChans", SSMNAME, 3600);


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
	lnode_t *cn;
	CStats *c;

	strcpy(segv_location, "StatServ-Is_Midnight");


	if (ltm->tm_hour == 0) {
		if (ltm->tm_min == 0) {
			/* its Midnight! */
			chanalert(s_StatServ, "Reseting Daily Statistics - Its Midnight here!");
			log("Resetting Daily Statistics");
			daily.servers = stats_network.servers;
			daily.t_servers = time(NULL);
			daily.users = stats_network.users;
			daily.t_users = time(NULL);
			daily.opers = stats_network.opers;
			daily.t_opers = time(NULL);
			daily.chans = stats_network.chans;
			daily.t_chans = time(NULL);
			for (t = tldhead; t; t = t->next) 
				t->daily_users = 0;
			cn = list_first(Chead);
			while (cn) {
				c = lnode_get(cn);
				c->maxmemtoday = c->members;;
				c->joinstoday = 0;
				c->maxkickstoday = 0;
				c->topicstoday = 0;
				c->t_maxmemtoday = time(NULL);
				cn = list_next(Chead, cn);
			}	
			
		}
	}	
}
