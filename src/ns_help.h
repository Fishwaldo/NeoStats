/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
extern const char *ns_help_userdump[];
extern const char *ns_help_chandump[];
extern const char *ns_help_serverdump[];
extern const char *ns_help_bandump[];
extern const char *ns_help_load[];
extern const char *ns_help_unload[];
extern const char *ns_help_modlist[];
extern const char *ns_help_jupe[];
extern const char *ns_help_level[];
extern const char *ns_help_botlist[];
extern const char *ns_help_socklist[];
extern const char *ns_help_timerlist[];
extern const char *ns_help_botchanlist[];
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
extern const char ns_help_botchanlist_oneline[];
extern const char ns_help_modlist_oneline[];
extern const char ns_help_userdump_oneline[];
extern const char ns_help_chandump_oneline[];
extern const char ns_help_serverdump_oneline[];
extern const char ns_help_bandump_oneline[];

extern const char *ns_help_set_pingtime[];
extern const char *ns_help_set_versionscan[];
extern const char *ns_help_set_servicecmode[];
extern const char *ns_help_set_serviceumode[];
extern const char *ns_help_set_loglevel[];
extern const char *ns_help_set_debug[];
extern const char *ns_help_set_debuglevel[];
extern const char *ns_help_set_debugchan[];
extern const char *ns_help_set_debugtochan[];
extern const char *ns_help_set_debugmodule[];

#endif /* _NS_HELP_H_ */
