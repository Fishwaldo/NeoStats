/* NeoStats - IRC Statistical Services 
** Copyright (c) 2006 Mark Hetherington
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
#include "main.h"

char testchannel[MAXCHANLEN];
const char ns_help_dummy_oneline[] ="dummy";
const char *ns_help_dummy[] = {
	"dummy",
	NULL
};

static int ns_cmd_ctcptest( CmdParams *cmdparams );
static int ns_cmd_chantest( CmdParams *cmdparams );

/** Bot pointer */
static Bot *ns_testbot;

/** Bot setting table */
bot_setting ns_settings[] =
{
	{"TESTCHANNEL",	testchannel,	SET_TYPE_CHANNEL,	0, MAXCHANLEN, 	NS_ULEVEL_ADMIN, NULL,	ns_help_dummy, NULL,( void* )"#test" },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/** Bot comand table */
bot_cmd ns_commands[]=
{
	{"CTCPTEST",ns_cmd_ctcptest,0,	NS_ULEVEL_LOCOPER,	ns_help_dummy,	ns_help_dummy_oneline },
	{"CHANTEST",ns_cmd_chantest,0,	NS_ULEVEL_LOCOPER,	ns_help_dummy,	ns_help_dummy_oneline },
	{NULL,		NULL,			0, 	0,					NULL, 			NULL}
};

/** BotInfo */
static BotInfo ns_testbotinfo = 
{
	"nstestbot", 
	"nstestbot", 
	"ns", 
	BOT_COMMON_HOST, 
	"nstest", 	
	0, 
	NULL, 
	NULL,
};

static int ns_cmd_ctcptest( CmdParams *cmdparams )
{
	irc_chanalert( ns_bot, "Testing CTCP VERSION request" );
	irc_ctcp_version_req( ns_bot, cmdparams->source );
	irc_chanalert( ns_bot, "Testing CTCP PING request" );
	irc_ctcp_ping_req( ns_bot, cmdparams->source );
	irc_chanalert( ns_bot, "Testing CTCP FINGER request" );
	irc_ctcp_finger_req( ns_bot, cmdparams->source );
	irc_chanalert( ns_bot, "Testing CTCP ACTION request" );
	irc_ctcp_action_req( ns_bot, cmdparams->source, "test action" );
	irc_chanalert( ns_bot, "Testing CTCP TIME request" );
	irc_ctcp_time_req( ns_bot, cmdparams->source );
	return NS_SUCCESS;
}

static int ns_cmd_chantest( CmdParams *cmdparams )
{
	irc_chanalert( ns_bot, "Testing AddBot" );
	ns_testbot = AddBot( &ns_testbotinfo );
	if( !ns_testbot ) 
	{
		return NS_FAILURE;
	}
	irc_chanalert( ns_bot, "Testing join" );
	irc_join( ns_testbot, testchannel, NULL );
	irc_chanalert( ns_bot, "Testing invite" );
	irc_invite( ns_testbot, cmdparams->source, testchannel );
	irc_chanalert( ns_bot, "Testing topic" );
	irc_topic( ns_testbot, FindChannel( testchannel ), "Test topic" );
	irc_chanalert( ns_bot, "Testing part" );
	irc_part( ns_testbot, testchannel, "quit" );
	irc_chanalert( ns_bot, "Testing quit" );
	irc_quit( ns_testbot, "quit" );
	return NS_SUCCESS;
}
