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
 *  You can copy this file as a template for writing your own modules
 */

#include "neostats.h"	/* Neostats API */

/** 
 *  A string to hold the name of our bot
 */
char s_module_bot_name[MAXNICK];

/** Module Info definition 
 *  Information about our module
 *  This structure is required for your module to load and run on NeoStats
 */
ModuleInfo module_info = {
	"Template",
	"Put your brief module description here",
	"NeoStats",
	NEOSTATS_VERSION,
	"1.0",
	__DATE__,
	__TIME__
};

/** Online event processing
 *  What we do when we first come online
 */
static int Online(char **av, int ac)
{
	/* Introduce a bot onto the network */
	template_bot = init_bot(template_module, s_module_bot_name, "user", me.name, "Real Name",
		  "-x", 0, NULL, NULL);

	if (init_bot(s_module_bot_name, "user", me.name, "Real Name", "-x",
		module_info.module_name) == -1) {
			/* Nick was in use */
			return 0;
	}
	return 1;
};

/** Module event list
 *  What events we will act on
 *  This is required if you want your module to respond to events on IRC
 *  see events.h for a list of all events available
 */
ModuleEvent module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

/** Init module
 *  Required if you need to do initialisation of your module when
 *  first loaded
 */
int ModInit(int modnum, int apiver)
{
	/* Check that our compiled version if compatible with the calling version of NeoStats */
	if(	ircstrncasecmp (me.version, NEOSTATS_VERSION, VERSIONSIZE) !=0) {
		return NS_ERR_VERSION;
	}
	strlcpy(s_module_bot_name, "TemplateBot", MAXNICK);
	return 1;
}

/** Init module
 *  Required if you need to do cleanup of your module when it ends
 */
void ModFini()
{

};
