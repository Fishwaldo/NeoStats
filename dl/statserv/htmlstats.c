/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      htmlstats.c, 
** Version: 3.1
** Date:    08/03/2002
*/

#include "statserv.h"
#include "hash.h"
#include <fcntl.h>


char *tmpbuf;
#define STARTBUFSIZE 20
int bufsize;

char *get_map();
char *get_top10chan();
char *strnrepl(char *, int size, const char *old, const char *new);
char *put_copyright();
char *get_srvlist();
char *get_srvlistdet();
char *get_netstats();
char *get_dailystats();
char *get_chantops();
char *get_chantop10();
char *get_chantop10eva();
char *get_unwelcomechan();
char *get_title();
char *get_tldmap();
void ss_html() {
	FILE *tpl, *opf;
	char *buf;
	int gothtml = 0;
	if (StatServ.html) {
		if (strlen(StatServ.htmlpath) <= 0) {
			log("Can't do HTML Writout as html path is not defined");
			return;
		}
	} else {
		return;
	}
	tpl = fopen("dl/statserv/html/index.tpl", "r");
	if (!tpl) {
		tpl = fopen("data/index.tpl", "r");
	}
	if (!tpl) {
		log("can't open StatServ HTML template");
		notice(s_StatServ, "Can't Open StatServ HTML Template");
		return;
	}
	opf = fopen(StatServ.htmlpath, "w");
	if (!opf) {
		log("Can't open StatServ HTML output file - Check Permissions");
		notice(s_StatServ, "Can't open StatServ HTML output file - Check Permissions");
		return;
	}
	buf = malloc(512*2);
	while (fgets(buf, 512, tpl)) {
		bufsize = STARTBUFSIZE;
		tmpbuf = malloc(bufsize); 
		if (strstr(buf, "!MAP!")) {
			tmpbuf = get_map("", 0);
			sprintf(tmpbuf, "%s </table>\n", tmpbuf);
			strnrepl(buf, bufsize, "!MAP!", tmpbuf);
#ifdef DEBUG
			log("gotmap");
#endif
		}
		if (strstr(buf, "!SRVLIST!")) {
			strnrepl(buf, bufsize, "!SRVLIST!", get_srvlist());
#ifdef DEBUG
			log("gotsrvlist");
#endif
		}
		if (strstr(buf, "!SRVLISTDET!")) {
			strnrepl(buf, bufsize, "!SRVLISTDET!", get_srvlistdet());
#ifdef DEBUG
			log("gotsrvlistdet");
#endif
		}
		if (strstr(buf, "</HTML>")) {
			strnrepl(buf, bufsize, "</html>", put_copyright());			
			gothtml = 1;
#ifdef DEBUG
			log("gotcopyright");
#endif
		}
		if (strstr(buf, "!NETSTATS!")) {
			strnrepl(buf, bufsize, "!NETSTATS!", get_netstats());
#ifdef DEBUG
			log("gotnetstats");
#endif
		}
		if (strstr(buf, "!DAILYSTATS!")) {
			strnrepl(buf, bufsize, "!DAILYSTATS!", get_dailystats());
#ifdef DEBUG
			log("gotdailystats");
#endif
		}
		if (strstr(buf, "!DAILYTOPCHAN!")) {
			strnrepl(buf, bufsize, "!DAILYTOPCHAN!", get_chantop10());
#ifdef DEBUG
			log("gotdailytopchan");
#endif
		}
		if (strstr(buf, "!TOP10CHAN!")) {
			strnrepl(buf, bufsize, "!TOP10CHAN!", get_chantop10eva());
#ifdef DEBUG
			log("gottop10");
#endif
		}
		if (strstr(buf, "!TOP10KICKS!")) {
			strnrepl(buf, bufsize, "!TOP10KICKS!", get_unwelcomechan());
#ifdef DEBUG
			log("gotdailykicks");
#endif
		}
		if (strstr(buf, "!TOP10TOPICS!")) {
			strnrepl(buf, bufsize, "!TOP10TOPICS!", get_chantops());
#ifdef DEBUG
			log("gotdailytopics");
#endif
		}
		if (strstr(buf, "!TLDMAP!")) {
			strnrepl(buf, bufsize, "!TLDMAP!", get_tldmap());
		}
		if (strstr(buf, "!VERSION!")) {
			strnrepl(buf, bufsize, "!VERSION!", version);
		}
		if (strstr(buf, "!TITLE!")) {
			strnrepl(buf, bufsize, "!TITLE!", get_title());
		}

		
		fputs(buf, opf);
		free(tmpbuf);
	}
	if (!gothtml) {
		tmpbuf = malloc(bufsize);
		tmpbuf = put_copyright();
		fputs(tmpbuf, opf);
		free(tmpbuf);
	}
	fclose(tpl);
	fclose(opf);

}

