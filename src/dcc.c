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
#include "modules.h"

typedef int (*dcc_cmd_handler) (CmdParams* cmdparams);

typedef struct dcc_cmd {
	const char* cmd;
	dcc_cmd_handler req_handler;
} dcc_cmd;

static int dcc_req_send (CmdParams* cmdparams);
static int dcc_req_chat (CmdParams* cmdparams);

static dcc_cmd dcc_cmds[]= {
	{"SEND", dcc_req_send},
	{"CHAT", dcc_req_chat},
	{NULL},
};

int dcc_req (CmdParams* cmdparams)
{
	dcc_cmd* cmd;
	int len;
    
	cmd = dcc_cmds;
	while (cmd->cmd) {	
		len = strlen (cmd->cmd);
		if ( ircstrncasecmp (cmd->cmd, cmdparams->param, len ) == 0)
		{
			cmdparams->param += (len + 1);		
			if (cmd->req_handler) {
				cmd->req_handler (cmdparams);
			}
			return NS_SUCCESS;
		}
		cmd++;
	}
	return NS_SUCCESS;
}

static int dcc_req_send (CmdParams* cmdparams)
{
	dlog (DEBUG5, "DCC SEND request from %s to %s", cmdparams->source->name, cmdparams->bot->name);
	SendModuleEvent (EVENT_DCCSEND, cmdparams, cmdparams->bot->moduleptr);
	return NS_SUCCESS;
}

static int dcc_req_chat (CmdParams* cmdparams)
{
	dlog (DEBUG5, "DCC CHAT request from %s to %s", cmdparams->source->name, cmdparams->bot->name);
	dlog (DEBUG5, "DCC CHAT requests currently not supported");
	return NS_SUCCESS;
}
