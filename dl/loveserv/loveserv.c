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
#include "ls_help.c"
#include "log.h"

char s_LoveServ[MAXNICK];

ModUser *ls_bot;

static int ls_rose(User * u, char **av, int ac);
static int ls_kiss(User * u, char **av, int ac);
static int ls_tonsil(User * u, char **av, int ac);
static int ls_hug(User * u, char **av, int ac);
static int ls_admirer(User * u, char **av, int ac);
static int ls_choco(User * u, char **av, int ac);
static int ls_candy(User * u, char **av, int ac);
static int ls_lovenote(User * u, char **av, int ac);
static int ls_apology(User * u, char **av, int ac);
static int ls_thankyou(User * u, char **av, int ac);
static int ls_version(User * u, char **av, int ac);
static int ls_about(User * u, char **av, int ac);

static int new_m_version(char *origin, char **av, int ac);

ModuleInfo __module_info = {
   "LoveServ",
   "A Network Love Service",
   "1.9",
	__DATE__,
	__TIME__
};

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(RPL_VERSION, origin,
		     "Module LoveServ Loaded, Version: %s %s %s",
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

static bot_cmd ls_commands[]=
{
	{"ABOUT",		ls_about,		0, 	0,	ls_help_about,		1,	ls_help_about_oneline },
	{"ROSE",		ls_rose,		1, 	0,	ls_help_rose,		1,	ls_help_rose_oneline },
	{"KISS",		ls_kiss,		1, 	0,	ls_help_kiss,		1,	ls_help_kiss_oneline },
	{"TONSIL",		ls_tonsil,		1, 	0,	ls_help_tonsil,		1,	ls_help_tonsil_oneline },
	{"HUG",			ls_hug,			1, 	0,	ls_help_hug,		1,	ls_help_hug_oneline },
	{"ADMIRER",		ls_admirer,		1, 	0,	ls_help_admirer,	1,	ls_help_admirer_oneline },
	{"CHOCOLATE",	ls_choco,		1, 	0,	ls_help_chocolate,	1,	ls_help_chocolate_oneline },
	{"CANDY",		ls_candy,		1, 	0,	ls_help_candy,		1,	ls_help_candy_oneline },
	{"LOVENOTE",	ls_lovenote,	2, 	0,	ls_help_lovenote,	1,	ls_help_lovenote_oneline },
	{"APOLOGY",		ls_apology,		2, 	0,	ls_help_apology,	1,	ls_help_apology_oneline },
	{"THANKYOU",	ls_thankyou,	2, 	0,	ls_help_thankyou,	1,	ls_help_thankyou_oneline },
	{"VERSION",		ls_version,		0, 	0,	ls_help_version,	1,	ls_help_version_oneline },
	{NULL,			NULL,			0, 	0,	NULL, 				0,	NULL}
};

int Online(char **av, int ac)
{
	ls_bot = init_mod_bot(s_LoveServ, "love", me.name, "Network Love Service",
		services_bot_modes, 0, ls_commands, __module_info.module_name);
	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
	strlcpy (s_LoveServ, "LoveServ", MAXNICK);
	return 1;
}

void __ModFini()
{
};

static int ls_rose(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "Your rose has been sent to %s!",
		cmd);
	prefmsg(cmd, s_LoveServ,
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a ROSE to %s", u->nick, cmd);
	return 1;
}


static int ls_kiss(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "You have virtually kissed %s!", cmd);
	prefmsg(cmd, s_LoveServ, "%s has virtually kissed you!", u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a KISS to %s", u->nick, cmd);
	return 1;
}


static int ls_tonsil(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ,
		"You have virtually tonsilly kissed %s!", cmd);
	prefmsg(cmd, s_LoveServ,
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a TONSIL KISS to %s", u->nick,
	     cmd);
	return 1;
}


static int ls_hug(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "%s has received your hug! :)", cmd);
	prefmsg(cmd, s_LoveServ, "%s has sent you a *BIG WARM HUG*!",
		u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a HUG to %s", u->nick, cmd);
	return 1;
}


