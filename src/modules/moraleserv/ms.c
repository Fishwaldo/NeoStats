/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

static Bot *ms_bot;
static BotInfo ms_botinfo = 
{
	"", 
	"", 
	"", 
	"", 
	"", 
};
static Module* ms_module;

static int ms_hail(User * u, char **av, int ac);
static int ms_ode(User * u, char **av, int ac);
static int ms_lapdance(User * u, char **av, int ac);
static int ms_version(User * u, char **av, int ac);
static int ms_about(User * u, char **av, int ac);
static int ms_poem(User * u, char **av, int ac);
static int ms_redneck(User * u, char **av, int ac);
static int ms_cheerup(User * u, char **av, int ac);
static int ms_behappy(User * u, char **av, int ac);
static int ms_wonderful(User * u, char **av, int ac);

ModuleInfo module_info = {
	"MoraleServ",
	"Network morale service",
	NULL,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static bot_cmd ms_commands[]=
{
	{"HAIL",		ms_hail,		2, 	0,	ms_help_hail,		ms_help_hail_oneline },
	{"ODE",			ms_ode,			2, 	0,	ms_help_ode,		ms_help_ode_oneline },
	{"LAPDANCE",	ms_lapdance,	1, 	0,	ms_help_lapdance,	ms_help_lapdance_oneline },
	{"VERSION",		ms_version,		0, 	0,	ms_help_version,	ms_help_version_oneline },
	{"ABOUT",		ms_about,		0, 	0,	ms_help_about,		ms_help_about_oneline },
	{"POEM",		ms_poem,		2, 	0,	ms_help_poem,		ms_help_poem_oneline },
	{"REDNECK",		ms_redneck,		1, 	0,	ms_help_redneck,	ms_help_redneck_oneline },
	{"CHEERUP",		ms_cheerup,		1, 	0,	ms_help_cheerup,	ms_help_cheerup_oneline },
	{"BEHAPPY",		ms_behappy,		1, 	0,	ms_help_behappy,	ms_help_behappy_oneline },
	{"WONDERFUL",	ms_wonderful,	1, 	0,	ms_help_wonderful,	ms_help_wonderful_oneline },
	{NULL,			NULL,			0, 	0,	NULL, 				NULL}
};

static bot_setting ms_settings[]=
{
	{"NICK",	&ms_botinfo.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",	&ms_botinfo.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",	&ms_botinfo.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",&ms_botinfo.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
};

static int Online(char **av, int ac)
{
	ms_bot = init_bot (ms_module, &ms_botinfo, services_bot_modes, BOT_FLAG_DEAF, ms_commands, ms_settings);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

int ModInit(Module* mod_ptr)
{
 	char *temp = NULL;

	if(GetConf((void *) &temp, CFGSTR, "Nick") < 0) {
		strlcpy(ms_botinfo.nick ,"MoraleServ" ,MAXNICK);
	}
	else {
		strlcpy(ms_botinfo.nick , temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(ms_botinfo.user, "SS", MAXUSER);
	}
	else {
		strlcpy(ms_botinfo.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(ms_botinfo.host, me.name, MAXHOST);
	}
	else {
		strlcpy(ms_botinfo.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(ms_botinfo.realname, "A Network Morale Service", MAXREALNAME);
	}
	else {
		strlcpy(ms_botinfo.realname, temp, MAXREALNAME);
		free(temp);
	}
	return 1;
}

void ModFini()
{
};

/* Routine for VERSION */
static int ms_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, ms_botinfo.nick, "\2%s Version Information\2", ms_botinfo.nick);
	prefmsg(u->nick, ms_botinfo.nick, "%s Version: %s Compiled %s at %s", ms_botinfo.nick, 
		module_info.version, module_info.build_date, module_info.build_time);
	prefmsg(u->nick, ms_botinfo.nick, "%s Author: ^Enigma^ <enigma@neostats.net>", ms_botinfo.nick);
	prefmsg(u->nick, ms_botinfo.nick, "http://www.neostats.net");
	return 1;
}

/* Routine for ABOUT */
static int ms_about(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	privmsg_list(u->nick, ms_botinfo.nick, ms_help_about);
	return 1;
}

/* Routine for HAIL */
static int ms_hail(User * u, char **av, int ac)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = av[2];
	target_nick = av[3];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ms_botinfo.nick, 
		"Your \"HAIL\" song greeting has been sent to %s!", target_nick);
	prefmsg(target_nick, ms_botinfo.nick, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, ms_botinfo.nick,
		"*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",
		about_nick, about_nick, about_nick);
	return 1;     
}

/* Routine for LAPDANCE */
static int ms_lapdance(User * u, char **av, int ac)
{
	char *target_nick;
	
	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, ms_botinfo.nick,
		"*%s Seductively walks up to %s and gives %s a sly look*",
		ms_botinfo.nick, target_nick, target_nick);
	prefmsg(target_nick, ms_botinfo.nick,
		"*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",
		ms_botinfo.nick, target_nick, target_nick);
	prefmsg(target_nick, ms_botinfo.nick,
		"*I Think we both need a cold shower now*... *wink*");
	return 1;
}

/* Routine for ODE */
static int ms_ode(User * u, char **av, int ac)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = av[2];
	target_nick = av[3];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ms_botinfo.nick,
		"Your ODE to %s has been sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, ms_botinfo.nick, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, ms_botinfo.nick, "*recites*");
	prefmsg(target_nick, ms_botinfo.nick, "How I wish to be a %s,", about_nick);
	prefmsg(target_nick, ms_botinfo.nick, "a %s I would like to be.", about_nick);
	prefmsg(target_nick, ms_botinfo.nick, "For if I was a %s,", about_nick);
	prefmsg(target_nick, ms_botinfo.nick, "I'd watch the network hail thee.");
	prefmsg(target_nick, ms_botinfo.nick, "*bows*");
	return 1;       
}

