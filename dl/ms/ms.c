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

static char s_MoraleServ[MAXNICK];
static ModUser *ms_bot;

struct ms_cfg { 
	char user[MAXUSER];
	char host[MAXHOST];
	char rname[MAXREALNAME];
} ms_cfg;

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

ModuleInfo __module_info = {
	"MoraleServ",
	"A Network Morale Service",
	NEOSTATS_VERSION,
	__DATE__,
	__TIME__
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
	{"NICK",		&s_MoraleServ,			SET_TYPE_STRING,	0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",		&ms_cfg.user,			SET_TYPE_STRING,	0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",		&ms_cfg.host,			SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",	&ms_cfg.rname,		SET_TYPE_STRING,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
};

static int Online(char **av, int ac)
{
	ms_bot = init_mod_bot(s_MoraleServ, ms_cfg.user, ms_cfg.host, ms_cfg.rname, 
		services_bot_modes, BOT_FLAG_DEAF, ms_commands, ms_settings, __module_info.module_name);
	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
 	char *temp = NULL;

	/* Check that our compiled version if compatible with the calling version of NeoStats */
	if(	ircstrncasecmp (me.version, NEOSTATS_VERSION, VERSIONSIZE) !=0) {
		return NS_ERR_VERSION;
	}
	if(GetConf((void *) &temp, CFGSTR, "Nick") < 0) {
		strlcpy(s_MoraleServ ,"MoraleServ" ,MAXNICK);
	}
	else {
		strlcpy(s_MoraleServ , temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(ms_cfg.user, "SS", MAXUSER);
	}
	else {
		strlcpy(ms_cfg.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(ms_cfg.host, me.name, MAXHOST);
	}
	else {
		strlcpy(ms_cfg.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(ms_cfg.rname, "A Network Morale Service", MAXREALNAME);
	}
	else {
		strlcpy(ms_cfg.rname, temp, MAXREALNAME);
		free(temp);
	}
	return 1;
}

void __ModFini()
{
};

/* Routine for VERSION */
static int ms_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, s_MoraleServ, "\2%s Version Information\2", s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ, "%s Version: %s Compiled %s at %s", s_MoraleServ, 
		__module_info.module_version, __module_info.module_build_date, __module_info.module_build_time);
	prefmsg(u->nick, s_MoraleServ, "%s Author: ^Enigma^ <enigma@neostats.net>", s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ, "http://www.neostats.net");
	return 1;
}

/* Routine for ABOUT */
static int ms_about(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	privmsg_list(u->nick, s_MoraleServ, ms_help_about);
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
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_MoraleServ, 
		"Your \"HAIL\" song greeting has been sent to %s!", target_nick);
	prefmsg(target_nick, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, s_MoraleServ,
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
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, s_MoraleServ,
		"*%s Seductively walks up to %s and gives %s a sly look*",
		s_MoraleServ, target_nick, target_nick);
	prefmsg(target_nick, s_MoraleServ,
		"*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",
		s_MoraleServ, target_nick, target_nick);
	prefmsg(target_nick, s_MoraleServ,
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
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_MoraleServ,
		"Your ODE to %s has been sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, s_MoraleServ, "*recites*");
	prefmsg(target_nick, s_MoraleServ, "How I wish to be a %s,", about_nick);
	prefmsg(target_nick, s_MoraleServ, "a %s I would like to be.", about_nick);
	prefmsg(target_nick, s_MoraleServ, "For if I was a %s,", about_nick);
	prefmsg(target_nick, s_MoraleServ, "I'd watch the network hail thee.");
	prefmsg(target_nick, s_MoraleServ, "*bows*");
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
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_MoraleServ,
		"Your POEM about %s has been sent to %s!", about_nick, target_nick);
	prefmsg(target_nick, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, s_MoraleServ, "*recites*");
	prefmsg(target_nick, s_MoraleServ, "I wish I was a %s,", about_nick);
	prefmsg(target_nick, s_MoraleServ, "A %s is never glum,", about_nick);
	prefmsg(target_nick, s_MoraleServ, "Coz how can you be grumpy,");
	prefmsg(target_nick, s_MoraleServ, "When the sun shines out your bum.");
	prefmsg(target_nick, s_MoraleServ, "*bows*");
	return 1;
}

/* Routine for REDNECK */
static int ms_redneck(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_MoraleServ,
		"Your redneck message has been sent to %s!", target_nick);
	prefmsg(target_nick, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, s_MoraleServ, "*recites*");
	prefmsg(target_nick, s_MoraleServ,
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of weird contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.");
	prefmsg(target_nick, s_MoraleServ, "*bows*");
	return 1;
}

static int ms_cheerup(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, s_MoraleServ, "Cheer up %s .....", target_nick);
	prefmsg(target_nick, s_MoraleServ,
		"All of us on the network love you! 3--<--<--<{4@");
	return 1;
}

