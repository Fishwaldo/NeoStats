/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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

#include "stats.h"

const char ss_help_about_oneline[]="About StatServ";
const char ss_help_version_oneline[]="Shows you the current StatServ Version";
const char ss_help_server_oneline[]="Request information about a server";
const char ss_help_map_oneline[]="Show the network map";
const char ss_help_chan_oneline[]="Channel Information";
const char ss_help_netstats_oneline[]="General Network Statistics";
const char ss_help_daily_oneline[]="Daily Network Statistics";
const char ss_help_tld_oneline[]="Show what country a TLD represents";
const char ss_help_tldmap_oneline[]="Statistics on TLD's";
const char ss_help_operlist_oneline[]="Show a listing of on-line IRCops";
#ifdef GOTBOTMODE
const char ss_help_botlist_oneline[]="Show a listing of on-line BOTS";
#endif
const char ss_help_clientversions_oneline[]="Shows you a list of Client Versions";
const char ss_help_set_oneline[]="Change StatServ Settings";
const char ss_help_forcehtml_oneline[]="Force an update of the HTML output file";
const char ss_help_stats_oneline[]="Modify Statistic Entries.";

const char *ss_help_about[] = {
	"\2StatServ\2 provides detailed statistics about your",
	"IRC network users, channels and servers.",
	NULL
};

const char *ss_help_clientversions[] = {
	"Syntax: \2CLIENTVERSIONS \37<limit>\37\2",
	"",
	"Provides Statistics on the Client Versions found",
	"The list will only be active if you have SecureServ",
	"installed and Active.",
	"<limit> Specifies how many results to show. Results are",
	"sorted by most to least popular",
	NULL
};

const char *ss_help_set[] = {
	"Syntax: \2SET LIST\2",
	"        \2SET <option> [<value>]\2",
	"",
	"LIST will display the current settings",
	"",
	"Available Options are:",
	"\2HTMLPATH <path>\2",
	"Set the Pathname (including filename) to write HTML",
	"statistics to if HTML Statistics are enabled",
	"",
	"\2HTML\2",
	"Toggle HTML Statistics Generation on/off",
	"",
	"\2MSGTHROTTLE [<seconds>/off]\2",
	"if <seconds> is greater than 0, then set 5 Wallops per",
	" <seconds> throttle",
	"if <seconds> is equal to 0, then disable Wallop Throttling",
	"if \"off\" then all Wallops are disabled",
	"",
	"\2LAGWALLOP <seconds>\2",
	"if <seconds> is greater than 0, then when servers are",
	"lagged by this many seconds, issue a warning",
	"if <seconds> is equal to 0, then disable Lag Monitoring",
	NULL
};

const char *ss_help_chan[] = {
	"Syntax: \2CHAN \37<POP/KICKS/TOPICS/<Channame>>\37\2",
	"",
	"Provides Statistics on Channels on the network",
	"\2CHAN\2 By itself provides a list of the top 10 channels",
	"    based on the current number of members",
	"\2CHAN POP\2 gives you information on the most popular",
	"    channels on the network based on the number of joins",
	"\2CHAN KICKS\2 Gives you the top 10 kicking channels",
	"\2CHAN TOPICS\2 Gives you the top 10 topic changing channels",
	"\2CHAN <name>\2 Gives you specific information on a channel",
	NULL
};

const char *ss_help_server[] = {
	"Syntax: \2SERVER \37<server name>\37\2",
	"",
	"Provides statistics on a specific server.",
	NULL
};

const char *ss_help_map[] = {
	"Syntax: \2MAP\2",
	"",
	"Provides a server listing with minimal statistics.",
	NULL
};

const char *ss_help_netstats[] = {
	"Syntax: \2NETSTATS\2",
	"",
	"Provides information about the",
	"performance of the network.",
	NULL
};

const char *ss_help_daily[] = {
	"Syntax: \2DAILY\2",
	"",
	"Provides information about records",
	"that have been set today.",
	NULL
};

const char *ss_help_tld[] = {
	"Syntax: \2TLD \37top_level_domain\37\2",
	"",
	"Provides the country-name for a specific TLD.",
	"An example of a TLD is \2.NET\2",
	NULL
};

const char *ss_help_tldmap[] = {
	"Syntax: \2TLDMAP\2",
	"",
	"Shows the network map in",
	"relation to top level domains.",
	NULL
};

const char *ss_help_operlist[] = {
	"Syntax: \2OPERLIST\2",
	"        \2OPERLIST NOAWAY\2",
	"        \2OPERLIST SERVER <servername>\2",
	"",
	"Shows a listing of IRCops. You can use NOAWAY to not show",
	"opers that are set away and SERVER with a server name to",
	"only show opers on that server.",
	NULL
};

#ifdef GOTBOTMODE
const char *ss_help_botlist[] = {
	"Syntax: \2BOTLIST\2",
	"",
	"Shows all current bots on the network.",
	"(umode +B users)",
	NULL
};
#endif

const char *ss_help_version[] = {
	"Syntax: \2VERSION\2",
	"",
	"Shows the current StatServ Version.",
	NULL
};

const char *ss_help_stats[] = {
	"Syntax: \2STATS LIST\2",
	"        \2STATS DEL <servername>\2",
	"        \2STATS COPY <oldservername> <newservername>\2",
	"",
	"Use, LIST to list all database entries. DEL to remove an",
	"entry and COPY to copy an entry.",
	NULL
};

const char *ss_help_forcehtml[] = {
	"Syntax: \2FORCEUPDATE\2",
	"",
	"Forces an update of the HTML data file with the most",
	"current network statistics.",
	NULL
};
