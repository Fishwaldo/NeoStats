/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "modules.h"
#include "ctcp.h"
#include "dcc.h"

typedef int( *ctcp_cmd_handler )( CmdParams* cmdparams );

typedef struct ctcp_cmd {
	const char* cmd;
	ctcp_cmd_handler req_handler;
	ctcp_cmd_handler rpl_handler;
} ctcp_cmd;

static int ctcp_req_version( CmdParams* cmdparams );
static int ctcp_rpl_version( CmdParams* cmdparams );
static int ctcp_req_finger( CmdParams* cmdparams );
static int ctcp_rpl_finger( CmdParams* cmdparams );
static int ctcp_req_action( CmdParams* cmdparams );
static int ctcp_req_dcc( CmdParams* cmdparams );
static int ctcp_rpl_dcc( CmdParams* cmdparams );
static int ctcp_req_time( CmdParams* cmdparams );
static int ctcp_rpl_time( CmdParams* cmdparams );
static int ctcp_req_ping( CmdParams* cmdparams );
static int ctcp_rpl_ping( CmdParams* cmdparams );

static ctcp_cmd ctcp_cmds[]= {
	{"VERSION",	ctcp_req_version,	ctcp_rpl_version },
	{"FINGER",	ctcp_req_finger,	ctcp_rpl_finger },
	{"ACTION",	ctcp_req_action,	NULL },
	{"DCC",		ctcp_req_dcc,		ctcp_rpl_dcc },
	{"TIME",	ctcp_req_time,		ctcp_rpl_time },
	{"PING",	ctcp_req_ping,		ctcp_rpl_ping },
	{NULL, NULL, NULL}
};

static void strip_ctcp_codes( char *line )
{
	char *outline = line;
	while( *line ) {
		if( (*line ) == 1 ) {
			line++;
		} else {
			*outline = *line;
			outline++;
			line++;
		}
	}
	*outline = 0;
}

int ctcp_private( CmdParams *cmdparams )
{
	ctcp_cmd* cmd;
	int len;
    
	if( cmdparams->param[0] == '\1' ) {
		strip_ctcp_codes( cmdparams->param );
		cmd = ctcp_cmds;
		while( cmd->cmd ) {	
			len = strlen( cmd->cmd );
			if( ircstrncasecmp( cmd->cmd, cmdparams->param, len  ) == 0 )
			{
				cmdparams->param +=( len + 1 );		
				if( cmd->req_handler ) {
					cmd->req_handler( cmdparams );
				}
				return NS_SUCCESS;
			}
			cmd++;
		}
	}
	return NS_SUCCESS;
}

int ctcp_notice( CmdParams *cmdparams )
{
	ctcp_cmd* cmd;
	int len;
    
	if( cmdparams->param[0] == '\1' ) {
		strip_ctcp_codes( cmdparams->param );
		cmd = ctcp_cmds;
		while( cmd->cmd ) {	
			len = strlen( cmd->cmd );
			if( ircstrncasecmp( cmd->cmd, cmdparams->param, len  ) == 0 )
			{
				cmdparams->param +=( len + 1 );		
				if( cmd->rpl_handler ) {
					cmd->rpl_handler( cmdparams );
				}
				return NS_SUCCESS;
			}
			cmd++;
		}
	}
	return NS_SUCCESS;
}

int ctcp_cprivate( CmdParams *cmdparams )
{
	dlog( DEBUG5, "Channel CTCP requests currently not supported" );
	return NS_SUCCESS;
}

int ctcp_cnotice( CmdParams *cmdparams )
{
	dlog( DEBUG5, "Channel CTCP replies currently not supported" );
	return NS_SUCCESS;
}

static int ctcp_req_version( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP VERSION request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPVERSIONREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

static int ctcp_rpl_version( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP VERSION reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPVERSIONRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

int irc_ctcp_version_req( Bot* botptr, Client* target ) 
{
	dlog( DEBUG5, "TX: CTCP VERSION request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1VERSION\1" );
	return NS_SUCCESS;
}

static int ctcp_req_finger( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP FINGER request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPFINGERREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

static int ctcp_rpl_finger( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP FINGER reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPFINGERRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

int irc_ctcp_finger_req( Bot* botptr, Client* target ) 
{
	dlog( DEBUG5, "TX: CTCP FINGER request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1FINGER\1" );
	return NS_SUCCESS;
}

static int ctcp_req_action( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP ACTION request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPACTIONREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

int irc_ctcp_action_req( Bot* botptr, Client* target, const char *action ) 
{
	dlog( DEBUG5, "TX: Sending CTCP ACTION request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1ACTION %s\1", action );
	return NS_SUCCESS;
}

int irc_ctcp_action_req_channel( Bot* botptr, Channel* channel, const char *action ) 
{
	dlog( DEBUG5, "TX: Sending CTCP ACTION request from %s to %s", botptr->name, channel->name );
	irc_chanprivmsg( botptr, channel->name, "\1ACTION %s\1", action );
	return NS_SUCCESS;
}

static int ctcp_req_dcc( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP DCC request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	dcc_req( cmdparams  );
	return NS_SUCCESS;
}

static int ctcp_rpl_dcc( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP DCC reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	dlog( DEBUG5, "CTCP DCC replies currently not supported" );
	return NS_SUCCESS;
}

static int ctcp_req_time( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP TIME request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPTIMEREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

static int ctcp_rpl_time( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP TIME reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPTIMERPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

int irc_ctcp_time_req( Bot* botptr, Client* target ) 
{
	dlog( DEBUG5, "TX: CTCP TIME request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1TIME\1" );
	return NS_SUCCESS;
}

static int ctcp_req_ping( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP PING request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPPINGREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

static int ctcp_rpl_ping( CmdParams* cmdparams )
{
	dlog( DEBUG5, "RX: CTCP PING reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPPINGRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

int irc_ctcp_ping_req( Bot* botptr, Client* target ) 
{
	dlog( DEBUG5, "TX: CTCP PING request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1PING\1" );
	return NS_SUCCESS;
}
