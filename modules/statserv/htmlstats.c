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

FILE *opf;

const char html_template[]="data/index.tpl";

void html_map (void);
void get_srvlist (void);
void get_srvlistdet (void);
void get_netstats (void);
void get_dailystats (void);
void get_weeklystats (void);
void get_monthlystats (void);
void get_chantop10 (void);
void get_chantop10eva (void);
void get_unwelcomechan (void);
void get_chantops (void);
void HTMLTLDMap (void);
void get_version (void);
void get_title (void);
void get_clientstats (void);


htmlfunc htmlfuncs[]=
{
	{"!MAP!", html_map},
	{"!SRVLIST!", get_srvlist},
	{"!SRVLISTDET!", get_srvlistdet},
	{"!NETSTATS!", get_netstats},
	{"!DAILYSTATS!", get_dailystats},
	{"!WEEKLYSTATS!", get_weeklystats},
	{"!MONTHLYSTATS!", get_monthlystats},
	{"!DAILYTOPCHAN!", get_chantop10},
	{"!TOP10CHAN!", get_chantop10eva},
	{"!TOP10KICKS!", get_unwelcomechan},
	{"!TOP10TOPICS!", get_chantops},
	{"!TLDMAP!", HTMLTLDMap},
	{"!VERSION!", get_version},
	{"!TITLE!", get_title},
	{"!CLIENTSTATS!", get_clientstats},
	{NULL, NULL},
};

void get_title()
{
	fprintf (opf, "Network Statistics for %s", me.netname);
}

void get_version()
{
	fputs (me.version, opf);
}

void put_copyright()
{
	fprintf (opf, "<br><br><center>Statistics last updated at %s<br>", sftime(time(NULL)));
	fprintf (opf, "<b>StatServ Information:</b>\n");
	fprintf (opf, "<br> %s compiled on %s at %s\n", me.name, version_date, version_time);
	fprintf (opf, "<br> %s compiled on %s at %s\n", module_info.name,
		module_info.build_date, module_info.build_time);
	fprintf (opf, "<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n");
	fprintf (opf, "</center></html>\n");
}

void get_srvlist()
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	fprintf (opf, "<table border=0><tr><th colspan = 2>Server name</th></tr>");
	hash_scan_begin(&hs, serverstathash);
	while ((sn = hash_scan_next(&hs))) {
		ss = hnode_get(sn);
		fprintf (opf, "<tr><td height=\"4\"></td>\n");
		fprintf (opf, "<td height=\"4\"><a href=#%s> %s (%s)</a></td></tr>\n",
			ss->name, ss->name, (ss->s) ? "ONLINE" : "OFFLINE");
	}
	fprintf (opf, "</table>");
}

void get_srvlistdet()
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;
	fprintf (opf, "<table border=0>");
	hash_scan_begin(&hs, serverstathash);
	while ((sn = hash_scan_next(&hs))) {
		ss = hnode_get(sn);
		fprintf (opf, "<tr><th><a name=%s>Server:</th><th colspan = 2><b>%s</b></th></tr>\n",
			ss->name, ss->name);
		if (!ss->s) {
			fprintf (opf, "<tr><td>Last Seen:</td><td colspan = 2>%s</td></tr>\n",
				sftime(ss->ts_lastseen));
		} else {
			fprintf (opf,"<tr><td>Current Users:</td><td>%d (%d%%)</td><td>Max %ld at %s</td></tr>\n",
				ss->users.current, (int)(((float) ss->users.current / (float) networkstats.users.current) * 100),
				ss->users.alltime.max, sftime(ss->users.alltime.ts_max));
			fprintf (opf,
				"<tr><td>Current Opers:</td><td>%d (%d%%)</td><td>Max %d at %s</td></tr>\n",
				ss->opers.current, (int)(((float) ss->opers.current / (float) networkstats.opers.current) * 100),
				ss->opers.alltime.max, sftime(ss->opers.alltime.ts_max));
		}
		fprintf (opf, "<tr><td>Total Users Connected:</td><td colspan = 2>%ld</td></tr>",
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
	fprintf (opf, "</table>");
}

void get_netstats()
{

	fprintf (opf, "<table border = 0>");
	fprintf (opf, "<tr><th><b></b></th><th><b>Total</b></th><th><b>Current</b></th><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n");
	fprintf (opf, "<td>Users:</td>\n");
	fprintf (opf, "<td>%ld</td>\n", networkstats.users.alltime.runningtotal);
	fprintf (opf, "<td>%ld (%d%%)</td>\n", networkstats.users.current,
		GetAllTimePercent (&networkstats.users));
	fprintf (opf, "<td>%ld</td>\n", networkstats.users.alltime.average);
	fprintf (opf, "<td>%ld</td>\n", networkstats.users.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.users.alltime.ts_max));
	fprintf (opf, "<tr><td>Channels:</td>\n");
	fprintf (opf, "<td>%ld</td>\n", networkstats.channels.alltime.runningtotal);
	fprintf (opf, "<td>%i (%d%%)</td>\n", networkstats.channels.current,
		GetAllTimePercent (&networkstats.channels));
	fprintf (opf, "<td>%i</td>\n", networkstats.channels.alltime.average);
	fprintf (opf, "<td>%ld</td>\n", networkstats.channels.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.channels.alltime.ts_max));
	fprintf (opf, "<tr><td>Opers:</td>\n");
	fprintf (opf, "<td>%ld</td>\n", networkstats.opers.alltime.runningtotal);
	fprintf (opf, "<td>%i (%d%%)</td>\n", networkstats.opers.current,
		GetAllTimePercent (&networkstats.opers));
	fprintf (opf, "<td>%i</td>\n", networkstats.opers.alltime.average);
	fprintf (opf, "<td>%i</td>\n", networkstats.opers.alltime.max);
	fprintf (opf, "<td>%s</td></tr>\n", sftime(networkstats.opers.alltime.ts_max));
	fprintf (opf, "<td>Servers:</td>\n");
	fprintf (opf, "<td>%ld</td>\n", networkstats.servers.alltime.runningtotal);
	fprintf (opf, "<td>%d (%d%%)</td>\n", networkstats.servers.current,
		GetAllTimePercent (&networkstats.servers));
	fprintf (opf, "<td>%d</td>\n", networkstats.servers.alltime.average);
	fprintf (opf, "<td>%d</td>\n", networkstats.servers.alltime.max);
	fprintf (opf, "<td>%s</td>\n", sftime(networkstats.servers.alltime.ts_max));
	fprintf (opf, "<tr><td colspan=\"3\">Users Set Away:</td>\n");
	fprintf (opf, "<td colspan=\"3\">%ld</td></tr></table>\n", me.awaycount);
}

