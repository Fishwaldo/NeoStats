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
#include "loveserv.h"

static int ls_rose(CmdParams* cmdparams);
static int ls_kiss(CmdParams* cmdparams);
static int ls_tonsil(CmdParams* cmdparams);
static int ls_hug(CmdParams* cmdparams);
static int ls_admirer(CmdParams* cmdparams);
static int ls_chocolate(CmdParams* cmdparams);
static int ls_candy(CmdParams* cmdparams);
static int ls_lovenote(CmdParams* cmdparams);
static int ls_apology(CmdParams* cmdparams);
static int ls_thankyou(CmdParams* cmdparams);

static Bot *ls_bot;

const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

ModuleInfo module_info = {
	"LoveServ",
	"Network love service",
	ns_copyright, //Shmad <shmad@neostats.net>"
	ls_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static BotInfo ls_botinfo = 
{
	"LoveServ", 
	"LoveServ", 
	"LS", 
	"", 
	"Network love service",
};

static bot_cmd ls_commands[]=
{
	{"ROSE",		ls_rose,		1, 	0,	ls_help_rose,		ls_help_rose_oneline },
	{"KISS",		ls_kiss,		1, 	0,	ls_help_kiss,		ls_help_kiss_oneline },
	{"TONSIL",		ls_tonsil,		1, 	0,	ls_help_tonsil,		ls_help_tonsil_oneline },
	{"HUG",			ls_hug,			1, 	0,	ls_help_hug,		ls_help_hug_oneline },
	{"ADMIRER",		ls_admirer,		1, 	0,	ls_help_admirer,	ls_help_admirer_oneline },
	{"CHOCOLATE",	ls_chocolate,	1, 	0,	ls_help_chocolate,	ls_help_chocolate_oneline },
	{"CANDY",		ls_candy,		1, 	0,	ls_help_candy,		ls_help_candy_oneline },
	{"LOVENOTE",	ls_lovenote,	2, 	0,	ls_help_lovenote,	ls_help_lovenote_oneline },
	{"APOLOGY",		ls_apology,		2, 	0,	ls_help_apology,	ls_help_apology_oneline },
	{"THANKYOU",	ls_thankyou,	2, 	0,	ls_help_thankyou,	ls_help_thankyou_oneline },
	{NULL,			NULL,			0, 	0,	NULL, 				NULL}
};

const char *ns_help_set_nick[] = {
	"\2NICK <newnick>\2 Change bot nickname",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_altnick[] = {
	"\2ALTNICK <newnick>\2 Change bot alternate nickname",
	NULL
};

const char *ns_help_set_user[] = {
	"\2USER <username>\2 Change bot username",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_host[] = {
	"\2HOST <host>\2 Change bot host",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_realname[] = {
	"\2REALNAME <realname>\2 Change bot realname",
	"(requires restart to take effect).",
	NULL
};

static bot_setting ls_settings[]=
{
	{"NICK",	&ls_botinfo.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick, NULL, (void*)"LoveServ" },
	{"ALTNICK",	&ls_botinfo.altnick,SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "AltNick",	NULL,	ns_help_set_altnick, NULL, (void*)"LoveServ" },
	{"USER",	&ls_botinfo.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user, NULL, (void*)"LS" },
	{"HOST",	&ls_botinfo.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host, NULL, (void*)"" },
	{"REALNAME",&ls_botinfo.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname, NULL, (void*)"Network love service" },
};

static int ls_event_online(CmdParams* cmdparams)
{
	ls_bot = init_bot(&ls_botinfo, me.servicesumode, 
		BOT_FLAG_DEAF, ls_commands, ls_settings);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ls_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	ModuleConfig(ls_settings);
	return 1;
}

void ModFini()
{
};

static int ls_rose(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"Rose has been sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_kiss(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"You have virtually kissed %s", target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s has virtually kissed you!", cmdparams->source.user->nick);
	return 1;
}

static int ls_tonsil(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick,
		"You have virtually tonsil kissed %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_hug(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"You have hugged %s", target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s has sent you a *BIG WARM HUG*!", cmdparams->source.user->nick);
	return 1;
}

static int ls_admirer(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"Secret admirer sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick, "You have a secret admirer! ;)");
	return 1;
}

static int ls_chocolate(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick,
		"Cholocates sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s would like you to have this YUMMY box of chocolates!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_candy(CmdParams* cmdparams)
{
	char *target_nick;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ls_bot->nick,
		"Candy sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s would like you to have this big YUMMY bag of heart shaped candies!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_lovenote(CmdParams* cmdparams)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"Love note sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s has sent you a love note which reads: \2%s\2", 
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}

static int ls_apology(CmdParams* cmdparams)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"Apology sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick,
		"%s is sorry, and would like to apologise for \2%s\2",
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}

static int ls_thankyou(CmdParams* cmdparams)
{
	char *target_nick;
	char *message;

	SET_SEGV_LOCATION();
	target_nick = cmdparams->av[0];
	if(!is_target_valid(ls_bot->nick, cmdparams->source.user, target_nick)) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	prefmsg(cmdparams->source.user->nick, ls_bot->nick, 
		"Thank you sent to %s", target_nick);
	prefmsg(target_nick, ls_bot->nick, "%s wishes to thank you for \2%s\2",
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}
