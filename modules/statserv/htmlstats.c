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
#include "hash.h"
#include <fcntl.h>


#define STARTBUFSIZE 8192
int bufsize;

void get_map();
void get_top10chan();
void put_copyright();
void get_srvlist();
void get_srvlistdet();
void get_netstats();
void get_dailystats();
void get_chantops();
void get_chantop10();
void get_chantop10eva();
void get_unwelcomechan();
void get_clientstats();
void get_title();
void get_tldmap();
FILE *tpl, *opf;

const char html_template[]="data/index.tpl";

int ss_html()
{
	char *buf;
	char *buf1;
	char *bufold;
	char startstr = 0;
	int gothtml = 0;
	
	if (StatServ.html) {
		if (StatServ.htmlpath[0] == 0) {
			nlog(LOG_WARNING,
				"Unable to write HTML: html path is not defined");
			return 1;
		}
	} else {
		return 1;
	}
	tpl = fopen(html_template, "r");
	if (!tpl) {
		nlog(LOG_WARNING,
			"Failed to open StatServ HTML template %s.", html_template);
		irc_chanalert(ss_bot, 
			"Failed to open StatServ HTML template %s.", html_template);
		return 1;
	}
	opf = fopen(StatServ.htmlpath, "w");
	if (!opf) {
		nlog(LOG_WARNING,
			"Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath);
		irc_chanalert(ss_bot,
			"Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath);
		return 1;
	}
	buf = smalloc(STARTBUFSIZE * 2);
	bufold = buf;
	buf1 = smalloc(STARTBUFSIZE * 2);
	while (fgets(buf, STARTBUFSIZE, tpl)) {

		buf1 = strstr(buf, "!MAP!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_map("", 0);
			fputs("</TABLE>\n", opf);
			buf = buf1 + strlen("!MAP!");
		}
		buf1 = strstr(buf, "!SRVLIST!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_srvlist();
			buf = buf1 + strlen("!SRVLIST!");
		}
		buf1 = strstr(buf, "!SRVLISTDET!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_srvlistdet();
			buf = buf1 + strlen("!SRVLISTDET!");

		}
		buf1 = strstr(buf, "</HTML>");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			put_copyright();
			buf = buf1;
			gothtml = 1;
		}
		buf1 = strstr(buf, "!NETSTATS!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_netstats();
			buf = buf1 + strlen("!NETSTATS!");
		}
		buf1 = strstr(buf, "!DAILYSTATS!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_dailystats();
			buf = buf1 + strlen("!DAILYSTATS!");
		}
		buf1 = strstr(buf, "!DAILYTOPCHAN!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_chantop10();
			buf = buf1 + strlen("!DAILYTOPCHAN!");
		}
		buf1 = strstr(buf, "!TOP10CHAN!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_chantop10eva();
			buf = buf1 + strlen("!TOP10CHAN!");
		}
		buf1 = strstr(buf, "!TOP10KICKS!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_unwelcomechan();
			buf = buf1 + strlen("!TOP10KICKS!");
		}
		buf1 = strstr(buf, "!TOP10TOPICS!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_chantops();
			buf = buf1 + strlen("!TOP10TOPICS!");
		}
		buf1 = strstr(buf, "!TLDMAP!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_tldmap();
			buf = buf1 + strlen("!TLDMAP!");
		}
		buf1 = strstr(buf, "!VERSION!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			fputs(me.version, opf);
			buf = buf1 + strlen("!VERSION!");
		}
		buf1 = strstr(buf, "!TITLE!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_title();
			buf = buf1 + strlen("!TITLE!");
		}
		buf1 = strstr(buf, "!CLIENTSTATS!");
		if (buf1) {
			startstr = strlen(buf) - strlen(buf1);
			fwrite(buf, startstr, 1, opf);
			get_clientstats();
			buf = buf1 + strlen("!CLIENTSTATS!");
		}


		fputs(buf, opf);
	}
	sfree(buf1);
	sfree(bufold);
	if (!gothtml) {
		put_copyright();
	}
	fclose(tpl);
	fclose(opf);
	return 1;
}

void get_title()
{
	fprintf(opf, "Network Statistics for %s", me.netname);
}

void put_copyright()
{
	fprintf(opf, "<br><br><center>Statistics last updated at %s<br>",
		sftime(time(NULL)));
	fprintf(opf, "<b>StatServ Information:</b>\n");
	fprintf(opf, "<br> %s compiled on %s at %s\n", me.name, version_date, version_time);
	fprintf(opf, "<br> %s compiled on %s at %s\n", module_info.name,
		module_info.build_date, module_info.build_time);
	fprintf(opf,
		"<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n");
	fprintf(opf, "</center></html>\n");
}

void get_srvlist()
{
	SStats *s;
	hscan_t hs;
	hnode_t *sn;

	fprintf(opf,
		"<table border=0><tr><th colspan = 2>Server name</th></tr>");
	hash_scan_begin(&hs, Shead);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		if (find_server(s->name)) {
			fprintf(opf,
				"<tr><td height=\"4\">Server: </td>\n");
			fprintf(opf,
				"<td height=\"4\"><a href=#%s> %s (*) </a></td></tr>\n",
				s->name, s->name);
		} else {
			fprintf(opf,
				"<tr><td height=\"4\">Server: </td>\n");
			fprintf(opf,
				"<td height=\"4\"><a href=#%s> %s </a></td></tr>\n",
				s->name, s->name);
		}
	}
	fprintf(opf, "</table>");
}

