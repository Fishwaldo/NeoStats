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

/** template.c 
 *  You can copy this file as a template for writing your own modules
 */

/* neostats.h is the only required include for your module to access the 
 * NeoStats module API. You should not need to include any other NeoStats
 * files in order to develop your module.
 */
#include "neostats.h"	/* NeoStats API */

/** When we create a a bot, we must store the handle returned to us for use
 *  when calling API bot functions
 */
static Bot *template_bot;

/** When a module loads, it is passed a handle that includes information
 *  we might need later so you should store this.
 */
static Module* template_module;

/** Define information about our bot
 */
BotInfo template_bot_info = 
{
	/* REQUIRED: 
	 * nick */
	"changeme",
	/* OPTIONAL: 
	 * altnick, use "" if not needed */
	"altnick",
	/* REQUIRED: 
	 * user */
	"changeme",
	/* REQUIRED: 
	 * host */
	"",
	/* REQUIRED: 
	 * realname */
	"Example NeoStats module",
};

/** 
 *  Example copyright text
 */
const char* template_copyright[] = 
{
	"Copyright (c) 1999-2004, NeoStats",
	NULL
};

/** 
 *  Help text for example command
 */
const char template_help_hello_world_oneline[]="Give someone a rose";
const char *template_help_hello_world[] = {
	"Syntax: \2HELLOWORLD\2",
	"",
	"Example of a privmsg command which just sends HELLO WORLD",
	"to the services channel.",
	NULL
};

/** 
 *  Example about text
 */
const char* template_about[] = 
{
	"Template is an example module to demonstrate features",
	"of the NeoStats API available to module coders",
	NULL
};

/** Module Info definition 
 *	This describes the module to the NeoStats core and provides information
 *  to end users when modules are queried.
 *  The presence of this structure is required but some fields are optional.
 */
ModuleInfo module_info = {
	/* REQUIRED: 
	 * name of module e.g. StatServ */
	"Template",
	/* REQUIRED: 
	 * one line brief description of module */
	"Put your brief module description here",
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with copyright information
	 * NeoStats will automatically provide a CREDITS command to output this
	 * use NULL for none */
	template_copyright,
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with extended description
	 * NeoStats will automatically provide an ABOUT command to output this
	 * use NULL for none */
	template_about,
	/* REQUIRED: 
	 * version of neostats used to build module
	 * must be NEOSTATS_VERSION */
	NEOSTATS_VERSION,
	/* REQUIRED: 
	 * string containing version of module */
	"1.0",
	/* REQUIRED: string containing build date of module 
	 * should be __DATE__ */
	__DATE__,
	/* REQUIRED: string containing build time of module 
	 * should be __TIME__ */
	__TIME__,
	/* OPTIONAL: 
	 * Module control flags, 
	 * use 0 if not needed */
	0,
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	0,
};

/** 
 *  example command
 *  Just sends "Hello World!" to the services channel
 */
static int template_hello_world(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	chanalert(template_bot->nick, "%s says \"Hello World!\"",
		cmdparams->source.user->nick);
	return 1;
}

/** OPTIONAL:
 *  Table of commands available in our bot
 *  This lists commands that a user can send via privmsg
 */
static bot_cmd template_commands[]=
{
	{"HELLO",	template_hello_world,	0, 	0,	template_help_hello_world,		template_help_hello_world_oneline },
	{NULL,		NULL,					0, 	0,	NULL, 			NULL}
};

/** OPTIONAL:
 *  Table of settings available in our bot
 *  This lists settings that a user can set via privmsg
 */
static bot_setting template_settings[]=
{
	{"NICK",		&template_bot_info.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick, NULL, (void*)"changeme" },
	{"ALTNICK",		&template_bot_info.altnick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "AltNick",	NULL,	ns_help_set_altnick, NULL, (void*)"altnick" },
	{"USER",		&template_bot_info.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user, NULL, (void*)"changeme" },
	{"HOST",		&template_bot_info.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host, NULL, (void*)"" },
	{"REALNAME",	&template_bot_info.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname, NULL, (void*)"Example NeoStats module" },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/** Online event processing
 *  What we do when we first come online
 */
static int tm_event_online(CmdParams* cmdparams)
{
	/* Introduce a bot onto the network saving the bot handle */
	template_bot = init_bot (
		&template_bot_info,		/* REQUIRED: pointer to bot info */
		"-x",					/* OPTIONAL: user modes we want 
								 * for the bot, use "" for none */
		0,						/* OPTIONAL: flags affecting our bot
								 * use 0 for none
								 */
		template_commands,		/* OPTIONAL: pointer to command list
								 * use NULL for none
								 */
		template_settings		/* OPTIONAL: pointer to set command list
								 * use NULL for none
								 */
		);
	return 1;
};

/** Module event list
 *  What events we will act on
 *  This is required if you want your module to respond to events on IRC
 *  see events.h for a list of all events available
 */
ModuleEvent module_events[] = {
	{EVENT_ONLINE,	tm_event_online},
	{EVENT_NULL,	NULL}
};

/** Init module
 *  Required if you need to do initialisation of your module when
 *  first loaded
 */
int ModInit(Module* mod_ptr)
{
	/* Save our module handle */
	template_module = mod_ptr;
	return 1;
}

/** Fini module
 *  Required if you need to do cleanup of your module when it ends
 */
void ModFini()
{

};
