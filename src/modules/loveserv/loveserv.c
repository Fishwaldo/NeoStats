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

const char *ls_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

ModuleInfo module_info = {
	"LoveServ",
	"Network love service",
	ls_copyright,
	ls_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
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

static BotInfo ls_botinfo = 
{
	"LoveServ", 
	"LoveServ1", 
	"LS", 
	BOT_COMMON_HOST, 
	"Network love service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ls_commands, 
	NULL,
};

static int ls_event_online(CmdParams* cmdparams)
{
	ls_bot = init_bot(&ls_botinfo);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ls_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	return 1;
}

void ModFini()
{
};

static int ls_rose(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"Rose has been sent to %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_kiss(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"You have virtually kissed %s", target);
	irc_prefmsg(ls_bot, target,  "%s has virtually kissed you!", cmdparams->source.user->nick);
	return 1;
}

static int ls_tonsil(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user, 
		"You have virtually tonsil kissed %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_hug(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"You have hugged %s", target);
	irc_prefmsg(ls_bot, target,  "%s has sent you a *BIG WARM HUG*!", cmdparams->source.user->nick);
	return 1;
}

static int ls_admirer(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"Secret admirer sent to %s", target);
	irc_prefmsg(ls_bot, target,  "You have a secret admirer! ;)");
	return 1;
}

static int ls_chocolate(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user, 
		"Cholocates sent to %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s would like you to have this YUMMY box of chocolates!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_candy(CmdParams* cmdparams)
{
	User *target;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	irc_prefmsg(ls_bot, cmdparams->source.user, 
		"Candy sent to %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s would like you to have this big YUMMY bag of heart shaped candies!",
		cmdparams->source.user->nick);
	return 1;
}

static int ls_lovenote(CmdParams* cmdparams)
{
	User *target;
	char *message;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"Love note sent to %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s has sent you a love note which reads: \2%s\2", 
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}

static int ls_apology(CmdParams* cmdparams)
{
	User *target;
	char *message;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"Apology sent to %s", target);
	irc_prefmsg(ls_bot, target, 
		"%s is sorry, and would like to apologise for \2%s\2",
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}

static int ls_thankyou(CmdParams* cmdparams)
{
	User *target;
	char *message;

	SET_SEGV_LOCATION();
	target = findvaliduser(ls_bot, cmdparams->source.user, cmdparams->av[0]);
	if(!target) {
		return 0;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source.user,  
		"Thank you sent to %s", target);
	irc_prefmsg(ls_bot, target,  "%s wishes to thank you for \2%s\2",
		cmdparams->source.user->nick, message);
	sfree(message);
	return 1;
}