void get_srvlistdet()
{
	SStats *s;
	Client *ss;
	hscan_t hs;
	hnode_t *sn;
	fprintf(opf, "<table border=0>");
	hash_scan_begin(&hs, Shead);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = find_server(s->name);
		fprintf(opf,
			"<tr><th><a name=%s>Server:</th><th colspan = 2><b>%s</b></th></tr>\n",
			s->name, s->name);
		if (!ss)
			fprintf(opf,
				"<tr><td>Last Seen:</td><td colspan = 2>%s</td></tr>\n",
				sftime(s->t_lastseen));
		if (ss)
			fprintf(opf,
				"<tr><td>Current Users:</td><td>%d (%2.0f%%)</td><td>Max %ld at %s</td></tr>\n",
				s->users,
				(float) s->users /
				(float) stats_network.users * 100,
				s->maxusers, sftime(s->t_maxusers));
		if (ss)
			fprintf(opf,
				"<tr><td>Current Opers:</td><td>%d (%2.0f%%)</td><td>Max %d at %s</td></tr>\n",
				s->opers,
				(float) s->opers /
				(float) stats_network.opers * 100,
				s->maxopers, sftime(s->t_maxopers));
		fprintf(opf,
			"<tr><td>Total Users Connected:</td><td colspan = 2>%ld</td></tr>",
			s->totusers);
		fprintf(opf,
			"<tr><td>IrcOp Kills</td><td colspan = 2>%d</td></tr>",
			s->operkills);
		fprintf(opf,
			"<tr><td>Server Kills</td><td colspan = 2>%d</td></tr>",
			s->serverkills);
		fprintf(opf,
			"<tr><td>Highest Ping</td><td>%d</td><td>at %s</td></tr>",
			(int)s->highest_ping, sftime(s->t_highest_ping));
		if (ss)
			fprintf(opf,
				"<tr><td>Current Ping</td><td colspan = 2>%d</td></tr>",
				ss->server->ping);
		fprintf(opf,
			"<tr><td>Server Splits</td><td colspan = 2>%d</td></tr>",
			s->numsplits);

	}
	fprintf(opf, "</table>");
}

