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

/** template.c 
 * You can copy this file as a template for writing your own modules
 */

#include <stdio.h>
#include "neostats.h"	/* Neostats API */

/** 
 * A string to hold the name of our bot
 */
char s_module_bot_name[MAXNICK];

/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo __module_info = {
	"Template",
	"Template Module Description",
	"Version 1.0",
	__DATE__,
	__TIME__
};

/** Module function list
 * A list of IRCd (server) commands that we will respond to
******************************** WARNING ********************************
This table is optional but depreciated. You should respond to module 
events to be portable across all ircds and only use this table as a
last resort if a module event is not available. This table will be 
removed from operation in a future release. If you need to use this 
table for anything, tell us so we can add an appropiate event handler.
******************************** WARNING ********************************
 */
Functions __module_functions[] = {
	{NULL, NULL, 0}
};

/** Channel message processing
 * What do we do with messages in channels
 * This is required if you want your module to respond to channel messages
 */
int __ChanMessage(char *origin, char **argv, int argc)
{
	char *chan = argv[0];
	return 1;
}

/** Bot message processing
 *  What do we do with messages sent to our bot with /msg
 *  This is required if you want your module to respond to /msg
 *  Parameters:
 *      origin - who sent the message to you. It could be a user nickname 
 *               or could be a server message
 *      argv[0] - Your bot name;
 *      argv[1] .. argv[argc] - the parameters sent in the message
 *      argc - The count of arguments received
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
 * see events.h for a list of all events available
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
	strlcpy(s_module_bot_name, "TemplateBot", MAXNICK);
	return 1;
}

/** Init module
 * This is required if you need to do cleanup of your module when it ends
 */
void __ModFini()
{

};
