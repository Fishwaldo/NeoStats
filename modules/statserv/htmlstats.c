/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
** $Id$
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "channel.h"
#include "version.h"
#include "tld.h"
#include <fcntl.h>

typedef void (*htmlhandler) (void);

typedef struct htmlfunc {
	char* directive;
	htmlhandler handler;
}htmlfunc;

static void html_map (void);
static void html_srvlist (void);
static void html_srvlistdet (void);
static void html_netstats (void);
static void html_dailystats (void);
static void html_weeklystats (void);
static void html_monthlystats (void);
static void html_channeltop10members (void);
static void html_channeltop10joins (void);
static void html_channeltop10kicks (void);
static void html_channeltop10topics (void);
static void html_tldmap (void);
static void html_version (void);
static void html_title (void);
static void html_clientstats (void);

static FILE *opf;
static const char html_template[]="data/index.tpl";

static htmlfunc htmlfuncs[]=
{
	{"!MAP!", html_map},
	{"!SRVLIST!", html_srvlist},
	{"!SRVLISTDET!", html_srvlistdet},
	{"!NETSTATS!", html_netstats},
	{"!DAILYSTATS!", html_dailystats},
	{"!WEEKLYSTATS!", html_weeklystats},
	{"!MONTHLYSTATS!", html_monthlystats},
	{"!TOP10CHAN!", html_channeltop10members},
	{"!TOP10JOIN!", html_channeltop10joins},
	{"!TOP10KICKS!", html_channeltop10kicks},
	{"!TOP10TOPICS!", html_channeltop10topics},
	{"!TLDMAP!", html_tldmap},
	{"!VERSION!", html_version},
	{"!TITLE!", html_title},
	{"!CLIENTSTATS!", html_clientstats},
	{NULL, NULL},
};

static void html_title (void)
{
	fprintf (opf, "Network Statistics for %s", me.netname);
}

static void html_version (void)
{
	fputs (me.version, opf);
}

void put_copyright (void)
{
	fprintf (opf, "<br><br><center>Statistics last updated at %s<br>", sftime(time(NULL)));
	fprintf (opf, "<b>StatServ Information:</b>\n");
	fprintf (opf, "<br> %s compiled on %s at %s\n", me.name, version_date, version_time);
	fprintf (opf, "<br> %s compiled on %s at %s\n", module_info.name,
		module_info.build_date, module_info.build_time);
	fprintf (opf, "<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n");
	fprintf (opf, "</center></html>\n");
}

static void serverlisthandler (serverstat *ss, void *v)
{
	fprintf (opf, "<tr><td height=\"4\"></td>\n");
	fprintf (opf, "<td height=\"4\"><a href=#%s> %s (%s)</a></td></tr>\n",
		ss->name, ss->name, (ss->s) ? "ONLINE" : "OFFLINE");
}

static void html_srvlist (void)
{
	fprintf (opf, "<table border=0><tr><th colspan = 2>Server name</th></tr>");
	GetServerStats (serverlisthandler, NULL);
	fprintf (opf, "</table>");
}

