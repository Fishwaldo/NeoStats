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
** $Id: ms.c,v 1.16 2003/06/13 14:49:32 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "log.h"
#include "ms_help.c"

const char msversion_date[] = __DATE__;
const char msversion_time[] = __TIME__;
char *s_MoraleServ;
extern const char *ms_help[];
static void ms_hail(User * u, char *cmd, char *m);
static void ms_ode(User * u, char *cmd, char *m);
static void ms_lapdance(User * u, char *cmd);
static void ms_version(User * u);
static void ms_poem(User * u, char *cmd, char *m);
static void ms_redneck(User * u, char *cmd);
static void ms_cheerup(User * u, char *cmd);
static void ms_behappy(User * u, char *cmd);
static void ms_wonderful(User * u, char *cmd);
static int new_m_version(char *origin, char **av, int ac);


Module_Info my_info[] = { {
			   "MoraleServ",
			   "A Network Morale Service",
			   "2.20"}
};


int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(351, origin,
		     "Module MoraleServ Loaded, Version: %s %s %s",
		     my_info[0].module_version, msversion_date,
		     msversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};


int __Bot_Message(char *origin, char **av, int ac)
{
	User *u;
	char *cmd;
	u = finduser(origin);

	if (!strcasecmp(av[1], "HELP")) {
		if (ac <= 2) {
			privmsg_list(u->nick, s_MoraleServ, ms_help);
			return 1;
		} else if (!strcasecmp(av[2], "HAIL")) {
			privmsg_list(u->nick, s_MoraleServ, ms_help_hail);
			return 1;
		} else if (!strcasecmp(av[2], "ODE")) {
			privmsg_list(u->nick, s_MoraleServ, ms_help_ode);
			return 1;
		} else if (!strcasecmp(av[2], "LAPDANCE")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_lapdance);
			return 1;
		} else if (!strcasecmp(av[2], "VERSION")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_version);
			return 1;
		} else if (!strcasecmp(av[2], "POEM")) {
			privmsg_list(u->nick, s_MoraleServ, ms_help_poem);
			return 1;
		} else if (!strcasecmp(av[2], "REDNECK")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_redneck);
			return 1;
		} else if (!strcasecmp(av[2], "CHEERUP")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_cheerup);
			return 1;
		} else if (!strcasecmp(av[2], "BEHAPPY")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_behappy);
			return 1;
		} else if (!strcasecmp(av[2], "WONDERFUL")) {
			privmsg_list(u->nick, s_MoraleServ,
				     ms_help_wonderful);
			return 1;
		} else
			prefmsg(u->nick, s_MoraleServ,
				"Unknown Help Topic: \2%s\2", av[2]);
	}

	if (!strcasecmp(av[1], "HAIL")) {
		if (ac < 4) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s HAIL <WHO TO HAIL> <NICK TO SEND HAIL TO>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_hail(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "ODE")) {
		if (ac < 4) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_ode(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "LAPDANCE")) {
		if (ac < 3) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s LAPDANCE <NICK>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_lapdance(u, av[2]);
	} else if (!strcasecmp(av[1], "VERSION")) {
		chanalert(s_Services,
			  "%s Wanted to know the current version information for %s",
			  u->nick, s_MoraleServ);
		ms_version(u);
	} else if (!strcasecmp(av[1], "POEM")) {
		if (ac < 4) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_poem(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "REDNECK")) {
		if (ac < 3) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s REDNECK <NICK>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_redneck(u, av[2]);
	} else if (!strcasecmp(av[1], "CHEERUP")) {
		if (ac < 3) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s CHEERUP <NICK>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_cheerup(u, av[2]);
	} else if (!strcasecmp(av[1], "BEHAPPY")) {
		if (ac < 3) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s BEHAPPY <NICK>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_behappy(u, av[2]);
	} else if (!strcasecmp(av[1], "WONDERFUL")) {
		if (ac < 3) {
			prefmsg(u->nick, s_MoraleServ,
				"Syntax: /msg %s WONDERFUL <NICK>",
				s_MoraleServ);
			prefmsg(u->nick, s_MoraleServ,
				"For addtional help: /msg %s HELP",
				s_MoraleServ);
			return -1;
		}
		ms_wonderful(u, av[2]);
	} else {
		chanalert(s_Services,
			  "%s requested the unknown command of: %s",
			  u->nick, av[1]);
		prefmsg(u->nick, s_MoraleServ,
			"Unknown Command: \2%s\2, perhaps you need some HELP?",
			av[1]);
	}
	return 1;


}

