/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"

hash_t *serverstathash;

void AverageServerStatistics (void)
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	hash_scan_begin (&hs, serverstathash);
	while ((sn = hash_scan_next (&hs))) {
		ss = hnode_get (sn);
		AverageStatistic (&ss->users);
		AverageStatistic (&ss->opers);
	}
}

void ResetServerStatistics (void)
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	hash_scan_begin (&hs, serverstathash);
	while ((sn = hash_scan_next (&hs))) {
		ss = hnode_get (sn);
		ResetStatistic (&ss->users);
		ResetStatistic (&ss->opers);
	}
}

static serverstat *new_server_stat (const char *name)
{
	serverstat *ss;

	SET_SEGV_LOCATION();
	dlog (DEBUG2, "new_server_stat(%s)", name);
	if (hash_isfull (serverstathash)) {
		nlog (LOG_CRITICAL, "StatServ Server hash is full!");
		return NULL;
	}
	ss = ns_calloc (sizeof(serverstat));
	memcpy (ss->name, name, MAXHOST);
	hnode_create_insert (serverstathash, ss, ss->name);
	return ss;
}

static serverstat *findserverstats (char *name)
{
	serverstat *stats;

	stats = (serverstat *)hnode_find (serverstathash, name);
	if (!stats) {
		dlog (DEBUG2, "findserverstats (%s) - not found", name);
	}
	return stats;
}

void AddServerUser (Client *u)
{
	serverstat *ss;

	ss = GetServerModValue (u->uplink);
	if (IncStatistic (&ss->users)) {
		announce_record ("\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!",
			ss->name, ss->s->server->users);
	}
}

void DelServerUser (Client *u)
{
	serverstat *ss;

	ss = GetServerModValue (u->uplink);
	DecStatistic (&ss->users);
}

void AddServerOper (Client *u)
{
	serverstat *ss;

	ss = GetServerModValue (u->uplink);
	if (IncStatistic (&ss->opers)) {
		announce_record ("\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d opers!",
			ss->name, ss->opers.alltime.runningtotal);
	}
}

void DelServerOper (Client *u)
{
	serverstat *ss;

	ss = GetServerModValue (u->uplink);
	DecStatistic (&ss->opers);
}

static void AddServerStat (Client *s, void *v)
{
	serverstat *ss;

	dlog (DEBUG2, "AddServerStat(%s)", s);
	ss = findserverstats (s->name);
	if (!ss) {
		ss = new_server_stat (s->name);
	}
	ss->ts_lastseen = me.now;
	AddNetworkServer ();
	SetServerModValue (s, (void *)ss);
	ss->s = s;
}

int ss_event_server (CmdParams *cmdparams)
{
	AddServerStat (cmdparams->source, NULL);
	return NS_SUCCESS;
}

static void DelServerStat (Client* s)
{
	serverstat *ss;
	
	ss = (serverstat *) GetServerModValue (s);
	ss->numsplits ++;
	ClearServerModValue (s);
	ss->s = NULL;
}

int ss_event_squit (CmdParams *cmdparams)
{
	DelServerStat (cmdparams->source);
	return NS_SUCCESS;
}

static void UpdatePingStats (Client* s)
{
	serverstat *ss;

	ss = (serverstat *) GetServerModValue (s);
	if (!ss)
		return;
	if (s->server->ping > ss->highest_ping) {
		ss->highest_ping = s->server->ping;
		ss->t_highest_ping = me.now;
	}
	if (s->server->ping < ss->lowest_ping) {
		ss->lowest_ping = s->server->ping;
		ss->t_lowest_ping = me.now;
	}
	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if (s->server->ping > StatServ.lagtime) {
		announce_lag ("\2%s\2 is lagged out with a ping of %d", s->name, s->server->ping);
	}
}

int ss_event_pong (CmdParams *cmdparams)
{
	/* we don't want negative pings! */
	if (cmdparams->source->server->ping > 0) {
		UpdatePingStats (cmdparams->source);
	}
	return NS_SUCCESS;
}

static void makemap(char *uplink, Client * u, int level)
{
	static char buf[256];
	hscan_t hs;
	hnode_t *sn;
	Client *s;
	serverstat *ss;
	int i;

	hash_scan_begin (&hs, GetServerHash ());
	while ((sn = hash_scan_next (&hs))) {
		s = hnode_get (sn);
		ss = (serverstat *) GetServerModValue (s);
		if ((level == 0) && (s->uplinkname[0] == 0)) {
			/* its the root server */
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			irc_prefmsg (ss_bot, u,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, s->server->users, (int)ss->users.alltime.max,
				ss->opers.current, ss->opers.alltime.max, (long)s->server->ping, ss->highest_ping);
		} else if ((level > 0) && !ircstrcasecmp (uplink, s->uplinkname)) {
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				strlcat (buf, "     |", 256);
			}
			irc_prefmsg (ss_bot, u,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, s->server->users, (int)ss->users.alltime.max,
				ss->opers.current, ss->opers.alltime.max, (long)s->server->ping, ss->highest_ping);
		}
		makemap(s->name, u, level + 1);
	}
}

