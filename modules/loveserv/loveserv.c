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

/** Bot command function prototypes */
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

/** Bot pointer */
static Bot *ls_bot;

/** Copyright info */
const char *ls_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
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

/** Bot comand table */
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

/** BotInfo */
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

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit (Module *mod_ptr)
{
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	ls_bot = init_bot(&ls_botinfo);
	if (!ls_bot) {
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
}

static int ls_rose(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source,  
		"Rose has been sent to %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s has sent you this beautiful rose! 3--<--<--<{4@",
		cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_kiss(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source,  
		"You have virtually kissed %s", target->name);
	irc_prefmsg(ls_bot, target,  "%s has virtually kissed you!", cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_tonsil(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source, 
		"You have virtually tonsil kissed %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",
		cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_hug(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source,  
		"You have hugged %s", target->name);
	irc_prefmsg(ls_bot, target,  "%s has sent you a *BIG WARM HUG*!", cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_admirer(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source,  
		"Secret admirer sent to %s", target->name);
	irc_prefmsg(ls_bot, target,  "You have a secret admirer! ;)");
	return NS_SUCCESS;
}

static int ls_chocolate(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source, 
		"Cholocates sent to %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s would like you to have this YUMMY box of chocolates!",
		cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_candy(CmdParams* cmdparams)
{
	Client *target;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	irc_prefmsg(ls_bot, cmdparams->source, 
		"Candy sent to %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s would like you to have this big YUMMY bag of heart shaped candies!",
		cmdparams->source->name);
	return NS_SUCCESS;
}

static int ls_lovenote(CmdParams* cmdparams)
{
	Client *target;
	char *message;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source,  
		"Love note sent to %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s has sent you a love note which reads: \2%s\2", 
		cmdparams->source->name, message);
	ns_free(message);
	return NS_SUCCESS;
}

static int ls_apology(CmdParams* cmdparams)
{
	Client *target;
	char *message;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source,  
		"Apology sent to %s", target->name);
	irc_prefmsg(ls_bot, target, 
		"%s is sorry, and would like to apologise for \2%s\2",
		cmdparams->source->name, message);
	ns_free(message);
	return NS_SUCCESS;
}

static int ls_thankyou(CmdParams* cmdparams)
{
	Client *target;
	char *message;

	SET_SEGV_LOCATION();
	target = find_valid_user(ls_bot, cmdparams->source, cmdparams->av[0]);
	if(!target) {
		return NS_FAILURE;
	}
	message = joinbuf(cmdparams->av, cmdparams->ac, 1);
	irc_prefmsg(ls_bot, cmdparams->source,  
		"Thank you sent to %s", target->name);
	irc_prefmsg(ls_bot, target,  "%s wishes to thank you for \2%s\2",
		cmdparams->source->name, message);
	ns_free(message);
	return NS_SUCCESS;
}