char *neosprintf(char *buf, char *format, ...) {
	va_list ap;
	char tmpbuf[512];
	int sizeoftmpbuf;
	va_start(ap, format);
	sizeoftmpbuf = vsnprintf(tmpbuf, 512, format, ap);
	va_end(ap);
	if (sizeoftmpbuf == -1) {
		log("Eeek, Problem with neosprintf in htmlstats. Report to www.neostats.net");
		return NULL;
	}
	if ((sizeoftmpbuf + strlen(buf)) >= bufsize) {
		buf = realloc(buf, (sizeoftmpbuf + strlen(buf) +1));
		bufsize = sizeoftmpbuf + strlen(buf) +1;
	}	
	sprintf(buf, "%s\n%s", buf, tmpbuf);
	return buf;
}


char *get_title() {
	neosprintf(tmpbuf,  "Network Statistics for %s", me.netname);
	return tmpbuf;

}
char *put_copyright() {
	neosprintf(tmpbuf,  "<br><br><center>Statistics last updated at %s<br>", sftime(time(NULL)));
	neosprintf(tmpbuf,  "%s<b>StatServ Information:</b>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<br> %s - %s Compiled %s at %s\n",tmpbuf, me.name, SSMNAME, version_date, version_time);
	neosprintf(tmpbuf,  "%s<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s</center></html>\n", tmpbuf);
	return tmpbuf;


}

char *get_srvlist() {
	SStats *s;
	hscan_t hs;
	hnode_t *sn;

	neosprintf(tmpbuf,  "<table border=0><tr><th colspan = 2>Server name</th></tr>");
	hash_scan_begin(&hs, Shead);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		if (findserver(s->name)) {
			neosprintf(tmpbuf,  "%s<tr><td height=\"4\">Server: </td>\n", tmpbuf);
			neosprintf(tmpbuf,  "%s<td height=\"4\"><a href=#%s> %s (*) </a></td></tr>\n", tmpbuf,s->name, s->name);
		} else {
			neosprintf(tmpbuf,  "%s<tr><td height=\"4\">Server: </td>\n", tmpbuf);
			neosprintf(tmpbuf,  "%s<td height=\"4\"><a href=#%s> %s </a></td></tr>\n",tmpbuf, s->name, s->name);
		}
	}
	neosprintf(tmpbuf,  "%s </table>", tmpbuf);
	return tmpbuf;
}
char *get_srvlistdet() {
	SStats *s;
	Server *ss;
	hscan_t hs;
	hnode_t *sn;
	neosprintf(tmpbuf,  "<table border=0>");
	hash_scan_begin(&hs, Shead);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findserver(s->name);
		neosprintf(tmpbuf,  "%s<tr><th><a name=%s>Server:</th><th colspan = 2><b>%s</b></th></tr>\n", tmpbuf, s->name, s->name);
		if (!ss) neosprintf(tmpbuf,  "%s<tr><td>Last Seen:</td><td colspan = 2>%s</td></tr>\n", tmpbuf, sftime(s->lastseen));
		if (ss) neosprintf(tmpbuf,  "%s<tr><td>Current Users:</td><td>%d (%2.0f%%)</td><td>Max %ld at %s</td></tr>\n", tmpbuf, s->users, (float)s->users / (float)stats_network.users * 100, s->maxusers, sftime(s->t_maxusers));
		if (ss) neosprintf(tmpbuf,  "%s<tr><td>Current Opers:</td><td>%d (%2.0f%%)</td><td>Max %d at %s</td></tr>\n",tmpbuf, s->opers, (float)s->opers / (float)stats_network.opers *100, s->maxopers, sftime(s->t_maxopers));
		neosprintf(tmpbuf,  "%s<tr><td>IrcOp Kills</td><td colspan = 2>%d</td></tr>", tmpbuf, s->operkills);
		neosprintf(tmpbuf,  "%s<tr><td>Server Kills</td><td colspan = 2>%d</td></tr>", tmpbuf, s->serverkills);
		neosprintf(tmpbuf,  "%s<tr><td>Highest Ping</td><td>%d</td><td>at %s</td></tr>", tmpbuf, s->highest_ping, sftime(s->t_highest_ping));
		if (ss) neosprintf(tmpbuf,  "%s<tr><td>Current Ping</td><td colspan = 2>%d</td></tr>", tmpbuf, ss->ping);
		neosprintf(tmpbuf,  "%s<tr><td>Server Splits</td><td colspan = 2>%d</td></tr>", tmpbuf, s->numsplits);

	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;
}

