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

/** template.c 
 * You can copy this file as a template for writing your own modules
 */

#include <stdio.h>
#include "dl.h"       /* Required for module */
#include "stats.h"    /* Required for bot support */
#include "log.h"      /* Log systems support */

/** 
 * A string to hold the name of our bot
 */
char *s_module_bot_name;

/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo __module_info = {
	"example",
	"example Module Description",
	"version 1.0"
	__DATE__,
	__TIME__
};

/** printf version information
 * respond to the /VERSION command on IRC with this text
 * This is recommended for your module to load and run on NeoStats
 */
int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(351, origin, "Module Template Loaded, Version: %s %s %s",
		__module_info.module_version, __module_info.module_build_date,
		__module_info.module_build_time);
	return 0;
}

/** Module function list
 * A list of IRCd (server) commands that we will respond to
 * e.g. VERSION
 * This table is required for your module to load and run on NeoStats
 * but you do not have to have any functions in it
 */
Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1},
#ifdef GOTTOKENSUPPORT
	{TOK_VERSION, new_m_version, 1},
#endif
	{NULL, NULL, 0}
};

/** Channel message processing
 * What do we do with messages in channels
 * This is required if you want your module to respond to channel messages
 */
int __ChanMessage(char *origin, char **argv, int argc)
{
	char *chan = av[0];
	return 1;
}

/** Bot message processing
 * What do we do with messages sent to our bot with /mag
 * This is required if you want your module to respond to /msg
 */
int __BotMessage(char *origin, char **argv, int argc)
{
	User *u;
	char *buf;
	u = finduser(origin);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE, "Unable to find user %s ", origin);
		return -1;
	}
	buf = joinbuf(argv, argc, 1);
	globops(me.name, "Bot recieved %s from (%s!%s@%s)", buf, u->nick, u->username, u->hostname);
	chanalert(s_module_bot_name, "Bot recieved %s from (%s!%s@%s)", buf, u->nick, u->username, u->hostname);
	nlog(LOG_NORMAL, LOG_MOD, "Bot recieved %s from (%s!%s@%s)", buf, u->nick, u->username, u->hostname);
	free(buf);
	return 1;
}

/** Online event processing
 * What we do when we first come online
 */
static int Online(char **av, int ac)
{
	/* Introduce a bot onto the network */
	if (init_bot(s_module_bot_name, "user", me.name, "Real Name", "-x",
		__module_info.module_name) == -1) {
			/* Nick was in use */
			return 0;
	}
	return 1;
};

/** Module event list
 * What events we will act on
 * This is required if you want your module to respond to events on IRC
 * see modules.txt for a list of all events available
 */
EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int __ModInit(int modnum, int apiver)
{
	s_module_bot_name = "TemplateBot";
	return 1;
}

/** Init module
 * This is required if you need to do cleanup of your module when it ends
 */
void __ModFini()
{

};
