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


/*************************************************************************/

/* strnrepl:  Replace occurrences of `old' with `new' in string `s'.  Stop
 *            replacing if a replacement would cause the string to exceed
 *            `size' bytes (including the null terminator).  Return the
 *            string.
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
	if (strncmp(ptr, old, oldlen) != 0) {
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



/* dump out the HTML info */
void ss_html()
{
    SStats *s;
    FILE *fp;
    hscan_t hs;
    hnode_t *sn;

    /* Approximate Segfault Location */
    strcpy(segv_location, "ss_html_output");

    /* Open HTML Output file for writting */
    fp = fopen(StatServ.htmlpath, "w");

    fprintf(fp, "<center><table border=\"0\" width=\"590\"><tr>\n");
    fprintf(fp, "<td colspan=\"4\"><center><b><font size=\"+1\">%s Current Network Statistics</center></b></font></td>\n", me.netname);
    fprintf(fp, "</tr><tr>\n");
    fprintf(fp, "<td colspan=\"4\"><center>\n");
    fprintf(fp, "<table border=\"0\">\n");

    /* Output of Online/Offline Servers */
    fprintf(fp, "</tr><tr>\n");
    hash_scan_begin(&hs, Shead);
    while ((sn = hash_scan_next(&hs))) {
    	s = hnode_get(sn);
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
    fprintf(fp, "<br> %s - %s Compiled %s at %s\n", me.name, SSMNAME, version_date, version_time);
    fprintf(fp, "<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n");
    fprintf(fp, "</center></td>\n");
    fprintf(fp, "</tr></table>\n");

    /* Close HTML file and exit routine */
    fclose(fp);
    return;

}

