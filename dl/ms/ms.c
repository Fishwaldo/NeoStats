/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "log.h"
#include "ms_help.c"

char s_MoraleServ[MAXNICK];

ModUser *ms_bot;

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
static int new_m_version(char *origin, char **av, int ac);

ModuleInfo __module_info = {
   "MoraleServ",
   "A Network Morale Service",
   "2.22",
	__DATE__,
	__TIME__
};

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(RPL_VERSION, origin,
		     "Module MoraleServ Loaded, Version: %s %s %s",
			 __module_info.module_version, __module_info.module_build_date,
			 __module_info.module_build_time);
	return 0;
}

Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};

static bot_cmd ms_commands[]=
{
	{"HAIL",		ms_hail,		0, 	0,	ms_help_hail,		1,	ms_help_hail_oneline },
	{"ODE",			ms_ode,			0, 	0,	ms_help_ode,		1,	ms_help_ode_oneline },
	{"LAPDANCE",	ms_lapdance,	0, 	0,	ms_help_lapdance,	1,	ms_help_lapdance_oneline },
	{"VERSION",		ms_version,		0, 	0,	ms_help_version,	1,	ms_help_version_oneline },
	{"ABOUT",		ms_about,		0, 	0,	ms_help_about,		1,	ms_help_about_oneline },
	{"POEM",		ms_poem,		0, 	0,	ms_help_poem,		1,	ms_help_poem_oneline },
	{"REDNECK",		ms_redneck,		0, 	0,	ms_help_redneck,	1,	ms_help_redneck_oneline },
	{"CHEERUP",		ms_cheerup,		0, 	0,	ms_help_cheerup,	1,	ms_help_cheerup_oneline },
	{"BEHAPPY",		ms_behappy,		0, 	0,	ms_help_behappy,	1,	ms_help_behappy_oneline },
	{"WONDERFUL",	ms_wonderful,	0, 	0,	ms_help_wonderful,	1,	ms_help_wonderful_oneline },
	{NULL,			NULL,			0, 	0,			NULL, 		0,	NULL}
};

int Online(char **av, int ac)
{
	ms_bot = init_mod_bot(s_MoraleServ, "MS", me.name, "A Network Morale Service",
		services_bot_modes,BOT_FLAG_RESTRICT_OPERS,__module_info.module_name);
	add_bot_cmd_list(ms_bot, ms_commands);
	return 1;
};


EventFnList __module_events[] = {
	{EVENT_ONLINE, Online}
	,
	{NULL, NULL}
};


int __ModInit(int modnum, int apiver)
{
	strlcpy(s_MoraleServ, "MoraleServ", MAXNICK);
	return 1;
}


void __ModFini()
{
};


/* Routine for HAIL */
static int ms_hail(User * u, char **av, int ac)
{
	char *cmd;
	char *m;

	SET_SEGV_LOCATION();
	if (ac < 4) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s HAIL <WHO TO HAIL> <NICK TO SEND HAIL TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	m = av[3];
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ, 
		"Your \"HAIL\" song greeting has been sent to %s!", m);
	chanalert(s_MoraleServ,
		  "%s Wanted %s to be hailed by sending the song to %s",
		  u->nick, cmd, m);
	prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(m, s_MoraleServ,
		"*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",
		cmd, cmd, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a HAIL to the %s song to %s",
	     u->nick, cmd, m);
	return 1;

}


/* Routine for LAPDANCE */
static int ms_lapdance(User * u, char **av, int ac)
{
	char *cmd;
	
	SET_SEGV_LOCATION();
	if (ac < 3) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s LAPDANCE <NICK>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(cmd, s_MoraleServ,
		"*%s Seductively walks up to %s and gives %s a sly look*",
		s_MoraleServ, cmd, cmd);
	prefmsg(cmd, s_MoraleServ,
		"*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",
		s_MoraleServ, cmd, cmd);
	prefmsg(cmd, s_MoraleServ,
		"*I Think we both need a cold shower now*... *wink*",
		s_MoraleServ, cmd);
	chanalert(s_MoraleServ, "%s Wanted a LAPDANCE to be preformed on %s",
		  u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s Wanted a LAPDANCE to be preformed on %s", u->nick, cmd);

	return 1;
}


/* Routine for ODE */
static int ms_ode(User * u, char **av, int ac)
{
	char *cmd;
	char *m;
	SET_SEGV_LOCATION();

	if (ac < 4) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	m = av[3];
	if (!m) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP", s_MoraleServ);
		return 1;
	}
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your ODE to %s has been sent to %s!", cmd, m);
	chanalert(s_MoraleServ, "%s Wanted an ODE to %s to be recited to %s",
		  u->nick, cmd, m);
	prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(m, s_MoraleServ, "*recites*", u->nick);
	prefmsg(m, s_MoraleServ, "How I wish to be a %s,", cmd);
	prefmsg(m, s_MoraleServ, "a %s I would like to be.", cmd);
	prefmsg(m, s_MoraleServ, "For if I was a %s,", cmd);
	prefmsg(m, s_MoraleServ, "I'd watch the network hail thee.", cmd);
	prefmsg(m, s_MoraleServ, "*bows*", u->nick);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s sent an ODE to %s to be recited to %s", u->nick, cmd, m);
	return 1;

}