static void serverlistdetailhandler (serverstat *ss, void *v)
{
	fprintf (opf, "<tr><th><a name=%s>Server:</th><th colspan = 2><b>%s</b></th></tr>\n",
		ss->name, ss->name);
	if (!ss->s) {
		fprintf (opf, "<tr><td>Last Seen:</td><td colspan = 2>%s</td></tr>\n",
			sftime(ss->ts_lastseen));
	} else {
		fprintf (opf,"<tr><td>Current Users:</td><td>%d (%d%%)</td><td>Max %d at %s</td></tr>\n",
			ss->users.current, (int)(((float) ss->users.current / (float) networkstats.users.current) * 100),
			ss->users.alltime.max, sftime(ss->users.alltime.ts_max));
		fprintf (opf,
			"<tr><td>Current Opers:</td><td>%d (%d%%)</td><td>Max %d at %s</td></tr>\n",
			ss->opers.current, (int)(((float) ss->opers.current / (float) networkstats.opers.current) * 100),
			ss->opers.alltime.max, sftime(ss->opers.alltime.ts_max));
	}
	fprintf (opf, "<tr><td>Total Users Connected:</td><td colspan = 2>%d</td></tr>",
		ss->users.alltime.runningtotal);
	fprintf (opf, "<tr><td>IrcOp Kills</td><td colspan = 2>%d</td></tr>", 
		ss->operkills.alltime.runningtotal);
	fprintf (opf, "<tr><td>Server Kills</td><td colspan = 2>%d</td></tr>",
		ss->serverkills.alltime.runningtotal);
	fprintf (opf, "<tr><td>Highest Ping</td><td>%d</td><td>at %s</td></tr>",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (ss->s)
		fprintf (opf, "<tr><td>Current Ping</td><td colspan = 2>%d</td></tr>",
			ss->s->server->ping);
	fprintf (opf, "<tr><td>Server Splits</td><td colspan = 2>%d</td></tr>",
		ss->splits.alltime.runningtotal);
}

static void html_srvlistdet (void)
{
	fprintf (opf, "<table border=0>");
	GetServerStats (serverlistdetailhandler, NULL);
	fprintf (opf, "</table>");
}

static void html_netstats (void)
{
	fprintf (opf, "<table border = 0>");
	fprintf (opf, "<tr><th><b></b></th><th><b>Total</b></th><th><b>Current</b></th><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n");
	fprintf (opf, "<td>Users:</td>\n");
	fprintf (opf, "<td>%d</td>\n", networkstats.users.alltime.runningtotal);
	fprintf (opf, "<td>%d (%d%%)</td>\n", networkstats.users.current,
		GetAllTimePercent (&networkstats.users));
	fprintf (opf, "<td>%d</td>\n", networkstats.users.alltime.average);
	fprintf (opf, "<td>%d</td>\n", networkstats.users.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.users.alltime.ts_max));
	fprintf (opf, "<tr><td>Channels:</td>\n");
	fprintf (opf, "<td>%d</td>\n", networkstats.channels.alltime.runningtotal);
	fprintf (opf, "<td>%i (%d%%)</td>\n", networkstats.channels.current,
		GetAllTimePercent (&networkstats.channels));
	fprintf (opf, "<td>%i</td>\n", networkstats.channels.alltime.average);
	fprintf (opf, "<td>%d</td>\n", networkstats.channels.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.channels.alltime.ts_max));
	fprintf (opf, "<tr><td>Opers:</td>\n");
	fprintf (opf, "<td>%d</td>\n", networkstats.opers.alltime.runningtotal);
	fprintf (opf, "<td>%i (%d%%)</td>\n", networkstats.opers.current,
		GetAllTimePercent (&networkstats.opers));
	fprintf (opf, "<td>%i</td>\n", networkstats.opers.alltime.average);
	fprintf (opf, "<td>%i</td>\n", networkstats.opers.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.opers.alltime.ts_max));
	fprintf (opf, "<td>Servers:</td>\n");
	fprintf (opf, "<td>%d</td>\n", networkstats.servers.alltime.runningtotal);
	fprintf (opf, "<td>%d (%d%%)</td>\n", networkstats.servers.current,
		GetAllTimePercent (&networkstats.servers));
	fprintf (opf, "<td>%d</td>\n", networkstats.servers.alltime.average);
	fprintf (opf, "<td>%d</td>\n", networkstats.servers.alltime.max);
	fprintf (opf, "<td>%s</td>\n", sftime(networkstats.servers.alltime.ts_max));
	fprintf (opf, "<tr><td colspan=\"3\">Users Set Away:</td>\n");
	fprintf (opf, "<td colspan=\"3\">%d</td></tr></table>\n", me.awaycount);
}