void get_netstats()
{

	fprintf(opf, "<table border = 0>");
	fprintf(opf,
		"<tr><th colspan=\"4\"><b>Network Statistics:</b></th></tr>\n");
	fprintf(opf, "<td>Current Users: </td>\n");
	fprintf(opf, "<td> %ld </td>\n", stats_network.users);
	fprintf(opf, "<td>Maximum Users: </td>\n");
	fprintf(opf, "<td> %ld [%s] </td></tr>\n", stats_network.maxusers,
		sftime(stats_network.t_maxusers));
	fprintf(opf,
		"<tr><td colspan=2>Total Users Ever Connected</td><td colspan=2>%ld</td></tr>",
		stats_network.totusers);
	fprintf(opf, "<tr><td>Current Channels: </td>\n");
	fprintf(opf, "<td> %i </td>\n", (int)stats_network.chans);
	fprintf(opf, "<td>Maximum Channels: </td>\n");
	fprintf(opf, "<td> %ld [%s] </td></tr>\n", stats_network.maxchans,
		sftime(stats_network.t_chans));
	fprintf(opf, "<tr><td>Current Opers: </td>\n");
	fprintf(opf, "<td> %i </td>\n", (int)stats_network.opers);
	fprintf(opf, "<td>Maximum Opers: </td>\n");
	fprintf(opf, "<td> %i [%s] </td></tr>\n", (int)stats_network.maxopers,
		sftime(stats_network.t_maxopers));
	fprintf(opf, "<td>Current Servers: </td>\n");
	fprintf(opf, "<td> %d </td>\n", (int)stats_network.servers);
	fprintf(opf, "<td>Maximum Servers: </td>\n");
	fprintf(opf, "<td> %d [%s] </td>\n", (int)stats_network.maxservers,
		sftime(stats_network.t_maxservers));
	fprintf(opf, "<tr><td colspan=\"2\">Users Set Away: </td>\n");
	fprintf(opf, "<td colspan=\"2\"> %ld </td></tr></table>\n",
		stats_network.away);

}

void get_dailystats()
{

	fprintf(opf, "<table border = 0>");
	fprintf(opf,
		"<tr><th colspan=\"4\"><b>Daily Network Statistics:</b></th></tr>\n");
	fprintf(opf, "<tr><td colspan=\"2\">Max Daily Users: </td>\n");
	fprintf(opf, "<td colspan=\"2\"> %-2d %s </td></tr>\n",
		daily.users, sftime(daily.t_users));
	fprintf(opf,
		"<tr><td colspan=\"2\">Total Users Connected:</td>\n");
	fprintf(opf, "<td colspan=\"2\"> %-2d</td></tr>\n",
		daily.tot_users);
	fprintf(opf,
		"<tr><td colspan=\"2\">Max Channels:</td>\n");
	fprintf(opf, "<td colspan=\"2\"> %-2ld</td></tr>\n",
		(long)daily.chans);
	fprintf(opf, "<tr><td colspan=\"2\">Max Daily Opers: </td>\n");
	fprintf(opf, "<td colspan=\"2\"> %-2d %s </td></tr>\n",
		daily.opers, sftime(daily.t_opers));
	fprintf(opf, "<tr><td colspan=\"2\">Max Daily Servers: </td>\n");
	fprintf(opf, "<td colspan=\"2\"> %-2d %s </td></tr>\n",
		daily.servers, sftime(daily.t_servers));
	fprintf(opf,
		"<tr><td colspan=\"4\"><center>(All Daily Statistics are reset at Midnight)</center></td>\n");
	fprintf(opf, "</tr></table>\n");
}

void get_chantop10()
{
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topchan)) {
		list_sort(Chead, topchan);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	fprintf(opf,
		"<table border = 0><tr><th>Channel</th><th align=right>Members</th></tr>");
	for (i = 0; i <= 10; i++) {
		/* only show hidden chans to operators */
		if (is_hidden_chan(find_chan(cs->name))) {
			i--;
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
			continue;
		}
		if (cs->members > 0)
			fprintf(opf,
				"<tr><td>%s</td><td align=right>%ld</td></tr>\n",
				cs->name, cs->members);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	fprintf(opf, "</table>");
}

void get_chantop10eva()
{
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topjoin)) {
		list_sort(Chead, topjoin);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	fprintf(opf,
		"<table border = 0><tr><th>Channel</th><th align=right>Total Joins</th></tr>");
	for (i = 0; i <= 10; i++) {
		/* only show hidden chans to operators */
		if (is_hidden_chan(find_chan(cs->name))) {
			i--;
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
			continue;
		}
		fprintf(opf,
			"<tr><td>%s %s</td><td align=right>%ld</td></tr>\n",
			cs->name, (find_chan(cs->name) ? "(*)" : ""),
			cs->totmem);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	fprintf(opf, "</table>");
}
void get_clientstats()
{
	CVersions *cv;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Vhead, topversions)) {
		list_sort(Vhead, topversions);
	}
	cn = list_first(Vhead);
	if (cn) {
		cv = lnode_get(cn);
		fprintf(opf,
			"<table border = 0><tr><th>Version</th><th align=right>Count</th></tr>");
		for (i = 0; i <= 10; i++) {
			fprintf(opf,
				"<tr><td>%s</td><td align=right>%d</td></tr>\n",
				cv->name, cv->count);
			cn = list_next(Vhead, cn);
			if (cn) {
				cv = lnode_get(cn);
			} else {
				break;
			}
		}
		fprintf(opf, "</table>");
	}
}