int ss_cmd_map (CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg (ss_bot, cmdparams->source, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2");
	makemap ("", cmdparams->source, 0);
	irc_prefmsg (ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

int ss_cmd_server_list (CmdParams *cmdparams)
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	irc_prefmsg (ss_bot, cmdparams->source, "Server listing:");
	hash_scan_begin (&hs, serverstathash);
	while ((sn = hash_scan_next (&hs))) {
		ss = hnode_get (sn);
		if (find_server(ss->name)) {
			irc_prefmsg (ss_bot, cmdparams->source, "%s (*)", ss->name);
		} else {
			irc_prefmsg (ss_bot, cmdparams->source, "%s", ss->name);
		}
	}
	irc_prefmsg (ss_bot,cmdparams->source, "(* indicates server is online)");
	irc_prefmsg (ss_bot,cmdparams->source, "End of list.");
	return NS_SUCCESS;
}


int ss_cmd_server (CmdParams *cmdparams)
{
	serverstat *ss;
	Client *s;
	char *server;

	SET_SEGV_LOCATION();
	if (cmdparams->ac ==0) {
		return ss_cmd_server_list (cmdparams);
	}
	server = cmdparams->av[0];
	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findserverstats (server);
	if (!ss) {
		nlog (LOG_CRITICAL, "Unable to find server statistics for %s", server);
		irc_prefmsg (ss_bot,cmdparams->source, 
			"Internal Error! Please Consult the Log file");
		return NS_SUCCESS;
	}
	irc_prefmsg (ss_bot, cmdparams->source, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->ts_start));
	s = ss->s;
	if (!s) {
		irc_prefmsg (ss_bot, cmdparams->source, "Server Last Seen: %s", 
			sftime(ss->ts_lastseen));
	} else {
		irc_prefmsg (ss_bot, cmdparams->source, "Current Users: %-3ld (%2.0f%%)", 
			(long) s->server->users, 
			(float) s->server->users / (float) networkstats.users.current * 100);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "Maximum users: %-3ld at %s",
		ss->users.alltime.max, sftime(ss->users.alltime.ts_max));
	irc_prefmsg (ss_bot, cmdparams->source, "Total users connected: %-3ld", ss->users.alltime.runningtotal);
	if (s) {
		irc_prefmsg (ss_bot, cmdparams->source, "Current opers: %-3ld", (long)ss->opers.current);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "Maximum opers: %-3ld at %s",
		(long)ss->opers.alltime.max, sftime(ss->opers.alltime.ts_max));
	irc_prefmsg (ss_bot, cmdparams->source, "IRCop kills: %d", ss->operkills);
	irc_prefmsg (ss_bot, cmdparams->source, "Server kills: %d", ss->serverkills);
	irc_prefmsg (ss_bot, cmdparams->source, "Lowest ping: %-3d at %s",
		(int)ss->lowest_ping, sftime(ss->t_lowest_ping));
	irc_prefmsg (ss_bot, cmdparams->source, "Higest ping: %-3d at %s",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) {
		irc_prefmsg (ss_bot, cmdparams->source, "Current Ping: %-3d", s->server->ping);
	}
	if (ss->numsplits >= 1) {
		irc_prefmsg (ss_bot, cmdparams->source, 
			"%s has split from the network %d time %s",
			ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	} else {
		irc_prefmsg (ss_bot, cmdparams->source, "%s has never split from the network.", 
			ss->name);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "***** End of Statistics *****");
	return NS_SUCCESS;
}

static int ss_stats_list (CmdParams *cmdparams)
{
	serverstat *ss;
	hnode_t *node;
	hscan_t scan;

	irc_prefmsg (ss_bot, cmdparams->source, "Servers statistics list:");
	hash_scan_begin (&scan, serverstathash);
	while ((node = hash_scan_next (&scan))) {
		ss = hnode_get (node);
		irc_prefmsg (ss_bot, cmdparams->source, "  %s", ss->name);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static int ss_stats_del (CmdParams *cmdparams)
{
	serverstat *ss;
	hnode_t *node;

	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	ss = findserverstats (cmdparams->av[1]);
	if (!ss) {
		irc_prefmsg (ss_bot, cmdparams->source, "%s is not in the database", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	if (ss->s) {
		irc_prefmsg (ss_bot, cmdparams->source, 
			"Cannot remove %s from the database, it is online!!", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	node = hash_lookup(serverstathash, cmdparams->av[1]);
	if (node) {
		ns_free (hnode_get (node));
		hash_delete (serverstathash, node);
		hnode_destroy (node);
		irc_prefmsg (ss_bot, cmdparams->source, "Removed %s from the database.",
			cmdparams->av[1]);
		nlog (LOG_NOTICE, "%s deleted stats for %s", cmdparams->source->name, cmdparams->av[1]);
	}
	return NS_SUCCESS;
}

static int ss_stats_copy (CmdParams *cmdparams)
{
	serverstat *dest;
	serverstat *src;

	if (cmdparams->ac < 3) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	dest = findserverstats (cmdparams->av[2]);
	if (dest)
	{
		if (dest->s) {
			irc_prefmsg (ss_bot, cmdparams->source, "Server %s is online!", cmdparams->av[2]);
			return NS_SUCCESS;
		}
		ns_free(dest);
	}
	src = findserverstats (cmdparams->av[1]);
	if (!src) {
		irc_prefmsg (ss_bot, cmdparams->source, "%s is not in the database", 
			cmdparams->av[1]);
		return NS_SUCCESS;
	}
	memcpy (dest, src, sizeof(serverstat));
	strlcpy (dest->name, cmdparams->av[2], MAXHOST);
	irc_prefmsg (ss_bot, cmdparams->source, "Moved database entry for %s to %s", 
		cmdparams->av[1], cmdparams->av[2]);
	nlog (LOG_NOTICE, "%s requested STATS COPY %s to %s", cmdparams->source->name, 
		cmdparams->av[1], cmdparams->av[2]);
	return NS_SUCCESS;
}

int ss_cmd_stats (CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	if (!ircstrcasecmp (cmdparams->av[0], "LIST")) {
		return ss_stats_list (cmdparams);
	} else if (!ircstrcasecmp (cmdparams->av[0], "DEL")) {
		return ss_stats_del (cmdparams);
	} else if (!ircstrcasecmp (cmdparams->av[0], "COPY")) {
		return ss_stats_copy (cmdparams);	
	}
	return NS_ERR_SYNTAX_ERROR;
}

static void SaveServer (serverstat *ss)
{
	dlog (DEBUG1, "Writing statistics to database for %s", ss->name);
	SetData ((void *)ss->ts_start, CFGINT, "ServerStats", ss->name, "ts_start");
	SetData ((void *)ss->ts_lastseen, CFGINT, "ServerStats", ss->name, "ts_lastseen");
	SetData ((void *)ss->numsplits, CFGINT, "ServerStats", ss->name, "Splits");
	SetData ((void *)ss->operkills, CFGINT, "ServerStats", ss->name, "OperKills");
	SetData ((void *)ss->serverkills, CFGINT, "ServerStats", ss->name, "ServerKills");

	SaveStatistic (&ss->users, "ServerStats", ss->name, "users");
	SaveStatistic (&ss->opers, "ServerStats", ss->name, "opers");
}

void SaveServerStats(void)
{
	serverstat *ss;
	hnode_t *sn;
	hscan_t hs;

	/* clear the old database */
	DelTable ("ServerStats");
	/* run through stats and save them */
	hash_scan_begin (&hs, serverstathash);
	while ((sn = hash_scan_next (&hs))) {
		ss = hnode_get (sn);
		SaveServer (ss);
	}
}

void LoadServerStats(void) 
{
	serverstat *ss;
	char **row;
	int count;

	if (GetTableData ("ServerStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			if (hash_isfull (serverstathash)) {
				nlog (LOG_CRITICAL, "StatServ server hash full");
				break;
			}
			ss = ns_calloc (sizeof(serverstat));
			strlcpy (ss->name, row[count], MAXHOST);
			GetData ((void *)&ss->ts_start, CFGINT, "ServerStats", ss->name, "ts_start");
			GetData ((void *)&ss->ts_lastseen, CFGINT, "ServerStats", ss->name, "ts_lastseen");
			GetData ((void *)&ss->numsplits, CFGINT, "ServerStats", ss->name, "Splits");
			GetData ((void *)&ss->operkills, CFGINT, "ServerStats", ss->name, "OperKills");
			GetData ((void *)&ss->serverkills, CFGINT, "ServerStats", ss->name, "ServerKills");

			LoadStatistic (&ss->users, "ServerStats", ss->name, "users");
			LoadStatistic (&ss->opers, "ServerStats", ss->name, "opers");

			dlog (DEBUG1, "Loaded statistics for %ss", ss->name);
			hnode_create_insert (serverstathash, ss, ss->name);
		}
	}       
	ns_free(row);                                 
}

void InitServerStats (void)
{
	serverstathash = hash_create (-1, 0, 0);
	LoadServerStats ();
	GetServerList (AddServerStat, NULL);
}

void FiniServerStats (void)
{
	serverstat *ss;
	hnode_t *sn;
	hscan_t hs;

	SaveServerStats();
	hash_scan_begin (&hs, serverstathash);
	while ((sn = hash_scan_next (&hs))) {
		ss = hnode_get (sn);
		ClearServerModValue (ss->s);
		hash_scan_delete(serverstathash, sn);
		hnode_destroy(sn);
		ns_free (ss);
	}
	hash_destroy(serverstathash);
}