/* Routine for BEHAPPY */
static int ms_behappy(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, s_MoraleServ, "%s thinks that you're a little sad.....",
		u->nick);
	prefmsg(target_nick, s_MoraleServ, "*starts singing*");
	prefmsg(target_nick, s_MoraleServ,
		"Here's a little song I wrote, You might want to sing it note for note");
	prefmsg(target_nick, s_MoraleServ, "Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ, " ");
	prefmsg(target_nick, s_MoraleServ,
		"In every life we have some trouble, But when you worry you make it Double");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry - Be Happy now");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ, " ");
	prefmsg(target_nick, s_MoraleServ,
		"Ain't got no place to lay your head, Somebody came and took your bed");
	prefmsg(target_nick, s_MoraleServ, "Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ, " ");
	prefmsg(target_nick, s_MoraleServ,
		"The landlord say your rent is late, He may have to litigate");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry - Be Happy, Look at Me - I'm Happy");
	prefmsg(target_nick, s_MoraleServ, "Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ,
		"Here I give you my phone number, When you worry call me, I make you happy");
	prefmsg(target_nick, s_MoraleServ, "Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ, " ");
	prefmsg(target_nick, s_MoraleServ,
		"Ain't got not cash, ain't got no style, Ain't got no gal to make you smile");
	prefmsg(target_nick, s_MoraleServ, "Don't Worry - Be Happy");
	prefmsg(target_nick, s_MoraleServ, " ");
	prefmsg(target_nick, s_MoraleServ,
		"'Cause when you worry your face will frown, and that will bring everybody down");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it");
	prefmsg(target_nick, s_MoraleServ,
		"Be Happy - Put a smile on your face, Don't bring everybody down");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry, it will soon pass, whatever it is");
	prefmsg(target_nick, s_MoraleServ,
		"Don't Worry - Be Happy, I'm not worried, I'm happy . . . .");
	return 1;
}

/* Routine for WONDERFUL */
static int ms_wonderful(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_MoraleServ, u, target_nick)) {
		return 0;
	}
	prefmsg(target_nick, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(target_nick, s_MoraleServ, "*starts singing*");
	prefmsg(target_nick, s_MoraleServ,
		"So excuse me forgetting but these things I do");
	prefmsg(target_nick, s_MoraleServ,
		"You see I've forgotten if they're green or they're blue");
	prefmsg(target_nick, s_MoraleServ,
		"Anyway the thing is what I really mean");
	prefmsg(target_nick, s_MoraleServ,
		"Yours are the sweetest eyes I've ever seen");
	prefmsg(target_nick, s_MoraleServ,
		"And you can tell everybody this is your song");
	prefmsg(target_nick, s_MoraleServ,
		"It may be quite simple but now that it's done");
	prefmsg(target_nick, s_MoraleServ,
		"I hope you don't mind, I hope you don't mind that I put down in words");
	prefmsg(target_nick, s_MoraleServ,
		"How wonderful life is while %s is in the world", target_nick);
	return 1;
}