void get_tldmap()
{
	lnode_t *tn;
	TLD *t;
	fprintf(opf,
		"<table border = 0><tr><th>tld</th><th>Country</th><th>Current Users</th><th>Daily Total</th></tr>");
	list_sort(Thead, sortusers);
	tn = list_first(Thead);
	while (tn) {
		t = lnode_get(tn);
		if (t->users > 0) {
			fprintf(opf,
				"<tr><td>%s</td><td>%s</td><td>%3d</td><td>%3d</td></tr>",
				t->tld, t->country, t->users,
				t->daily_users);
		}
		tn = list_next(Thead, tn);
	}
	fprintf(opf, "</table>");
}




void get_unwelcomechan()
{
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topkick)) {
		list_sort(Chead, topkick);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	fprintf(opf,
		"<table border = 0><tr><th>Channel</th><th>Total Kicks</th></tr>");
	for (i = 0; i <= 10; i++) {
		/* only show hidden chans to operators */
		if (is_hidden_chan(find_chan(cs->name))) {
			i--;
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
			continue;
		}
		fprintf(opf,
			"<tr><td>%s %s</td><td align=right>%ld</td></tr>\n",
			cs->name, (find_chan(cs->name) ? "(*)" : ""),
			cs->kicks);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	fprintf(opf, "</table>");

}

void get_chantops()
{
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, toptopics)) {
		list_sort(Chead, toptopics);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	fprintf(opf,
		"<table border = 0><tr><th>Channel</th><th>Total Topics</th></tr>");
	for (i = 0; i <= 10; i++) {
		/* only show hidden chans to operators */
		if (is_hidden_chan(find_chan(cs->name))) {
			i--;
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
			continue;
		}
		fprintf(opf,
			"<tr><td>%s %s</td><td align=right>%ld</td></tr>\n",
			cs->name, (find_chan(cs->name) ? "(*)" : ""),
			cs->topics);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	fprintf(opf, "</table>");
}


void get_map(char *uplink, int level)
{
	hscan_t hs;
	hnode_t *sn;
	Client *s;
	SStats *ss;
	char buf[256];
	int i;

	hash_scan_begin(&hs, GetServerHash ());
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findserverstats(s->name);

		if ((level == 0) && (strlen(s->uplink) <= 0)) {
			/* its the root server */
			fprintf(opf,
				"<table border=0><tr><th>Server Name</th><th>Users/Max</th><th>Opers/Max</th><th>Lag/Max</th></tr>");
			fprintf(opf,
				"<tr><td>%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				ss->name, ss->users, ss->maxusers,
				ss->opers, ss->maxopers, s->server->ping,
				(int)ss->highest_ping);
			get_map(s->name, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplink)) {
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				ircsnprintf(buf,256,
					"%s&nbsp&nbsp&nbsp&nbsp&nbsp|",
					buf);
			}
			fprintf(opf,
				"<tr><td>%s\\_%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				buf, ss->name, ss->users, ss->maxusers,
				ss->opers, ss->maxopers, s->server->ping,
				(int)ss->highest_ping);
			get_map(s->name, level + 1);
		}
	}
}
