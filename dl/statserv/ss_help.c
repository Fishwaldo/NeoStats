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

const char *ss_help[] = {
	"StatServ Help ***",
	"",
	"Commands:",
	"\2SERVER\2          Request information about a server.",
	"\2MAP\2             Show the network map.",
	"\2CHAN\2            Channel Information.",
	"\2NETSTATS\2        General Network Statistics.",
	"\2DAILY\2           Daily Network Statistics.",
	"\2TLD\2             Show what country a TLD represents.",
	"\2TLDMAP\2          Statistics on TLD's.",
	"\2OPERLIST\2        Show a listing of on-line IRCops.",
#ifdef HAVE_BOT_MODE
	"\2BOTLIST\2         Show a listing of on-line BOTS.",
#endif
	"\2CLIENTVERSIONS\2  Shows you a list of Client Versions.",
	"\2VERSION\2         Shows you the current StatServ Version.",
	"",
	NULL
};

const char *ss_clientversions_help[] = {
	"StatServ: \2CLIENTVERSIONS\2 Help ***",
	"Usage: \2CLIENTVERSIONS \37<limit>\37\2",
	"",
	"Provides Statistics on the Client Versions found",
	"The list will only be active if you have SecureServ installed and Active",
	"<limit> Specifies how many results to show. Results are sorted by",
	"Most Popular to Least Popular",
	"",
	NULL
};

const char *ss_myuser_help[] = {
	"",
	"*** Additional Commands For Net & Tech Admins:***",
	"",
	"\2SET\2        Change StatServ Settings",
	"\2FORCEHTML\2  Force an update of the HTML ouput file",
	"\2STATS\2      Modify Statistic Entries.",
	"",
	NULL
};

const char *ss_set_help[] = {
	"*** Configuration: \2SET\2 Help ***",
	"Usage: \2Set <option> [<value>]\2",
	"",
	"" "Available Options are:",
	"\2HTMLPATH <path>\2",
	"Set the Pathname (including filename) to write HTML statistics to",
	"if HTML Statistics are enabled",
	"",
	"\2HTML\2",
	"Toggle HTML Statistics Generation on/off",
	"",
	"\2MSGTHROTTLE [<seconds>/off]\2",
	"if <seconds> is greater than 0, then set 5 Wallops per <seconds> throttle",
	"if <seconds> is equal to 0, then disable Wallop Throttling",
	"if \"off\" then all Wallops are disabled",
	"",
	"\2LAGWALLOP <seconds>\2",
	"if <seconds> is greater than 0, then when servers are lagged by this many seconds, issue a warning",
	"if <seconds> is equal to 0, then disable Lag Monitoring",
	"",
	"If you specify \2SET\2 without any options, the current settings are disabled",
	NULL
};

const char *ss_chan_help[] = {
	"StatServ: \2CHAN\2 Help ***",
	"Usage: \2CHAN \37<POP/KICKS/TOPICS/<Channame>>\37\2",
	"",
	"Provides Statistics on Channels on the network",
	"\2CHAN\2 By itself provides a list of the top10 Channels based on the current number of members",
	"\2CHAN POP\2 gives you information on the most popular channels on the network based on the number of joins",
	"\2CHAN KICKS\2 Gives you the top 10 kicking channels",
	"\2CHAN TOPICS\2 Gives you the top10 Topic Changing Channels",
	"\2CHAN <name>\2 Gives you specific information on a channel",
	"",
	NULL
};

const char *ss_server_help[] = {
	"StatServ: \2SERVER\2 Help ***",
	"Usage: \2SERVER \37<server name>\37\2",
	"",
	"Provides you with statistics on a",
	"specific server.",
	"",
	NULL
};

const char *ss_map_help[] = {
	"StatServ: \2MAP\2 Help ***",
	"Usage: \2MAP\2",
	"",
	"Provides a server listing with",
	"minimal statistics.",
	"",
	NULL
};

const char *ss_netstats_help[] = {
	"StatServ: \2NETSTATS\2 Help ***",
	"Usage: \2NETSTATS\2",
	"",
	"Provides information about the",
	"performance of the network.",
	"",
	NULL
};

const char *ss_daily_help[] = {
	"StatServ: \2DAILY\2 Help ***",
	"Usage: \2DAILY\2",
	"",
	"Provides information about records",
	"that have been set today.",
	"",
	NULL
};

const char *ss_tld_help[] = {
	"StatServ: \2TLD\2 Help ***",
	"Usage: \2TLD \37top_level_domain\37\2",
	"",
	"Provides the country-name",
	"for a specific TLD.  An example",
	"of a TLD is \2.NET\2",
	"",
	NULL
};

const char *ss_tld_map_help[] = {
	"StatServ: \2TLDMAP\2 Help ***",
	"Usage: \2TLDMAP\2",
	"",
	"Shows the network map in",
	"relation to top level domains.",
	"",
	NULL
};

const char *ss_operlist_help[] = {
	"StatServ: \2OPERLIST\2 Help ***",
	"Usage: \2OPERLIST (\37options|server_name\37)\2",
	"",
	"Shows a listing of IRCops.",
	"",
	"Flags:",
	" \2NOAWAY\2   Don't show opers that are set away.",
	" \2SERVER\2   Only show opers on that server.",
	"",
	NULL
};

#ifdef HAVE_BOT_MODE
const char *ss_botlist_help[] = {
	"StatServ: \2BOTLIST\2 Help ***",
	"Usage: \2BOTLIST\2",
	"",
	"Shows all current bots on the network.",
	"(umode +B users)",
	"",
	NULL
};
#endif

const char *ss_version_help[] = {
	"StatServ: \2VERSION\2 Help ***",
	"Usage: \2VERSION\2",
	"",
	"Shows the current StatServ Version.",
	"",
	NULL
};

const char *ss_stats_help[] = {
	"StatServ: \2STATS\2 Help ***",
	"Usage: \2STATS \37[DEL|LIST|COPY]\37\2",
	"",
	"\2LIST\2  List all database entries.",
	"\2DEL \37name\37\2  Remove an entry.",
	"\2COPY \37name newname\37\2  Copy an entry.",
	"",
	NULL
};

const char *ss_forcehtml_help[] = {
	"StatServ: \2FORCEUPDATE\2 Help ***",
	"Usage: \2FORCEUPDATE\2",
	"",
	"Forces an update of the HTML data file with the most current",
	"network statistics.",
	"",
	NULL
};
