/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _NS_HELP_H_
#define _NS_HELP_H_

/* ns.c */
extern const char *ns_help_shutdown[];
extern const char *ns_help_reload[];
extern const char *ns_help_exclude[];
#ifdef USE_RAW
extern const char *ns_help_raw[];
#endif
extern const char *ns_help_userlist[];
extern const char *ns_help_channellist[];
extern const char *ns_help_serverlist[];
extern const char *ns_help_banlist[];
extern const char *ns_help_load[];
extern const char *ns_help_unload[];
extern const char *ns_help_modlist[];
extern const char *ns_help_jupe[];
extern const char *ns_help_level[];
extern const char *ns_help_botlist[];
extern const char *ns_help_socklist[];
extern const char *ns_help_timerlist[];
extern const char *ns_help_status[];

extern const char ns_help_level_oneline[];
extern const char ns_help_status_oneline[];
extern const char ns_help_shutdown_oneline[];
extern const char ns_help_reload_oneline[];
extern const char ns_help_load_oneline[];
extern const char ns_help_unload_oneline[];
extern const char ns_help_jupe_oneline[];
extern const char ns_help_exclude_oneline[];
#ifdef USE_RAW
extern const char ns_help_raw_oneline[];
#endif
extern const char ns_help_botlist_oneline[];
extern const char ns_help_socklist_oneline[];
extern const char ns_help_timerlist_oneline[];
extern const char ns_help_modlist_oneline[];
extern const char ns_help_userlist_oneline[];
extern const char ns_help_channellist_oneline[];
extern const char ns_help_serverlist_oneline[];
extern const char ns_help_banlist_oneline[];

extern const char *ns_help_set_nick[];
extern const char *ns_help_set_altnick[];
extern const char *ns_help_set_user[];
extern const char *ns_help_set_host[];
extern const char *ns_help_set_realname[];
extern const char *ns_help_set_joinserviceschan[];
extern const char *ns_help_set_splittime[];
extern const char *ns_help_set_msgsampletime[];
extern const char *ns_help_set_msgthreshold[];
extern const char *ns_help_set_pingtime[];
extern const char *ns_help_set_versionscan[];
extern const char *ns_help_set_servicecmode[];
extern const char *ns_help_set_serviceumode[];
extern const char *ns_help_set_loglevel[];
extern const char *ns_help_set_cmdchar[];
extern const char *ns_help_set_cmdreport[];
extern const char *ns_help_set_debug[];
extern const char *ns_help_set_debuglevel[];
extern const char *ns_help_set_debugchan[];
extern const char *ns_help_set_debugtochan[];
extern const char *ns_help_set_debugmodule[];

extern const char cmd_help_oneline[];
extern const char cmd_help_about_oneline[];
extern const char cmd_help_credits_oneline[];
extern const char cmd_help_version_oneline[];
extern const char cmd_help_levels_oneline[];
extern const char *cmd_help_help[];
extern const char *cmd_help_about[];
extern const char *cmd_help_credits[];
extern const char *cmd_help_version[];
extern const char *cmd_help_levels[];
extern const char *cmd_help_set[];

#endif /* _NS_HELP_H_ */
