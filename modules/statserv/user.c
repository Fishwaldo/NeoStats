/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "tld.h"

int ss_event_mode(CmdParams *cmdparams)
{
	int add = 1;
	serverstat *ss;
	char *modes = cmdparams->param;

	SET_SEGV_LOCATION();
	ss = GetServerModValue (cmdparams->source->uplink);
	if (!ss) {
		nlog (LOG_WARNING, "Unable to find stats for %s", cmdparams->source->uplink->name);
		return NS_SUCCESS;
	}
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		case 'O':
		case 'o':
			if (add) {
				dlog(DEBUG1, "Increasing OperCount for %s", ss->name);
				AddNetworkOper ();
				AddServerOper (cmdparams->source);
			} else {
				if (is_oper(cmdparams->source)) {
					dlog(DEBUG1, "Decreasing OperCount for %s", ss->name);
					DelNetworkOper ();
					DelServerOper (cmdparams->source);
				}
			}
			break;
		default:
			break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

static void AddUser (Client * u, void *v)
{
	SET_SEGV_LOCATION();
	AddServerUser (u);
	AddNetworkUser ();
}

static void DelUser (Client * u)
{
	if (is_oper(u)) {
		dlog(DEBUG2, "Decreasing OperCount on %s due to signoff", u->uplink->name);
		DelServerOper (u);
	}
	DelServerUser (u);
	DelNetworkUser ();
	DelTLDUser (u);
}

int ss_event_globalkill(CmdParams *cmdparams)
{
	serverstat *ss;

	DelUser (cmdparams->source);
	ss = GetServerModValue (cmdparams->source->uplink);
	ss->operkills ++;
	return NS_SUCCESS;
}

int ss_event_serverkill(CmdParams *cmdparams)
{
	serverstat *ss;

	DelUser (cmdparams->source);
	ss = GetServerModValue (cmdparams->source);
	ss->serverkills ++;
	return NS_SUCCESS;
}

int ss_event_quit(CmdParams *cmdparams)
{
	DelUser (cmdparams->source);
	return NS_SUCCESS;
}

int ss_event_signon(CmdParams *cmdparams)
{
	AddUser (cmdparams->source, NULL);
	return NS_SUCCESS;
}

static int operlistaway = 0;
static char* operlistserver;

static void operlist(Client * u, void * v)
{
	Client * listu;

	listu = (Client *)v;
	if (!is_oper(u))
		return;
	if (operlistaway && u->user->is_away)
		return;
	if (!operlistserver) {
		irc_prefmsg (ss_bot, listu, "%-15s %-15s %-10d",
			u->name, u->uplink->name, UserLevel(u));
	} else {
		if (ircstrcasecmp(operlistserver, u->uplink->name))
			return;
		irc_prefmsg (ss_bot, listu, "%-15s %-15s %-10d", 
			u->name, u->uplink->name, UserLevel(u));
	}
}

int ss_cmd_operlist(CmdParams *cmdparams)
{
	char *flags = NULL;

	SET_SEGV_LOCATION();
	operlistaway = 0;
	operlistserver = NULL;
	if (cmdparams->ac == 0) {
		irc_prefmsg (ss_bot, cmdparams->source, "Online IRCops:");
		irc_prefmsg (ss_bot, cmdparams->source, "ID  %-15s %-15s %-10s", 
			"Nick", "Server", "Level");
	}
	if (cmdparams->ac != 0) {
		flags = cmdparams->av[0];
		operlistserver = cmdparams->av[1];
	}
	if (flags && !ircstrcasecmp(flags, "NOAWAY")) {
		operlistaway = 1;
		flags = NULL;
		irc_prefmsg (ss_bot, cmdparams->source, "Online IRCops (not away):");
	}
	if (!operlistaway && flags && strchr(flags, '.')) {
		operlistserver = flags;
		irc_prefmsg (ss_bot, cmdparams->source, "Online IRCops on server %s", operlistserver);
	}
	GetUserList (operlist, (void *)cmdparams->source);
	irc_prefmsg (ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static void botlist(Client * u, void * v)
{
	Client * listu;

	listu = (Client *)v;
	if is_bot(u) { 
		irc_prefmsg (ss_bot, listu, "%-15s %s", u->name, u->uplink->name);
	}
}

int ss_cmd_botlist(CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg (ss_bot, cmdparams->source, "Online bots:");
	GetUserList (botlist, (void *)cmdparams->source);
	irc_prefmsg (ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

void InitUserStats (void)
{
	GetUserList (AddUser, NULL);
}