/* Routine for POEM */
static int ms_poem(User * u, char **av, int ac)
{
	char *about_nick;
	char *target_nick;

	SET_SEGV_LOCATION();
	about_nick = av[2];
	target_nick = av[3];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ms_botinfo.nick,
		"Your POEM about %s has been sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, ms_botinfo.nick, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, ms_botinfo.nick, "*recites*");
	prefmsg(target_nick, ms_botinfo.nick, "I wish I was a %s,", about_nick);
	prefmsg(target_nick, ms_botinfo.nick, "A %s is never glum,", about_nick);
	prefmsg(target_nick, ms_botinfo.nick, "Coz how can you be grumpy,");
	prefmsg(target_nick, ms_botinfo.nick, "When the sun shines out your bum.");
	prefmsg(target_nick, ms_botinfo.nick, "*bows*");
	return 1;
}

/* Routine for REDNECK */
static int ms_redneck(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ms_botinfo.nick,
		"Your redneck message has been sent to %s!", target_nick);
	prefmsg(target_nick, ms_botinfo.nick, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, ms_botinfo.nick, "*recites*");
	prefmsg(target_nick, ms_botinfo.nick,
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of weird contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.");
	prefmsg(target_nick, ms_botinfo.nick, "*bows*");
	return 1;
}

static int ms_cheerup(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, ms_botinfo.nick, "Cheer up %s .....", target_nick);
	prefmsg(target_nick, ms_botinfo.nick,
		"All of us on the network love you! 3--<--<--<{4@");
	return 1;
}

/* Routine for BEHAPPY */
static int ms_behappy(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, ms_botinfo.nick, "%s thinks that you're a little sad.....",
		u->nick);
	prefmsg(target_nick, ms_botinfo.nick, "*starts singing*");
	prefmsg(target_nick, ms_botinfo.nick,
		"Here's a little song I wrote, You might want to sing it note for note");
	prefmsg(target_nick, ms_botinfo.nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick, " ");
	prefmsg(target_nick, ms_botinfo.nick,
		"In every life we have some trouble, But when you worry you make it Double");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry - Be Happy, Don't Worry - Be Happy now");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry - Be Happy, Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick, " ");
	prefmsg(target_nick, ms_botinfo.nick,
		"Ain't got no place to lay your head, Somebody came and took your bed");
	prefmsg(target_nick, ms_botinfo.nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick, " ");
	prefmsg(target_nick, ms_botinfo.nick,
		"The landlord say your rent is late, He may have to litigate");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry - Be Happy, Look at Me - I'm Happy");
	prefmsg(target_nick, ms_botinfo.nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick,
		"Here I give you my phone number, When you worry call me, I make you happy");
	prefmsg(target_nick, ms_botinfo.nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick, " ");
	prefmsg(target_nick, ms_botinfo.nick,
		"Ain't got not cash, ain't got no style, Ain't got no gal to make you smile");
	prefmsg(target_nick, ms_botinfo.nick, "Don't Worry - Be Happy");
	prefmsg(target_nick, ms_botinfo.nick, " ");
	prefmsg(target_nick, ms_botinfo.nick,
		"'Cause when you worry your face will frown, and that will bring everybody down");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it");
	prefmsg(target_nick, ms_botinfo.nick,
		"Be Happy - Put a smile on your face, Don't bring everybody down");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry, it will soon pass, whatever it is");
	prefmsg(target_nick, ms_botinfo.nick,
		"Don't Worry - Be Happy, I'm not worried, I'm happy . . . .");
	return 1;
}

/* Routine for WONDERFUL */
static int ms_wonderful(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ms_botinfo.nick, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, ms_botinfo.nick, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, ms_botinfo.nick, "*starts singing*");
	prefmsg(target_nick, ms_botinfo.nick,
		"So excuse me forgetting but these things I do");
	prefmsg(target_nick, ms_botinfo.nick,
		"You see I've forgotten if they're green or they're blue");
	prefmsg(target_nick, ms_botinfo.nick,
		"Anyway the thing is what I really mean");
	prefmsg(target_nick, ms_botinfo.nick,
		"Yours are the sweetest eyes I've ever seen");
	prefmsg(target_nick, ms_botinfo.nick,
		"And you can tell everybody this is your song");
	prefmsg(target_nick, ms_botinfo.nick,
		"It may be quite simple but now that it's done");
	prefmsg(target_nick, ms_botinfo.nick,
		"I hope you don't mind, I hope you don't mind that I put down in words");
	prefmsg(target_nick, ms_botinfo.nick,
		"How wonderful life is while %s is in the world", target_nick);
	return 1;
}
