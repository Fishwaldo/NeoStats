/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "ms.h"

static int ms_hail(CmdParams* cmdparams);
static int ms_ode(CmdParams* cmdparams);
static int ms_lapdance(CmdParams* cmdparams);
static int ms_poem(CmdParams* cmdparams);
static int ms_redneck(CmdParams* cmdparams);
static int ms_cheerup(CmdParams* cmdparams);
static int ms_behappy(CmdParams* cmdparams);
static int ms_wonderful(CmdParams* cmdparams);

/** Bot pointer */
static Bot *ms_bot;

/** Copyright info */
const char *ms_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"MoraleServ",
	"Network morale service",
	ms_copyright, // Author: ^Enigma^ <enigma@neostats.net>
	ms_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot comand table */
static bot_cmd ms_commands[]=
{
	{"LAPDANCE",	ms_lapdance,	1, 	0,	ms_help_lapdance,	ms_help_lapdance_oneline },
	{"REDNECK",		ms_redneck,		1, 	0,	ms_help_redneck,	ms_help_redneck_oneline },
	{"CHEERUP",		ms_cheerup,		1, 	0,	ms_help_cheerup,	ms_help_cheerup_oneline },
	{"BEHAPPY",		ms_behappy,		1, 	0,	ms_help_behappy,	ms_help_behappy_oneline },
	{"WONDERFUL",	ms_wonderful,	1, 	0,	ms_help_wonderful,	ms_help_wonderful_oneline },
	{"HAIL",		ms_hail,		2, 	0,	ms_help_hail,		ms_help_hail_oneline },
	{"ODE",			ms_ode,			2, 	0,	ms_help_ode,		ms_help_ode_oneline },
	{"POEM",		ms_poem,		2, 	0,	ms_help_poem,		ms_help_poem_oneline },
	{NULL,			NULL,			0, 	0,	NULL, 				NULL}
};

/** BotInfo */
static BotInfo ms_botinfo = 
{
	"MoraleServ", 
	"MoraleServ1", 
	"MS", 
	BOT_COMMON_HOST, 
	"Network morale service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ms_commands, 
	NULL,
};

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
	ms_bot = AddBot (&ms_botinfo);
	if (!ms_bot) {
		return NS_FAILURE;
	}
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
}

/* Routine for HAIL */
static int ms_hail(CmdParams* cmdparams)
{
	char *about_nick;
	Client *target;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[1]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Your \"HAIL\" song greeting has been sent to %s!", target->name);
	irc_prefmsg(ms_bot, target, "Courtesy of your friend %s:", cmdparams->source->name);
	irc_prefmsg(ms_bot, target, 
		"*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",
		about_nick, about_nick, about_nick);
	return NS_SUCCESS;     
}

/* Routine for LAPDANCE */
static int ms_lapdance(CmdParams* cmdparams)
{
	Client *target;
	
	SET_SEGV_LOCATION();
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Lap dance sent to %s!", target->name);
	irc_prefmsg(ms_bot, target, 
		"*%s Seductively walks up to %s and gives %s a sly look*",
		ms_bot->name, target->name, target->name);
	irc_prefmsg(ms_bot, target, 
		"*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",
		ms_bot->name, target->name, target->name);
	irc_prefmsg(ms_bot, target, 
		"*I Think we both need a cold shower now*... *wink*");
	return NS_SUCCESS;
}

/* Routine for ODE */
static int ms_ode(CmdParams* cmdparams)
{
	char *about_nick;
	Client *target;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[1]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Ode to %s sent to %s!", about_nick, target->name);
	irc_prefmsg(ms_bot, target, "Courtesy of your friend %s:", cmdparams->source->name);
	irc_prefmsg(ms_bot, target, "*recites*");
	irc_prefmsg(ms_bot, target, "How I wish to be a %s,", about_nick);
	irc_prefmsg(ms_bot, target, "a %s I would like to be.", about_nick);
	irc_prefmsg(ms_bot, target, "For if I was a %s,", about_nick);
	irc_prefmsg(ms_bot, target, "I'd watch the network hail thee.");
	irc_prefmsg(ms_bot, target, "*bows*");
	return NS_SUCCESS;       
}