static void html_dailystats (void)
{
	fprintf (opf, "<table border = 0>");
	fprintf (opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n");
	fprintf (opf, "<tr><td>Users:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.daily.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetDailyPercent (&networkstats.users));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.daily.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.daily.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.users.daily.ts_max));
	fprintf (opf, "<tr><td>Channels:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.daily.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetDailyPercent (&networkstats.channels));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.daily.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.daily.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.channels.daily.ts_max));
	fprintf (opf, "<tr><td>Opers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.daily.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetDailyPercent (&networkstats.opers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.daily.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.daily.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.opers.daily.ts_max));
	fprintf (opf, "<tr><td>Servers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.daily.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetDailyPercent (&networkstats.servers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.daily.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.daily.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.servers.daily.ts_max));
	fprintf (opf, "</tr></table>\n");
}

static void html_weeklystats (void)
{
	fprintf (opf, "<table border = 0>");
	fprintf (opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n");
	fprintf (opf, "<tr><td>Users:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.weekly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetWeeklyPercent (&networkstats.users));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.weekly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.weekly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.users.weekly.ts_max));
	fprintf (opf, "<tr><td>Channels:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.weekly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetWeeklyPercent (&networkstats.channels));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.weekly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.weekly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.channels.weekly.ts_max));
	fprintf (opf, "<tr><td>Opers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.weekly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetWeeklyPercent (&networkstats.opers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.weekly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.weekly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.opers.weekly.ts_max));
	fprintf (opf, "<tr><td>Servers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.weekly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetWeeklyPercent (&networkstats.servers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.weekly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.weekly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.servers.weekly.ts_max));
	fprintf (opf, "</tr></table>\n");
}

static void html_monthlystats (void)
{
	fprintf (opf, "<table border = 0>");
	fprintf (opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n");
	fprintf (opf, "<tr><td>Users:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.monthly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetMonthlyPercent (&networkstats.users));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.monthly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.users.monthly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.users.monthly.ts_max));
	fprintf (opf, "<tr><td>Channels:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.monthly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetMonthlyPercent (&networkstats.channels));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.monthly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.channels.monthly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.channels.monthly.ts_max));
	fprintf (opf, "<tr><td>Opers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.monthly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetMonthlyPercent (&networkstats.opers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.monthly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.opers.monthly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.opers.monthly.ts_max));
	fprintf (opf, "<tr><td>Servers:</td>\n");
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.monthly.runningtotal);
	fprintf (opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetMonthlyPercent (&networkstats.servers));
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.monthly.average);
	fprintf (opf, "<td>%-2d</td>\n", networkstats.servers.monthly.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.servers.monthly.ts_max));
	fprintf (opf, "</tr></table>\n");
}

static void top10membershandler (channelstat *cs, void *v)
{
	fprintf (opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->c->users);
}

static void html_channeltop10members (void)
{
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th align=right>Members</th></tr>");
	GetChannelStats (top10membershandler, CHANNEL_SORT_MEMBERS, 10, 1, NULL);
	fprintf (opf, "</table>");
}

static void top10joinshandler (channelstat *cs, void *v)
{
	fprintf (opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->users.alltime.runningtotal);
}

static void html_channeltop10joins (void)
{
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th align=right>Total Joins</th></tr>");
	GetChannelStats (top10joinshandler, CHANNEL_SORT_JOINS, 10, 1, NULL);
	fprintf (opf, "</table>");
}

static void top10kickshandler (channelstat *cs, void *v)
{
	fprintf (opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->kicks.alltime.runningtotal);
}

static void html_channeltop10kicks (void)
{
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th>Total Kicks</th></tr>");
	GetChannelStats (top10kickshandler, CHANNEL_SORT_KICKS, 10, 1, NULL);
	fprintf (opf, "</table>");
}

static void top10topicshandler (channelstat *cs, void *v)
{
	fprintf (opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->topics.alltime.runningtotal);
}

static void html_channeltop10topics (void)
{
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th>Total Topics</th></tr>");
	GetChannelStats (top10topicshandler, CHANNEL_SORT_TOPICS, 10, 1, NULL);
	fprintf (opf, "</table>");
}

static void html_clientstats (void)
{
	ctcpversionstat *cv;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(versionstatlist, topcurrentversions)) {
		list_sort(versionstatlist, topcurrentversions);
	}
	cn = list_first(versionstatlist);
	fprintf (opf, "<table border = 0><tr><th>Version</th><th align=right>Count</th></tr>");
	for (i = 0; i < 10 && cn; i++) {
		cv = lnode_get(cn);
		fprintf (opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
			cv->name, cv->users.current);
		cn = list_next(versionstatlist, cn);
	}
	fprintf (opf, "</table>");
}

void HTMLTLDReport (TLD *tld, void *v)
{
	fprintf (opf, "<tr><td>%s</td><td>%s</td><td>%3d</td><td>%3d</td><td>%3d</td><td>%3d</td><td>%3d</td></tr>",
		tld->tld, tld->country, tld->users.current, tld->users.daily.runningtotal, tld->users.weekly.runningtotal, tld->users.monthly.runningtotal, tld->users.alltime.runningtotal);
}

static void html_tldmap (void)
{
	fprintf (opf, "<table border = 0><tr><th>tld</th><th>Country</th><th>Current</th><th>Day</th><th>Week</th><th>Month</th><th>All Time</th></tr>");
	GetTLDStats (HTMLTLDReport, NULL);
	fprintf (opf, "</table>");
}

void get_map(char *uplink, int level)
{
#define MAPBUFSIZE 512
	static char buf[MAPBUFSIZE];
	hscan_t hs;
	hnode_t *sn;
	Client *s;
	serverstat *ss;
	int i;

	hash_scan_begin(&hs, GetServerHash ());
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = GetServerModValue (s);

		if ((level == 0) && (s->uplinkname[0] == 0)) {
			/* its the root server */
			fprintf (opf, "<table border=0><tr><th>Server Name</th><th>Users/Max</th><th>Opers/Max</th><th>Ping/Max</th></tr>");
			fprintf (opf, "<tr><td>%s</td><td>%d/%d</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				s->server->ping, (int)ss->highest_ping);
			get_map(s->name, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplinkname)) {
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				ircsnprintf (buf, MAPBUFSIZE, "%s&nbsp&nbsp&nbsp&nbsp&nbsp|", buf);
			}
			fprintf (opf, "<tr><td>%s\\_%s</td><td>%d/%d</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				buf, ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				s->server->ping, (int)ss->highest_ping);
			get_map(s->name, level + 1);
		}
	}
}

static void html_map (void)
{
	get_map("", 0);
	fputs ("</TABLE>\n", opf);
}

int ss_html (void)
{
#define READBUFSIZE 512
	static char buf[READBUFSIZE];
	FILE *tpl;
	char *buftemp;
	char *bufptr;
	int gothtml = 0;
	htmlfunc* htmlfuncptr;

	tpl = fopen(html_template, "r");
	if (!tpl) {
		nlog (LOG_WARNING, "Failed to open StatServ HTML template %s.", html_template);
		irc_chanalert(ss_bot, "Failed to open StatServ HTML template %s.", html_template);
		return NS_SUCCESS;
	}
	opf = fopen(StatServ.htmlpath, "w");
	if (!opf) {
		nlog (LOG_WARNING, "Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath);
		irc_chanalert(ss_bot, "Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath);
		return NS_SUCCESS;
	}
	while (fgets(buf, READBUFSIZE, tpl)) {
		bufptr = buf;
		htmlfuncptr = htmlfuncs;
		while (htmlfuncptr->directive) {
			buftemp = strstr (bufptr, htmlfuncptr->directive);
			if (buftemp) {
				fwrite (bufptr, (int)buftemp - (int)bufptr, 1, opf);
				htmlfuncptr->handler();
				bufptr = buftemp + strlen (htmlfuncptr->directive);
			}		
			htmlfuncptr++;
		}
		buftemp = strstr (bufptr, "</HTML>");
		if (buftemp) {
			fwrite (bufptr, (int)buftemp - (int)bufptr, 1, opf);
			put_copyright();
			bufptr = buftemp;
			gothtml = 1;
		}
		fputs (bufptr, opf);
	}
	if (!gothtml) {
		put_copyright();
	}
	fclose (tpl);
	fclose (opf);
	return NS_SUCCESS;
}

int ss_cmd_forcehtml (CmdParams *cmdparams)
{
	nlog (LOG_NOTICE, "%s!%s@%s forced an update of the HTML file.",
		    cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname);
	ss_html();
	return NS_SUCCESS;
}
