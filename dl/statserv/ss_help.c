/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      ss_help.c, 
** Version: 1.2
** Date:    22/11/2001
*/

#include "stats.h"

const char *ss_help[] = {
"*** Statistics Help ***",
"",
"Commands:",
"\2SERVER\2     Request information about a server.",
"\2MAP\2        Show the network map.",
"\2NETSTATS\2   General Network Statistics.",
"\2DAILY\2      Daily Network Statistics.",
"\2TLD\2        Show what country a TLD represents.",
"\2TLDMAP\2     Statistics on TLD's.",
"\2OPERLIST\2   Show a listing of on-line IRCops.",
"\2BOTLIST\2    Show a listing of on-line BOTS.",
"\2VERSION\2    Shows you the current StatServ Version.",
"",
"End of Help.",
NULL
};

const char *ss_myuser_help[] = {
"",
"*** Additional Commands For Net & Tech Admins:***",
"",
"\2HTMLSTATS\2  Output IRC statistics to a HTML file",
"\2FORCEHTML\2  Force an update of the HTML ouput file",
"\2RESET\2      DELETES data files and starts stats over new!",
"\2JOIN\2       Join a Channel.",
"\2STATS\2      Modify Statistic Entries.",
"\2NOTICES\2    Enable/Disable 'User requested to see...' Notices.",
"End of Help",
NULL
};


const char *ss_server_help[] = {
"*** Statistics: \2SERVER\2 Help ***",
"",
"Usage: \2SERVER \37<server name>\37\2",
"",
"Provides you with statistics on a",
"specific server.",
"",
"End of Help.",
NULL
};

const char *ss_map_help[] = {
"*** Statistics: \2MAP\2 Help ***",
"",
"Usage: \2MAP\2",
"",
"Provides a server listing with",
"minimal statistics.",
"",
"End of Help.",
NULL
};

const char *ss_netstats_help[] = {
"*** Statistics: \2NETSTATS\2 Help ***",
"",
"Usage: \2NETSTATS\2",
"",
"Provides information about the",
"performance of the network.",
"",
"End of Help.",
NULL
};

const char *ss_daily_help[] = {
"*** Statistics: \2DAILY\2 Help ***",
"",
"Usage: \2DAILY\2",
"",
"Provides information about records",
"that have been set today.",
"",
"End of Help.",
NULL
};

const char *ss_tld_help[] = {
"*** Statistics: \2TLD\2 Help ***",
"Usage: \2TLD \37top_level_domain\37\2",
"",
"Provides the country-name",
"for a specific TLD.  An example",
"of a TLD is \2.NET\2",
"",
"End of Help.",
NULL
};

const char *ss_tld_map_help[] = {
"*** Statistics: \2TLDMAP\2 Help ***",
"Usage: \2TLDMAP\2",
"",
"Shows the network map in",
"relation to top level domains.",
"",
"End of Help.",
NULL
};

const char *ss_operlist_help[] = {
"*** Statistics: \2OPERLIST\2 Help ***",
"Usage: \2OPERLIST (\37options|server_name\37)\2",
"",
"Shows a listing of IRCops.",
"",
"Flags:",
" \2NOAWAY\2   Don't show opers that are set away.",
" \2TECH\2     Only Show Tech Admins on the Network.",
" \2NET\2      Only Show Net Admins on the Network.",
" \2BOTS\2     Show Online Bots.",
" \2SERVER\2   Only show opers on that server.",
"",
"End of Help.",
NULL
};

const char *ss_botlist_help[] = {
"*** Statistics: \2BOTLIST\2 Help ***",
"Usage: \2BOTLIST\2",
"",
"Shows all current bots on the network.",
"(umode +B users)",
"",
"End of Help.",
NULL
};

const char *ss_version_help[] = {
"*** Statistics: \2VERSION\2 Help ***",
"Usage: \2VERSION\2",
"",
"Shows the current StatServ Version.",
"",
"End of Help.",
NULL
};

const char *ss_reset_help[] = {
"*** Statistics: \2RESET\2 Help ***",
"Usage: \2RESET\2",
"",
"Force NeoStats to DELETE datafiles and",
"re-connect to the network.  Note: this",
"will reset all statistics.",
"EVIL EVIL command ;)",
"",
"End of Help.",
NULL
};

const char *ss_stats_help[] = {
"*** Statistics: \2STATS\2 Help ***",
"Usage: \2STATS \37[DEL|LIST|COPY]\37\2",
"",
"\2LIST\2  List all database entries.",
"\2DEL \37name\37\2  Remove an entry.",
"\2COPY \37name newname\37\2  Copy an entry.",
"",
"End of Help.",
NULL
};

const char *ss_join_help[] = {
"*** Statistics: \2JOIN\2 Help ***",
"Usage: \2JOIN <Channel>\2",
"",
"Get StatServ to Join a Channel",
"It will then Echo Events as they happen to that Channel, So it Shouldn't Join Public Channels",
"",
"End of Help.",
NULL
};

const char *icq_help[] = {
"*** Statistics: \2JOIN\2 Help ***",
"Usage: \2JOIN <Channel>\2",
"",
"Get StatServ to Join a Channel",
"It will then Echo Events as they happen to that Channel, So it Shouldn't Join Public Channels",
"",
"End of Help.",
NULL
};

const char *ss_htmlstats_help[] = {
"*** Statistics: \2HTMLSTATS\2 Help ***",
"",
"Usage: \2HTMLSTATS <ON/OFF> <PATH TO HTML FILE IF HTMLTURNING STATS ON>\2",
"",
"Print the Statistics to a .html file for veiwing on a website.",
"The path to the html file should be from the /NeoStats-2.x directory eg:",
"'../public_html/neostats/mystats.html'",
"",
"End of Help.",
NULL
};

const char *ss_forcehtml_help[] = {
"*** Statistics: \2FORCEUPDATE\2 Help ***",
"",
"Usage: \2FORCEUPDATE\2",
"",
"Forces an update of the HTML data file with the most current",
"network statistics.",
"",
"End of Help.",
NULL
};

const char *ss_notices_help[] = {
"*** Statistics: \2NOTICES\2 Help ***",
"",
"Usage: \2NOTICES\2",
"",
"Turns StatServ Information requests on and off. The default",
"setting when neostats is loaded is 'off'",
"",
"End of Help.",
NULL
};
