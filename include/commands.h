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

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

int add_bot_cmd_list( Bot *bot_ptr, bot_cmd *bot_cmd_list );
int del_bot_cmd_list( Bot *bot_ptr, bot_cmd *bot_cmd_list );
int del_all_bot_cmds( Bot* bot_ptr );
int add_bot_cmd( hash_t *cmd_hash, bot_cmd *cmd_ptr );
bot_cmd *find_bot_cmd( Bot *bot_ptr, char *cmd);
int del_bot_cmd( hash_t *cmd_hash, bot_cmd *cmd_ptr );
int run_bot_cmd( CmdParams * cmdparams, int ischancmd );
int getuserlevel( CmdParams *cmdparams );
void msg_permission_denied( CmdParams *cmdparams, char *subcommand );
void msg_error_need_more_params( CmdParams *cmdparams );
void msg_error_param_out_of_range( CmdParams *cmdparams );
void msg_syntax_error( CmdParams *cmdparams );
void msg_unknown_command( CmdParams *cmdparams );
void msg_only_opers( CmdParams *cmdparams );

#endif /* _COMMANDS_H_ */
