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

/*  TODO:
 *  - Identify bootup and netjoin stats and handle differently
 *    to improve accuracy
 */

#include "neostats.h"
#include "statserv.h"
#include "statsrta.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "channel.h"
#include "user.h"
#include "version.h"
#include "tld.h"
#include "htmlstats.h"

static int ss_set_htmltime_cb (CmdParams *cmdparams, SET_REASON reason);
static int ss_set_exclusions_cb (CmdParams *cmdparams, SET_REASON reason);
static int ss_set_html_cb (CmdParams *cmdparams, SET_REASON reason);
static int ss_set_htmlpath_cb (CmdParams *cmdparams, SET_REASON reason);

/** Bot pointer */
Bot *ss_bot;

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_PONG,		ss_event_pong,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVER,		ss_event_server,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SQUIT,		ss_event_squit,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SIGNON,		ss_event_signon,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NICKIP,		ss_event_nickip,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_UMODE,		ss_event_mode,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_QUIT,		ss_event_quit,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_GLOBALKILL,	ss_event_globalkill,EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVERKILL,	ss_event_serverkill,EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NEWCHAN,		ss_event_newchan,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_DELCHAN,		ss_event_delchan,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_JOIN,		ss_event_join,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_PART,		ss_event_part,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_KICK,		ss_event_kick,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_TOPIC,		ss_event_topic,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_CTCPVERSIONRPL,	ss_event_ctcpversion,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NULL,		NULL}
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"StatServ",
	"Network statistical service",
	ns_copyright,
	ss_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot comand table */
static bot_cmd ss_commands[]=
{
	{"SERVER",		ss_cmd_server,		0, 	0,		ss_help_server,		 	ss_help_server_oneline},
	{"MAP",			ss_cmd_map,			0, 	0,		ss_help_map, 		 	ss_help_map_oneline},
	{"CHANNEL",		ss_cmd_channel,		0, 	0,		ss_help_channel, 		 	ss_help_channel_oneline},
	{"NETSTATS",	ss_cmd_netstats,	0, 	0,		ss_help_netstats, 	 	ss_help_netstats_oneline},
	{"DAILY",		ss_cmd_daily,		0, 	0,		ss_help_daily, 		 	ss_help_daily_oneline},
	{"TLDMAP",		ss_cmd_tldmap,		0, 	0,		ss_help_tldmap, 	 	ss_help_tldmap_oneline},
	{"OPERLIST",	ss_cmd_operlist,	0, 	0,		ss_help_operlist, 	 	ss_help_operlist_oneline},
	{"BOTLIST",		ss_cmd_botlist,		0, 	0,		ss_help_botlist, 	 	ss_help_botlist_oneline},
	{"USERVERSION",	ss_cmd_userversion,	0,	0,		ss_help_userversion, ss_help_userversion_oneline},
	{"FORCEHTML",	ss_cmd_forcehtml,	0, 	NS_ULEVEL_ADMIN,	ss_help_forcehtml, 		ss_help_forcehtml_oneline},
	{"STATS",		ss_cmd_stats,		1, 	NS_ULEVEL_ADMIN,	ss_help_stats, 			ss_help_stats_oneline},
	{NULL,			NULL,				0, 	0,					NULL, 					NULL}
};

/** Bot setting table */
static bot_setting ss_settings[]=
{
	{"HTML",		&StatServ.html,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "HTML_Enabled",NULL,		ss_help_set_html, ss_set_html_cb, (void *)0},
	{"HTMLPATH",	&StatServ.htmlpath,		SET_TYPE_STRING,	0, MAXPATH,		NS_ULEVEL_ADMIN, "HTML_Path",	NULL,		ss_help_set_htmlpath, ss_set_htmlpath_cb, (void *)""},
	{"HTMLTIME",	&StatServ.htmltime,		SET_TYPE_INT,		600, 3600,			NS_ULEVEL_ADMIN, "htmltime",		"seconds",	ss_help_set_htmltime, ss_set_htmltime_cb, (void*)3600},
	{"MSGINTERVAL",	&StatServ.msginterval,	SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, "MsgInterval",	"seconds",	ss_help_set_msginterval, NULL, (void *)60},
	{"MSGLIMIT",	&StatServ.msglimit,		SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, "MsgLimit",	NULL,		ss_help_set_msglimit, NULL, (void *)5},
	{"LAGTIME",		&StatServ.lagtime,		SET_TYPE_INT,		1, 256,			NS_ULEVEL_ADMIN, "LagTime",		"seconds",	ss_help_set_lagtime, NULL, (void *)30},
	{"LAGALERT",	&StatServ.lagalert,		SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, "LagAlert",	NULL,		ss_help_set_lagalert, NULL, (void *)1},
	{"RECORDALERT", &StatServ.recordalert,	SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, "RecordAlert",	NULL,		ss_help_set_recordalert, NULL, (void *)1},
	{"EXCLUSIONS",	&StatServ.exclusions,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "Exclusions",	NULL,		ss_help_set_exclusions, ss_set_exclusions_cb, (void *)0},
	{NULL,			NULL,					0,					0, 0,			0,				 NULL,			NULL,		NULL, NULL, (void *)0},
};

