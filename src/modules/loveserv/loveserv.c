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

static Bot *ls_bot;
static BotInfo ls_botinfo = 
{
	"", 
	"", 
	"", 
	"", 
	"", 	
};
static Module* ls_module;

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

ModuleInfo module_info = {
	"LoveServ",
	"Network love service",
	NULL,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
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
	{"NICK",	&ls_botinfo.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",	&ls_botinfo.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",	&ls_botinfo.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",&ls_botinfo.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
};

static int Online(char **av, int ac)
{
	ls_bot = init_bot(ls_module, &ls_botinfo, services_bot_modes, BOT_FLAG_DEAF, ls_commands, ls_settings);
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
		strlcpy(ls_botinfo.nick ,"LoveServ" ,MAXNICK);
	}
	else {
		strlcpy(ls_botinfo.nick , temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(ls_botinfo.user, "SS", MAXUSER);
	}
	else {
		strlcpy(ls_botinfo.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(ls_botinfo.host, me.name, MAXHOST);
	}
	else {
		strlcpy(ls_botinfo.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(ls_botinfo.realname, "Network Love Service", MAXREALNAME);
	}
	else {
		strlcpy(ls_botinfo.realname, temp, MAXREALNAME);
		free(temp);
	}
	return 1;
}

void ModFini()
{
};

static int ls_rose(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick, "Your rose has been sent to %s!",
		target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		u->nick);
	return 1;
}

static int ls_kiss(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick, "You have virtually kissed %s!", target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s has virtually kissed you!", u->nick);
	return 1;
}

static int ls_tonsil(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick,
		"You have virtually tonsilly kissed %s!", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		u->nick);
	return 1;
}

static int ls_hug(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick, "%s has received your hug! :)", target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s has sent you a *BIG WARM HUG*!", u->nick);
	return 1;
}

static int ls_admirer(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick, "Secret admirer sent to %s :)", target_nick);
	prefmsg(target_nick, ls_bot->nick, "You have a secret admirer! ;)");
	return 1;
}

static int ls_choco(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick,
		"A box of cholocates has been sent to %s :)", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s would like you to have this YUMMY box of chocolates!",
		u->nick);
	return 1;
}

static int ls_candy(User * u, char **av, int ac)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = av[2];
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	prefmsg(u->nick, ls_bot->nick,
		"A bag of yummy heart shaped candies has been sent to %s :)", target_nick);
	prefmsg(target_nick, ls_bot->nick,
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
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, ls_bot->nick, 
		"Your lovenote to %s has been sent! :)", target_nick);
	prefmsg(target_nick, ls_bot->nick,
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
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, ls_bot->nick, "Your apology has been sent to %s",
		target_nick);
	prefmsg(target_nick, ls_bot->nick,
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
	if(!is_target_valid(ls_bot->nick, u, target_nick)) {
		return 0;
	}
	message = joinbuf(av, ac, 3);
	prefmsg(u->nick, ls_bot->nick, "Your Thank You has been sent to %s",
		target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s wishes to thank you for \2%s\2",
		u->nick, message);
	free(message);
	return 1;
}

static int ls_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, ls_bot->nick, "\2%s Version Information\2", ls_bot->nick);
	prefmsg(u->nick, ls_bot->nick, "%s Version: %s Compiled %s at %s", ls_bot->nick, 
		module_info.version, module_info.build_date, module_info.build_time);
	prefmsg(u->nick, ls_bot->nick, "%s Author: Shmad <shmad@neostats.net>", ls_bot->nick);
	prefmsg(u->nick, ls_bot->nick, "http://www.neostats.net");
	return 1;
}

static int ls_about(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	privmsg_list(u->nick, ls_bot->nick, ls_help_about);
	return 1;
}