void get_dailystats()
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

void get_weeklystats()
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

void get_monthlystats()
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

void get_chantop10()
{
	channelstat *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(channelstatlist, topcurrentchannel)) {
		list_sort(channelstatlist, topcurrentchannel);
	}
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th align=right>Members</th></tr>");
	cn = list_first(channelstatlist);
	for (i = 0; i < 10 && cn; i++) {
		cs = lnode_get(cn);
		/* only show hidden chans to operators */
		if (is_hidden_chan(cs->c)) {
			i--;
			cn = list_next(channelstatlist, cn);
			continue;
		}
		if (cs->c->users > 0)
			fprintf (opf, "<tr><td>%s</td><td align=right>%ld</td></tr>\n",
				cs->name, cs->c->users);
		cn = list_next(channelstatlist, cn);
	}
	fprintf (opf, "</table>");
}

void get_chantop10eva()
{
	channelstat *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(channelstatlist, topjoinrunningtotalchannel)) {
		list_sort(channelstatlist, topjoinrunningtotalchannel);
	}
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th align=right>Total Joins</th></tr>");
	cn = list_first(channelstatlist);
	for (i = 0; i < 10 && cn; i++) {
		cs = lnode_get(cn);
		/* only show hidden chans to operators */
		if (is_hidden_chan(cs->c)) {
			i--;
			cn = list_next(channelstatlist, cn);
			continue;
		}
		fprintf (opf, "<tr><td>%s</td><td align=right>%ld</td></tr>\n",
			cs->name, cs->users.alltime.runningtotal);
		cn = list_next(channelstatlist, cn);
	}
	fprintf (opf, "</table>");
}
void get_clientstats()
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

void HTMLTLDMap (void)
{
	fprintf (opf, "<table border = 0><tr><th>tld</th><th>Country</th><th>Current</th><th>Day</th><th>Week</th><th>Month</th><th>All Time</th></tr>");
	GetTLDStats (HTMLTLDReport, NULL);
	fprintf (opf, "</table>");
}

void get_unwelcomechan()
{
	channelstat *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(channelstatlist, topkickrunningtotalchannel)) {
		list_sort(channelstatlist, topkickrunningtotalchannel);
	}
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th>Total Kicks</th></tr>");
	cn = list_first(channelstatlist);
	for (i = 0; i < 10 && cn; i++) {
		cs = lnode_get(cn);
		/* only show hidden chans to operators */
		if (is_hidden_chan(cs->c)) {
			i--;
			cn = list_next(channelstatlist, cn);
			continue;
		}
		fprintf (opf, "<tr><td>%s</td><td align=right>%ld</td></tr>\n",
			cs->name, cs->kicks);
		cn = list_next(channelstatlist, cn);
	}
	fprintf (opf, "</table>");

}

void get_chantops()
{
	channelstat *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(channelstatlist, toptopicrunningtotalchannel)) {
		list_sort(channelstatlist, toptopicrunningtotalchannel);
	}
	fprintf (opf, "<table border = 0><tr><th>Channel</th><th>Total Topics</th></tr>");
	cn = list_first(channelstatlist);
	for (i = 0; i < 10 && cn; i++) {
		cs = lnode_get(cn);
		/* only show hidden chans to operators */
		if (is_hidden_chan(cs->c)) {
			i--;
			cn = list_next(channelstatlist, cn);
			continue;
		}
		fprintf (opf, "<tr><td>%s</td><td align=right>%ld</td></tr>\n",
			cs->name, cs->topics);
		cn = list_next(channelstatlist, cn);
	}
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
			fprintf (opf, "<tr><td>%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				s->server->ping, (int)ss->highest_ping);
			get_map(s->name, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplinkname)) {
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				ircsnprintf (buf, MAPBUFSIZE, "%s&nbsp&nbsp&nbsp&nbsp&nbsp|", buf);
			}
			fprintf (opf, "<tr><td>%s\\_%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				buf, ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				s->server->ping, (int)ss->highest_ping);
			get_map(s->name, level + 1);
		}
	}
}

void html_map()
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