/* Routine for VERSION */
static int ms_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(s_MoraleServ,
		"%s Wanted to know the current version information for %s",
		u->nick, s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ, "\2%s Version Information\2",
		s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ, "%s Version: %s - running on: %s",
		s_MoraleServ, __module_info.module_version, me.name);
	prefmsg(u->nick, s_MoraleServ,
		"%s Author: ^Enigma^ <enigma@neostats.net>", s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ,
		"Neostats Statistical Software: http://www.neostats.net");
	return 1;
}

/* Routine for ABOUT */
static int ms_about(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	privmsg_list(u->nick, s_MoraleServ, ms_help_about);
	return 1;
}


/* Routine for POEM */
static int ms_poem(User * u, char **av, int ac)
{
	char *cmd;
	char *m;
	SET_SEGV_LOCATION();
	if (ac < 4) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	m = av[3];
	if (!m) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP", s_MoraleServ);
		return 1;
	}
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your POEM about %s has been sent to %s!", cmd, m);
	chanalert(s_MoraleServ,
		  "%s Wanted a POEM about %s to be recited to %s", u->nick,
		  cmd, m);
	prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(m, s_MoraleServ, "*recites*");
	prefmsg(m, s_MoraleServ, "I wish I was a %s,", cmd);
	prefmsg(m, s_MoraleServ, "A %s is never glum,", cmd);
	prefmsg(m, s_MoraleServ, "Coz how can you be grumpy,");
	prefmsg(m, s_MoraleServ, "When the sun shines out your bum.");
	prefmsg(m, s_MoraleServ, "*bows*");
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a POEM about %s to %s", u->nick,
	     cmd, m);

	return 1;
}


/* Routine for REDNECK */
static int ms_redneck(User * u, char **av, int ac)
{
	char *cmd;
	SET_SEGV_LOCATION();
	if (ac < 3) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s REDNECK <NICK>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your redneck message has been sent to %s!", cmd);
	chanalert(s_MoraleServ,
		  "%s Wanted a REDNECK \"dubbing\" to be preformed on %s",
		  u->nick, cmd);
	prefmsg(cmd, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(cmd, s_MoraleServ, "*recites*", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of weird contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "*bows*", u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a REDNECK \"dubbing\" to %s",
	     u->nick, cmd);

	return 1;
}



static int ms_cheerup(User * u, char **av, int ac)
{
	char *cmd;
	SET_SEGV_LOCATION();
	if (ac < 3) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s CHEERUP <NICK>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(cmd, s_MoraleServ, "Cheer up %s .....", cmd);
	prefmsg(cmd, s_MoraleServ,
		"All of us on the network love you! 3--<--<--<{4@",
		u->nick);
	chanalert(s_MoraleServ, "%s Wanted %s to CHEERUP", u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s Wanted %s to CHEERUP", u->nick, cmd);
	return 1;

}


/* Routine for BEHAPPY */
static int ms_behappy(User * u, char **av, int ac)
{
	char *cmd;
	SET_SEGV_LOCATION();
	if (ac < 3) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s BEHAPPY <NICK>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(cmd, s_MoraleServ, "%s thinks that your a little sad.....",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "*starts singing*", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Here's a little song I wrote, You might want to sing it note for note",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, " ", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"In every life we have some trouble, But when you worry you make it Double",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry - Be Happy now",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, " ", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Ain't got no place to lay your head, Somebody came and took your bed",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, " ", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"The landlord say your rent is late, He may have to litigate",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry - Be Happy, Look at Me - I'm Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Here I give you my phone number, When you worry call me, I make you happy",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, " ", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Ain't got not cash, ain't got no style, Ain't got no gal to make you smile",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy", u->nick);
	prefmsg(cmd, s_MoraleServ, " ", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"'Cause when you worry your face will frown, and that will bring everybody down",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Be Happy - Put a smile on your face, Don't bring everybody down",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry, it will soon pass, whatever it is", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Don't Worry - Be Happy, I'm not worried, I'm happy . . . .",
		u->nick);

	chanalert(s_MoraleServ, "%s Wanted %s to BEHAPPY", u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s Wanted %s to BEHAPPY", u->nick, cmd);
	return 1;

}


/* Routine for WONDERFUL */
static int ms_wonderful(User * u, char **av, int ac)
{
	char *cmd;
	SET_SEGV_LOCATION();
	if (ac < 3) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s WONDERFUL <NICK>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For additional help: /msg %s HELP",
			s_MoraleServ);
		return 1;
	}
	cmd = av[2];
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surely we have better things to do with our time than make a service message itself?");
		chanalert(s_MoraleServ,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(cmd, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(cmd, s_MoraleServ, "*starts singing*", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"So excuse me forgetting but these things I do", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"You see I've forgotten if they're green or they're blue",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Anyway the thing is what I really mean", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"Yours are the sweetest eyes I've ever seen", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"And you can tell everybody this is your song", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"It may be quite simple but now that it's done", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"I hope you don't mind, I hope you don't mind that I put down in words",
		u->nick);
	prefmsg(cmd, s_MoraleServ,
		"How wonderful life is while %s is in the world", cmd);

	chanalert(s_MoraleServ, "%s Wanted to express how WONDERFUL %s is",
		  u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s Wanted to express how WONDERFUL %s is", u->nick, cmd);
	return 1;
}