char *get_netstats() {

	neosprintf(tmpbuf,  "<table border = 0>");
	neosprintf(tmpbuf,  "%s<tr><th colspan=\"4\"><b>Network Statistics:</b></th></tr>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td>Current Users: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %ld </td>\n", tmpbuf, stats_network.users);
	neosprintf(tmpbuf,  "%s<td>Maximum Users: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %ld [%s] </td>\n", tmpbuf, stats_network.maxusers, sftime(stats_network.t_maxusers));
	neosprintf(tmpbuf,  "%s<tr><td>Current Opers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %i </td>\n", tmpbuf, stats_network.opers);
	neosprintf(tmpbuf,  "%s<td>Maximum Opers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %i [%s] </td></tr>\n", tmpbuf, stats_network.maxopers, sftime(stats_network.t_maxopers));
	neosprintf(tmpbuf,  "%s<td>Current Servers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %d </td>\n", tmpbuf, stats_network.servers);
	neosprintf(tmpbuf,  "%s<td>Maximum Servers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td> %d [%s] </td>\n", tmpbuf, stats_network.maxservers, sftime(stats_network.t_maxservers));
	neosprintf(tmpbuf,  "%s<tr><td colspan=\"2\">Users Set Away: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td colspan=\"2\"> %ld </td></tr></table>\n", tmpbuf, stats_network.away);

	return tmpbuf;
}

char *get_dailystats() {

	neosprintf(tmpbuf,  "<table border = 0>");
	neosprintf(tmpbuf,  "%s<tr><th colspan=\"4\"><b>Daily Network Statistics:</b></th></tr>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<tr><td colspan=\"2\">Max Daily Users: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td colspan=\"2\"> %-2d %s </td></tr>\n", tmpbuf, daily.users, sftime(daily.t_users));
	neosprintf(tmpbuf,  "%s<tr><td colspan=\"2\">Max Daily Opers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td colspan=\"2\"> %-2d %s </td></tr>\n", tmpbuf, daily.opers, sftime(daily.t_opers));
	neosprintf(tmpbuf,  "%s<tr><td colspan=\"2\">Max Daily Servers: </td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s<td colspan=\"2\"> %-2d %s </td></tr>\n", tmpbuf, daily.servers, sftime(daily.t_servers));
	neosprintf(tmpbuf,  "%s<tr><td colspan=\"4\"><center>(All Daily Statistics are reset at Midnight)</center></td>\n", tmpbuf);
	neosprintf(tmpbuf,  "%s</tr></table>\n", tmpbuf);
	return tmpbuf;
}

