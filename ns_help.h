/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* ns_help.c */
extern const char *ns_help[];
extern const char *ns_help_on_help[];
extern const char *ns_sa_help[];
extern const char *ns_sr_help[];
extern const char *ns_shutdown_help[];
extern const char *ns_reload_help[];
extern const char *ns_logs_help[];
#ifdef USE_RAW
extern const char *ns_raw_help[];
#endif
extern const char *ns_debug_help[];
extern const char *ns_userdump_help[];
extern const char *ns_chandump_help[];
extern const char *ns_serverdump_help[];
extern const char *ns_version_help[];
extern const char *ns_load_help[];
extern const char *ns_unload_help[];
extern const char *ns_modlist_help[];
extern const char *ns_jupe_help[];
extern const char *ns_level_help[];
extern const char *ns_botlist_help[];
extern const char *ns_socklist_help[];
extern const char *ns_timerlist_help[];
extern const char *ns_botchanlist_help[];
extern const char *ns_info_help[];

extern const char ns_help_help_oneline[];
extern const char ns_level_help_oneline[];
extern const char ns_info_help_oneline[];
extern const char ns_version_help_oneline[];
extern const char ns_shutdown_help_oneline[];
extern const char ns_reload_help_oneline[];
extern const char ns_logs_help_oneline[];
extern const char ns_load_help_oneline[];
extern const char ns_unload_help_oneline[];
extern const char ns_jupe_help_oneline[];
#ifdef USE_RAW
extern const char ns_raw_help_oneline[];
#endif
extern const char ns_debug_help_oneline[];
extern const char ns_botlist_help_oneline[];
extern const char ns_socklist_help_oneline[];
extern const char ns_timerlist_help_oneline[];
extern const char ns_botchanlist_help_oneline[];
extern const char ns_modlist_help_oneline[];
extern const char ns_userdump_help_oneline[];
extern const char ns_chandump_help_oneline[];
extern const char ns_serverdump_help_oneline[];

#endif /* _NS_HELP_H_ */