static int ls_admirer(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "Anonymous admire sent to %s :)",
		cmd);
	prefmsg(cmd, s_LoveServ, "You have a secret admirer! ;)");
	nlog(LOG_NORMAL, LOG_MOD, "%s sent an ADMIRER to %s", u->nick, cmd);
	return 1;
}


static int ls_choco(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ,
		"A box of cholocates has been sent to %s :)", cmd);
	prefmsg(cmd, s_LoveServ,
		"%s would like you to have this YUMMY box of chocolates!",
		u->nick);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a Box of Chocolates to %s",
	     u->nick, cmd);
	return 1;
}


static int ls_candy(User * u, char **av, int ac)
{
	char *cmd;

	SET_SEGV_LOCATION();
	cmd = av[2];
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		return 1;
	}

	prefmsg(u->nick, s_LoveServ,
		"A bag of yummy heart shaped candies has been sent to %s :)",
		cmd);
	prefmsg(cmd, s_LoveServ,
		"%s would like you to have this big YUMMY bag of heart shaped candies!",
		u->nick);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s sent a BAG OF HEART SHAPED CANDIES to %s", u->nick, cmd);
	return 1;
}


static int ls_lovenote(User * u, char **av, int ac)
{
	char *cmd;
	char *m;

	SET_SEGV_LOCATION();
	m = av[2];
	cmd = joinbuf(av, ac, 3);
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		free(cmd);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		free(cmd);
		return 1;
	}

	prefmsg(u->nick, s_LoveServ,
		"Your lovenote to %s has been sent! :)", cmd);
	prefmsg(cmd, s_LoveServ,
		"%s has sent you a LoveNote which reads: \2%s\2", u->nick,
		m);
	nlog(LOG_NORMAL, LOG_MOD,
	     "%s sent a LOVE NOTE to %s which reads %s", u->nick, cmd, m);
	free(cmd);
	return 1;
}


static int ls_apology(User * u, char **av, int ac)
{
	char *cmd;
	char *m;

	SET_SEGV_LOCATION();
	m = av[2];
	cmd = joinbuf(av, ac, 3);
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		free(cmd);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		free(cmd);
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "Your apology has been sent to %s",
		cmd);
	prefmsg(cmd, s_LoveServ,
		"%s is sorry, and would like to apologise for \2%s\2",
		u->nick, m);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent an APOLOGY to %s for %s",
	     u->nick, cmd, m);
	free(cmd);
	return 1;
}


static int ls_thankyou(User * u, char **av, int ac)
{
	char *cmd;
	char *m;

	SET_SEGV_LOCATION();
	m = av[2];
	cmd = joinbuf(av, ac, 3);
	if (!strcasecmp(cmd, s_LoveServ)) {
		prefmsg(u->nick, s_LoveServ,
			"Surely we have better things to do with our time than make a service message itself?");
		free(cmd);
		return 1;
	}
	if (!finduser(cmd)) {
		prefmsg(u->nick, s_LoveServ,
			"That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
		free(cmd);
		return 1;
	}

	prefmsg(u->nick, s_LoveServ, "Your Thank You has been sent to %s",
		cmd);
	prefmsg(cmd, s_LoveServ, "%s wishes to thank you for \2%s\2",
		u->nick, m);
	nlog(LOG_NORMAL, LOG_MOD, "%s sent a THANKYOU to %s for %s",
	     u->nick, cmd, m);
	free(cmd);
	return 1;
}


static int ls_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, s_LoveServ, "\2%s Version Information\2", s_LoveServ);
	prefmsg(u->nick, s_LoveServ, "%s Version: %s Compiled %s at %s", s_LoveServ, 
		__module_info.module_version, __module_info.module_build_date, __module_info.module_build_time);
	prefmsg(u->nick, s_LoveServ, "%s Author: Shmad <shmad@neostats.net>", s_LoveServ);
	prefmsg(u->nick, s_LoveServ, "http://www.neostats.net");
	return 1;

}

static int ls_about(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	privmsg_list(u->nick, s_LoveServ, ls_help_about);
	return 1;
}