char *get_chantop10() {
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topchan)) {
		list_sort(Chead, topchan);
	} 
	cn = list_first(Chead);
	cs = lnode_get(cn);
	neosprintf(tmpbuf,  "<table border = 0><tr><th>Channel</th><th align=right>Members</th></tr>");
	for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
		if (cs->members > 0) neosprintf(tmpbuf,  "%s<tr><td>%s</td><td align=right>%ld</td></tr>\n", tmpbuf, cs->name, cs->members);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;
}
char *get_chantop10eva() {
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topjoin)) {
		list_sort(Chead, topjoin);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	neosprintf(tmpbuf,  "<table border = 0><tr><th>Channel</th><th align=right>Total Joins</th></tr>");
	for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
		neosprintf(tmpbuf,  "%s<tr><td>%s %s</td><td align=right>%ld</td></tr>\n", tmpbuf, cs->name, (findchan(cs->name) ? "(*)" : ""), cs->totmem);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;
}
char *get_tldmap() {
	TLD *t;
	neosprintf(tmpbuf,  "<table border = 0><tr><th>tld</th><th>Country</th><th>Current Users</th><th>Daily Total</th></tr>");
	for (t = tldhead; t; t = t->next) {
		if (t->users > 0) {
			neosprintf(tmpbuf,  "%s\n<tr><td>%s</td><td>%s</td><td>%3d</td><td>%3d</td></tr>", tmpbuf, t->tld, t->country, t->users, t->daily_users);
		}
	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;
}




char *get_unwelcomechan() {
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, topkick)) {
		list_sort(Chead, topkick);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	neosprintf(tmpbuf,  "<table border = 0><tr><th>Channel</th><th>Total Kicks</th></tr>");
	for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
		neosprintf(tmpbuf,  "%s<tr><td>%s %s</td><td align=right>%ld</td></tr>\n", tmpbuf, cs->name, (findchan(cs->name) ? "(*)" : ""), cs->kicks); 
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;

}	
char *get_chantops() {
	CStats *cs;
	lnode_t *cn;
	int i;
	if (!list_is_sorted(Chead, toptopics)) {
		list_sort(Chead, toptopics);
	}
	cn = list_first(Chead);
	cs = lnode_get(cn);
	neosprintf(tmpbuf,  "<table border = 0><tr><th>Channel</th><th>Total Topics</th></tr>");
	for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
		neosprintf(tmpbuf,  "%s<tr><td>%s %s</td><td align=right>%ld</td></tr>\n", tmpbuf, cs->name, (findchan(cs->name) ? "(*)" : ""), cs->topics);
		cn = list_next(Chead, cn);
		if (cn) {
			cs = lnode_get(cn);
		} else {
			break;
		}
	}
	neosprintf(tmpbuf,  "%s</table>", tmpbuf);
	return tmpbuf;
}	


char *get_map(char *uplink, int level) {
	hscan_t hs;
	hnode_t *sn;
	Server *s;
	SStats *ss;
	char buf[256];
	int i;
	hash_scan_begin(&hs, sh);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findstats(s->name);

		if ((level == 0) && (strlen(s->uplink) <= 0)) { 
			/* its the root server */
			neosprintf(tmpbuf,  "<table border=0><tr><th>Server Name</th><th>Users/Max</th><th>Opers/Max</th><th>Lag/Max</th></tr>");
			neosprintf(tmpbuf,  "%s<tr><td>%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n", tmpbuf, ss->name, ss->users, ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);
			get_map(s->name, level+1);
		} else if ((level > 0) && !strcasecmp(uplink, s->uplink)) {
			/* its not the root server */
			neosprintf(buf, " ");
			for (i = 1; i < level; i++) {
				neosprintf(buf, "%s&nbsp&nbsp&nbsp&nbsp&nbsp|", buf);
			}	
			neosprintf(tmpbuf,  "%s<tr><td>%s\\_%s</td><td>%d/%ld</td><td>%d/%d</td><td>%d/%d</td></tr>\n",tmpbuf, buf, ss->name, ss->users, ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);
			get_map(s->name, level+1);
		}
	}
	return tmpbuf;
}


/*************************************************************************/

/* strnrepl:  Replace occurrences of `old' with `new' in string `s'.  Stop
 *			replacing if a replacement would cause the string to exceed
 *			`size' bytes (including the null terminator).  Return the
 *			string.
 */

char *strnrepl(char *s, int size, const char *old, const char *new)
{
	char *ptr = s;
	int left = strlen(s);
	int avail = size - (left+1);
	int oldlen = strlen(old);
	int newlen = strlen(new);
	int diff = newlen - oldlen;

	while (left >= oldlen) {
	if (strncasecmp(ptr, old, oldlen) != 0) {
		left--;
		ptr++;
		continue;
	}
	if (diff > avail)
		break;
	if (diff != 0)
		memmove(ptr+oldlen+diff, ptr+oldlen, left+1);
	strncpy(ptr, new, newlen);
	ptr += newlen;
	left -= oldlen;
	}
	return s;
}



