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
#include "loveserv.h"

static char s_LoveServ[MAXNICK];
static ModUser *ls_bot;

struct ls_cfg { 
	char user[MAXUSER];
	char host[MAXHOST];
	char rname[MAXREALNAME];
} ls_cfg;

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

ModuleInfo __module_info = {
   "LoveServ",
   "A Network Love Service",
   "1.9",
	__DATE__,
	__TIME__
};

static bot_cmd ls_commands[]=
{
	{"ABOUT",		ls_about,		0, 	0,	ls_help_about,		ls_help_about_oneline },
	{"ROSE",		ls_rose,		1, 	0,	ls_help_rose,		ls_help_rose_oneline },
	{"KISS",		ls_kiss,		1, 	0,	ls_help_kiss,		ls_help_kiss_oneline },
	{"TONSIL",		ls_tonsil,		1, 	0,	ls_help_tonsil,		ls_help_tonsil_oneline },
	{"HUG",			ls_hug,			1, 	0,	ls_help_hug,		ls_help_hug_oneline },
	{"ADMIRER",		ls_admirer,		1, 	0,	ls_help_admirer,	ls_help_admirer_oneline },
	{"CHOCOLATE",	ls_choco,		1, 	0,	ls_help_chocolate,	ls_help_chocolate_oneline },
	{"CANDY",		ls_candy,		1, 	0,	ls_help_candy,		ls_help_candy_oneline },
	{"LOVENOTE",	ls_lovenote,	2, 	0,	ls_help_lovenote,	ls_help_lovenote_oneline },
	{"APOLOGY",		ls_apology,		2, 	0,	ls_help_apology,	ls_help_apology_oneline },
	{"THANKYOU",	ls_thankyou,	2, 	0,	ls_help_thankyou,	ls_help_thankyou_oneline },
	{"VERSION",		ls_version,		0, 	0,	ls_help_version,	ls_help_version_oneline },
	{NULL,			NULL,			0, 	0,	NULL, 				NULL}
};

static bot_setting ls_settings[]=
{
	{"NICK",		&s_LoveServ,			SET_TYPE_STRING,	0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",		&ls_cfg.user,			SET_TYPE_STRING,	0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",		&ls_cfg.host,			SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",	&ls_cfg.rname,		SET_TYPE_STRING,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
};

static int Online(char **av, int ac)
{
	ls_bot = init_mod_bot(s_LoveServ, ls_cfg.user, ls_cfg.host, ls_cfg.rname, 
		services_bot_modes, BOT_FLAG_DEAF, ls_commands, ls_settings, __module_info.module_name);
	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
 	char *temp = NULL;

	if(GetConf((void *) &temp, CFGSTR, "Nick") < 0) {
		strlcpy(s_LoveServ ,"LoveServ" ,MAXNICK);
	}
	else {
		strlcpy(s_LoveServ , temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(ls_cfg.user, "SS", MAXUSER);
	}
	else {
		strlcpy(ls_cfg.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(ls_cfg.host, me.name, MAXHOST);
	}
	else {
		strlcpy(ls_cfg.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(ls_cfg.rname, "Network Love Service", MAXREALNAME);
	}
	else {
		strlcpy(ls_cfg.rname, temp, MAXREALNAME);
		free(temp);
	}
	return 1;
}

void __ModFini()
{
};

static int ls_rose(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ, "Your rose has been sent to %s!",
		target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		u->nick);
	return 1;
}

static int ls_kiss(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ, "You have virtually kissed %s!", target_nick);
	prefmsg(target_nick, s_LoveServ, "%s has virtually kissed you!", u->nick);
	return 1;
}

static int ls_tonsil(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ,
		"You have virtually tonsilly kissed %s!", target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		u->nick);
	return 1;
}

static int ls_hug(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ, "%s has received your hug! :)", target_nick);
	prefmsg(target_nick, s_LoveServ, "%s has sent you a *BIG WARM HUG*!", u->nick);
	return 1;
}

static int ls_admirer(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ, "Secret admirer sent to %s :)", target_nick);
	prefmsg(target_nick, s_LoveServ, "You have a secret admirer! ;)");
	return 1;
}

static int ls_choco(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ,
		"A box of cholocates has been sent to %s :)", target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s would like you to have this YUMMY box of chocolates!",
		u->nick);
	return 1;
}

static int ls_candy(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, s_LoveServ,
		"A bag of yummy heart shaped candies has been sent to %s :)", target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s would like you to have this big YUMMY bag of heart shaped candies!",
		u->nick);
	return 1;
}

static int ls_lovenote(User * u, char **av, int ac)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, s_LoveServ, 
		"Your lovenote to %s has been sent! :)", target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s has sent you a LoveNote which reads: \2%s\2", u->nick, message);
	free(message);
	return 1;
}

static int ls_apology(User * u, char **av, int ac)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, s_LoveServ, "Your apology has been sent to %s",
		target_nick);
	prefmsg(target_nick, s_LoveServ,
		"%s is sorry, and would like to apologise for \2%s\2",
		u->nick, message);
	free(message);
	return 1;
}

static int ls_thankyou(User * u, char **av, int ac)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(s_LoveServ, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, s_LoveServ, "Your Thank You has been sent to %s",
		target_nick);
	prefmsg(target_nick, s_LoveServ, "%s wishes to thank you for \2%s\2",
		u->nick, message);
	free(message);
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
