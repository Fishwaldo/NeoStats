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

static Bot *ms_bot;

const char *ms_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

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

static int ms_event_online(CmdParams* cmdparams)
{
	ms_bot = init_bot (&ms_botinfo);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ms_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	return 1;
}

void ModFini()
{
};

/* Routine for HAIL */
static int ms_hail(CmdParams* cmdparams)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target_nick = cmdparams->av[1];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick, 
		"Your \"HAIL\" song greeting has been sent to %s!", target_nick);
	prefmsg(target_nick, ms_bot->nick, "Courtesy of your friend %s:", cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick,
		"*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",
		about_nick, about_nick, about_nick);
	return 1;     
}

/* Routine for LAPDANCE */
static int ms_lapdance(CmdParams* cmdparams)
{
	char *target_nick;
	
	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick, 
		"Lap dance sent to %s!", target_nick);
	prefmsg(target_nick, ms_bot->nick,
		"*%s Seductively walks up to %s and gives %s a sly look*",
		ms_bot->nick, target_nick, target_nick);
	prefmsg(target_nick, ms_bot->nick,
		"*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",
		ms_bot->nick, target_nick, target_nick);
	prefmsg(target_nick, ms_bot->nick,
		"*I Think we both need a cold shower now*... *wink*");
	return 1;
}

/* Routine for ODE */
static int ms_ode(CmdParams* cmdparams)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target_nick = cmdparams->av[1];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick,
		"Ode to %s sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, ms_bot->nick, "Courtesy of your friend %s:", cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick, "*recites*");
	prefmsg(target_nick, ms_bot->nick, "How I wish to be a %s,", about_nick);
	prefmsg(target_nick, ms_bot->nick, "a %s I would like to be.", about_nick);
	prefmsg(target_nick, ms_bot->nick, "For if I was a %s,", about_nick);
	prefmsg(target_nick, ms_bot->nick, "I'd watch the network hail thee.");
	prefmsg(target_nick, ms_bot->nick, "*bows*");
	return 1;       
}

/* Routine for POEM */
static int ms_poem(CmdParams* cmdparams)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = cmdparams->av[0];
	target_nick = cmdparams->av[1];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick,
		"Poem about %s sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, ms_bot->nick, "Courtesy of your friend %s:", cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick, "*recites*");
	prefmsg(target_nick, ms_bot->nick, "I wish I was a %s,", about_nick);
	prefmsg(target_nick, ms_bot->nick, "A %s is never glum,", about_nick);
	prefmsg(target_nick, ms_bot->nick, "Coz how can you be grumpy,");
	prefmsg(target_nick, ms_bot->nick, "When the sun shines out your bum.");
	prefmsg(target_nick, ms_bot->nick, "*bows*");
	return 1;
}

/* Routine for REDNECK */
static int ms_redneck(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick,
		"Redneck message sent to %s!", target_nick);
	prefmsg(target_nick, ms_bot->nick, "Courtesy of your friend %s:", cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick, "*recites*");
	prefmsg(target_nick, ms_bot->nick,
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of weird contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.");
	prefmsg(target_nick, ms_bot->nick, "*bows*");
	return 1;
}

static int ms_cheerup(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, ms_bot->nick, "Cheer up %s .....", target_nick);
	prefmsg(target_nick, ms_bot->nick,
		"All of us on the network love you! 3--<--<--<{4@");
	return 1;
}

/* Routine for BEHAPPY */
static int ms_behappy(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick, 
		"Behappy sent to %s!", target_nick);
	prefmsg(target_nick, ms_bot->nick, "%s thinks that you're a little sad.....",
		cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick, "*starts singing*");
	prefmsg(target_nick, ms_bot->nick,
		"Here's a little song I wrote, You might want to sing it note for note");
	prefmsg(target_nick, ms_bot->nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick, " ");
	prefmsg(target_nick, ms_bot->nick,
		"In every life we have some trouble, But when you worry you make it Double");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry - Be Happy, Don't Worry - Be Happy now");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry - Be Happy, Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick, " ");
	prefmsg(target_nick, ms_bot->nick,
		"Ain't got no place to lay your head, Somebody came and took your bed");
	prefmsg(target_nick, ms_bot->nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick, " ");
	prefmsg(target_nick, ms_bot->nick,
		"The landlord say your rent is late, He may have to litigate");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry - Be Happy, Look at Me - I'm Happy");
	prefmsg(target_nick, ms_bot->nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick,
		"Here I give you my phone number, When you worry call me, I make you happy");
	prefmsg(target_nick, ms_bot->nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick, " ");
	prefmsg(target_nick, ms_bot->nick,
		"Ain't got not cash, ain't got no style, Ain't got no gal to make you smile");
	prefmsg(target_nick, ms_bot->nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_bot->nick, " ");
	prefmsg(target_nick, ms_bot->nick,
		"'Cause when you worry your face will frown, and that will bring everybody down");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it");
	prefmsg(target_nick, ms_bot->nick,
		"Be Happy - Put a smile on your face, Don't bring everybody down");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry, it will soon pass, whatever it is");
	prefmsg(target_nick, ms_bot->nick,
		"Don't Worry - Be Happy, I'm not worried, I'm happy . . . .");
	return 1;
}

/* Routine for WONDERFUL */
static int ms_wonderful(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ms_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ms_bot->nick, 
		"wonderful sent to %s!", target_nick);
	prefmsg(target_nick, ms_bot->nick, "Courtesy of your friend %s:", 
		cmdparams->source.user->nick);
	prefmsg(target_nick, ms_bot->nick, "*starts singing*");
	prefmsg(target_nick, ms_bot->nick,
		"So excuse me forgetting but these things I do");
	prefmsg(target_nick, ms_bot->nick,
		"You see I've forgotten if they're green or they're blue");
	prefmsg(target_nick, ms_bot->nick,
		"Anyway the thing is what I really mean");
	prefmsg(target_nick, ms_bot->nick,
		"Yours are the sweetest eyes I've ever seen");
	prefmsg(target_nick, ms_bot->nick,
		"And you can tell everybody this is your song");
	prefmsg(target_nick, ms_bot->nick,
		"It may be quite simple but now that it's done");
	prefmsg(target_nick, ms_bot->nick,
		"I hope you don't mind, I hope you don't mind that I put down in words");
	prefmsg(target_nick, ms_bot->nick,
		"How wonderful life is while %s is in the world", target_nick);
	return 1;
}