/** BotInfo */
static BotInfo ss_botinfo = 
{
	"StatServ", 
	"StatServ1", 
	"SS", 
	BOT_COMMON_HOST, 
	"Statistics service", 
	BOT_FLAG_SERVICEBOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	ss_commands, 
	ss_settings,
};

int SaveStats(void)
{
	SET_SEGV_LOCATION();
	SaveServerStats ();
	SaveChanStats ();
	SaveNetworkStats ();
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit (Module *mod_ptr)
{
	SET_SEGV_LOCATION();
	StatServ.shutdown = 0;
	ModuleConfig(ss_settings);
	if (StatServ.html && StatServ.htmlpath[0] == 0) {
		nlog(LOG_NOTICE, "HTML stats disabled as HTML_PATH is not set");
		StatServ.html = 0;
	}
	InitNetworkStats ();
	InitChannelStats ();
	InitServerStats ();
	InitVersionStats ();
	InitTLDStatistics ();	
	InitUserStats ();	
#ifdef USE_BERKELEY
	DBAOpenTable();
#endif
	
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	SET_SEGV_LOCATION();
	/* RTA init must be in synch since core does not start 
	   RTA during the init cycle when NeoStats first boots */
	InitRTAStats ();
	ss_bot = AddBot (&ss_botinfo);
	if (!ss_bot) {
		return NS_FAILURE;
	}
	/* Timer to save the database */
	add_timer (TIMER_TYPE_INTERVAL, SaveStats, "SaveStats", DBSAVETIME);
	/* Timer to output html */
	if (StatServ.html) {
		add_timer (TIMER_TYPE_INTERVAL, ss_html, "ss_html", StatServ.htmltime);
		/* Initial output at load */
		ss_html ();
	}
	/* Timer to reset timeslice stats */
	add_timer (TIMER_TYPE_MIDNIGHT, ResetStatistics, "ResetStatistics", 0);
	/* Timer to average stats */
	add_timer (TIMER_TYPE_INTERVAL, AverageStatistics, "AverageStatistics", 3600);
	/* Initial average at load */
	AverageStatistics();
	/* Timer to delete old channels */
	add_timer (TIMER_TYPE_INTERVAL, DelOldChan, "DelOldChan", 3600);
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
	StatServ.shutdown = 1;
	FiniServerStats ();
	FiniChannelStats ();
	FiniTLDStatistics ();
	FiniVersionStats ();
	FiniRTAStats();
	FiniNetworkStats();
#ifdef USE_BERKELEY
	DBACloseTable();
#endif      
}

static int ss_set_html_cb (CmdParams *cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	if (StatServ.html && StatServ.htmlpath[0] == 0) {
		irc_prefmsg  (ss_bot, cmdparams->source, 
			"You need to SET HTMLPATH. HTML output disabled.");
		StatServ.html = 0;
		return NS_SUCCESS;
	}
	if (StatServ.html) {
		add_timer (TIMER_TYPE_INTERVAL, ss_html, "ss_html", StatServ.htmltime);
	} else {
		del_timer ("ss_html");
	}
	return NS_SUCCESS;
}

static int ss_set_htmlpath_cb (CmdParams *cmdparams, SET_REASON reason)
{
	FILE *opf;

	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	opf = fopen (StatServ.htmlpath, "w");
	if (!opf) {
		irc_prefmsg (ss_bot, cmdparams->source, 
			"Failed to open HTML output file %s. Check file permissions. HTML output disabled.", StatServ.htmlpath);
		return NS_SUCCESS;
	}
	fclose (opf);
	ss_html ();
	return NS_SUCCESS;
}

static int ss_set_htmltime_cb (CmdParams *cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	set_timer_interval ("ss_html", StatServ.htmltime);
	return NS_SUCCESS;
}

static int ss_set_exclusions_cb (CmdParams *cmdparams, SET_REASON reason)
{
	SetAllEventFlags (EVENT_FLAG_USE_EXCLUDE, StatServ.exclusions);
	return NS_SUCCESS;
}