/* Routine for POEM */
static int ms_poem(CmdParams* cmdparams)
{
	char *about_nick;
	Client *target;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[1]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Poem about %s sent to %s!", about_nick, target->name);
	irc_prefmsg(ms_bot, target, "Courtesy of your friend %s:", cmdparams->source->name);
	irc_prefmsg(ms_bot, target, "*recites*");
	irc_prefmsg(ms_bot, target, "I wish I was a %s,", about_nick);
	irc_prefmsg(ms_bot, target, "A %s is never glum,", about_nick);
	irc_prefmsg(ms_bot, target, "Coz how can you be grumpy,");
	irc_prefmsg(ms_bot, target, "When the sun shines out your bum.");
	irc_prefmsg(ms_bot, target, "*bows*");
	return NS_SUCCESS;
}

/* Routine for REDNECK */
static int ms_redneck(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Redneck message sent to %s!", target->name);
	irc_prefmsg(ms_bot, target,  "Courtesy of your friend %s:", cmdparams->source->name);
	irc_prefmsg(ms_bot, target,  "*recites*");
	irc_prefmsg(ms_bot, target, 
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of weird contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.");
	irc_prefmsg(ms_bot, target,  "*bows*");
	return NS_SUCCESS;
}

static int ms_cheerup(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source, 
		"Cheerup sent to %s!", target->name);
	irc_prefmsg(ms_bot, target,  "Cheer up %s .....", target->name);
	irc_prefmsg(ms_bot, target, 
		"All of us on the network love you! 3--<--<--<{4@");
	return NS_SUCCESS;
}

/* Routine for BEHAPPY */
static int ms_behappy(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source,  
		"Behappy sent to %s!", target->name);
	irc_prefmsg(ms_bot, target,  "%s thinks that you're a little sad.....",
		cmdparams->source->name);
	irc_prefmsg(ms_bot, target,  "*starts singing*");
	irc_prefmsg(ms_bot, target, 
		"Here's a little song I wrote, You might want to sing it note for note");
	irc_prefmsg(ms_bot, target,  "Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target,  " ");
	irc_prefmsg(ms_bot, target, 
		"In every life we have some trouble, But when you worry you make it Double");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry - Be Happy, Don't Worry - Be Happy now");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry - Be Happy, Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target,  " ");
	irc_prefmsg(ms_bot, target, 
		"Ain't got no place to lay your head, Somebody came and took your bed");
	irc_prefmsg(ms_bot, target,  "Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target,  " ");
	irc_prefmsg(ms_bot, target, 
		"The landlord say your rent is late, He may have to litigate");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry - Be Happy, Look at Me - I'm Happy");
	irc_prefmsg(ms_bot, target,  "Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target, 
		"Here I give you my phone number, When you worry call me, I make you happy");
	irc_prefmsg(ms_bot, target,  "Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target,  " ");
	irc_prefmsg(ms_bot, target, 
		"Ain't got not cash, ain't got no style, Ain't got no gal to make you smile");
	irc_prefmsg(ms_bot, target,  "Don't Worry - Be Happy");
	irc_prefmsg(ms_bot, target,  " ");
	irc_prefmsg(ms_bot, target, 
		"'Cause when you worry your face will frown, and that will bring everybody down");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it");
	irc_prefmsg(ms_bot, target, 
		"Be Happy - Put a smile on your face, Don't bring everybody down");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry, it will soon pass, whatever it is");
	irc_prefmsg(ms_bot, target, 
		"Don't Worry - Be Happy, I'm not worried, I'm happy . . . .");
	return NS_SUCCESS;
}

/* Routine for WONDERFUL */
static int ms_wonderful(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ms_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ms_bot, cmdparams->source,  
		"wonderful sent to %s!", target->name);
	irc_prefmsg(ms_bot, target, "Courtesy of your friend %s:", 
		cmdparams->source->name);
	irc_prefmsg(ms_bot, target, "*starts singing*");
	irc_prefmsg(ms_bot, target, 
		"So excuse me forgetting but these things I do");
	irc_prefmsg(ms_bot, target, 
		"You see I've forgotten if they're green or they're blue");
	irc_prefmsg(ms_bot, target, 
		"Anyway the thing is what I really mean");
	irc_prefmsg(ms_bot, target, 
		"Yours are the sweetest eyes I've ever seen");
	irc_prefmsg(ms_bot, target, 
		"And you can tell everybody this is your song");
	irc_prefmsg(ms_bot, target, 
		"It may be quite simple but now that it's done");
	irc_prefmsg(ms_bot, target, 
		"I hope you don't mind, I hope you don't mind that I put down in words");
	irc_prefmsg(ms_bot, target, 
		"How wonderful life is while %s is in the world", target->name);
	return NS_SUCCESS;
}