int Online(char **av, int ac)
{
	if (init_bot
	    (s_MoraleServ, "MS", me.name, "A Network Morale Service",
	     "+oS", my_info[0].module_name) == -1) {
		/* Nick was in use */
		s_MoraleServ = strcat(s_MoraleServ, "_");
		init_bot(s_MoraleServ, "MS", me.name,
			 "A Network Morale Service", "+oS",
			 my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{"ONLINE", Online}
	,
	{NULL, NULL}
};



Module_Info *__module_get_info()
{
	return my_info;
};

Functions *__module_get_functions()
{
	return my_fn_list;
};

EventFnList *__module_get_events()
{
	return my_event_list;
};

void _init()
{
	s_MoraleServ = "MoraleServ";
}


void _fini()
{
};


/* Routine for HAIL */
static void ms_hail(User * u, char *cmd, char *m)
{
	strcpy(segv_location, "ms_hail");
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your \"HAIL\" song greeting has been sent to %s!", m);
	chanalert(s_Services,
		  "%s Wanted %s to be hailed by sending the song to %s",
		  u->nick, cmd, m);
	prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(m, s_MoraleServ,
		"*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",
		cmd, cmd, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a HAIL to the %s song to %s",
	     u->nick, cmd, m);

}


/* Routine for LAPDANCE */
static void ms_lapdance(User * u, char *cmd)
{
	strcpy(segv_location, "ms_lapdance");
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
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
	chanalert(s_Services, "%s Wanted a LAPDANCE to be preformed on %s",
		  u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s Wanted a LAPDANCE to be preformed on %s", u->nick, cmd);

}


/* Routine for ODE */
static void ms_ode(User * u, char *cmd, char *m)
{
	strcpy(segv_location, "ms_ode");
	if (!m) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For addtional help: /msg %s HELP", s_MoraleServ);
		return;
	}
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your ODE to %s has been sent to %s!", cmd, m);
	chanalert(s_Services, "%s Wanted an ODE to %s to be recited to %s",
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

}



/* Routine for VERSION */
static void ms_version(User * u)
{
	strcpy(segv_location, "ms_version");
	prefmsg(u->nick, s_MoraleServ, "\2%s Version Information\2",
		s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ, "%s Version: %s - running on: %s",
		s_MoraleServ, my_info[0].module_version, me.name);
	prefmsg(u->nick, s_MoraleServ,
		"%s Author: ^Enigma^ <enigma@neostats.net>", s_MoraleServ);
	prefmsg(u->nick, s_MoraleServ,
		"Neostats Satistical Software: http://www.neostats.net");
}


/* Routine for POEM */
static void ms_poem(User * u, char *cmd, char *m)
{
	strcpy(segv_location, "ms_poem");
	if (!m) {
		prefmsg(u->nick, s_MoraleServ,
			"Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>",
			s_MoraleServ);
		prefmsg(u->nick, s_MoraleServ,
			"For addtional help: /msg %s HELP", s_MoraleServ);
		return;
	}
	if (!strcasecmp(m, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(m)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your POEM about %s has been sent to %s!", cmd, m);
	chanalert(s_Services,
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

}


/* Routine for REDNECK */
static void ms_redneck(User * u, char *cmd)
{
	strcpy(segv_location, "ms_redneck");
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(u->nick, s_MoraleServ,
		"Your redneck message has been sent to %s!", cmd);
	chanalert(s_Services,
		  "%s Wanted a REDNECK \"dubbing\" to be preformed on %s",
		  u->nick, cmd);
	prefmsg(cmd, s_MoraleServ, "Courtesy of your friend %s:", u->nick);
	prefmsg(cmd, s_MoraleServ, "*recites*", u->nick);
	prefmsg(cmd, s_MoraleServ,
		"I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of wierd contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.",
		u->nick);
	prefmsg(cmd, s_MoraleServ, "*bows*", u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a REDNECK \"dubbing\" to %s",
	     u->nick, cmd);

}



static void ms_cheerup(User * u, char *cmd)
{
	strcpy(segv_location, "ms_cheerup");
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
	}
	/* The user has passed the minimum requirements for input */

	prefmsg(cmd, s_MoraleServ, "Cheer up %s .....", cmd);
	prefmsg(cmd, s_MoraleServ,
		"All of us on the network love you! 3--<--<--<{4@",
		u->nick);
	chanalert(s_Services, "%s Wanted %s to CHEERUP", u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s Wanted %s to CHEERUP", u->nick, cmd);

}


/* Routine for BEHAPPY */
static void ms_behappy(User * u, char *cmd)
{
	strcpy(segv_location, "ms_behappy");
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
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

	chanalert(s_Services, "%s Wanted %s to BEHAPPY", u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD, "%s Wanted %s to BEHAPPY", u->nick, cmd);

}


/* Routine for WONDERFUL */
static void ms_wonderful(User * u, char *cmd)
{
	strcpy(segv_location, "ms_wonderful");
	if (!strcasecmp(cmd, s_MoraleServ)) {
		prefmsg(u->nick, s_MoraleServ,
			"Surley we have better things to do with our time than make a service message itself?");
		chanalert(s_Services,
			  "Prevented %s from making %s message %s",
			  u->nick, s_MoraleServ, s_MoraleServ);
		return;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_MoraleServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return;
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

	chanalert(s_Services, "%s Wanted to express how WONDERFUL %s is",
		  u->nick, cmd);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s Wanted to express how WONDERFUL %s is", u->nick, cmd);

}
